// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 * Based on:
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <cpsw.h>
#include <dm.h>
#include <errno.h>
#include <environment.h>
#include <i2c.h>
#include <miiphy.h>
#include <serial.h>
#include <spl.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/arch/clk_synthesizer.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/omap_mmc.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <power/tps65910.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"
#include "../common/tqc_sdmmc.h"
#include "tqma335x.h"

DECLARE_GLOBAL_DATA_PTR;

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUDEN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{OFFSET(spi0_cs1), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* GPIO0_6 */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	/* SPI0_SCLK (needs RXACTIVE to work) */
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},
	/* SPI0_D0 */
	{OFFSET(spi0_d0), (MODE(0) | RXACTIVE | PULLUDDIS)},
	/* SPI0_D1 */
	{OFFSET(spi0_d1), (MODE(0) | PULLUDEN | PULLUP_EN)},
	/* SPI0_CS0 */
	{OFFSET(spi0_cs0), (MODE(0) | PULLUDEN | PULLUP_EN)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_uart4_pin_mux(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_mmc0_pin_mux(void)
{
	configure_module_pin_mux(mmc0_pin_mux);
}

void set_mux_conf_regs(void)
{
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(i2c0_pin_mux);
	enable_board_pin_mux();
}

#ifdef CONFIG_SPL_BUILD
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

  #define DDR_CMD0_IOCTRL_512MB			DDR3L_512MB_IOCTRL_VALUE
  #define DATA_PHY_RD_DQS_SLAVE_RATIO_512MB	DDR3L_512MB_RD_DQS
  #define DATA_PHY_FIFO_WE_SLAVE_RATIO_512MB	DDR3L_512MB_PHY_FIFO_WE
  #define DATA_PHY_WR_DQS_SLAVE_RATIO_512MB	DDR3L_512MB_WR_DQS
  #define DATA_PHY_WR_DATA_SLAVE_RATIO_512MB	DDR3L_512MB_PHY_WR_DATA
  #define CMD_PHY_CTRL_SLAVE_RATIO_512MB	DDR3L_512MB_RATIO
  #define CMD_PHY_INVERT_CLKOUT_512MB		DDR3L_512MB_INVERT_CLKOUT
  #define ALLOPP_DDR3_SDRAM_CONFIG_512MB	DDR3L_512MB_EMIF_SDCFG
  #define ALLOPP_DDR3_REF_CTRL_512MB		DDR3L_512MB_EMIF_SDREF
  #define ALLOPP_DDR3_ZQ_CONFIG_512MB		DDR3L_512MB_ZQ_CFG
  #define ALLOPP_DDR3_SDRAM_TIMING1_512MB	DDR3L_512MB_EMIF_TIM1
  #define ALLOPP_DDR3_SDRAM_TIMING2_512MB	DDR3L_512MB_EMIF_TIM2
  #define ALLOPP_DDR3_SDRAM_TIMING3_512MB	DDR3L_512MB_EMIF_TIM3
  #define EMIF_DDR_PHY_CTRL_1_REG_512MB		DDR3L_512MB_EMIF_READ_LATENCY

  #define DDR_CMD0_IOCTRL_256MB			DDR3L_256MB_IOCTRL_VALUE
  #define DATA_PHY_RD_DQS_SLAVE_RATIO_256MB	DDR3L_256MB_RD_DQS
  #define DATA_PHY_FIFO_WE_SLAVE_RATIO_256MB	DDR3L_256MB_PHY_FIFO_WE
  #define DATA_PHY_WR_DQS_SLAVE_RATIO_256MB	DDR3L_256MB_WR_DQS
  #define DATA_PHY_WR_DATA_SLAVE_RATIO_256MB	DDR3L_256MB_PHY_WR_DATA
  #define CMD_PHY_CTRL_SLAVE_RATIO_256MB	DDR3L_256MB_RATIO
  #define CMD_PHY_INVERT_CLKOUT_256MB		DDR3L_256MB_INVERT_CLKOUT
  #define ALLOPP_DDR3_SDRAM_CONFIG_256MB	DDR3L_256MB_EMIF_SDCFG
  #define ALLOPP_DDR3_REF_CTRL_256MB		DDR3L_256MB_EMIF_SDREF
  #define ALLOPP_DDR3_ZQ_CONFIG_256MB		DDR3L_256MB_ZQ_CFG
  #define ALLOPP_DDR3_SDRAM_TIMING1_256MB	DDR3L_256MB_EMIF_TIM1
  #define ALLOPP_DDR3_SDRAM_TIMING2_256MB	DDR3L_256MB_EMIF_TIM2
  #define ALLOPP_DDR3_SDRAM_TIMING3_256MB	DDR3L_256MB_EMIF_TIM3
  #define EMIF_DDR_PHY_CTRL_1_REG_256MB	DDR3L_256MB_EMIF_READ_LATENCY

static const struct ddr_data ddr3_data_256mb = {
	.datardsratio0 = DATA_PHY_RD_DQS_SLAVE_RATIO_256MB,
	.datawdsratio0 = DATA_PHY_WR_DQS_SLAVE_RATIO_256MB,
	.datafwsratio0 = DATA_PHY_FIFO_WE_SLAVE_RATIO_256MB,
	.datawrsratio0 = DATA_PHY_WR_DATA_SLAVE_RATIO_256MB,
};

static const struct cmd_control ddr3_cmd_ctrl_data_256mb = {
	.cmd0csratio = CMD_PHY_CTRL_SLAVE_RATIO_256MB,
	.cmd0iclkout = CMD_PHY_INVERT_CLKOUT_256MB,

	.cmd1csratio = CMD_PHY_CTRL_SLAVE_RATIO_256MB,
	.cmd1iclkout = CMD_PHY_INVERT_CLKOUT_256MB,

	.cmd2csratio = CMD_PHY_CTRL_SLAVE_RATIO_256MB,
	.cmd2iclkout = CMD_PHY_INVERT_CLKOUT_256MB,
};

static struct emif_regs ddr3_emif_reg_data_256mb = {
	.sdram_config = ALLOPP_DDR3_SDRAM_CONFIG_256MB,
	.ref_ctrl = ALLOPP_DDR3_REF_CTRL_256MB,
	.sdram_tim1 = ALLOPP_DDR3_SDRAM_TIMING1_256MB,
	.sdram_tim2 = ALLOPP_DDR3_SDRAM_TIMING2_256MB,
	.sdram_tim3 = ALLOPP_DDR3_SDRAM_TIMING3_256MB,
	.zq_config = ALLOPP_DDR3_ZQ_CONFIG_256MB,
	.emif_ddr_phy_ctlr_1 = EMIF_DDR_PHY_CTRL_1_REG_256MB,
};

const struct ctrl_ioregs ioregs_256mb = {
	.cm0ioctl		= DDR_CMD0_IOCTRL_256MB,
	.cm1ioctl		= DDR_CMD0_IOCTRL_256MB,
	.cm2ioctl		= DDR_CMD0_IOCTRL_256MB,
	.dt0ioctl		= DDR_CMD0_IOCTRL_256MB,
	.dt1ioctl		= DDR_CMD0_IOCTRL_256MB,
};

static const struct ddr_data ddr3_data_512mb = {
	.datardsratio0 = DATA_PHY_RD_DQS_SLAVE_RATIO_512MB,
	.datawdsratio0 = DATA_PHY_WR_DQS_SLAVE_RATIO_512MB,
	.datafwsratio0 = DATA_PHY_FIFO_WE_SLAVE_RATIO_512MB,
	.datawrsratio0 = DATA_PHY_WR_DATA_SLAVE_RATIO_512MB,
};

static const struct cmd_control ddr3_cmd_ctrl_data_512mb = {
	.cmd0csratio = CMD_PHY_CTRL_SLAVE_RATIO_512MB,
	.cmd0iclkout = CMD_PHY_INVERT_CLKOUT_512MB,

	.cmd1csratio = CMD_PHY_CTRL_SLAVE_RATIO_512MB,
	.cmd1iclkout = CMD_PHY_INVERT_CLKOUT_512MB,

	.cmd2csratio = CMD_PHY_CTRL_SLAVE_RATIO_512MB,
	.cmd2iclkout = CMD_PHY_INVERT_CLKOUT_512MB,
};

static struct emif_regs ddr3_emif_reg_data_512mb = {
	.sdram_config = ALLOPP_DDR3_SDRAM_CONFIG_512MB,
	.ref_ctrl = ALLOPP_DDR3_REF_CTRL_512MB,
	.sdram_tim1 = ALLOPP_DDR3_SDRAM_TIMING1_512MB,
	.sdram_tim2 = ALLOPP_DDR3_SDRAM_TIMING2_512MB,
	.sdram_tim3 = ALLOPP_DDR3_SDRAM_TIMING3_512MB,
	.zq_config = ALLOPP_DDR3_ZQ_CONFIG_512MB,
	.emif_ddr_phy_ctlr_1 = EMIF_DDR_PHY_CTRL_1_REG_512MB,
};

const struct ctrl_ioregs ioregs_512mb = {
	.cm0ioctl		= DDR_CMD0_IOCTRL_512MB,
	.cm1ioctl		= DDR_CMD0_IOCTRL_512MB,
	.cm2ioctl		= DDR_CMD0_IOCTRL_512MB,
	.dt0ioctl		= DDR_CMD0_IOCTRL_512MB,
	.dt1ioctl		= DDR_CMD0_IOCTRL_512MB,
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
#ifdef CONFIG_SPL_SERIAL_SUPPORT
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;
#endif

	return 0;
}
#endif

const struct dpll_params *get_dpll_ddr_params(void)
{
	int ind = get_sys_clk_index();

	return &dpll_ddr3_400MHz[ind];
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	int ind = get_sys_clk_index();
	int freq = am335x_get_efuse_mpu_max_freq(cdev);

	switch (freq) {
	case MPUPLL_M_1000:
		return &dpll_mpu_opp[ind][5];
	case MPUPLL_M_800:
		return &dpll_mpu_opp[ind][4];
	case MPUPLL_M_720:
		return &dpll_mpu_opp[ind][3];
	case MPUPLL_M_600:
		return &dpll_mpu_opp[ind][2];
	case MPUPLL_M_500:
		return &dpll_mpu_opp100;
	case MPUPLL_M_300:
		return &dpll_mpu_opp[ind][0];
	}

	return &dpll_mpu_opp[ind][0];
}

static void scale_vcores_generic(int freq)
{
	int sil_rev, mpu_vdd;

	/*
	 * The GP EVM, IDK and EVM SK use a TPS65910 PMIC.  For all
	 * MPU frequencies we support we use a CORE voltage of
	 * 1.10V.  For MPU voltage we need to switch based on
	 * the frequency we are running at.
	 */
#ifndef CONFIG_DM_I2C
	if (i2c_probe(TPS65910_CTRL_I2C_ADDR)) {
#else
	if (power_tps65910_init(0)) {
#endif
		puts("WARN: cannot setup PMIC, not found!\n");
		return;
	}
	puts("PMIC:	TPS65910\n");

	/*
	 * Depending on MPU clock and PG we will need a different
	 * VDD to drive at that speed.
	 */
	sil_rev = readl(&cdev->deviceid) >> 28;
	mpu_vdd = am335x_get_tps65910_mpu_vdd(sil_rev, freq);

	/* Tell the TPS65910 to use i2c */
	tps65910_set_i2c_control();

	/* First update MPU voltage. */
	if (tps65910_voltage_update(MPU, mpu_vdd)) {
		puts("WARN: MPU voltage update failed\n");
		return;
	}

	/* Second, update the CORE voltage. */
	if (tps65910_voltage_update(CORE, TPS65910_OP_REG_SEL_1_1_0)) {
		puts("WARN: CORE voltage update failed\n");
		return;
	}

}

void gpi2c_init(void)
{
	/* When needed to be invoked prior to BSS initialization */
	static bool first_time = true;

	if (first_time) {
		enable_i2c0_pin_mux();
#ifndef CONFIG_DM_I2C
		i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED,
			 CONFIG_SYS_OMAP24_I2C_SLAVE);
#endif
		first_time = false;
	}
}

void scale_vcores(void)
{
	int freq;

	gpi2c_init();
	freq = am335x_get_efuse_mpu_max_freq(cdev);
	scale_vcores_generic(freq);
}

void am33xx_spl_board_init(void)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	printf("CPU speed grade: %d MHz\n", dpll_mpu_opp100.m);

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

void sdram_init(void)
{
	config_ddr(400, &ioregs_512mb, &ddr3_data_512mb,
		   &ddr3_cmd_ctrl_data_512mb, &ddr3_emif_reg_data_512mb, 0);
	if (get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, SZ_512M) == SZ_512M)
		return;
	config_ddr(400, &ioregs_256mb, &ddr3_data_256mb,
		   &ddr3_cmd_ctrl_data_256mb, &ddr3_emif_reg_data_256mb, 0);
	if (get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, SZ_256M) == SZ_256M)
		return;

	puts("ERROR: unknown DRAM, please reset board\n");
	hang();
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	save_omap_boot_params();
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#if defined(CONFIG_BOARD_LATE_INIT) && !defined(CONFIG_SPL_BUILD)
int board_late_init(void)
{
	int ret;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	struct tqc_eeprom_data eedat;

	puts("BOOT:\t");
	switch (gd->arch.omap_boot_device) {
	case BOOT_DEVICE_MMC1:
		puts("MMC1 (SD)\n");
		break;
	case BOOT_DEVICE_MMC2:
		puts("MMC2 (e-MMC)\n");
		break;
	case BOOT_DEVICE_SPI:
		puts("SPI (SPI-NOR)\n");
		break;
	default:
		printf("unknown (%u)\n", gd->arch.omap_boot_device);
		break;
	}

	ret = tqc_read_eeprom_buf(CONFIG_SYS_EEPROM_BUS_NUM,
				  CONFIG_SYS_I2C_EEPROM_ADDR,
				  CONFIG_SYS_I2C_EEPROM_ADDR_LEN, 0,
				  sizeof(eedat), (void *)&eedat);
	if (!ret) {
		/* ID */
		tqc_parse_eeprom_id(&eedat, safe_string,
				    ARRAY_SIZE(safe_string));
		if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
			if (!strncmp(safe_string, "TQMA335", 3))
				env_set("boardtype", safe_string);
			if (!tqc_parse_eeprom_serial(&eedat, safe_string,
						     ARRAY_SIZE(safe_string)))
				env_set("serial#", safe_string);
			else
				env_set("serial#", "???");
		}

		/*
		 * Do not set MAC addresses from EEPROM;
		 * the CPSW driver will set the correct ethaddrs from efuses
		 */

		tqc_show_eeprom(&eedat, "TQMA335");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	board_late_mmc_env_init();

	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct omap_hsmmc_plat am335x_mmc1_platdata = {
	.base_addr = (struct hsmmc *)OMAP_HSMMC2_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_8BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

U_BOOT_DEVICE(am335x_mmc1) = {
	.name = "omap_hsmmc",
	.platdata = &am335x_mmc1_platdata,
};
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *fdt, bd_t *bd)
{
	static const struct node_info nodes[] = {
		{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
	};

	tqc_ft_setup_spinor_by_alias(fdt, "spi0", nodes, ARRAY_SIZE(nodes));

	return tqc_bb_ft_board_setup(fdt, bd);
}
#endif
