/*
 * Copyright 2016 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "tqmls102xa_bb.h"
#include "../common/tqc_emmc.h"
#include <spi.h>
#include <spi_flash.h>

DECLARE_GLOBAL_DATA_PTR;

static u8 tqmls102xa_module_eeprom_addr = CONFIG_SYS_I2C_EEPROM_ADDR;
static const uint16_t tqmls102xa_emmc_dsr = 0x0100;

void ddrmc_init(void)
{
	struct ccsr_ddr *ddr = (struct ccsr_ddr *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 temp_sdram_cfg;

	printf("DDR ECC: %s - ", (DDR_SDRAM_CFG & (1 << 29)) ? "ON" : "OFF");

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG);

	out_be32(&ddr->cs0_bnds, DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, DDR_CS0_CONFIG);
#ifdef CONFIG_SYS_2G_CS0CS1
	out_be32(&ddr->cs1_bnds, DDR_CS1_BNDS);
	out_be32(&ddr->cs1_config, DDR_CS1_CONFIG);
#endif

	out_be32(&ddr->timing_cfg_0, DDR_TIMING_CFG_0);
	out_be32(&ddr->timing_cfg_1, DDR_TIMING_CFG_1);
	out_be32(&ddr->timing_cfg_2, DDR_TIMING_CFG_2);
	out_be32(&ddr->timing_cfg_3, DDR_TIMING_CFG_3);
	out_be32(&ddr->timing_cfg_4, DDR_TIMING_CFG_4);
	out_be32(&ddr->timing_cfg_5, DDR_TIMING_CFG_5);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		out_be32(&ddr->sdram_cfg_2,
				 DDR_SDRAM_CFG_2 & ~SDRAM_CFG2_D_INIT);
		out_be32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		out_be32(&ddr->init_ext_addr, (1 << 31));

		/* DRAM VRef will not be trained */
		out_be32(&ddr->ddr_cdr2,
				 DDR_DDR_CDR2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		out_be32(&ddr->sdram_cfg_2, DDR_SDRAM_CFG_2);
		out_be32(&ddr->ddr_cdr2, DDR_DDR_CDR2);
	}

	out_be32(&ddr->sdram_mode, DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, DDR_SDRAM_MODE_2);

	out_be32(&ddr->sdram_interval, DDR_SDRAM_INTERVAL);

	out_be32(&ddr->ddr_wrlvl_cntl, DDR_DDR_WRLVL_CNTL);

	out_be32(&ddr->ddr_wrlvl_cntl_2, DDR_DDR_WRLVL_CNTL_2);
	out_be32(&ddr->ddr_wrlvl_cntl_3, DDR_DDR_WRLVL_CNTL_3);

	out_be32(&ddr->ddr_cdr1, DDR_DDR_CDR1);

	out_be32(&ddr->sdram_clk_cntl, DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->ddr_zq_cntl, DDR_DDR_ZQ_CNTL);

	out_be32(&ddr->cs0_config_2, DDR_CS0_CONFIG_2);
	udelay(1);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg |= SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);

		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN & ~SDRAM_CFG_BI);

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG | temp_sdram_cfg);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* exit self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg &= ~SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);
	}
#endif
}

int dram_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	ddrmc_init();
#endif

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{CONFIG_SYS_FSL_ESDHC_ADDR},
};

int board_mmc_init(bd_t *bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (emmc != mmc)
		return;

	if (tqc_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqmls102xa_emmc_dsr);
}
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_FSL_IFC
	init_early_memctl_regs();
#endif

	arch_soc_init();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot()) {
		timer_init();
		dram_init();
	}
#endif

	tqmls102xa_bb_early_init();

	return 0;
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	get_clocks();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	preloader_console_init();

	dram_init();

	/* Allow OCRAM access permission as R/W */
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
	enable_layerscape_ns_access();
#endif

	board_init_r(NULL, 0);
}
#endif

int board_init(void)
{
#ifndef CONFIG_SYS_FSL_NO_SERDES
	fsl_serdes_init();
#endif

	ls102xa_smmu_stream_id_init();

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_U_QE
	u_qe_init();
#endif

	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#ifdef CONFIG_FSL_CAAM
	return sec_init();
#endif
}
#endif

#if defined(CONFIG_DEEP_SLEEP)
void board_sleep_prepare(void)
{
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret = -1;
	struct tqmaxx_eeprom_data eedat;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	int addr = CONFIG_SYS_I2C_EEPROM_ADDR;
	int content_valid = 1;

	ret = tqmaxx_read_eeprom(CONFIG_SYS_EEPROM_BUS_NUM,
				 addr,
				 &eedat);

	/* TQMLS102xA.0203 EEPROM i2c address is 0x54,
	 * try old address in case running on a pre-0203 module
	 */
	if (ret && (0x50 != addr)) {
		addr = 0x50;
		ret = tqmaxx_read_eeprom(CONFIG_SYS_EEPROM_BUS_NUM,
					 addr,
					 &eedat);
	}

	if (ret) {
		printf("ERROR: Module EEPROM could not be read (err %d)\n", ret);
	} else {
		/* ID */
		tqmaxx_parse_eeprom_id(&eedat, safe_string,
				       ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQM", 3))
			env_set("boardtype", safe_string);
		else
			content_valid = 0;

		/* Serial# */
		if (0 == tqmaxx_parse_eeprom_serial(&eedat,
						    safe_string,
						    ARRAY_SIZE(safe_string)))
		{
			env_set("serial#", safe_string);
		} else {
			content_valid = 0;
			env_set("serial#", "???");
		}

		/* MAC */
		if (0 == tqmaxx_parse_eeprom_mac(&eedat,
						 safe_string,
						 ARRAY_SIZE(safe_string)))
		{
			uint32_t mac = 0;
			uint8_t addr[6];
			char *ethaddr = env_get("ethaddr");

			if (ethaddr &&
			    strncmp(safe_string, ethaddr, 18)) {
				printf("\nWarning: MAC addresses don't match:\n");
				printf("Address in EEPROM is      %s\n",
				       safe_string);
				printf("Address in environment is %s\n",
				       ethaddr);
			} else {
				env_set("ethaddr", safe_string);

				/* Increment ethaddr for eth1addr, eth2addr */
				eth_parse_enetaddr(safe_string, addr);
				mac = (addr[3] << 16) | (addr[4] << 8) | addr[5];

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth1addr", addr);

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth2addr", addr);
			}
		} else {
			content_valid = 0;
		}

		if (content_valid)
			tqmls102xa_module_eeprom_addr = addr;

		tqmaxx_show_eeprom(&eedat, "TQM");
	}

	tqmls102xa_bb_late_init();

	return 0;
}
#endif

#if 0
/* Disable code to detect presence of second spi flash chip
 * because Linux DTB has the node for second spi flash removed anyway.
 * TODO: Update code to use DM functions when reenabling.
 */
int tqmls102xa_qspi_has_second_chip(void)
{
	struct spi_slave *spi;
	int ret;
	u8 idcode[5], cmd_read_id = 0x9f;

	spi = spi_setup_slave(CONFIG_SF_DEFAULT_BUS, 1,
			      CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (spi) {
		ret = spi_claim_bus(spi);
		if (ret)
			return -1;

		ret = spi_xfer(spi, 8, &cmd_read_id, NULL, SPI_XFER_BEGIN);
		ret += spi_xfer(spi, sizeof(idcode) * 8, NULL, idcode, SPI_XFER_END);
		if (ret)
			return -1;

		if (idcode[0] == 0xff && idcode[1] == 0xff &&
		    idcode[2] == 0xff && idcode[3] == 0xff &&
		    idcode[4] == 0xff)
			return 0;

		return 1;
	}

	return -1;
}
#else
int tqmls102xa_qspi_has_second_chip(void)
{
	return 1;
}
#endif


/*
 * Device Tree paths for eeprom nodes have been changed in Linux kernel 5.4.
*/
static const char * const tqmls102xa_eeprom_dt_path[] = {
	"/soc/i2c@2180000/24c64@54", /* befor Linux kernel 5.4 */
	"/soc/i2c@2180000/eeprom@54", /* since Linux kernel 5.4 */
};

void tqmls102xa_ft_fixup_eeprom(void *blob, const char *path, u32 addr)
{
	do_fixup_by_path_u32(blob, path, "reg", addr, 0);
}

int tqmls102xa_ft_try_fixup_eeprom(void *blob, const char * const *path_list,
				   size_t list_len, u32 addr)
{
	size_t idx;
	int node_off;

	for (idx = 0; idx < list_len; ++idx) {
		node_off = fdt_path_offset(blob, path_list[idx]);

		if (node_off >= 0) {
			tqmls102xa_ft_fixup_eeprom(blob, path_list[idx], addr);
			break;
		}
	}

	return (idx < list_len) ? 0 : -ENOENT;
};

int ft_board_setup(void *blob, bd_t *bd)
{
	int off;
	int present;
	int err;
#ifdef CONFIG_FSL_ESDHC
	struct mmc *mmc = find_mmc_device(0);

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tqc_emmc_need_dsr(mmc) > 0)
			tqc_ft_fixup_emmc_dsr(blob,
					      "/soc/esdhc@1560000",
					      tqmls102xa_emmc_dsr);
	} else {
		puts("e-MMC: not present?\n");
	}
#endif

	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	off = fdt_node_offset_by_compat_reg(blob, FSL_IFC_COMPAT,
					    CONFIG_SYS_IFC_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);

	off = fdt_node_offset_by_compat_reg(blob, FSL_QSPI_COMPAT,
					    QSPI0_BASE_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_OKAY, 0);

	/* Detect second qspi flash chip */
	present = tqmls102xa_qspi_has_second_chip();
	if (present == 0) {
		int offset, ret = 0;

		printf("ft_board_setup: single qspi flash found");

		offset = fdt_path_offset(blob,
					 "/soc/quadspi@1550000/mt25ql512@1");
		if (offset >= 0) {
			fdt_del_node(blob, offset);
			ret += 1;
		}

		offset = fdt_path_offset(blob,
					 "/soc/quadspi@1550000");
		if (!fdt_delprop(blob, offset, "fsl,qspi-has-second-chip"))
			ret += 2;

		switch(ret) {
		case 0: printf("\n");
			break;
		case 1: printf(", removed node for second chip.\n");
			break;
		case 2: printf(", removed property for second chip\n");
			break;
		case 3: printf(", removed references to second chip\n");
			break;
		}
	} else if (present < 0) {
		printf("ft_board_setup: chip detect error: %d\n", present);
	}

	/* Modify device tree for module eeprom address */
	if (tqmls102xa_module_eeprom_addr != CONFIG_SYS_I2C_EEPROM_ADDR) {
		err = tqmls102xa_ft_try_fixup_eeprom(blob,
						     tqmls102xa_eeprom_dt_path,
						     ARRAY_SIZE(tqmls102xa_eeprom_dt_path),
						     (u32)tqmls102xa_module_eeprom_addr
						    );

		if (err) {
			puts("ERROR: failed to patch eeprom address in DT\n");
		} else {
			printf("ft_board_setup: setting module eeprom address to 0x%x\n",
			       tqmls102xa_module_eeprom_addr);
		}
	}

	return 0;
}
