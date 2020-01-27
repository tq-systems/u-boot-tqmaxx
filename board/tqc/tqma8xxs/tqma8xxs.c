/*
 * Copyright 2018-2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <fsl_esdhc.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <dm.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <power-domain.h>
#include <cdns3-uboot.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_board_gpio.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

enum {
	USB_OTG2_PWR,
};

#if !defined(CONFIG_SPL_BUILD)

static struct tqc_gpio_init_data tqma8xxs_gid[] = {
	GPIO_INIT_DATA_ENTRY(USB_OTG2_PWR, "GPIO4_4", GPIOD_IS_OUT),
};

#endif

int board_early_init_f(void)
{
	tqc_bb_board_early_init_f();

	return 0;
}

static void board_gpio_init(void)
{

}

static const char *tqma8xxs_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8QXP:
		return "TQMa8XQPS";
		break;
	case MXC_CPU_IMX8DX:
		return "TQMa8XDS";
		break;
	default:
		return "??";
	}

	return "UNKNOWN";
}

int checkboard(void)
{
	print_bootinfo();

	printf("Board: %s on a %s\n", tqma8xxs_get_boardname(),
	       tqc_bb_get_boardname());

	/* Note:  After reloc, ipcHndl will no longer be valid.  If handle
	 *        returned by sc_ipc_open matches SC_IPC_CH, use this
	 *        macro (valid after reloc) for subsequent SCI calls.
	 */
	if (gd->arch.ipc_channel_handle != SC_IPC_CH)
		printf("\nSCI error! Invalid handle\n");

#ifdef SCI_FORCE_ABORT
	sc_rpc_msg_t abort_msg;

	puts("Send abort request\n");
	RPC_SIZE(&abort_msg) = 1;
	RPC_SVC(&abort_msg) = SC_RPC_SVC_ABORT;
	sc_ipc_write(SC_IPC_CH, &abort_msg);

	/* Close IPC channel */
	sc_ipc_close(SC_IPC_CH);
#endif /* SCI_FORCE_ABORT */

	return tqc_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_MXC_GPIO
	board_gpio_init();
#endif

#if !defined(CONFIG_SPL_BUILD)
	tqc_board_gpio_init(tqma8xxs_gid, ARRAY_SIZE(tqma8xxs_gid));
#endif

	tqc_bb_board_init();

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	puts("SCI reboot request");
	sc_pm_reboot(SC_IPC_CH, SC_PM_RESET_TYPE_COLD);
	while (1)
		putc('.');
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD)
	struct tqc_eeprom_data eeprom;
	char sstring[0x41];

	if (!tqc_read_eeprom_at(1, 0x51, 1, 0, &eeprom)) {
		tqc_parse_eeprom_id(&eeprom, sstring, ARRAY_SIZE(sstring));
		if (strncmp(sstring, "TQMa8X", 6) == 0)
			env_set("boardtype", sstring);
		if (tqc_parse_eeprom_serial(&eeprom, sstring,
					    ARRAY_SIZE(sstring)) == 0)
			env_set("serial#", sstring);
		else
			env_set("serial#", "???");

		tqc_show_eeprom(&eeprom, "TQMa8X");
	} else {
		puts("EEPROM: read error\n");
	}
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", TQMA8_BOARD_NAME);
	env_set("board_rev", TQMA8_BOARD_REV);
#endif

	tqc_bb_board_late_init();

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}

