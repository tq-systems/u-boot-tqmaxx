// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Common sysinfo helpers for TQ-Systems SOMs
 *
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <fdt_support.h>
#include <mtd_node.h>
#include <net.h>
#include <spi_flash.h>
#include <sysinfo/tq_eeprom.h>

__weak size_t tq_common_sysinfo_macaddr_num(void) {
	return CONFIG_TQ_COMMON_SYSINFO_MACADDR_NUM;
}

static void tq_common_sysinfo_set_macaddrs(const u8 *macaddr)
{
	size_t i, macaddr_num = tq_common_sysinfo_macaddr_num();
	u8 macaddr_buf[ETH_ALEN];

	memcpy(macaddr_buf, macaddr, ETH_ALEN);

	for (i = 0; ; i++) {
		eth_env_set_enetaddr_by_index("eth", CONFIG_TQ_COMMON_SYSINFO_MACADDR_OFFSET + i,
					      macaddr_buf);
		if (i > macaddr_num)
			break;

		if (++macaddr_buf[5])
			continue;
		if (++macaddr_buf[4])
			continue;
		if (++macaddr_buf[3])
			continue;

		printf("Warning: End of MAC address block\n");
		break;
	}
}

void tq_common_sysinfo_setup(void)
{
	struct udevice *sysinfo;
	size_t macaddr_size;
	void *macaddr;
	char buf[80] = "";
	int ret;

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		pr_err("Failed to get sysinfo data: %d\n", ret);
		return;
	}

	if (!sysinfo_get_str(sysinfo, SYSID_TQ_MODEL, sizeof(buf), buf))
		env_set_runtime("boardtype", buf);

	if (!sysinfo_get_str(sysinfo, SYSID_TQ_SERIAL, sizeof(buf), buf))
		env_set_runtime("serial#", buf);

	if (!sysinfo_get_data(sysinfo, SYSID_TQ_MAC_ADDR, &macaddr, &macaddr_size)
	    && macaddr_size == ETH_ALEN)
		tq_common_sysinfo_set_macaddrs(macaddr);
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)
void tq_common_sysinfo_ft_board_setup(void *fdt)
{
#if IS_ENABLED(CONFIG_DM_SPI_FLASH)
	const char *path = CONFIG_TQ_COMMON_SPI_FLASH_PATH;
	const struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR },
	};
	struct udevice *new;
	int ret;

	/* TODO: Might use VARD data from EEPROM instead of probing for the flash */
	puts("Testing for SPI-NOR flash...\n");
	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, &new);
	if (!ret) {
		if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
			puts("Updating MTD partitions...\n");
			/* Update MTD partition nodes using info from mtdparts env var */
			fdt_fixup_mtdparts(fdt, nodes, ARRAY_SIZE(nodes));
		}
	} else {
		puts("No flash found.\n");
		if (*path)
			fdt_status_disabled_by_pathf(fdt, path);
	}
#endif /* IS_ENABLED(CONFIG_DM_SPI_FLASH) */
}
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) */
