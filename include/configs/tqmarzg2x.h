/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#ifndef __TQMARZG2X_MBARZG2X_H
#define __TQMARZG2X_MBARZG2X_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS                                             \
	"boot_os=booti ${loadaddr} - ${fdt_addr};\0"                          \
	"console=ttySC0\0"                                                   \
	"image=Image\0"                                                       \
	"loadaddr=0x48080000\0"                                               \
	"fdt_addr=0x48000000\0"                                               \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                              \
	"mmcdev=0\0"                                                          \
	"mmcpart=1\0"                                                         \
	"loadfdt=mmc dev ${mmcdev}; mmc rescan;"                              \
		"load mmc ${mmcdev}:${mmcpart} ${fdt_addr} "                  \
			"${mmcpath}${fdt_file}\0"                             \
	"loadimage=mmc dev ${mmcdev}; mmc rescan;"                            \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} "                  \
			"${mmcpath}${image}\0"                                \
	"mmcargs=run addtty addearlycon addmmc\0"                             \
	"mmcblkdev=0\0"                                                       \
	"mmcrootpart=2\0"                                                     \
	"mmcboot=echo Booting from mmc ...; "                                 \
		"setenv bootargs; "                                           \
		"run mmcargs; "                                               \
		"if run loadfdt; then "                                       \
			"if run loadimage; then "                             \
				"run boot_os; "                               \
			"else "                                               \
				"echo WARN: Cannot load the kernel; "         \
			"fi; "                                                \
		"else "                                                       \
			"echo WARN: Cannot load the DT; "                     \
		"fi;\0"                                                       \
	"netboot=echo Booting from net ...; "                                 \
		"setenv bootargs; "                                           \
		"run netargs;  "                                              \
		"run set_getcmd; "                                            \
		"${get_cmd} ${loadaddr} ${image}; "                           \
		"if ${get_cmd} ${fdt_addr} ${fdt_file}; then "                \
			"run boot_os; "                                       \
		"else "                                                       \
			"echo WARN: Cannot load the DT; "                     \
		"fi; "                                                        \
		"fi;\0"                                                       \
	"set_getcmd=if test \"${ip_dyn}\" = yes; then "                       \
			"setenv get_cmd dhcp; "                               \
		"else "                                                       \
			"setenv get_cmd tftp; "                               \
		"fi; \0"                                                      \
	"rootfsmode=rw\0"                                                     \
	"update_kernel_mmc=run set_getcmd; "                                  \
		"if ${get_cmd} ${image}; then "                               \
			"if itest ${filesize} > 0; then "                     \
				"echo Write to mmc ${mmcdev}:${mmcpart}...; " \
				"mmc dev ${mmcdev}; mmc rescan;"              \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "  \
					"${mmcpath}${image} ${filesize}; "    \
			"fi; "                                                \
		"fi; "                                                        \
		"setenv filesize; setenv get_cmd \0"                          \
	"update_fdt_mmc=run set_getcmd; "                                     \
		"if ${get_cmd} ${fdt_file}; then "                            \
			"if itest ${filesize} > 0; then "                     \
				"echo Write to mmc ${mmcdev}:${mmcpart}...; " \
				"mmc dev ${mmcdev}; mmc rescan;"              \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "  \
					"${mmcpath}${fdt_file} ${filesize}; " \
			"fi; "                                                \
		"fi; "                                                        \
		"setenv filesize; setenv get_cmd \0"                          \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0" \
	"netargs=run addnfs addip addtty addearlycon\0"                       \
	"addnfs=setenv bootargs ${bootargs} "                                 \
		"root=/dev/nfs rw "                                           \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"                   \
	"netdev=eth0\0"                                                       \
	"rootpath=/srv/nfs\0"                                                 \
	"ipmode=static\0"                                                     \
	"addearlycon=setenv bootargs ${bootargs} earlycon\0"                  \
	"addmmc=setenv bootargs ${bootargs} "                                 \
		"root=/dev/mmcblk${mmcblkdev}p${mmcrootpart} ${rootfsmode} "  \
		"rootwait\0"                                                  \
	"addip_static=setenv bootargs ${bootargs} "                           \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"           \
		"${hostname}:${netdev}:off\0"                                 \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                 \
	"addip=if test \"${ipmode}\" != static; then "                        \
		"run addip_dynamic; else run addip_static; fi\0"

#endif /* __TQMARZG2X_MBARZG2X_H */
