/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 *
 * Copyright (C) 2017 TQ Systems (ported AM57xx IDK to TQMa57xx)
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/omap_common.h>
#include <asm/arch/omap.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux_dra7xx.h>

#ifdef CONFIG_DRIVER_TI_CPSW
#include <cpsw.h>
#endif

const struct pad_conf_entry core_padconf_array_bb_tqma57xx[] = {
	/* TODO: update config to MBa57xx usage */

	{GPMC_A0, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a0.vin4b_d0 */
	{GPMC_A1, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a1.vin4b_d1 */
	{GPMC_A2, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a2.vin4b_d2 */
	{GPMC_A3, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a3.vin4b_d3 */
	{GPMC_A4, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a4.vin4b_d4 */
	{GPMC_A5, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a5.vin4b_d5 */
	{GPMC_A6, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a6.vin4b_d6 */
	{GPMC_A7, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a7.vin4b_d7 */
	{GPMC_A8, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a8.vin4b_hsync1 */
	{GPMC_A9, (M6 | PIN_INPUT | MANUAL_MODE)},      /* gpmc_a9.vin4b_vsync1 */
	{GPMC_A10, (M6 | PIN_INPUT | MANUAL_MODE)},     /* gpmc_a10.vin4b_clk1 */
	{GPMC_A11, (M6 | PIN_INPUT | MANUAL_MODE)},     /* gpmc_a11.vin4b_de1 */
	{GPMC_A12, (M6 | PIN_INPUT | MANUAL_MODE)},     /* gpmc_a12.vin4b_fld1 */

	{GPMC_CLK, (M9 | PIN_INPUT_PULLDOWN)},  /* gpmc_clk.dma_evt1 */
	{GPMC_ADVN_ALE, (M14 | PIN_INPUT_PULLUP)},      /* gpmc_advn_ale.gpio2_23 */
	{GPMC_OEN_REN, (M14 | PIN_INPUT_PULLUP)},       /* gpmc_oen_ren.gpio2_24 */
	{GPMC_WEN, (M14 | PIN_INPUT_PULLUP)},   /* gpmc_wen.gpio2_25 */
	{GPMC_BEN0, (M9 | PIN_INPUT_PULLDOWN)}, /* gpmc_ben0.dma_evt3 */
	{GPMC_BEN1, (M9 | PIN_INPUT_PULLDOWN)}, /* gpmc_ben1.dma_evt4 */
	{GPMC_WAIT0, (M14 | PIN_INPUT_PULLUP | SLEWCONTROL)},   /* gpmc_wait0.gpio2_28 */

	/* MII */
	{VIN1A_D5, (M14 | PIN_OUTPUT)}, /* vin1a_d5.gpio3_9 */
	{VIN1A_D6, (M14 | PIN_OUTPUT)}, /* vin1a_d6.gpio3_10 */
	{VIN1A_D7, (M14 | PIN_OUTPUT)}, /* vin1a_d7.gpio3_11 */
	{VIN1A_D8, (M14 | PIN_OUTPUT)}, /* vin1a_d8.gpio3_12 */
	{VIN1A_D10, (M14 | PIN_INPUT_PULLDOWN)},        /* vin1a_d10.gpio3_14 */
	{VIN1A_D12, (M14 | PIN_INPUT)}, /* vin1a_d12.gpio3_16 */
	{VIN1A_D13, (M14 | PIN_OUTPUT)},        /* vin1a_d13.gpio3_17 */
	{VIN1A_D14, (M14 | PIN_OUTPUT)},        /* vin1a_d14.gpio3_18 */
	{VIN1A_D15, (M14 | PIN_OUTPUT)},        /* vin1a_d15.gpio3_19 */
	{VIN1A_D17, (M14 | PIN_OUTPUT)},        /* vin1a_d17.gpio3_21 */
	{VIN1A_D18, (M14 | PIN_OUTPUT_PULLDOWN)},       /* vin1a_d18.gpio3_22 */
	{VIN1A_D19, (M14 | PIN_OUTPUT_PULLUP)}, /* vin1a_d19.gpio3_23 */
	{VIN1A_D22, (M14 | PIN_INPUT)}, /* vin1a_d22.gpio3_26 */
	{VIN2A_CLK0, (M14 | PIN_INPUT_PULLUP)}, /* vin2a_clk0.gpio3_28 */
	{VIN2A_DE0, (M14 | PIN_INPUT_PULLUP)},  /* vin2a_de0.gpio3_29 */
	{VIN2A_FLD0, (M14 | PIN_INPUT_PULLUP)}, /* vin2a_fld0.gpio3_30 */
	{VIN2A_HSYNC0, (M14 | PIN_INPUT_PULLUP)},       /* vin2a_hsync0.gpio3_31 */
	{VIN2A_VSYNC0, (M14 | PIN_INPUT)},      /* vin2a_vsync0.gpio4_0 */
	{VIN2A_D0, (M11 | PIN_INPUT)},  /* vin2a_d0.pr1_uart0_rxd */
	{VIN2A_D1, (M11 | PIN_OUTPUT)}, /* vin2a_d1.pr1_uart0_txd */
	{VIN2A_D2, (M10 | PIN_OUTPUT)}, /* vin2a_d2.eCAP1_in_PWM1_out */
	{VIN2A_D3, (M11 | PIN_INPUT_PULLDOWN)}, /* vin2a_d3.pr1_edc_latch0_in */
	{VIN2A_D4, (M11 | PIN_OUTPUT)}, /* vin2a_d4.pr1_edc_sync0_out */
	{VIN2A_D5, (M13 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d5.pr1_pru1_gpo2 */
	{VIN2A_D10, (M11 | PIN_OUTPUT_PULLDOWN)},       /* vin2a_d10.pr1_mdio_mdclk */
	{VIN2A_D11, (M11 | PIN_INPUT)}, /* vin2a_d11.pr1_mdio_data */
	{VIN2A_D12, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d12.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d13.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d14.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d15.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d16.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},  /* vin2a_d17.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},   /* vin2a_d18.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},   /* vin2a_d19.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT_PULLUP | MANUAL_MODE)},     /* vin2a_d20.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT_PULLUP | MANUAL_MODE)},     /* vin2a_d21.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT_PULLUP | MANUAL_MODE)},     /* vin2a_d22.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT_PULLUP | MANUAL_MODE)},     /* vin2a_d23.rgmii1_rxd0 */

	{VOUT1_CLK, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_clk.vout1_clk */
	{VOUT1_DE, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_de.vout1_de */
	{VOUT1_FLD, (M14 | PIN_OUTPUT)},        /* vout1_fld.gpio4_21 */
	{VOUT1_HSYNC, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},   /* vout1_hsync.vout1_hsync */
	{VOUT1_VSYNC, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},   /* vout1_vsync.vout1_vsync */
	{VOUT1_D0, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d0.vout1_d0 */
	{VOUT1_D1, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d1.vout1_d1 */
	{VOUT1_D2, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d2.vout1_d2 */
	{VOUT1_D3, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d3.vout1_d3 */
	{VOUT1_D4, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d4.vout1_d4 */
	{VOUT1_D5, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d5.vout1_d5 */
	{VOUT1_D6, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d6.vout1_d6 */
	{VOUT1_D7, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d7.vout1_d7 */
	{VOUT1_D8, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d8.vout1_d8 */
	{VOUT1_D9, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},      /* vout1_d9.vout1_d9 */
	{VOUT1_D10, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d10.vout1_d10 */
	{VOUT1_D11, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d11.vout1_d11 */
	{VOUT1_D12, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d12.vout1_d12 */
	{VOUT1_D13, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d13.vout1_d13 */
	{VOUT1_D14, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d14.vout1_d14 */
	{VOUT1_D15, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d15.vout1_d15 */
	{VOUT1_D16, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d16.vout1_d16 */
	{VOUT1_D17, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d17.vout1_d17 */
	{VOUT1_D18, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d18.vout1_d18 */
	{VOUT1_D19, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d19.vout1_d19 */
	{VOUT1_D20, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d20.vout1_d20 */
	{VOUT1_D21, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d21.vout1_d21 */
	{VOUT1_D22, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d22.vout1_d22 */
	{VOUT1_D23, (M0 | PIN_OUTPUT | SLEWCONTROL | MANUAL_MODE)},     /* vout1_d23.vout1_d23 */

	/* RGMII */
	{MDIO_MCLK, (M0 | PIN_INPUT_SLEW)},     /* mdio_mclk.mdio_mclk */
	{MDIO_D, (M0 | PIN_INPUT | SLEWCONTROL)},       /* mdio_d.mdio_d */
	{RMII_MHZ_50_CLK, (M14 | PIN_INPUT_PULLUP)},    /* RMII_MHZ_50_CLK.gpio5_17 */
	{UART3_RXD, (M14 | PIN_INPUT_SLEW)},    /* uart3_rxd.gpio5_18 */
	{UART3_TXD, (M14 | PIN_INPUT_SLEW)},    /* uart3_txd.gpio5_19 */
	{RGMII0_TXC, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)}, /* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},       /* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},        /* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},        /* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},        /* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_OUTPUT_PULLDOWN | MANUAL_MODE)},        /* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},  /* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},        /* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT_PULLUP | MANUAL_MODE)},   /* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT_PULLUP | MANUAL_MODE)},   /* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT_PULLUP | MANUAL_MODE)},   /* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT_PULLUP | MANUAL_MODE)},   /* rgmii0_rxd0.rgmii0_rxd0 */
	{USB1_DRVVBUS, (M0 | PIN_OUTPUT | SLEWCONTROL)},        /* usb1_drvvbus.usb1_drvvbus */
	{USB2_DRVVBUS, (M0 | PIN_OUTPUT | SLEWCONTROL)},        /* usb2_drvvbus.usb2_drvvbus */

	{GPIO6_14, (M0 | PIN_OUTPUT)},  /* gpio6_14.gpio6_14 */
	{GPIO6_15, (M0 | PIN_OUTPUT)},  /* gpio6_15.gpio6_15 */
	{GPIO6_16, (M0 | PIN_INPUT_PULLUP)},    /* gpio6_16.gpio6_16 */

	{XREF_CLK0, (M11 | PIN_INPUT_PULLDOWN)},        /* xref_clk0.pr2_mii1_col */
	{XREF_CLK1, (M11 | PIN_INPUT_PULLDOWN)},        /* xref_clk1.pr2_mii1_crs */
	{XREF_CLK2, (M14 | PIN_OUTPUT)},        /* xref_clk2.gpio6_19 */
	{XREF_CLK3, (M9 | PIN_OUTPUT_PULLDOWN)},        /* xref_clk3.clkout3 */

	{MCASP1_ACLKX, (M11 | PIN_OUTPUT_PULLDOWN)},    /* mcasp1_aclkx.pr2_mdio_mdclk */
	{MCASP1_FSX, (M11 | PIN_INPUT | SLEWCONTROL)},  /* mcasp1_fsx.pr2_mdio_data */
	{MCASP1_ACLKR, (M14 | PIN_INPUT)},      /* mcasp1_aclkr.gpio5_0 */
	{MCASP1_FSR, (M14 | PIN_INPUT)},        /* mcasp1_fsr.gpio5_1 */
	{MCASP1_AXR0, (M11 | PIN_INPUT_PULLUP | SLEWCONTROL)},  /* mcasp1_axr0.pr2_mii0_rxer */
	{MCASP1_AXR1, (M11 | PIN_INPUT_PULLUP | SLEWCONTROL)},  /* mcasp1_axr1.pr2_mii_mt0_clk */
	{MCASP1_AXR2, (M14 | PIN_INPUT)},       /* mcasp1_axr2.gpio5_4 */
	{MCASP1_AXR3, (M14 | PIN_INPUT)},       /* mcasp1_axr3.gpio5_5 */
	{MCASP1_AXR4, (M14 | PIN_OUTPUT)},      /* mcasp1_axr4.gpio5_6 */
	{MCASP1_AXR5, (M14 | PIN_OUTPUT)},      /* mcasp1_axr5.gpio5_7 */
	{MCASP1_AXR6, (M14 | PIN_OUTPUT)},      /* mcasp1_axr6.gpio5_8 */
	{MCASP1_AXR7, (M14 | PIN_OUTPUT)},      /* mcasp1_axr7.gpio5_9 */
	{MCASP1_AXR8, (M11 | PIN_OUTPUT_PULLUP | SLEWCONTROL)}, /* mcasp1_axr8.pr2_mii0_txen */
	{MCASP1_AXR9, (M11 | PIN_OUTPUT_PULLUP | SLEWCONTROL)}, /* mcasp1_axr9.pr2_mii0_txd3 */
	{MCASP1_AXR10, (M11 | PIN_OUTPUT_PULLUP | SLEWCONTROL)},        /* mcasp1_axr10.pr2_mii0_txd2 */
	{MCASP1_AXR11, (M11 | PIN_OUTPUT_PULLUP | SLEWCONTROL)},        /* mcasp1_axr11.pr2_mii0_txd1 */
	{MCASP1_AXR12, (M11 | PIN_OUTPUT_PULLUP | SLEWCONTROL)},        /* mcasp1_axr12.pr2_mii0_txd0 */
	{MCASP1_AXR13, (M11 | PIN_INPUT_PULLUP | SLEWCONTROL)}, /* mcasp1_axr13.pr2_mii_mr0_clk */
	{MCASP1_AXR14, (M11 | PIN_INPUT_SLEW)}, /* mcasp1_axr14.pr2_mii0_rxdv */
	{MCASP1_AXR15, (M11 | PIN_INPUT_SLEW)}, /* mcasp1_axr15.pr2_mii0_rxd3 */
	{MCASP2_ACLKX, (M11 | PIN_INPUT_PULLDOWN)},     /* mcasp2_aclkx.pr2_mii0_rxd2 */
	{MCASP2_FSX, (M11 | PIN_INPUT_SLEW)},   /* mcasp2_fsx.pr2_mii0_rxd1 */
	{MCASP2_AXR2, (M11 | PIN_INPUT_SLEW)},  /* mcasp2_axr2.pr2_mii0_rxd0 */
	{MCASP2_AXR3, (M11 | PIN_INPUT_SLEW)},  /* mcasp2_axr3.pr2_mii0_rxlink */
	{MCASP2_AXR4, (M14 | PIN_OUTPUT)},      /* mcasp2_axr4.gpio1_4 */
	{MCASP2_AXR5, (M14 | PIN_OUTPUT)},      /* mcasp2_axr5.gpio6_7 */
	{MCASP2_AXR6, (M14 | PIN_OUTPUT)},      /* mcasp2_axr6.gpio2_29 */
	{MCASP2_AXR7, (M14 | PIN_OUTPUT)},      /* mcasp2_axr7.gpio1_5 */
	{MCASP3_ACLKX, (M11 | PIN_INPUT_PULLDOWN)},     /* mcasp3_aclkx.pr2_mii0_crs */
	{MCASP3_FSX, (M11 | PIN_INPUT_SLEW)},   /* mcasp3_fsx.pr2_mii0_col */
	{MCASP3_AXR0, (M11 | PIN_INPUT_PULLUP | SLEWCONTROL)},  /* mcasp3_axr0.pr2_mii1_rxer */
	{MCASP3_AXR1, (M11 | PIN_INPUT_PULLUP | SLEWCONTROL)},  /* mcasp3_axr1.pr2_mii1_rxlink */
	{MCASP4_ACLKX, (M2 | PIN_INPUT)},       /* mcasp4_aclkx.spi3_sclk */
	{MCASP4_FSX, (M2 | PIN_INPUT)}, /* mcasp4_fsx.spi3_d1 */
	{MCASP4_AXR1, (M2 | PIN_INPUT_PULLUP)}, /* mcasp4_axr1.spi3_cs0 */
	{MCASP5_ACLKX, (M13 | PIN_OUTPUT | MANUAL_MODE)},       /* mcasp5_aclkx.pr2_pru1_gpo1 */
	{MCASP5_FSX, (M12 | PIN_INPUT | MANUAL_MODE)},  /* mcasp5_fsx.pr2_pru1_gpi2 */

	/* MMC1: sd card */
	{MMC1_CLK, (M0 | PIN_INPUT_PULLUP)},    /* mmc1_clk.mmc1_clk */
	{MMC1_CMD, (M0 | PIN_INPUT_PULLUP)},    /* mmc1_cmd.mmc1_cmd */
	{MMC1_DAT0, (M0 | PIN_INPUT_PULLUP)},   /* mmc1_dat0.mmc1_dat0 */
	{MMC1_DAT1, (M0 | PIN_INPUT_PULLUP)},   /* mmc1_dat1.mmc1_dat1 */
	{MMC1_DAT2, (M0 | PIN_INPUT_PULLUP)},   /* mmc1_dat2.mmc1_dat2 */
	{MMC1_DAT3, (M0 | PIN_INPUT_PULLUP)},   /* mmc1_dat3.mmc1_dat3 */
	{MMC1_SDCD, (M14 | PIN_INPUT | SLEWCONTROL)},   /* mmc1_sdcd.gpio6_27 */
	{MMC1_SDWP, (M14 | PIN_INPUT | SLEWCONTROL)},   /* mmc1_sdwp.gpio6_28 */
	{GPIO6_10, (M11 | PIN_INPUT_PULLUP)},   /* gpio6_10.pr2_mii_mt1_clk */
	{GPIO6_11, (M11 | PIN_OUTPUT_PULLUP)},  /* gpio6_11.pr2_mii1_txen */
	{MMC3_CLK, (M11 | PIN_OUTPUT_PULLUP)},  /* mmc3_clk.pr2_mii1_txd3 */
	{MMC3_CMD, (M11 | PIN_OUTPUT_PULLUP)},  /* mmc3_cmd.pr2_mii1_txd2 */
	{MMC3_DAT0, (M11 | PIN_OUTPUT_PULLUP)}, /* mmc3_dat0.pr2_mii1_txd1 */
	{MMC3_DAT1, (M11 | PIN_OUTPUT_PULLUP)}, /* mmc3_dat1.pr2_mii1_txd0 */
	{MMC3_DAT2, (M11 | PIN_INPUT_PULLUP)},  /* mmc3_dat2.pr2_mii_mr1_clk */
	{MMC3_DAT3, (M11 | PIN_INPUT_PULLDOWN)},        /* mmc3_dat3.pr2_mii1_rxdv */
	{MMC3_DAT4, (M11 | PIN_INPUT_PULLDOWN)},        /* mmc3_dat4.pr2_mii1_rxd3 */
	{MMC3_DAT5, (M11 | PIN_INPUT_PULLDOWN)},        /* mmc3_dat5.pr2_mii1_rxd2 */
	{MMC3_DAT6, (M11 | PIN_INPUT_PULLDOWN)},        /* mmc3_dat6.pr2_mii1_rxd1 */
	{MMC3_DAT7, (M11 | PIN_INPUT_PULLDOWN)},        /* mmc3_dat7.pr2_mii1_rxd0 */
	{SPI1_SCLK, (M14 | PIN_OUTPUT)},        /* spi1_sclk.gpio7_7 */
	{SPI1_D1, (M14 | PIN_OUTPUT)},  /* spi1_d1.gpio7_8 */
	{SPI1_D0, (M14 | PIN_OUTPUT)},  /* spi1_d0.gpio7_9 */
	{SPI1_CS0, (M14 | PIN_OUTPUT)}, /* spi1_cs0.gpio7_10 */
	{SPI1_CS1, (M14 | PIN_OUTPUT)}, /* spi1_cs1.gpio7_11 */
	{SPI1_CS2, (M14 | PIN_INPUT_SLEW)},     /* spi1_cs2.gpio7_12 */
	{SPI1_CS3, (M6 | PIN_INPUT_PULLUP | SLEWCONTROL)},      /* spi1_cs3.hdmi1_cec */
	{SPI2_SCLK, (M0 | PIN_INPUT)},  /* spi2_sclk.spi2_sclk */
	{SPI2_D1, (M0 | PIN_INPUT | SLEWCONTROL)},      /* spi2_d1.spi2_d1 */
	{SPI2_D0, (M0 | PIN_INPUT | SLEWCONTROL)},      /* spi2_d0.spi2_d0 */
	{SPI2_CS0, (M0 | PIN_INPUT | SLEWCONTROL)},     /* spi2_cs0.spi2_cs0 */
	{DCAN1_TX, (M15 | PULL_UP)},    /* dcan1_tx.safe for dcan1_tx */
	{DCAN1_RX, (M15 | PULL_UP)},    /* dcan1_rx.safe for dcan1_rx */
	{UART1_RXD, (M14 | PIN_OUTPUT | SLEWCONTROL)},  /* uart1_rxd.gpio7_22 */
	{UART1_TXD, (M14 | PIN_OUTPUT | SLEWCONTROL)},  /* uart1_txd.gpio7_23 */
	{UART2_RXD, (M4 | PIN_INPUT)},  /* uart2_rxd.uart2_rxd */
	{UART2_TXD, (M0 | PIN_OUTPUT)}, /* uart2_txd.uart2_txd */
	{UART2_CTSN, (M2 | PIN_INPUT)}, /* uart2_ctsn.uart3_rxd */
	{UART2_RTSN, (M1 | PIN_OUTPUT)},        /* uart2_rtsn.uart3_txd */
	{I2C2_SDA, (M1 | PIN_INPUT)},   /* i2c2_sda.hdmi1_ddc_scl */
	{I2C2_SCL, (M1 | PIN_INPUT)},   /* i2c2_scl.hdmi1_ddc_sda */
	{WAKEUP0, (M0 | PIN_INPUT)},    /* Wakeup0.Wakeup0 */
	{WAKEUP1, (M0 | PIN_INPUT)},    /* Wakeup1.Wakeup1 */
	{WAKEUP2, (M0 | PIN_INPUT)},    /* Wakeup2.Wakeup2 */
	{WAKEUP3, (M0 | PIN_INPUT)},    /* Wakeup3.Wakeup3 */
	{NMIN_DSP, (M0 | PIN_INPUT)},   /* nmin_dsp.nmin_dsp */
};

#ifdef CONFIG_IODELAY_RECALIBRATION
const struct iodelay_cfg_entry iodelay_cfg_array_bb_tqma57xx[] = {
	/* TODO: add config here */
};
#endif

const char *tqma57xx_bb_get_boardname(void)
{
	return "MBa57xx";
}

#ifdef CONFIG_IODELAY_RECALIBRATION
int tqma57xx_bb_recalibrate_iodelay(void)
{
	const struct pad_conf_entry *bb_pconf;
	const struct iodelay_cfg_entry *bb_iod;
	int bb_pconf_sz, bb_iod_sz;
	int ret;

	bb_pconf = core_padconf_array_bb_tqma57xx;
	bb_pconf_sz = ARRAY_SIZE(core_padconf_array_bb_tqma57xx);
	bb_iod = iodelay_cfg_array_bb_tqma57xx;
	bb_iod_sz = ARRAY_SIZE(iodelay_cfg_array_bb_tqma57xx);

	/* setup pad configuration and muxing */
	do_set_mux32((*ctrl)->control_padconf_core_base, bb_pconf, bb_pconf_sz);

	/* setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, bb_iod, bb_iod_sz);

	return ret;
}
#endif

#if defined(CONFIG_MMC)
int tqma57xx_bb_board_mmc_init(bd_t *bis)
{
	/* MMC1: sd card */
	omap_mmc_init(0, 0, 0, -1, -1);
	return 0;
}
#endif /* CONFIG_MMC */

#ifdef CONFIG_DRIVER_TI_CPSW
static void cpsw_control(int enabled)
{
	/* VTP can be added here */
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 2,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 3,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int tqma57xx_bb_board_eth_init(bd_t *bis)
{
	int ret;
	u32 ctrl_val;

	/* set GMII 1,2 selection setting to RGMII */
	ctrl_val = readl((*ctrl)->control_core_control_io1) & (~0x33);
	ctrl_val |= 0x22;
	writel(ctrl_val, (*ctrl)->control_core_control_io1);

	ret = cpsw_register(&cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	return ret;
}
#endif /* CONFIG_DRIVER_TI_CPSW */
