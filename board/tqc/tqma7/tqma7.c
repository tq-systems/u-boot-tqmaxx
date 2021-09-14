/*
 * Copyright (C) 2016-2018 TQ Systems GmbH
 * Author: Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/spi.h>
#include <asm/io.h>
#include <common.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <libfdt.h>
#include <linux/sizes.h>
#include <i2c.h>
#include <mmc.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pfuze3000_pmic.h>
#include <power/pmic.h>
#if defined(CONFIG_FSL_QSPI)
#include <spi.h>
#include <spi_flash.h>
#endif

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"
#include "../common/tqc_emmc.h"

DECLARE_GLOBAL_DATA_PTR;

/* TODO: check drive strength and pin config with hardware */

#define USDHC_PAD_CTRL		(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CMD_PAD_CTRL	(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CLK_PAD_CTRL	(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

/* HW Rev.0200, DDR50 */
#define USDHC_CLK_PAD_CTRL_R0200	(PAD_CTL_DSE_3P3V_49OHM | \
	PAD_CTL_SRE_SLOW | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define GPIO_IN_PAD_CTRL	(PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_DSE_3P3V_196OHM | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)
#define GPIO_OUT_PAD_CTRL	(PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_DSE_3P3V_98OHM | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define I2C_PAD_CTRL		(PAD_CTL_DSE_3P3V_196OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU100KOHM)

#define QSPI_DATA_PAD_CTRL	(PAD_CTL_PUS_PU47KOHM | PAD_CTL_PUE | \
	PAD_CTL_HYS | PAD_CTL_SRE_FAST | PAD_CTL_DSE_3P3V_98OHM)
#define QSPI_CLK_PAD_CTRL	(PAD_CTL_PUS_PD100KOHM | PAD_CTL_PUE | \
	PAD_CTL_SRE_FAST | PAD_CTL_DSE_3P3V_49OHM)
#define QSPI_SS_PAD_CTRL	(PAD_CTL_PUS_PU47KOHM | PAD_CTL_PUE | \
	PAD_CTL_SRE_SLOW | PAD_CTL_DSE_3P3V_196OHM)
#define QSPI_RST_PAD_CTRL	(PAD_CTL_PUS_PU47KOHM | PAD_CTL_PUE | \
	PAD_CTL_SRE_FAST | PAD_CTL_DSE_3P3V_98OHM)

static const uint16_t tqma7_emmc_dsr = 0x0100;

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

/* eMMC on USDHCI3 always present */
static iomux_v3_cfg_t const tqma7_usdhc3_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_SD3_CLK__SD3_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_CMD__SD3_CMD,		USDHC_CMD_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA0__SD3_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA1__SD3_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA2__SD3_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA3__SD3_DATA3,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA4__SD3_DATA4,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA5__SD3_DATA5,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA6__SD3_DATA6,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA7__SD3_DATA7,	USDHC_PAD_CTRL),
	/* TODO: is this correct? */
	NEW_PAD_CTRL(MX7D_PAD_SD3_STROBE__SD3_STROBE,	USDHC_PAD_CTRL),
	/* eMMC reset */
	/* TODO: should we mux it as GPIO ? */
	NEW_PAD_CTRL(MX7D_PAD_SD3_RESET_B__SD3_RESET_B,	USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const tqma7_usdhc3_r0200_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_SD3_CLK__SD3_CLK,		USDHC_CLK_PAD_CTRL_R0200),
};

/*
 * According to board_mmc_init() the following map is done:
 * (U-boot device node)    (Physical Port)
 * mmc0                    eMMC (SD3) on TQMa7
 * mmc1 .. n               optional slots used on baseboard
 */
struct fsl_esdhc_cfg tqma7_usdhc_cfg = {
	.esdhc_base = USDHC3_BASE_ADDR,
	.max_bus_width = 8,
};

bool tqma7_emmc_needs_dsr(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (!emmc) {
		puts("e-MMC: not present?\n");
		return false;
	}

	if (mmc && (emmc != mmc))
		return false;

	if ((!mmc) && (mmc_init(emmc))) {
		puts("e-MMC: not present?\n");
		return false;
	}

	return (tqc_emmc_need_dsr(emmc) > 0) ? true : false;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 1;
	else
		ret = tqc_bb_board_mmc_getcd(mmc);

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
		ret = tqc_bb_board_mmc_getwp(mmc);

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(tqma7_usdhc3_pads,
					 ARRAY_SIZE(tqma7_usdhc3_pads));
	tqma7_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	if (fsl_esdhc_initialize(bis, &tqma7_usdhc_cfg))
		puts("Warning: failed to initialize eMMC dev\n");

	tqc_bb_board_mmc_init(bis);

	return 0;
}

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	if (tqma7_emmc_needs_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqma7_emmc_dsr);
	else
		imx_iomux_v3_setup_multiple_pads(tqma7_usdhc3_r0200_pads,
						ARRAY_SIZE(tqma7_usdhc3_r0200_pads));

}

static struct i2c_pads_info tqma7_i2c1_pads = {
	/* I2C1: on board LM75, M24C64,  */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX7D_PAD_I2C1_SCL__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX7D_PAD_I2C1_SCL__GPIO4_IO8,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 8)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX7D_PAD_I2C1_SDA__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX7D_PAD_I2C1_SDA__GPIO4_IO9,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 9)
	}
};

static void tqma7_setup_i2c(void)
{
	int ret;

	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f,
			&tqma7_i2c1_pads);
	if (ret)
		printf("setup I2C1 failed: %d\n", ret);
}

static iomux_v3_cfg_t const tqma7_quadspi_pads[] = {
	MX7D_PAD_EPDC_DATA00__QSPI_A_DATA0 | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX7D_PAD_EPDC_DATA01__QSPI_A_DATA1 | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX7D_PAD_EPDC_DATA02__QSPI_A_DATA2 | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX7D_PAD_EPDC_DATA03__QSPI_A_DATA3 | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX7D_PAD_EPDC_DATA05__QSPI_A_SCLK  | MUX_PAD_CTRL(QSPI_CLK_PAD_CTRL),
	MX7D_PAD_EPDC_DATA06__QSPI_A_SS0_B | MUX_PAD_CTRL(QSPI_SS_PAD_CTRL),
	MX7D_PAD_EPDC_DATA07__QSPI_A_SS1_B | MUX_PAD_CTRL(QSPI_SS_PAD_CTRL),
	/* TODO: QSPI chip Reset */
	MX7D_PAD_EPDC_DATA04__GPIO2_IO4    | MUX_PAD_CTRL(QSPI_RST_PAD_CTRL) |
		MUX_MODE_SION,
};

#define QSPI_RESET_GPIO	IMX_GPIO_NR(2, 4)

static void tqma7_setup_qspi(void)
{
	gpio_request(QSPI_RESET_GPIO, "qspi-rst#");
	gpio_direction_output(QSPI_RESET_GPIO, 1);

	/* Set the iomux */
	imx_iomux_v3_setup_multiple_pads(tqma7_quadspi_pads,
					 ARRAY_SIZE(tqma7_quadspi_pads));
	/* Set the clock */
	set_clk_qspi();
}

int board_early_init_f(void)
{
	return tqc_bb_board_early_init_f();
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tqma7_setup_i2c();

	tqma7_setup_qspi();

	tqc_bb_board_init();

	return 0;
}

static const char *tqma7_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX7S:
		return "TQMa7S";
		break;
	case MXC_CPU_MX7D:
		return "TQMa7D";
		break;
	default:
		return "??";
	};
}


/* TODO: */
/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 reg, rev;

	power_pfuze3000_init(0);
	p = pmic_get("PFUZE3000");
	if (p && !pmic_probe(p)) {
		pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
		pmic_reg_read(p, PFUZE3000_REVID, &rev);
		printf("PMIC: PFUZE3000 ID=0x%02x REV=0x%02x\n", reg, rev);
	}

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode tqma7_board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd1", MAKE_CFGVAL(0x10, 0x12, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x10, 0x2a, 0x00, 0x00)},
/*	{"qspi", MAKE_CFGVAL(0x00, 0x40, 0x00, 0x00)}, */
	{NULL,   0},
};
#endif

/* TODO: use define or const for "TQMa7", check with PM if TQMa7 or TQMA7 */
int board_late_init(void)
{
	int ret;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	struct tqc_eeprom_data eedat;

	add_board_boot_modes(tqma7_board_boot_modes);

	setenv("board_name", tqma7_get_boardname());

	ret = tqc_read_eeprom(0, CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);
	if (!ret) {
		/* ID */
		tqc_parse_eeprom_id(&eedat, safe_string,
				    ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQMa7", 5))
			setenv("boardtype", safe_string);
		if (0 == tqc_parse_eeprom_serial(&eedat, safe_string,
						   ARRAY_SIZE(safe_string)))
			setenv("serial#", safe_string);
		else
			setenv("serial#", "???");

		tqc_show_eeprom(&eedat, "TQMa7");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	tqc_bb_board_late_init();

	return 0;
}

u32 get_board_rev(void)
{
	return (tqma7_emmc_needs_dsr(0) ? 100 : 200);
}

int checkboard(void)
{
	printf("Board: %s rev 0%u on %s\n", tqma7_get_boardname(),
	       get_board_rev(), tqc_bb_get_boardname());
	return 0;
}

int board_get_rtc_bus(void)
{
	return 0;
}

int board_get_dtt_bus(void)
{
	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
static void tqma7_ft_qspi_setup(void *blob, bd_t *bd)
{
	int off;
	int enable_flash = 0;

	if (QSPI_BOOT == get_boot_device()) {
		enable_flash = 1;
	} else {
#if defined(CONFIG_FSL_QSPI)
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
	off = fdt_node_offset_by_compatible(blob, -1, "fsl,imx7d-qspi");
	if (off >= 0)
		fdt_set_node_status(blob, off, (enable_flash) ?
				    FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
				    0);
}

#if defined(CONFIG_IMX_BOOTAUX)
ulong board_get_usable_ram_top(ulong total_size)
{
	/* Reserve last 2MiB for M4 on modules with 512MiB RAM */
	if (gd->ram_size == SZ_512M)
		return gd->ram_top - SZ_2M;
	else
		return gd->ram_top;
}

int tqma7_ft_m4_setup(void *blob, bd_t *bd)
{
	int ret;
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,imx7d-rpmsg");

	ret = arch_auxiliary_core_check_up(0);
	if (ret) {
		int areas = 1;
		u64 start[2], size[2];

		/*
		 * Reserve 2MiB of memory for M4 (1MiB is also the minimum
		 * alignment for Linux due to MMU section size restrictions).
		 */
		start[0] = gd->bd->bi_dram[0].start;
		size[0] = SZ_512M - SZ_2M;

		/* If needed, create a second entry for memory beyond 512M */
		if (gd->bd->bi_dram[0].size > SZ_512M) {
			start[1] = gd->bd->bi_dram[0].start + SZ_512M;
			size[1] = gd->bd->bi_dram[0].size - SZ_512M;
			areas = 2;
		}

		ret = fdt_set_usable_memory(blob, start, size, areas);
		if (ret) {
			eprintf("Cannot set usable memory\n");
			return ret;
		}

		if (off > 0)
			fdt_fixup_reg_property(blob, off,
					      (u64)(gd->bd->bi_dram[0].start +
					      SZ_512M - SZ_1M),
			(u64)SZ_1M);
	} else {
		if (off > 0)
			fdt_status_disabled(blob, off);
	}

	return ret;
}
#else
inline int tqma7_ft_m4_setup(void *blob, bd_t *bd) {}
#endif

static const char * const tqma7_emmc_dt_path[] = {
	"/soc/aips-bus@030800000/usdhc@030b60000",
	"/soc/aips-bus@30800000/usdhc@30b60000",
	"/soc/bus@30800000/mmc@30b60000",
};

#define MODELSTRLEN 32u
int ft_board_setup(void *blob, bd_t *bd)
{
	char modelstr[MODELSTRLEN];
	int err;

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma7_get_boardname(),
		 tqc_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);
	tqma7_ft_m4_setup(blob, bd);

	/* bring in eMMC dsr settings if needed */
	if (tqma7_emmc_needs_dsr(0)) {
		err = tqc_ft_try_fixup_emmc_dsr(blob,
						tqma7_emmc_dt_path,
						ARRAY_SIZE(tqma7_emmc_dt_path),
						(u32)tqma7_emmc_dsr
					       );
		if (err)
			puts("ERROR: failed to patch e-MMC DSR in DT\n");
	}

	tqma7_ft_qspi_setup(blob, bd);
	tqc_bb_ft_board_setup(blob, bd);

	return 0;
}
#else
#error "need CONFIG_OF_BOARD_SETUP and CONFIG_OF_LIBFDT"
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
