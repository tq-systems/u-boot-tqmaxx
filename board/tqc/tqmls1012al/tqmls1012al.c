/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#include <common.h>

#include "tqmls1012al_bb.h"
#include "../common/tqmaxx_eeprom.h"
#include "../common/tqmaxx.h"

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = tfa_get_dram_size();

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
	if (current_el() == 3)
		out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	return 0;
}

#ifdef CONFIG_MISC_INIT_R

int misc_init_r(void)
{
	int ret = -1;
	struct tqmaxx_eeprom_data eedat_modul, eedat_board;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	ret = tqmaxx_read_eeprom(CONFIG_SYS_I2C_EEPROM_BUS,
				 TQMLS1012AL_I2C_EEPROM1_ADDR, &eedat_modul);

	if (ret) {
		printf("EEPROM: (0x%x) err %d\n", TQMLS1012AL_I2C_EEPROM1_ADDR,
		       ret);
	} else {
		/* Modul ID */
		tqmaxx_parse_eeprom_id(&eedat_modul, safe_string,
				       ARRAY_SIZE(safe_string));
		if (strncmp(safe_string, "TQMLS1012AL", 11) == 0)
			tqmaxx_env_set("modultype", safe_string);

		/* Modul Serial# */
		if (tqmaxx_parse_eeprom_serial(&eedat_modul, safe_string,
					       ARRAY_SIZE(safe_string)) == 0)
			tqmaxx_env_set("serial#", safe_string);
		else
			tqmaxx_env_set("serial#", "???");

		/* Modul MAC */
		if (tqmaxx_parse_eeprom_mac(&eedat_modul, safe_string,
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
			}
		}
	}

	ret = tqmaxx_read_eeprom_at(CONFIG_SYS_I2C_EEPROM_BUS,
				    TQMLS1012AL_I2C_EEPROM1_ADDR,
				    &eedat_board, 0x80);

	if (ret) {
		printf("EEPROM: (0x%x) err %d\n", TQMLS1012AL_I2C_EEPROM1_ADDR,
		       ret);
	} else {
		/* Board ID */
		tqmaxx_parse_eeprom_id(&eedat_board, safe_string,
				       ARRAY_SIZE(safe_string));
		if (strncmp(safe_string, "MBLS1012AL", 10) == 0)
			tqmaxx_env_set("boardtype", safe_string);

		/* Board Serial# */
		if (tqmaxx_parse_eeprom_serial(&eedat_board, safe_string,
					       ARRAY_SIZE(safe_string)) == 0)
			tqmaxx_env_set("boardserial#", safe_string);
		else
			tqmaxx_env_set("boardserial#", "???");

		/* Board MAC */
		if (tqmaxx_parse_eeprom_mac(&eedat_board, safe_string,
					    ARRAY_SIZE(safe_string)) == 0) {
			u32 mac = 0;
			u8 addr[6];
			char *eth2addr = env_get("eth2addr");

			if (eth2addr &&
			    strncmp(safe_string, eth2addr, 18)) {
				printf("\n");
				printf("Warning: MAC addresses don't match:\n");
				printf("Address in EEPROM is      %s\n",
				       safe_string);
				printf("Address in environment is %s\n",
				       eth2addr);
			} else {
				env_set("eth2addr", safe_string);

				/* Increment ethaddr for eth1addr, eth2addr */
				eth_parse_enetaddr(safe_string, addr);
				mac = (addr[3] << 16) | (addr[4] << 8) |
				       addr[5];

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth3addr", addr);

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth4addr", addr);

				mac++;
				addr[3] = (uint8_t)(mac >> 16);
				addr[4] = (uint8_t)(mac >>  8);
				addr[5] = (uint8_t)(mac >>  0);
				eth_env_set_enetaddr("eth5addr", addr);
			}
		}

		tqmaxx_show_eeprom(&eedat_modul, "TQMLS1012AL");
		tqmaxx_show_eeprom(&eedat_board, "MBLS1012AL");
	}

	tqmls1012al_bb_late_init();

	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}
