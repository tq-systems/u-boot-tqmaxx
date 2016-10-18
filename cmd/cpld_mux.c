/*
 * (C) Copyright 2016 Michael Krummsdorf, TQ-Systems GmbH, <michael.krummsdorf@tq-group.com>
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <stdlib.h>

int do_cpld_mux (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	unsigned char cpld_mux;
	unsigned int oldbus;
	char *cmd = CONFIG_CPLD_MUX_MATCH_COMMAND;

	if (!cmd)
		return 0;

	/* Parse DIP switch for CPLD mux mode */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_CPLD_MUX_DIP_BUS);
	ret = i2c_read(CONFIG_CPLD_MUX_DIP_ADDRESS,
		       CONFIG_CPLD_MUX_DIP_REGISTER,
		       1, &cpld_mux, sizeof(cpld_mux));
	i2c_set_bus_num(oldbus);

	if ((!ret) && (cpld_mux & CONFIG_CPLD_MUX_DIP_MASK))
		run_command_list(cmd, -1, 0);

	return 0;
}
/***************************************************/

U_BOOT_CMD(
	cpld_mux,	CONFIG_SYS_MAXARGS,	0,	do_cpld_mux,
	"run command on read i2c value",
	     "- Reads from i2c bus/address/register,\n"
	     "         - compares to predefined mask and on a match\n"
	     "         - runs predefined command"
);
