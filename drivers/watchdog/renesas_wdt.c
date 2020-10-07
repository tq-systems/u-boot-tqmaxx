// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <linux/io.h>
#include <sysreset.h>
#include <efi_loader.h>
#include <fdtdec.h>
#include <stdio.h>
#include <watchdog.h>
#include <console.h>
#include <linux/delay.h>
#include <linux/types.h>

#define CODE_VAL1       0x5a5a0000 /* Code value of RWTCNT */
#define CODE_VAL2_3     0xa5a5a500 /* Code value of RWTCSRA and RWTCSRB */
#define MAX_VAL         65536      /* Max count value of watchdog timer */
#define RWTCNT          0
#define RWTCSRA         4
#define RWTCSRA_WRFLG   BIT(5)
#define RWTCSRA_TME     BIT(7)
#define RWTCSRB         8

/*
 * In probe, clk_rate is checked to be not more than 16 bit x biggest
 * clock divider (12 bits). d is only a factor to fully utilize the
 * WDT counter and will not exceed its 16 bits. Thus, no overflow, we
 * stay below 32 bits.
 */

#define MUL_BY_CLKS_PER_SEC(p, d) \
	DIV_ROUND_UP((d) * (p)->clk_rate, clk_divs[(p)->cks])

static const unsigned int clk_divs[] = { 1, 4, 16, 32, 64, 128, 1024, 4096 };

struct rwdt_priv {
	struct clk clk;
	void __iomem *base;
	u8 cks;
	unsigned long clk_rate;
	unsigned int timeout;
};

static const struct udevice_id rwdt_match[] = {
	{ .compatible = "renesas,r8a774a1-wdt", },
	{ .compatible = "renesas,r8a774b1-wdt", },
	{ .compatible = "renesas,r8a774c0-wdt", },
	{ .compatible = "renesas,r8a774e1-wdt", },
	{ /* sentinel */ }
};

static void rwdt_write(struct rwdt_priv *priv, u32 val, unsigned int reg)
{
	if (reg == RWTCNT) {
		val &= 0xffff;
		val |= CODE_VAL1;	/* RWTCNT */
	} else {
		val &= 0xff;
		val |= CODE_VAL2_3;	/* RWTCSRA and RWTCSRB */
	}

	writel(val, priv->base + reg);
}

static int rwdt_init_timeout(struct udevice *watchdog_dev)
{
	struct rwdt_priv *priv = dev_get_priv(watchdog_dev);

	rwdt_write(priv, MAX_VAL - MUL_BY_CLKS_PER_SEC(priv, priv->timeout),
			RWTCNT);

	return 0;
}

void hw_watchdog_reset(void)
{
	struct udevice *watchdog_dev;

	uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev);
	if (watchdog_dev)
		rwdt_init_timeout(watchdog_dev);
}

void rwdt_set_timeout(unsigned int watchdog_timeout, bool flag)
{
	struct udevice *watchdog_dev;

	uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev);
	if (watchdog_dev) {
		struct rwdt_priv *priv = dev_get_priv(watchdog_dev);
		u8 val;

		priv->timeout = watchdog_timeout;
		if (flag == true) {
			/* Stop the timer of watchdog */
			val = readl(priv->base + RWTCSRA) & ~RWTCSRA_TME;
			rwdt_write(priv, val, RWTCSRA);

			/* Re-init timeout of watchdog */
			rwdt_init_timeout(watchdog_dev);

		/* Write CKS0[2:0] to RWTCSRA and CKS1[5:0] to RWTCSRB */
			rwdt_write(priv, priv->cks, RWTCSRA);
			rwdt_write(priv, 0, RWTCSRB);

			/* Wait until bit RWTCSRA_WRFG is 0 */
			while (readl(priv->base + RWTCSRA) & RWTCSRA_WRFLG)
				cpu_relax();

			/* Start timer of watchdog */
			rwdt_write(priv, priv->cks | RWTCSRA_TME, RWTCSRA);
		}
	}
}

static int rwdt_start(struct udevice *watchdog_dev, u64 timeout_ms, ulong flag)
{
	struct rwdt_priv *priv = dev_get_priv(watchdog_dev);
	u8 val;

	priv->timeout = fdtdec_get_int(gd->fdt_blob,
				dev_of_offset(watchdog_dev), "timeout-sec", 1);

	/* Stop the timer before we modify any register */
	val = readl(priv->base + RWTCSRA) & ~RWTCSRA_TME;
	rwdt_write(priv, val, RWTCSRA);

	rwdt_init_timeout(watchdog_dev);

	rwdt_write(priv, priv->cks, RWTCSRA);
	rwdt_write(priv, 0, RWTCSRB);

	while (readl(priv->base + RWTCSRA) & RWTCSRA_WRFLG)
		cpu_relax();

	rwdt_write(priv, priv->cks | RWTCSRA_TME, RWTCSRA);
	return 0;
}

static int rwdt_probe(struct udevice *watchdog_dev)
{
	struct rwdt_priv *priv = dev_get_priv(watchdog_dev);
	int ret, i;
	unsigned long clks_per_sec;

	priv->base = (void *) devfdt_get_addr(watchdog_dev);
	printf("WDT:   watchdog@%p\n", priv->base);
	if (!priv->base) {
		printf("failed to get wdt addr\n");
		return -EINVAL;
	}

	ret = clk_get_by_index(watchdog_dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;
	priv->clk_rate = clk_get_rate(&priv->clk);

	for (i = ARRAY_SIZE(clk_divs) - 1; i >= 0; i--) {
		clks_per_sec = priv->clk_rate / clk_divs[i];
		if (clks_per_sec && clks_per_sec < 65536) {
			priv->cks = i;
			break;
		}
	}

	return 0;
}

static const struct wdt_ops rwdt_ops = {
	.start    = rwdt_start,
};

U_BOOT_DRIVER(renesas_wdt) = {
	.name     = "renesas-wdt",
	.id       = UCLASS_WDT,
	.of_match = rwdt_match,
	.probe    = rwdt_probe,
	.ops      = &rwdt_ops,
	.priv_auto_alloc_size = sizeof(struct rwdt_priv),
};
