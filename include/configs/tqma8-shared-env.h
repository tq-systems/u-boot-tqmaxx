/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022 TQ-Systems GmbH
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#if !defined(__TQMA8_SHARED_ENV_H)
#define __TQMA8_SHARED_ENV_H

#define TQMA8_SHARED_ENV_SETTINGS \
	"loadimage_spi=sf probe; " \
		"ubi part ${ubirootfspart}; ubifsmount ubi0:${ubirootfsvol}; " \
		"ubifsload ${loadaddr} /boot/${image}; " \
		"ubifsumount; ubi detach\0" \
	"loadfdt_spi=sf probe; " \
		"ubi part ${ubirootfspart}; ubifsmount ubi0:${ubirootfsvol}; " \
		"ubifsload ${fdt_addr} /boot/${fdt_file}; "                    \
		"ubifsumount; ubi detach\0"                                    \
	"ubiboot=echo Booting from UBI ...; "                                  \
		"setenv bootargs; "                                            \
		"run ubiargs; "                                                \
		"if run loadfdt_spi; then "                                    \
			"if run loadimage_spi; then "                          \
				"run boot_os "                                 \
			"else "                                                \
				"echo ERR: cannot load FDT; "                  \
			"fi; "                                                 \
		"fi;\0"                                                        \
	"ubirootfs=rootfs.ubifs\0"                                             \
	"ubirootfspart=ubi\0"                                                  \
	"ubirootfsvol=rootfs\0"                                                \
	"ubimtdidx=3\0"                                                        \
	"prepare_ubi_part=if sf probe; then "                                  \
			"mtd erase ubi; "                                      \
			"ubi part ubi; ubi create rootfs; ubi detach; "        \
		"fi \0"                                                        \
	"update_uboot_spi=run set_getcmd; if ${get_cmd} ${uboot}; then "       \
		"if itest ${filesize} > 0; then "                              \
			"echo Write u-boot image to flexspi ...; "             \
			"if itest ${filesize} <= ${uboot_fspi_size}; then "    \
				"if sf probe; then "                           \
					"sf update ${loadaddr} "               \
						"${uboot_fspi_start} "         \
						"${filesize}; "                \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize \0"                                           \
	"update_rootfs_spi=run set_getcmd; if ${get_cmd} ${ubirootfs}; then "  \
		"if itest ${filesize} > 0; then "                              \
			"echo Write rootfs image to UBI ...; "                 \
			"if sf probe; then "                                   \
				"ubi part ${ubirootfspart}; "                  \
				"ubi write ${loadaddr} ${ubirootfsvol} "       \
					"${filesize}; "                        \
				"ubi detach; "                                 \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize \0"                                           \
	"addubi=setenv bootargs ${bootargs} rootfstype=ubifs "                 \
		"ubi.mtd=${ubimtdidx} "                                        \
		"root=ubi0:${ubirootfsvol} ${rootfsmode} rootwait\0"           \
	"ubiargs=run addubi addtty\0"                                          \

#endif
