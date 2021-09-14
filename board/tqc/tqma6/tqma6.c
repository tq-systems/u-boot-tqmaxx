/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_device.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/spi.h>
#include <i2c.h>
#include <fsl_esdhc.h>
#include <libfdt.h>
#include <linux/errno.h>
#include <mmc.h>
#include <power/pfuze100_pmic.h>
#include <power/pmic.h>
#if defined(CONFIG_MXC_SPI)
#include <spi.h>
#include <spi_flash.h>
#endif

#include <fdt_support.h>
#include <spl.h>

#include "../common/tqc_emmc.h"
#include "tqma6_bb.h"
#include "tqma6_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

/*!
 * Rev. 0200 and newer optionally implements GIGE ping issue fix
 * us a global to signal presence of the fix. this should be checked
 * early. If fix is present, system I2C is I2C1 - otherwise I2C3
 */
static int tqma6_has_enet_workaround = -1;
static int tqma6_system_i2c_busnum = -1;

int tqma6_get_enet_workaround(void)
{
	return tqma6_has_enet_workaround;
}

int tqma6_get_system_i2c_bus(void)
{
	return tqma6_system_i2c_busnum;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static const uint16_t tqma6_emmc_dsr = 0x0100;

/* eMMC on USDHCI3 always present */
static iomux_v3_cfg_t tqma6_usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* eMMC reset */
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET |	MUX_PAD_CTRL(GPIO_OUT_PAD_CTRL)),
};

/*
 * According to board_mmc_init() the following map is done:
 * (U-Boot device node)    (Physical Port)
 * mmc0                    eMMC (SD3) on TQMa6
 * mmc1 .. n               optional slots used on baseboard
 */
struct fsl_esdhc_cfg tqma6_usdhc_cfg = {
	.esdhc_base = USDHC3_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 1;
	else
		ret = tqma6_bb_board_mmc_getcd(mmc);

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 0;
	else
		ret = tqma6_bb_board_mmc_getwp(mmc);

	return ret;
}

static int tqma6_emmc_init(bd_t *bis)
{
	SETUP_IOMUX_PADS(tqma6_usdhc3_pads);
	tqma6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	if (fsl_esdhc_initialize(bis, &tqma6_usdhc_cfg))
		puts("Warning: failed to initialize eMMC dev\n");

	return 0;
}

int board_mmc_init(bd_t *bis)
{
#if defined(CONFIG_SPL_BUILD)
	if (BOOT_DEVICE_MMC1 == spl_boot_device()) {
		if (2 == spl_boot_device_instance())
			return tqma6_emmc_init(bis);
		else
			return tqma6_bb_board_mmc_init(bis);
	}
	return -ENODEV;
#else
	tqma6_emmc_init(bis);
	tqma6_bb_board_mmc_init(bis);
	return 0;
#endif
 }

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);
	if (emmc != mmc)
		return;

	if (tqc_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqma6_emmc_dsr);
}

static iomux_v3_cfg_t tqma6_ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19 | (MUX_PAD_CTRL(SPI_PAD_CTRL) |
		   MUX_MODE_SION)),
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

#define TQMA6_SF_CS_GPIO IMX_GPIO_NR(3, 19)

static unsigned const tqma6_ecspi1_cs[] = {
	TQMA6_SF_CS_GPIO,
};

__weak void tqma6_iomuxc_spi(void)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(tqma6_ecspi1_cs); ++i) {
		#if defined(CONFIG_SPL_BUILD)
		gpio_request(tqma6_ecspi1_cs[i], 0);
#else
		gpio_requestf(tqma6_ecspi1_cs[i], "ecspi1-cs%d", i);
#endif
		gpio_direction_output(tqma6_ecspi1_cs[i], 1);
	}
	SETUP_IOMUX_PADS(tqma6_ecspi1_pads);
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return ((bus == CONFIG_SF_DEFAULT_BUS) &&
		(cs == CONFIG_SF_DEFAULT_CS)) ? TQMA6_SF_CS_GPIO : -1;
}

static struct i2c_pads_info tqma6dl_i2c3_pads = {
	/* I2C3: on board LM75, M24C64,  */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_GPIO_5__I2C3_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_GPIO_5__GPIO1_IO05,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_GPIO_6__I2C3_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_GPIO_6__GPIO1_IO06,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 6)
	}
};

static struct i2c_pads_info tqma6q_i2c3_pads = {
	/* I2C3: on board LM75, M24C64,  */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_GPIO_5__I2C3_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_GPIO_5__GPIO1_IO05,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_GPIO_6__I2C3_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_GPIO_6__GPIO1_IO06,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 6)
	}
};

static struct i2c_pads_info tqma6dl_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static struct i2c_pads_info tqma6q_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

void tqma6_setup_i2c(void)
{
	int ret;
	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	switch (tqma6_has_enet_workaround) {
	case 1:
		tqma6_system_i2c_busnum = 0;
		if (is_mx6dqp() || is_mx6dq())
			ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f,
					&tqma6q_i2c1_pads);
		else
			ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f,
					&tqma6dl_i2c1_pads);
		if (ret)
			printf("setup I2C1 failed: %d\n", ret);
		break;
	case 0:
		tqma6_system_i2c_busnum = 2;
		if (is_mx6dqp() || is_mx6dq())
			ret = setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f,
					&tqma6q_i2c3_pads);
		else
			ret = setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f,
					&tqma6dl_i2c3_pads);
		if (ret)
			printf("setup I2C3 failed: %d\n", ret);
		break;
	default:
		puts("No default I2C bus detected\n");
	};
}

#define TQMA6_REVDET_GPIO IMX_GPIO_NR(1, 6)

#define GPIO_REVDET_PAD_CTRL  (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW | \
			       PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#if defined(CONFIG_SPL_BUILD)
int board_early_init_f(void)
{
	return tqma6_bb_board_early_init_f();
}

void spl_board_init(void)
{
	tqma6_iomuxc_spi();
	enable_spi_clk(1, 0);
}
#else

static iomux_v3_cfg_t const tqma6_revdet_pads[] = {
	IOMUX_PADS(PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(GPIO_REVDET_PAD_CTRL)),
};

static void tqma6_detect_enet_workaround(void)
{
	SETUP_IOMUX_PADS(tqma6_revdet_pads);

	gpio_request(TQMA6_REVDET_GPIO, "tqma6-revdet");
	gpio_direction_input(TQMA6_REVDET_GPIO);
	if (gpio_get_value(TQMA6_REVDET_GPIO) == 0) {
		tqma6_has_enet_workaround = 1;
	} else if (gpio_get_value(TQMA6_REVDET_GPIO) > 0) {
		tqma6_has_enet_workaround = 0;
		gpio_direction_output(TQMA6_REVDET_GPIO, 1);
	}
	gpio_free(TQMA6_REVDET_GPIO);
}

int board_early_init_f(void)
{
	tqma6_detect_enet_workaround();
	return tqma6_bb_board_early_init_f();
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tqma6_detect_enet_workaround();
	tqma6_iomuxc_spi();
	tqma6_setup_i2c();

	tqma6_bb_board_init();

	return 0;
}
#endif

static const char *tqma6_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6SOLO:
		return "TQMa6S";
		break;
	case MXC_CPU_MX6DL:
		return "TQMa6DL";
		break;
	case MXC_CPU_MX6D:
		return "TQMa6D";
		break;
	case MXC_CPU_MX6Q:
		return "TQMa6Q";
		break;
	case MXC_CPU_MX6DP:
		return "TQMa6DP";
		break;
	case MXC_CPU_MX6QP:
		return "TQMa6QP";
		break;
	default:
		return "??";
	};
}

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 reg, rev;

	power_pfuze100_init(tqma6_system_i2c_busnum);
	p = pmic_get("PFUZE100");
	if (p && !pmic_probe(p)) {
		pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
		pmic_reg_read(p, PFUZE100_REVID, &rev);
		printf("PMIC: PFUZE100 ID=0x%02x REV=0x%02x\n", reg, rev);
	}

	return 0;
}

int board_late_init(void)
{
	int ret;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	struct tqma6_eeprom_data eedat;

	env_set("board_name", tqma6_get_boardname());

	ret = tqma6_read_eeprom(tqma6_system_i2c_busnum,
				CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);
	if (!ret) {
		/* ID */
		tqma6_parse_eeprom_id(&eedat, safe_string,
				      ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQMa6", 5))
			env_set("boardtype", safe_string);
		if (0 == tqma6_parse_eeprom_serial(&eedat, safe_string,
						   ARRAY_SIZE(safe_string)))
			env_set("serial#", safe_string);
		else
			env_set("serial#", "???");

		tqma6_show_eeprom(&eedat, "TQMa6");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	tqma6_bb_board_late_init();

	return 0;
}

int checkboard(void)
{
	printf("Board: %s on a %s\n", tqma6_get_boardname(),
	       tqma6_bb_get_boardname());

	puts("Enet workaround: ");
	switch (tqma6_get_enet_workaround()) {
	case 0:
		puts("NO");
		break;
	case 1:
		puts("OK");
		break;
	default:
		puts("???");
		break;
	};
	puts("\n");
	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#define MODELSTRLEN 32u
static const char * const tqma6_emmc_dt_path[] = {
	"/soc/aips-bus@02100000/usdhc@02198000",
	"/soc/aips-bus@2100000/usdhc@2198000",
	"/soc/bus@2100000/mmc@2198000",
};

int ft_board_setup(void *blob, bd_t *bd)
{
	struct mmc *mmc = find_mmc_device(0);
	int off;
	char modelstr[MODELSTRLEN];
	int enable_flash = 0;
	int err;

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma6_get_boardname(),
		 tqma6_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);

	if (MXC_CPU_MX6SOLO == (((get_cpu_rev() & 0xFF000) >> 12))) {
		off = fdt_node_offset_by_prop_value(blob, -1,
						    "device_type",
						    "cpu", 4);
		while (off != -FDT_ERR_NOTFOUND) {
			u32 *reg = (u32 *)fdt_getprop(blob, off, "reg", 0);
			if (*reg > 0) {
				fdt_del_node(blob, off);
			}
			off = fdt_node_offset_by_prop_value(blob, off,
							    "device_type",
							    "cpu", 4);
		}
	}

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tqc_emmc_need_dsr(mmc) > 0) {
			err = tqc_ft_try_fixup_emmc_dsr(blob,
							tqma6_emmc_dt_path,
							ARRAY_SIZE(tqma6_emmc_dt_path),
							(u32)tqma6_emmc_dsr
						       );
			if (err)
				puts("ERROR: failed to patch e-MMC DSR in DT\n");
		}
	} else {
		puts("e-MMC: not present?\n");
	}

	if (BOOT_DEVICE_SPI == imx_boot_device()) {
		enable_flash = 1;
	} else {
#if defined(CONFIG_MXC_SPI)
		unsigned int bus = CONFIG_SF_DEFAULT_BUS;
		unsigned int cs = CONFIG_SF_DEFAULT_CS;
		unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
		unsigned int mode = CONFIG_SF_DEFAULT_MODE;
#ifdef CONFIG_DM_SPI_FLASH
		struct udevice *new, *bus_dev;
		int ret;

		/* Remove the old device, otherwise probe will just be a nop */
		ret = spi_find_bus_and_cs(bus, cs, &bus_dev, &new);
		if (!ret) {
			device_remove(new);
			device_unbind(new);
		}
		ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
		if (!ret) {
			device_remove(new);
			device_unbind(new);
			enable_flash = 1;
		}
#else
		struct spi_flash *new;

		new = spi_flash_probe(bus, cs, speed, mode);
		if (new) {
			spi_flash_free(new);
			enable_flash = 1;
		}
#endif
#endif
	}
	off = fdt_node_offset_by_compatible(blob, -1, "jedec,spi-nor");
	fdt_set_node_status(blob, off,
			    (enable_flash) ? FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
			    0);

	tqma6_bb_ft_board_setup(blob, bd);

	return 0;
}
#else
#error "need CONFIG_OF_BOARD_SETUP and CONFIG_OF_LIBFDT"
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
