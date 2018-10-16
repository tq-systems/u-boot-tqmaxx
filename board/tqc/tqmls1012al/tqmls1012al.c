/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#include "tqmls1012al_bb.h"
#include <spi.h>
#include <spi_flash.h>

DECLARE_GLOBAL_DATA_PTR;

int esdhc_status_fixup(void *blob, const char *compat)
{
	char esdhc0_path[] = "/soc/esdhc@1560000";
	char esdhc1_path[] = "/soc/esdhc@1580000";

	do_fixup_by_path(blob, esdhc0_path, "status", "okay",
			 sizeof("okay"), 1);

	do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
			 sizeof("disabled"), 1);
	return 0;
}

int dram_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	/* RAM timing rev 0006 */
#ifdef CONFIG_TQMLS1012AL_512MB
	static const struct fsl_mmdc_info mparam = {
		0x04180000,	/* mdctl 512MB RAM */
		0x0002002D,	/* mdpdc */
		0x09444040,	/* mdotc */
		0xBABF7954,	/* mdcfg0 (RDB)*/
		0xDB328F64,	/* mdcfg1 */
		0x01FF00DB,	/* mdcfg2 */
		0x00000680,	/* mdmisc */
		0x079E8000,	/* mdref */
		0x00002000,	/* mdrwd */
		0x00551023,	/* mdor */
		0x0000003f,	/* mdasp (not used)*/
		0x0000022A,	/* mpodtctrl */
		0xA1390003,	/* mpzqhwctrl */
	};
#elif CONFIG_TQMLS1012AL_256MB
	static const struct fsl_mmdc_info mparam = {
		0x03180000,	/* mdctl 256MB RAM */
		0x0002002D,	/* mdpdc */
		0x09444040,	/* mdotc */
		0xBABF7954,	/* mdcfg0 (RDB)*/
		0xDB328F64,	/* mdcfg1 */
		0x01FF00DB,	/* mdcfg2 */
		0x00000680,	/* mdmisc */
		0x079E8000,	/* mdref */
		0x00002000,	/* mdrwd */
		0x00551023,	/* mdor */
		0x0000003f,	/* mdasp (not used)*/
		0x0000022A,	/* mpodtctrl */
		0xA1390003,	/* mpzqhwctrl */
	};
#else
	printf("ERROR: undefined RAM size\n");
#endif

	mmdc_init(&mparam);
#endif

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);

	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif
	return 0;
}

#ifdef CONFIG_DM_I2C
int enable_i2c_clk(unsigned char enable, unsigned int i2c_num)
{
	return 1;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	int ret = -1;
	struct tqmaxx_eeprom_data eedat;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	ret = tqmaxx_read_eeprom(CONFIG_SYS_I2C_EEPROM_BUS,
				 CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);

	if (ret) {
		printf("EEPROM: (0x%x) err %d\n", CONFIG_SYS_I2C_EEPROM_ADDR,
		       ret);
	} else {
		/* ID */
		tqmaxx_parse_eeprom_id(&eedat, safe_string,
				       ARRAY_SIZE(safe_string));
		if (strncmp(safe_string, "TQM", 3) == 0)
			env_set("boardtype", safe_string);

		/* Serial# */
		if (tqmaxx_parse_eeprom_serial(&eedat, safe_string,
					       ARRAY_SIZE(safe_string)) == 0)
			env_set("serial#", safe_string);
		else
			env_set("serial#", "???");

		/* MAC */
		if (tqmaxx_parse_eeprom_mac(&eedat, safe_string,
					    ARRAY_SIZE(safe_string)) == 0) {
			u32 mac = 0;
			u8 addr[6];
			char *ethaddr = env_get("ethaddr");

			if (ethaddr &&
			    strncmp(safe_string, ethaddr, 18)) {
				printf("\n");
				printf("Warning: MAC addresses don't match:\n");
				printf("Address in EEPROM is      %s\n",
				       safe_string);
				printf("Address in environment is %s\n",
				       ethaddr);
			} else {
				env_set("ethaddr", safe_string);

				/* Increment ethaddr for eth1addr, eth2addr */
				eth_parse_enetaddr(safe_string, addr);
				mac = (addr[3] << 16) | (addr[4] << 8) |
				       addr[5];

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
		}

		tqmaxx_show_eeprom(&eedat, "TQM");
	}

	tqmls1012al_bb_late_init();

	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	return 0;
}
