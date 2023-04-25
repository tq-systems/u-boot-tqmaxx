/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 *
 * Configuration settings for the TQ-Systems TQMa6UL[L]x[L] SOM family.
 */

#ifndef __TQMA6UL_CONFIG_H
#define __TQMA6UL_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <linux/stringify.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

/* 128 MiB offset as suggested in ARM related Linux docs */
#define TQMA6UL_FDT_ADDRESS		0x88000000

/* 16MiB above TQMA6UL_FDT_ADDRESS */
#define TQMA6UL_INITRD_ADDRESS		0x89000000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#ifndef MTDIDS_DEFAULT
# define MTDIDS_DEFAULT "nor0=nor0\0"
#endif

#ifndef MTDPARTS_DEFAULT
# define MTDPARTS_DEFAULT \
	"mtdparts=nor0:" \
		"832k@0k(U-Boot)," \
		"64k@832k(ENV1)," \
		"64k@896k(ENV2)," \
		"64k@960k(DTB)," \
		"7M@1M(Linux)," \
		"56M@8M(RootFS)"
#endif

#if defined(CONFIG_TQMA6UL_MMC_BOOT)

#define TQMA6UL_UBOOT_SECTOR_START	0x2
#define TQMA6UL_UBOOT_SECTOR_COUNT	0x7fe

#define TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS \
	"uboot_start=" __stringify(TQMA6UL_UBOOT_SECTOR_START) "\0" \
	"uboot_size=" __stringify(TQMA6UL_UBOOT_SECTOR_COUNT) "\0" \
	"firmwarepart=1\0" \
	"loadimage=run kernel_name; " \
		"load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} \0" \
	"loadfdt=" \
		"load mmc ${mmcdev}:${firmwarepart} ${fdt_addr} ${fdt_file} \0" \
	"update_uboot=run set_getcmd; " \
		"if ${getcmd} ${uboot}; then " \
			"if itest ${filesize} > 0; then " \
				"mmc dev ${mmcdev}; mmc rescan; " \
				"setexpr blkc ${filesize} + 0x1ff; " \
				"setexpr blkc ${blkc} / 0x200; " \
				"if itest ${blkc} <= ${uboot_size}; then " \
					"mmc write ${loadaddr} ${uboot_start} " \
						"${blkc}; " \
				"fi; " \
			"fi; " \
		"fi; " \
		"setenv filesize; setenv blkc; setenv getcmd \0" \
	"update_kernel=run kernel_name; run set_getcmd; " \
		"if ${getcmd} ${kernel}; then " \
			"if itest ${filesize} > 0; then " \
				"mmc dev ${mmcdev}; mmc rescan; " \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${kernel} ${filesize}; " \
			"fi; " \
		"fi; " \
		"setenv filesize; setenv getcmd \0" \
	"update_fdt=run set_getcmd; " \
		"if ${getcmd} ${fdt_file}; then " \
			"if itest ${filesize} > 0; then " \
				"mmc dev ${mmcdev}; mmc rescan; " \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; " \
			"fi; " \
		"fi; " \
		"setenv filesize; setenv getcmd \0" \
	""

#elif defined(CONFIG_TQMA6UL_QSPI_BOOT)

#define TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS \
	"update_uboot=run set_getcmd; if ${getcmd} ${uboot}; then " \
		"if itest ${filesize} > 0; then " \
			"sf probe 0; " \
			"sf update ${loadaddr} ${uboot_mtdpart} ${filesize}; " \
		"fi; fi; " \
		"setenv filesize; setenv getcmd \0" \
	"update_kernel=run kernel_name; run set_getcmd; " \
		"if ${getcmd} ${kernel}; then " \
			"if itest ${filesize} > 0; then " \
				"sf probe 0; " \
				"sf update ${loadaddr} ${kernel_mtdpart} " \
					"${filesize};" \
		"fi; fi; " \
		"setenv filesize; setenv getcmd \0" \
	"update_fdt=run set_getcmd; if ${getcmd} ${fdt_file}; then " \
		"if itest ${filesize} > 0; then " \
			"sf probe 0; " \
			"sf update ${loadaddr} ${fdt_mtdpart} ${filesize}; " \
		"fi; fi; " \
		"setenv filesize; setenv getcmd \0" \
	"loadimage=sf probe 0; sf read ${loadaddr} ${kernel_mtdpart}\0" \
	"loadfdt=sf probe 0; sf read ${fdt_addr} ${fdt_mtdpart}\0" \

#else

#error "need to define boot source"

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"board=tqma6ul\0" \
	"zimage=linuximage\0" \
	"boot_type=bootz\0" \
	"kernel_name=setenv kernel ${zimage}\0" \
	"uboot=u-boot.imx\0" \
	"fdt_addr=" __stringify(TQMA6UL_FDT_ADDRESS) "\0" \
	"console=" CONSOLE_DEV "\0" \
	"fdt_high=0x90000000\0" \
	"initrd_high=0x90000000\0" \
	"rootfsmode=ro\0" \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate} " \
		"consoleblank=0\0" \
	"mmcautodetect=yes\0" \
	"mmcdev=-1\0" \
	"mmcpart=2\0" \
	"mmcargs=run addmmc addtty\0" \
	"addmmc=setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk${mmcdev}p${mmcpart} ${rootfsmode} " \
		"rootwait\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"setenv bootargs; " \
		"run mmcargs; " \
		"if run loadimage; then " \
			"if run loadfdt; then " \
				"echo boot device tree kernel ...; " \
				"${boot_type} ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"else " \
			"${boot_type}; " \
		"fi;\0" \
		"setenv bootargs \0" \
	"qspiboot=echo Booting from qspi ...; " \
		"setenv bootargs; " \
		"run qspiargs; " \
		"if run loadimage; then " \
			"if run loadfdt; then " \
				"echo boot device tree kernel ...; " \
				"${boot_type} ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"else " \
			"${boot_type}; " \
		"fi;\0" \
		"setenv bootargs \0" \
	"netdev=eth1\0" \
	"rootpath=/srv/nfs/tqma6ul\0" \
	"ipmode=static\0" \
	"netargs=run addnfs addip addtty\0" \
	"addnfs=setenv bootargs ${bootargs} " \
		"root=/dev/nfs rw " \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0" \
	"addip_static=setenv bootargs ${bootargs} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
		"${hostname}:${netdev}:off\0" \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0" \
	"addip=if test \"${ipmode}\" != static; then " \
		"run addip_dynamic; else run addip_static; fi\0" \
	"set_getcmd=if test \"${ipmode}\" != static; then " \
		"setenv getcmd dhcp; setenv autoload yes; " \
		"else setenv getcmd tftp; setenv autoload no; fi\0" \
	"netboot=echo Booting from net ...; " \
		"run kernel_name; " \
		"run set_getcmd; " \
		"setenv bootargs; " \
		"run netargs; " \
		"if ${getcmd} ${fdt_addr} ${fdt_file}; then " \
			"if ${getcmd} ${loadaddr} ${kernel}; then " \
				"${boot_type} ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"fi; " \
		"echo ... failed\0" \
	"panicboot=echo No boot device !!! reset\0" \
	"rootfs_mtddev=5\0" \
	"addqspi=setenv bootargs ${bootargs} root=ubi0:root ${rootfsmode} " \
		"rootfstype=ubifs ubi.mtd=${rootfs_mtddev}\0" \
	"qspiargs=run addqspi addtty\0" \
	"uboot_mtdpart=U-Boot\0" \
	"fdt_mtdpart=DTB\0" \
	"kernel_mtdpart=Linux\0" \
	"rootfs_mtdpart=RootFS\0" \
	TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS \
	BOOT_ENV_BOARD \
	""

#endif /* __TQMA6UL_CONFIG_H */
