/*
 * Copyright (C) 2016 - 2018 TQ Systems GmbH
 * Author: Marco Felsch <Marco.Felsch@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
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
#include <i2c.h>
#include <libfdt.h>
#include <linux/sizes.h>
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

#define PAD_CTL_SPEED_MED_1	(1 << 6)

#define I2C_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |			\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |			\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_ODE)

#define GPIO_OUT_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

/* handle MX6ULL first, MX6ULL implies MX6UL */
#if defined(CONFIG_MX6ULL)

#define USDHC_DATA_PAD_CTRL (PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_34ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL  (PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_CMD_PAD_CTRL (USDHC_DATA_PAD_CTRL)

#elif defined(CONFIG_MX6UL)

#define USDHC_DATA_PAD_CTRL (PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_MED_1 |	\
	PAD_CTL_DSE_120ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (USDHC_DATA_PAD_CTRL)

#define USDHC_CMD_PAD_CTRL (USDHC_DATA_PAD_CTRL)

#else
#error
#endif

#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)

#define QSPI_SS_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MED |	\
	PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_DSE_60ohm)

#define QSPI_DATA_PAD_CTRL (PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MED |	\
	PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_DSE_34ohm)

#define QSPI_CLK_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MED |	\
	PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_DSE_34ohm)

#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)

#define QSPI_SS_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MED |	\
	PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_DSE_48ohm)

#define QSPI_DATA_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MED |	\
	PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_DSE_48ohm)

#define QSPI_CLK_PAD_CTRL (QSPI_DATA_PAD_CTRL)

#else

#error

#endif

static const uint16_t tqma6ul_emmc_dsr = 0x0100;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

/* eMMC on USDHC2 always present */
static iomux_v3_cfg_t const tqma6ul_usdhc2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_NAND_RE_B__USDHC2_CLK,	USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_WE_B__USDHC2_CMD,	USDHC_CMD_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA00__USDHC2_DATA0,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA01__USDHC2_DATA1,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA02__USDHC2_DATA2,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA03__USDHC2_DATA3,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA04__USDHC2_DATA4,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA05__USDHC2_DATA5,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA06__USDHC2_DATA6,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NAND_DATA07__USDHC2_DATA7,	USDHC_DATA_PAD_CTRL),
	/* eMMC reset */
	/* TODO: should we mux it as GPIO ? */
	NEW_PAD_CTRL(MX6_PAD_NAND_ALE__USDHC2_RESET_B,	GPIO_OUT_PAD_CTRL),
};

/*
 * According to board_mmc_init() the following map is done:
 * (U-boot device node)    (Physical Port)
 * mmc0                    eMMC (SD2) on TQMa6UL
 * mmc1 .. n               optional slots used on baseboard
 */
struct fsl_esdhc_cfg tqma6ul_usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		/* eMMC/uSDHC2 is always present */
		ret = 1;
	else
		ret = tqc_bb_board_mmc_getcd(mmc);

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		/* eMMC/uSDHC2 is always present */
		ret = 0;
	else
		ret = tqc_bb_board_mmc_getwp(mmc);

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(tqma6ul_usdhc2_pads,
					 ARRAY_SIZE(tqma6ul_usdhc2_pads));
	tqma6ul_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &tqma6ul_usdhc_cfg))
		puts("Warning: failed to initialize eMMC dev\n");

	tqc_bb_board_mmc_init(bis);

	return 0;
}

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);
	if (emmc != mmc)
		return;

	if (tqc_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqma6ul_emmc_dsr);
}

static struct i2c_pads_info tqma6ul_i2c4_pads = {
	/* I2C4: on board LM75, M24C64, */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_UART2_TX_DATA__I2C4_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_UART2_TX_DATA__GPIO1_IO20,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 20)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_UART2_RX_DATA__I2C4_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_UART2_RX_DATA__GPIO1_IO21,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 21)
	}
};

static void tqma6ul_setup_i2c(void)
{
	int ret;

	if (check_module_fused(MX6_MODULE_I2C4)) {
		puts("I2C4: fused\n");
		return;
	}
	/*
	 * use logical index for bus, e.g. I2C4 -> 3
	 * warn on error
	 */
	ret = setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f,
			&tqma6ul_i2c4_pads);
	if (ret)
		printf("setup I2C4 failed: %d\n", ret);
}

static iomux_v3_cfg_t const tqma6ul_quadspi_pads[] = {
	MX6_PAD_NAND_READY_B__QSPI_A_DATA00 | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX6_PAD_NAND_CE0_B__QSPI_A_DATA01   | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX6_PAD_NAND_CE1_B__QSPI_A_DATA02   | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX6_PAD_NAND_CLE__QSPI_A_DATA03     | MUX_PAD_CTRL(QSPI_DATA_PAD_CTRL),
	MX6_PAD_NAND_WP_B__QSPI_A_SCLK      | MUX_PAD_CTRL(QSPI_CLK_PAD_CTRL),
	MX6_PAD_NAND_DQS__QSPI_A_SS0_B      | MUX_PAD_CTRL(QSPI_SS_PAD_CTRL),
	/* TODO: QSPI chip Reset is not connected */
};

static void tqma6ul_setup_qspi(void)
{
	/* QSPI chip Reset is provided from PMIC */
	/* Set the iomux */
	imx_iomux_v3_setup_multiple_pads(tqma6ul_quadspi_pads,
					 ARRAY_SIZE(tqma6ul_quadspi_pads));
	/* Set the clock */
	enable_qspi_clk(0);
}

int board_early_init_f(void)
{
	return tqc_bb_board_early_init_f();
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tqma6ul_setup_i2c();
	tqma6ul_setup_qspi();
	tqc_bb_board_init();

	return 0;
}

static const char *tqma6ul_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6UL:
#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)
		return "TQMa6ULx REV.030x";
#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)
		return "TQMa6ULxL REV.020x";
#else
#error
#endif
		break;
	case MXC_CPU_MX6ULL:
#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)
		return "TQMa6ULLx REV.030x";
#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)
		return "TQMa6ULLxL REV.020x";
#else
#error
#endif
		break;
	default:
		return "??";
	};
}

#define PFUZE3000_VLDOXEN_B 4
#define PFUZE3000_VLDO_DISABLE(ctlreg) (ctlreg &= ~(1 << PFUZE3000_VLDOXEN_B))

#define PFUZE3000_SWX_MODE_OMODE	BIT(5)
#define PFUZE3000_SWX_MODE_SW_MMODE	0x0f
#define PFUZE3000_SW_DISABLE(modereg) (modereg &= \
	~((PFUZE3000_SWX_MODE_OMODE) | PFUZE3000_SWX_MODE_SW_MMODE))

int power_init_board(void)
{
	struct pmic *p;
	u32 reg, rev;
	int ret;

	/* PMIC on I2C4 */
	ret = power_pfuze3000_init(3);
	if (ret)
		return ret;

	p = pmic_get("PFUZE3000");
	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(p, PFUZE3000_REVID, &rev);
	printf("PMIC: PFUZE3000 ID=0x%02x REV=0x%02x\n", reg, rev);

	/* disable VLDO2, NC */
	pmic_reg_read(p, PFUZE3000_VLDO2CTL, &reg);
	PFUZE3000_VLDO_DISABLE(reg);
	pmic_reg_write(p, PFUZE3000_VLDO2CTL, reg);

	/* set VLDO3 voltage 1.8, VCC1V8 @ base board connector */
	pmic_reg_read(p, PFUZE3000_VLDO3CTL, &reg);
	reg &= ~(0x0F);
	pmic_reg_write(p, PFUZE3000_VLDO3CTL, reg);

	/* set VLDO4 voltage 1.8, e-MMC / QSPI VCC IO */
	pmic_reg_read(p, PFUZE3000_VLD4CTL, &reg);
	reg &= ~(0x0F);
	pmic_reg_write(p, PFUZE3000_VLD4CTL, reg);

	/* disable SW1A */
	pmic_reg_read(p, PFUZE3000_SW1AMODE, &reg);
	PFUZE3000_SW_DISABLE(reg);
	pmic_reg_write(p, PFUZE3000_SW1AMODE, reg);

#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)

	/* disable VLDO1, reserved */
	pmic_reg_read(p, PFUZE3000_VLDO1CTL, &reg);
	PFUZE3000_VLDO_DISABLE(reg);
	pmic_reg_write(p, PFUZE3000_VLDO1CTL, reg);

#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)

	/* set VLDO1 voltage 2.5, VCC2V5 @ base board connector */
	pmic_reg_read(p, PFUZE3000_VLDO1CTL, &reg);
	reg &= ~(0x0F);
	reg |= 0x07;
	pmic_reg_write(p, PFUZE3000_VLDO1CTL, reg);

	/* disable SW2 */
	pmic_reg_read(p, PFUZE3000_SW2MODE, &reg);
	PFUZE3000_SW_DISABLE(reg);
	pmic_reg_write(p, PFUZE3000_SW2MODE, reg);

#else
#error
#endif

	return 0;
}


#ifdef CONFIG_CMD_BMODE
static const struct boot_mode tqma6ul_board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd", MAKE_CFGVAL(0x42, 0x20, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"qspi", MAKE_CFGVAL(0x10, 0x00, 0x00, 0x00)},
	{NULL, 0},
};
#endif

/* TODO: use define or const for "TQMa6UL", check with PM if TQMa6UL or TQMA6UL */
int board_late_init(void)
{
	int ret;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	struct tqc_eeprom_data eedat;

	add_board_boot_modes(tqma6ul_board_boot_modes);

	setenv("board_name", tqma6ul_get_boardname());

	ret = tqc_read_eeprom(3, CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);
	if (!ret) {
		/* ID */
		tqc_parse_eeprom_id(&eedat, safe_string,
				    ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQMa6UL", 5))
			setenv("boardtype", safe_string);
		if (0 == tqc_parse_eeprom_serial(&eedat, safe_string,
						   ARRAY_SIZE(safe_string)))
			setenv("serial#", safe_string);
		else
			setenv("serial#", "???");

		tqc_show_eeprom(&eedat, "TQMa6UL");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	tqc_bb_board_late_init();

	return 0;
}

int checkboard(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6UL:
#if defined(CONFIG_MX6ULL)
		printf("*** ERROR: image not compiled for i.MX6UL\n");
#endif
		break;
	case MXC_CPU_MX6ULL:
#if !defined(CONFIG_MX6ULL)
		printf("*** ERROR: image not compiled for i.MX6ULL!\n");
#endif
		break;
	default:
		printf("*** ERROR: image not compiled for this CPU!\n");
	}

	printf("Board: %s on a %s\n", tqma6ul_get_boardname(),
	       tqc_bb_get_boardname());
	return 0;
}

int board_get_rtc_bus(void)
{
	return 3;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)

#define MODELSTRLEN 48u

static void tqma6ul_ft_qspi_setup(void *blob, bd_t *bd)
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
#elif defined(CONFIG_SPI_FLASH)
		struct spi_flash *new;

		new = spi_flash_probe(bus, cs, speed, mode);
		if (new) {
			spi_flash_free(new);
			enable_flash = 1;
		}
#endif
#endif
	}
	off = fdt_node_offset_by_compatible(blob, -1, "fsl,imx6ul-qspi");
	if (off >= 0)
		fdt_set_node_status(blob, off, (enable_flash) ?
				    FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
				    0);
}


static const char * const tqma6ul_emmc_dt_path[] = {
	"/soc/aips-bus@02100000/usdhc@02194000",
	"/soc/aips-bus@2100000/usdhc@2194000",
	"/soc/bus@2100000/mmc@2194000",
};

int ft_board_setup(void *blob, bd_t *bd)
{
	struct mmc *mmc = find_mmc_device(0);
	char modelstr[MODELSTRLEN];
	int err;

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma6ul_get_boardname(),
		 tqc_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tqc_emmc_need_dsr(mmc) > 0) {
			err = tqc_ft_try_fixup_emmc_dsr(blob,
							tqma6ul_emmc_dt_path,
							ARRAY_SIZE(tqma6ul_emmc_dt_path),
							(u32)tqma6ul_emmc_dsr
						       );
			if (err)
				puts("ERROR: failed to patch e-MMC DSR in DT\n");
		}
	} else {
		puts("e-MMC: not present?\n");
	}

	tqma6ul_ft_qspi_setup(blob, bd);
	tqc_bb_ft_board_setup(blob, bd);

	return 0;
}
#else
#error "need CONFIG_OF_BOARD_SETUP and CONFIG_OF_LIBFDT"
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
