// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2020, Texas Instruments Incorporated - http://www.ti.com
 * Written by Tero Kristo <t-kristo@ti.com>
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <wdt.h>
#include <dm.h>
#include <errno.h>
#include <clk.h>

/* Hardware timeout in seconds */
#define WDT_HW_TIMEOUT	60

/* Timer register set definition */
#define RTIDWDCTRL	0x90
#define RTIDWDPRLD	0x94
#define RTIWDSTATUS	0x98
#define RTIWDKEY	0x9c
#define RTIDWDCNTR	0xa0
#define RTIWWDRXCTRL	0xa4
#define RTIWWDSIZECTRL	0xa8

#define RTIWWDRX_NMI	0xa

#define RTIWWDSIZE_50P	0x50

#define WDENABLE_KEY	0xa98559da

#define WDKEY_SEQ0		0xe51a
#define WDKEY_SEQ1		0xa35c

#define WDT_PRELOAD_SHIFT	13

#define WDT_PRELOAD_MAX		0xfff

#define DWDST			BIT(1)

struct rti_wdt_priv {
	void __iomem *base;
	unsigned long freq;
	unsigned long min_hw_heartbeat_ms;
	unsigned long last_ping;
};

static int rti_wdt_reset(struct udevice *dev)
{
	struct rti_wdt_priv *wdt = dev_get_priv(dev);
	ulong now;

	now = get_timer(0);

	/*
	 * If we attempt to ping too early, bail out. Pinging RTI
	 * WDT outside the window (too early or too late) causes the
	 * watchdog to fire.
	 */
	if (now < wdt->last_ping + wdt->min_hw_heartbeat_ms)
		return 0;

	/* put watchdog in service state */
	writel_relaxed(WDKEY_SEQ0, wdt->base + RTIWDKEY);
	/* put watchdog in active state */
	writel_relaxed(WDKEY_SEQ1, wdt->base + RTIWDKEY);

	wdt->last_ping = now;

	return 0;
}

static int rti_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct rti_wdt_priv *wdt = dev_get_priv(dev);
	u32 timer_margin;

	/* set timeout period */
	timer_margin = (u64)timeout_ms / 1000 * wdt->freq;
	timer_margin >>= WDT_PRELOAD_SHIFT;
	if (timer_margin > WDT_PRELOAD_MAX)
		timer_margin = WDT_PRELOAD_MAX;
	writel_relaxed(timer_margin, wdt->base + RTIDWDPRLD);

	/*
	 * RTI only supports a windowed mode, where the watchdog can only
	 * be petted during the open window; not too early or not too late.
	 * The HW configuration options only allow for the open window size
	 * to be 50% or less than that; we obviouly want to configure the open
	 * window as large as possible so we select the 50% option. To avoid
	 * any glitches, we accommodate 5% safety margin also, so we setup
	 * the min_hw_hearbeat at 55% of the timeout period.
	 */
	wdt->min_hw_heartbeat_ms = 11 * timeout_ms / 20;

	/* Generate NMI when wdt expires */
	writel_relaxed(RTIWWDRX_NMI, wdt->base + RTIWWDRXCTRL);

	/* Open window size 50%; this is the largest window size available */
	writel_relaxed(RTIWWDSIZE_50P, wdt->base + RTIWWDSIZECTRL);

	readl_relaxed(wdt->base + RTIWWDSIZECTRL);

	/* enable watchdog */
	writel_relaxed(WDENABLE_KEY, wdt->base + RTIDWDCTRL);

	wdt->last_ping = get_timer(0);

	debug("%s: margin=%u, min_hw_hb=%lu, last_ping=%lu\n", __func__,
	      timer_margin, wdt->min_hw_heartbeat_ms, wdt->last_ping);

	return 0;
}

static int rti_wdt_probe(struct udevice *dev)
{
	struct rti_wdt_priv *wdt = dev_get_priv(dev);
	struct clk clk;
	int ret;

	wdt->base = (void *)devfdt_get_addr(dev);
	if (!wdt->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret)
		wdt->freq = clk_get_rate(&clk);
	else
		return ret;

	debug("%s: Probing wdt%u, freq=%lu\n", __func__, dev->seq, wdt->freq);
	return 0;
}

static const struct wdt_ops rti_wdt_ops = {
	.start = rti_wdt_start,
	.reset = rti_wdt_reset,
};

static const struct udevice_id rti_wdt_ids[] = {
	{ .compatible = "ti,j7-rti-wdt" },
	{ }
};

U_BOOT_DRIVER(rti_wdt) = {
	.name = "rti-wdt",
	.id = UCLASS_WDT,
	.of_match = rti_wdt_ids,
	.ops = &rti_wdt_ops,
	.probe = rti_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct rti_wdt_priv),
};
