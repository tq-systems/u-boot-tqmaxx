// SPDX-License-Identifier: GPL-2.0+
/*
 * U-boot Watchdog commands
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <watchdog.h>
#include <wdt.h>
#include <dm.h>

struct udevice *watchdog_curdev;

int check_curdev(void)
{
	if (uclass_get_device(UCLASS_WDT, 0, &watchdog_curdev)) {
		printf("No watchdog timer device\n");
		return CMD_RET_FAILURE;
	}
	return 0;
}

static int do_wdt_get_info(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	int ret;
	u64 timeout;

	ret = check_curdev();
	if (ret)
		return ret;
	printf("Watchdog device: %s\n", watchdog_curdev->name);
	printf("Current status: %s\n",
	       env_get_ulong("wdt_status", 2, 0) ? "active" : "stopped");
	timeout = env_get_ulong("wdt_timeout", 10, 0);
	if (!timeout)
		printf("No watchdog timeout found\n");
	else
		printf("Current timeout: %lld\n", timeout);
	return CMD_RET_SUCCESS;
}

static int do_wdt_expire_now(cmd_tbl_t *cmdtp, int flag, int argc,
			     char *const argv[])
{
	int ret;

	ret = check_curdev();
	if (ret)
		return ret;
	ret = wdt_expire_now(watchdog_curdev, 0);
	if (ret < 0) {
		printf("No WDT device active!\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int do_wdt_start(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	u64 timeout;

	ret = check_curdev();
	if (ret)
		return ret;
	if (argv[1] != NULL)
		timeout = simple_strtoul(argv[1], NULL, 10);
	else
		timeout = env_get_ulong("wdt_timeout", 10, 0);
	ret = wdt_start(watchdog_curdev, timeout, 1);

	if (ret < 0) {
		printf("Cannot start watchdog\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int do_wdt_stop(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	ret = check_curdev();
	if (ret)
		return ret;
	ret = wdt_stop(watchdog_curdev);
	if (ret < 0) {
		printf("Cannot stop watchdog\n");
		return CMD_RET_FAILURE;
	}
	printf("Watchdog: Stopped!\n");
	return CMD_RET_SUCCESS;
}

static cmd_tbl_t wdt_sub_cmd[] = {
	U_BOOT_CMD_MKENT(info, 2, 1, do_wdt_get_info, "", ""),
	U_BOOT_CMD_MKENT(start, 3, 1, do_wdt_start, "", ""),
	U_BOOT_CMD_MKENT(stop, 2, 1, do_wdt_stop, "", ""),
	U_BOOT_CMD_MKENT(expire, 2, 1, do_wdt_expire_now, "", ""),
};

static int do_wdt_sub_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "wdt" arg */
	argc--;
	argv++;
	cp = find_cmd_tbl(argv[0], wdt_sub_cmd, ARRAY_SIZE(wdt_sub_cmd));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);
	return CMD_RET_USAGE;
}

static char wdt_help_text[] =
	"wdt info - get information about current watchdog device\n"
	"wdt start [timeout (s)] - start watchdog timer\n"
	"wdt stop - stop watchdog timer\n"
	"wdt expire - expire watchdog timer immediately\n";

U_BOOT_CMD(
	wdt, 3, 0, do_wdt_sub_cmd, "U-boot watchdog commands", wdt_help_text
);
