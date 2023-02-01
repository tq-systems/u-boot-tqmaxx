/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA93XX_H
#define __TQMA93XX_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include "imx_env.h"

#define CONFIG_SYS_BOOTM_LEN		(SZ_64M)
#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x20519dd0
#define CONFIG_SPL_BSS_START_ADDR	0x2051a000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K	/* 8 KB */
/* Need disable simple malloc where still uses malloc_f area */
#define CONFIG_SYS_SPL_MALLOC_START	0x83200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 KB */

/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif

#define CONFIG_MFG_ENV_SETTINGS                                        \
	CONFIG_MFG_ENV_SETTINGS_DEFAULT                                \
	"initrd_addr=0x83800000\0"                                     \
	"emmc_dev=0\0"                                                 \
	"sd_dev=1\0"

/* Initial environment variables */
#define TQMA93XX_MODULE_ENV_SETTINGS		\
	"scriptaddr=0x83500000\0" \
	"image=Image\0" \
	"splashimage=0x90000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdt_addr_r=0x83000000\0"			\
	"boot_fit=no\0"                                                \
	"initrd_addr=0x83800000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0"             \
	"mmcpart=1\0"                                                  \
	"mmcpath=/\0" \
	"mmcautodetect=yes\0" \
	"loadbootscript=mmc dev ${mmcdev}; mmc rescan;" \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${mmcpath}${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=mmc dev ${mmcdev}; mmc rescan;" \
		"load mmc ${mmcdev}:${mmcpart} ${kernel_addr_r} " \
			"${mmcpath}${image}\0" \
	"loadfdt=mmc dev ${mmcdev}; mmc rescan;" \
		"load mmc ${mmcdev}:${mmcpart} ${fdt_addr_r} " \
			"${mmcpath}${fdt_file}\0" \
	"boot_os=booti ${kernel_addr_r} - ${fdt_addr_r};\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"setenv bootargs; " \
		"run mmcargs; " \
		"if test ${boot_fit} = yes || test ${boot_fit} = try; then " \
			"bootm ${loadaddr}; " \
		"else " \
			"if run loadfdt; then " \
				"if run loadimage; then " \
					"run boot_os; " \
				"else " \
					"echo WARN: Cannot load the kernel; " \
				"fi; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"fi;\0" \
	"netboot=echo Booting from net ...; " \
		"setenv bootargs; " \
		"run netargs;  " \
		"run set_getcmd; " \
		"${get_cmd} ${kernel_addr_r} ${image}; " \
		"if test ${boot_fit} = yes || test ${boot_fit} = try; then " \
			"bootm ${loadaddr}; " \
		"else " \
			"if ${get_cmd} ${fdt_addr_r} ${fdt_file}; then " \
				"run boot_os; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"fi;\0" \
	"update_kernel_mmc=run set_getcmd; "                                   \
		"if ${get_cmd} ${image}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"echo Write to mmc ${mmcdev}:${mmcpart}...; "  \
				"mmc dev ${mmcdev}; "                  \
				"mmc rescan;"                          \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "   \
					"${mmcpath}${image} ${filesize}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv get_cmd \0"                           \
	"update_fdt_mmc=run set_getcmd; "                                      \
		"if ${get_cmd} ${fdt_file}; then "                             \
			"if itest ${filesize} > 0; then "                      \
				"echo Write to mmc ${mmcdev}:${mmcpart}...; "  \
				"mmc dev ${mmcdev}; "                  \
				"mmc rescan;"                          \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "\
					"${mmcpath}${fdt_file} ${filesize}; "  \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv get_cmd \0"                           \
	"uboot_mmc_start=0x40\0"                                               \
	"uboot_mmc_size=0xfc0\0"                                               \
	"uboot_fspi_start=0x0\0"                                               \
	"uboot_fspi_size=0x400000\0"                                           \
	"uboot=bootstream.bin\0"                                               \
	"update_uboot_mmc=run set_getcmd; if ${get_cmd} ${uboot}; then "       \
		"if itest ${filesize} > 0; then "                              \
			"echo Write u-boot image to mmc ${mmcdev} ...; "       \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} + 0x1ff; "                   \
			"setexpr blkc ${blkc} / 0x200; "                       \
			"if itest ${blkc} <= ${uboot_mmc_size}; then "         \
				"mmc write ${loadaddr} ${uboot_mmc_start} "    \
					"${blkc}; "                            \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \
	"set_getcmd=if test \"${ip_dyn}\" = yes; then "                        \
			"setenv get_cmd dhcp; "                                \
		"else "                                                        \
			"setenv get_cmd tftp; "                                \
		"fi; \0"                                                       \
	"rootfsmode=ro\0"                                                      \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
	"mmcargs=run addtty addearlycon addmmc\0"                              \
	"mmcrootpart=2\0"                                                      \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcrootpart} ${rootfsmode} "   \
		"rootwait\0"                                                   \
	"netargs=run addnfs addip addtty addearlycon\0"                        \
	"addnfs=setenv bootargs ${bootargs} "                                  \
		"root=/dev/nfs rw "                                            \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"                    \
	"rootpath=/srv/nfs\0"                                                  \
	"ipmode=static\0"                                                      \
	"addip_static=setenv bootargs ${bootargs} "                            \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"            \
		"${hostname}:${netdev}:off\0"                                  \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                  \
	"addip=if test \"${ipmode}\" != static; then "                         \
		"run addip_dynamic; else run addip_static; fi\0"

#if !defined(CONFIG_BOOTCOMMAND)
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; "                                          \
	"if mmc rescan; then "                                         \
		"if run loadbootscript; then "                         \
			"run bootscript; "                             \
		 "else "                                               \
			"if run loadimage; then "                      \
				"run mmcboot; "                        \
			"else "                                        \
				"run netboot; "                        \
			"fi; "                                         \
		 "fi; "                                                \
	"else "                                                        \
		"run boot_os; "                                        \
	"fi"
#endif

/* Link Definitions */
#if defined(CONFIG_BLOBLIST_SIZE)
#define INIT_RAM_OFFSET			(CONFIG_BLOBLIST_SIZE)
#else
#define INIT_RAM_OFFSET			0x0
#endif
#define CONFIG_SYS_INIT_RAM_ADDR	(0x80000000 + INIT_RAM_OFFSET)
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			SZ_1G /* 1GB DDR */

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_IMX_BOOTAUX

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG3_BASE_ADDR

#define CONFIG_SYS_I2C_SPEED		100000

#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

#if defined(CONFIG_TQMa93XX_BB_MBA93XXCA)
#include "tqma93xx-mba93xxca.h"
#else
#error
#endif

#if defined(CONFIG_ETHPRIME)
#define NETDEV_ENV "netdev=" CONFIG_ETHPRIME "\0"
#else
#define NETDEV_ENV "netdev=eth0\0"
#endif

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)                                              \
	func(USB, usb, 0)

#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/*
 * TODO: add TQMA8_SHARED_ENV_SETTINGS
 * #include "tqma8-shared-env.h"
 */

#define CONFIG_EXTRA_ENV_SETTINGS                                      \
	TQMA93XX_MODULE_ENV_SETTINGS                                   \
	BB_ENV_SETTINGS                                                \
	CONFIG_MFG_ENV_SETTINGS                                        \
	BOOTENV                                                        \
	NETDEV_ENV                                                     \
	AHAB_ENV

#endif /* __TQMA93XX_H */
