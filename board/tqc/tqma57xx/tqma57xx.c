/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 *
 * Copyright (C) 2017 TQ-Systems GmbH (ported AM57xx IDK to TQMa57xx)
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <palmas.h>
#include <sata.h>
#include <usb.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/gpio.h>
#include <asm/arch/omap.h>
#include <asm/arch/spl.h>
#include <environment.h>
#include <fdt_support.h>
#include <spi.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <ti-usb-phy-uboot.h>
#include <mmc.h>

#include "mux_data.h"
#include "ddr.h"
#include "tqma57xx_bb.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define SYSINFO_BOARD_NAME_MAX_LEN	45

#define TPS65903X_PRIMARY_SECONDARY_PAD2	0xFB
#define TPS65903X_PAD2_POWERHOLD_MASK		0x20

const struct omap_sysinfo sysinfo = {
	"Board: UNKNOWN TQMa57xx ??? REV UNKNOWN ?????\n"
};

static const struct dmm_lisa_map_regs tqma571x_lisa_regs = {
	.dmm_lisa_map_3 = 0x80600100,	/* 1GiB on EMIF1 */
	.is_ma_present  = 0x1
};

static const struct dmm_lisa_map_regs tqma572x_lisa_regs = {
	.dmm_lisa_map_3 = 0x80740300,	/* 2GiB on EMIF1+2 */
	.is_ma_present  = 0x1
};

static const struct dmm_lisa_map_regs tqma574x_lisa_regs = {
	.dmm_lisa_map_2 = 0xc0600200, /* 1GiB on EMIF2 */
	.dmm_lisa_map_3 = 0x80600100, /* 1GiB on EMIF1 */
	.is_ma_present  = 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
/*
 * TQMA571x: DRA722-GP ES2.0
 * TQMa572x: DRA752-GP ES2.0
 * TQMa574x: DRA762-GP ES1.0 ABZ package
 */
	if (tqc_has_memsize(TQC_VARD_MEMSIZE_1G)) {
		switch(omap_revision()) {
		case DRA722_ES2_0:
			*dmm_lisa_regs = &tqma571x_lisa_regs;
			break;
		default:
			puts("VARD: Invalid MEMSIZE/CPU combination.\n");
		}
	} else if (tqc_has_memsize(TQC_VARD_MEMSIZE_2G)) {
		switch(omap_revision()) {
		case DRA752_ES2_0:
			*dmm_lisa_regs = &tqma572x_lisa_regs;
			break;
		case DRA762_ABZ_ES1_0:
			*dmm_lisa_regs = &tqma574x_lisa_regs;
			break;
		default:
			puts("VARD: Invalid MEMSIZE/CPU combination.\n");
		}
	} else {
		puts("VARD: Invalid MEMSIZE data, fallback to omap_rev()\n");
		switch(omap_revision()) {
		case DRA722_ES2_0:
			*dmm_lisa_regs = &tqma571x_lisa_regs;
			break;
		case DRA752_ES2_0:
			*dmm_lisa_regs = &tqma572x_lisa_regs;
			break;
		case DRA762_ABZ_ES1_0:
			*dmm_lisa_regs = &tqma574x_lisa_regs;
			break;
		default:
			puts("VARD: Invalid omap_revision.\n");
		}
	}
}

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	/* TODO: implement more memtype cases when known */
	if (!tqc_has_memtype(TQC_VARD_MEMTYPE_RAM1))
		puts("VARD: Invalid MEMTYPE data, fallback to omap_rev()\n");

	if (emif_nr == 1) {
		switch(omap_revision()) {
		case DRA722_ES2_0:
			*regs = &tqma571x_emif1_ddr3_666mhz_emif_regs;
			break;
		case DRA752_ES2_0:
			*regs = &tqma572x_emif1_ddr3_532mhz_emif_regs;
			break;
		case DRA762_ABZ_ES1_0:
#ifdef CONFIG_TQMA57XX_ECC
			if (!tqc_has_memtype(TQC_VARD_MEMTYPE_ECC_MASK))
				puts("VARD: Error! ECC not placed, but enabled!\n");
#endif
			*regs = &tqma574x_emif1_ddr3_666mhz_emif_regs;
			break;
		default:
			puts("VARD: Invalid omap_revision.\n");
		}
	} else if (emif_nr == 2) {
		switch(omap_revision()) {
		case DRA752_ES2_0:
			*regs = &tqma572x_emif2_ddr3_532mhz_emif_regs;
			break;
		case DRA762_ABZ_ES1_0:
			*regs = &tqma574x_emif2_ddr3_666mhz_emif_regs;
			break;
		default:
			puts("VARD: Invalid omap_revision.\n");
		}
	}
}

void emif_get_ext_phy_ctrl_const_regs(u32 emif_nr, const u32 **regs, u32 *size)
{
	/* TODO: implement more memtype cases when known */
	if (!tqc_has_memtype(TQC_VARD_MEMTYPE_RAM1))
		puts("VARD: Invalid MEMTYPE data, fallback to omap_rev()\n");

	if (emif_nr == 1) {
		switch(omap_revision()) {
		case DRA722_ES2_0:
			*regs = tqma571x_emif1_ddr3_ext_phy_ctrl_const_regs;
			*size = ARRAY_SIZE(tqma571x_emif1_ddr3_ext_phy_ctrl_const_regs);
			break;
		case DRA752_ES2_0:
			*regs = tqma572x_emif1_ddr3_ext_phy_ctrl_const_regs;
			*size = ARRAY_SIZE(tqma572x_emif1_ddr3_ext_phy_ctrl_const_regs);
			break;
		case DRA762_ABZ_ES1_0:
			*regs = tqma574x_emif1_ddr3_ext_phy_ctrl_const_regs;
			*size = ARRAY_SIZE(tqma574x_emif1_ddr3_ext_phy_ctrl_const_regs);
			break;
		default:
			puts("VARD: Invalid omap_revision.\n");
		}
	} else if (emif_nr == 2) {
		switch(omap_revision()) {
		case DRA752_ES2_0:
			*regs = tqma572x_emif2_ddr3_ext_phy_ctrl_const_regs;
			*size = ARRAY_SIZE(tqma572x_emif2_ddr3_ext_phy_ctrl_const_regs);
			break;
		case DRA762_ABZ_ES1_0:
			*regs = tqma574x_emif2_ddr3_ext_phy_ctrl_const_regs;
			*size = ARRAY_SIZE(tqma574x_emif2_ddr3_ext_phy_ctrl_const_regs);
			break;
		default:
			puts("VARD: Invalid omap_revision.\n");
		}
	}
}

struct vcores_data tqma57xx_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask	= OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

int get_voltrail_opp(int rail_offset)
{
	int opp;

	switch (rail_offset) {
	case VOLT_MPU:
		opp = DRA7_MPU_OPP;
		break;
	case VOLT_CORE:
		opp = DRA7_CORE_OPP;
		break;
	case VOLT_GPU:
		opp = DRA7_GPU_OPP;
		break;
	case VOLT_EVE:
		opp = DRA7_DSPEVE_OPP;
		break;
	case VOLT_IVA:
		opp = DRA7_IVA_OPP;
		break;
	default:
		opp = OPP_NOM;
	}

	return opp;
}

const char *tqma57xx_get_boardname(void)
{
	switch(omap_revision()) {
	case DRA722_ES2_0:
		return "TQMa571x";
	case DRA752_ES2_0:
		return "TQMa572x";
	case DRA762_ABZ_ES1_0:
		return "TQMa574x";
	default:
		return "unknown";
	}
}

void do_board_detect(void)
{
	snprintf(sysinfo.board_string, SYSINFO_BOARD_NAME_MAX_LEN,
		 "Board: %s on a %s\n", tqma57xx_get_boardname(),
		 tqma57xx_bb_get_boardname());
}

void vcores_init(void)
{
	*omap_vcores = &tqma57xx_volts;
}

void hw_data_init(void)
{
	*prcm = &dra7xx_prcm;
	if (is_dra72x())
		*dplls_data = &dra72x_dplls;
	else if (is_dra76x())
		*dplls_data = &dra76x_dplls;
	else
		*dplls_data = &dra7xx_dplls;
	*ctrl = &dra7xx_ctrl;
}

int board_init(void)
{
	gpmc_init();
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
int board_late_init(void)
{
	u8 val;
	int ret = -1;
	struct tqc_eeprom_data eedat;
	char safe_string[0x41];
	int addr = TQC_VARD_ADDR;

	/*
	 * DEV_CTRL.DEV_ON = 1 please - else palmas switches off in 8 seconds
	 * This is the POWERHOLD-in-Low behavior.
	 */
	palmas_i2c_write_u8(TPS65903X_CHIP_P1, 0xA0, 0x1);

	/*
	 * Default FIT boot on HS devices. Non FIT images are not allowed
	 * on HS devices.
	 */
	if (get_device_type() == HS_DEVICE)
		env_set("boot_fit", "1");

	/*
	 * Set the GPIO7 Pad to POWERHOLD. This has higher priority
	 * over DEV_CTRL.DEV_ON bit. This can be reset in case of
	 * PMIC Power off. So to be on the safer side set it back
	 * to POWERHOLD mode irrespective of the current state.
	 */
	palmas_i2c_read_u8(TPS65903X_CHIP_P1,
			   TPS65903X_PRIMARY_SECONDARY_PAD2, &val);
	val = val | TPS65903X_PAD2_POWERHOLD_MASK;
	palmas_i2c_write_u8(TPS65903X_CHIP_P1,
			    TPS65903X_PRIMARY_SECONDARY_PAD2, val);

	omap_die_id_serial();
	omap_set_fastboot_vars();

	ret = tqc_read_eeprom_buf(TQC_VARD_BUS, TQC_VARD_ADDR, 1, 0,
				  sizeof(eedat), (void *)&eedat);

	if (ret) {
		printf("EEPROM: (0x%x) err %d\n", addr, ret);
	} else {
		/* ID */
		tqc_parse_eeprom_id(&eedat, safe_string,
				       ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQM", 3))
			env_set("boardtype", safe_string);

		/* Serial# */
		if (0 == tqc_parse_eeprom_serial(&eedat,
						    safe_string,
						    ARRAY_SIZE(safe_string)))
			env_set("serial#", safe_string);
		else
			env_set("serial#", "???");

		/* MAC */
		if (0 == tqc_parse_eeprom_mac(&eedat,
						 safe_string,
						 ARRAY_SIZE(safe_string)))
		{
			uint32_t mac = 0;
			uint8_t addr[6];
			char *eth2addr = env_get("eth2addr");

			if (eth2addr &&
			    strncmp(safe_string, eth2addr, 18)) {
				printf("\nWarning: MAC addresses don't match:\n");
				printf("Address in EEPROM is      %s\n",
				       safe_string);
				printf("Address in environment is %s\n",
				       eth2addr);
			} else {
				env_set("eth2addr", safe_string);

				/* Increment ethaddr for eth1addr, eth2addr */
				eth_parse_enetaddr(safe_string, addr);
				mac = (addr[3] << 16) | (addr[4] << 8) | addr[5];

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth3addr", addr);
			}
		}

		tqc_show_eeprom(&eedat, "TQM");
	}

	tqma57xx_bb_board_late_init();

	{
		u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);
		struct omap_boot_parameters *omap_boot_params;
		u8 boot_device;

		omap_boot_params = (struct omap_boot_parameters *)boot_params;
		boot_device = omap_boot_params->boot_device;

		switch (boot_device) {
		case BOOT_DEVICE_MMC1:
			env_set("mmcblkdev", "0");
			env_set("mmcdev", "0");
			break;
		default:
			env_set("mmcblkdev", "1");
			env_set("mmcdev", "1");
			break;
		}
	}

	return 0;
}
#endif

void set_muxconf_regs(void)
{
	/* setup early pad configuration and muxing */
	do_set_mux32((*ctrl)->control_padconf_core_base, early_padconf,
		     ARRAY_SIZE(early_padconf));
}

#ifdef CONFIG_IODELAY_RECALIBRATION
void recalibrate_iodelay(void)
{
	const struct pad_conf_entry *pconf;
	const struct iodelay_cfg_entry *iod;
	int pconf_sz, iod_sz;
	int ret;

	pconf = core_padconf_array_essential_tqma57xx;
	pconf_sz = ARRAY_SIZE(core_padconf_array_essential_tqma57xx);
	iod = iodelay_cfg_array_tqma57xx;
	iod_sz = ARRAY_SIZE(iodelay_cfg_array_tqma57xx);

	/* Setup I/O isolation */
	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* setup pad configuration and muxing */
	do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);

	/* setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iod, iod_sz);
	if (ret)
		goto err;

	/* setup baseboard pad configuration, muxing, IOdelay configuration */
	ret = tqma57xx_bb_recalibrate_iodelay();
err:
	/* Closeup.. remove isolation */
	__recalibrate_iodelay_end(ret);
}
#endif

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	tqma57xx_bb_board_mmc_init(bis);
	/* MMC2: eMMC */
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	if (env_get_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

void spl_board_prepare_for_boot(void)
{
	if (tqc_has_feature1(TQC_VARD_FEATURES1_EMMC))
		printf("MMC present\n");

	if (tqc_has_feature1(TQC_VARD_FEATURES1_EEPROM))
		printf("EEPROM present\n");

	if (tqc_has_feature1(TQC_VARD_FEATURES1_QSPI))
		printf("QSPI present\n");

	if (tqc_has_feature2(TQC_VARD_FEATURES2_RTC))
		printf("RTC present\n");
}
#endif

#ifdef CONFIG_USB_DWC3
static struct dwc3_device usb_otg_ss2 = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = DRA7_USB_OTG_SS2_BASE,
	.tx_fifo_resize = false,
	.index = 1,
};

static struct dwc3_omap_device usb_otg_ss2_glue = {
	.base = (void *)DRA7_USB_OTG_SS2_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 1,
};

static struct ti_usb_phy_device usb_phy2_device = {
	.usb2_phy_power = (void *)DRA7_USB2_PHY2_POWER,
	.index = 1,
};

int usb_gadget_handle_interrupts(int index)
{
	u32 status;

	status = dwc3_omap_uboot_interrupt_status(index);
	if (status)
		dwc3_uboot_handle_interrupt(index);

	return 0;
}
#endif /* CONFIG_USB_DWC3 */

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_OMAP)
int board_usb_init(int index, enum usb_init_type init)
{
	int ret;

	/* do baseboard-specific usb initialisation */
	ret = tqma57xx_bb_board_usb_init();
	if (ret)
		printf("error on tqma57xx_bb_board_usb_init\n");

	enable_usb_clocks(index);

	switch (index) {
	case 0:
		if (init == USB_INIT_DEVICE) {
			printf("port %d can't be used as device\n", index);
			disable_usb_clocks(index);
			return -EINVAL;
		}
		break;
	case 1:
		if (init == USB_INIT_DEVICE) {
#ifdef CONFIG_USB_DWC3
			usb_otg_ss2.dr_mode = USB_DR_MODE_PERIPHERAL;
			usb_otg_ss2_glue.vbus_id_status = OMAP_DWC3_VBUS_VALID;
			ti_usb_phy_uboot_init(&usb_phy2_device);
			dwc3_omap_uboot_init(&usb_otg_ss2_glue);
			dwc3_uboot_init(&usb_otg_ss2);
		}
#endif
		break;
	default:
		printf("Invalid Controller Index\n");
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
#ifdef CONFIG_USB_DWC3
	switch (index) {
	case 0:
	case 1:
		if (init == USB_INIT_DEVICE) {
			ti_usb_phy_uboot_exit(index);
			dwc3_uboot_exit(index);
			dwc3_omap_uboot_exit(index);
		}
		break;
	default:
		printf("Invalid Controller Index\n");
	}
#endif
	disable_usb_clocks(index);
	return 0;
}
#endif /* defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_OMAP) */

#ifdef CONFIG_DRIVER_TI_CPSW
int board_eth_init(bd_t *bis)
{
	int ret;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	/* try reading mac address from efuse */
	mac_lo = readl((*ctrl)->control_core_mac_id_0_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_0_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

	ret = tqma57xx_bb_board_eth_init(bis);

	return ret;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	return 0;
}
#endif

#ifdef CONFIG_TI_SECURE_DEVICE
void board_fit_image_post_process(void **p_image, size_t *p_size)
{
	secure_boot_verify_image(p_image, p_size);
}

void board_tee_image_process(ulong tee_image, size_t tee_size)
{
	secure_tee_install((u32)tee_image);
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_TEE, board_tee_image_process);
#endif

#ifdef CONFIG_SPL_BUILD
void board_boot_order(u32 *spl_boot_list)
{
	u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);
	struct omap_boot_parameters *omap_boot_params;
	u8 boot_device;

	omap_boot_params = (struct omap_boot_parameters *)boot_params;
	boot_device = omap_boot_params->boot_device;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case BOOT_DEVICE_MMC2_2:
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	case BOOT_DEVICE_SPI:
		spl_boot_list[0] = BOOT_DEVICE_SPI;
	default:
		break;
	}
}
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_ENV_IS_IN_MMC
int mmc_get_env_dev(void)
{
	int mmcdev = -1;
	u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);
	struct omap_boot_parameters *omap_boot_params;
	u8 boot_device;

	omap_boot_params = (struct omap_boot_parameters *)boot_params;
	boot_device = omap_boot_params->boot_device;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		mmcdev = 0;
		break;
	case BOOT_DEVICE_MMC2_2:
		mmcdev = 1;
		break;
	default:
		break;
	}

	return mmcdev;
}
#endif

#if 0
enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);
	struct omap_boot_parameters *omap_boot_params;
	u8 boot_device;

	omap_boot_params = (struct omap_boot_parameters *)boot_params;
	boot_device = omap_boot_params->boot_device;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2_2:
		return ENVL_MMC;
		break;
	case BOOT_DEVICE_SPI:
		return ENVL_SPI_FLASH;
		break;
	default:
		return ENVL_MMC;
		break;
	}
}
#endif

int tqma57xx_qspi_present(void)
{
	struct spi_slave *spi;
	int ret;
	u8 idcode[5], cmd_read_id = 0x9f;

	spi = spi_setup_slave(CONFIG_SF_DEFAULT_BUS, 0,
			      CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (spi) {
		ret = spi_claim_bus(spi);
		if (ret)
			return -1;

		ret = spi_xfer(spi, 8, &cmd_read_id, NULL, SPI_XFER_BEGIN);
		ret += spi_xfer(spi, sizeof(idcode) * 8, NULL, idcode, SPI_XFER_END);
		if (ret)
			return -1;

		if (idcode[0] == 0x00 && idcode[1] == 0x00 &&
		    idcode[2] == 0x00 && idcode[3] == 0x00 &&
		    idcode[4] == 0x00)
			return 0;

		return 1;
	}

	return -1;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	int present;
	int offset;

	/* Detect qspi flash chip */
	present = tqma57xx_qspi_present();
	if (!present) {
		offset = fdt_path_offset(blob, "/ocp/qspi@4b300000");
		if (offset >= 0) {
			fdt_set_node_status(blob, offset, FDT_STATUS_DISABLED, 0);
			printf("ft_board_setup: disabled qspi node.\n");
		}
	}

	return 0;
}
