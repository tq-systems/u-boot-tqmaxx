/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for TQ-Systems TQMa65xx
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2020-2022 TQ-Systems GmbH
 */

#ifndef __CONFIG_TQMA65XX_H
#define __CONFIG_TQMA65XX_H

#include <linux/sizes.h>

/* DDR Configuration */
#define CONFIG_SYS_SDRAM_BASE1		0x880000000

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END (CONFIG_SYS_SDRAM_BASE + 0x7C000000)

/* SPL Loader Configuration */
#ifdef CONFIG_TARGET_AM654_A53_TQMA65XX
#  define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SPL_TEXT_BASE +	CONFIG_SYS_K3_NON_SECURE_MSRAM_SIZE)
#else
/*
 * Maximum size in memory allocated to the SPL BSS. Keep it as tight as
 * possible (to allow the build to go through), as this directly affects
 * our memory footprint. The less we use for BSS the more we have available
 * for everything else.
 */
#  define CONFIG_SPL_BSS_MAX_SIZE 0x5000
/*
 * Link BSS to be within SPL in a dedicated region located near the top of
 * the MCU SRAM, this way making it available also before relocation. Note
 * that we are not using the actual top of the MCU SRAM as there is a memory
 * location filled in by the boot ROM that we want to read out without any
 * interference from the C context.
 */
#  define CONFIG_SPL_BSS_START_ADDR \
	(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX - CONFIG_SPL_BSS_MAX_SIZE)
/* Set the stack right below the SPL BSS section */
#  define CONFIG_SYS_INIT_SP_ADDR	CONFIG_SPL_BSS_START_ADDR
/* Configure R5 SPL post-relocation malloc pool in DDR */
#  define CONFIG_SYS_SPL_MALLOC_START	0x84000000
#  define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_16M
#endif

#ifdef CONFIG_SYS_K3_SPL_ATF
#  define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"tispl.bin"
#endif

#ifndef CONFIG_CPU_V7R
#  define CONFIG_SKIP_LOWLEVEL_INIT
#endif

#define CONFIG_SPL_MAX_SIZE		CONFIG_SYS_K3_MAX_DOWNLODABLE_IMAGE_SIZE

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

#ifndef PARTS_DEFAULT
#  define PARTS_DEFAULT ""
#endif

/* Override via mmc_get_env_dev() */
#define CONFIG_SYS_MMC_ENV_DEV 0

/* U-Boot general configuration */
#define EXTRA_ENV_TQMA65XX_SETTINGS \
	"findfdt=setenv fdtfile " CONFIG_DEFAULT_FDT_FILE "\0" \
	"loadaddr=0x80080000\0" \
	"ramdisk_addr=-\0" \
	"fdt_addr=0x82000000\0" \
	"overlayaddr=0x83000000\0" \
	"bootdir=/boot\0" \
	"name_kern=Image\0" \
	"stdin=serial\0" \
	"args_reset=setenv bootargs ''\0" \
	"args_extra=true\0" \
	"run_kern=booti ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	""

/* U-Boot MMC-specific configuration */
#define EXTRA_ENV_TQMA65XX_SETTINGS_MMC \
	"firmwarepart=1\0" \
	"bootpart=2\0" \
	"rootpart=2\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"args_mmc=setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk${mmcdev}p${rootpart} rw " \
		"rootfstype=${mmcrootfstype}\0" \
	"init_mmc=run args_reset args_mmc args_board args_extra\0" \
	"get_overlay_mmc=" \
		"fdt address ${fdt_addr}; " \
		"fdt resize 0x100000; " \
		"for overlay in ${name_overlay1} ${name_overlay2} ${name_overlay_extra}; do " \
			"load mmc ${mmcdev}:${bootpart} ${overlayaddr} ${bootdir}/${overlay}; " \
			"fdt apply ${overlayaddr}; " \
		"done\0" \
	"get_fdt_mmc=load mmc ${mmcdev}:${bootpart} ${fdt_addr} ${bootdir}/${fdtfile}\0" \
	"get_kern_mmc=load mmc ${mmcdev}:${bootpart} ${loadaddr} " \
		"${bootdir}/${name_kern}\0" \
	""

#define EXTRA_ENV_TQMA65XX_SETTINGS_UBI \
	"args_ubi=setenv bootargs ${bootargs} " \
		"rootfstype=ubifs root=ubi0:rootfs rw rootwait ubi.mtd=ospi.rootfs\0" \
	"init_ubi=run args_reset args_ubi args_board args_extra; sf probe; " \
		"ubi part ospi.rootfs; ubifsmount ubi:rootfs\0" \
	"get_overlay_ubi=" \
		"fdt address ${fdt_addr}; " \
		"fdt resize 0x100000; " \
		"for overlay in ${name_overlay1} ${name_overlay2} ${name_overlay_extra}; do " \
			"ubifsload ${overlayaddr} ${bootdir}/${overlay}; " \
			"fdt apply ${overlayaddr}; " \
		"done\0" \
	"get_fdt_ubi=ubifsload ${fdt_addr} ${bootdir}/${fdtfile}\0" \
	"get_kern_ubi=ubifsload ${loadaddr} ${bootdir}/${name_kern}\0" \
	""

#define EXTRA_ENV_TQMA65XX_SETTINGS_NET \
	"autoload=no\0" \
	"ipmode=static\0" \
	"rootpath=/srv/nfs/tqma65xx\0" \
	"args_net_static=setenv bootargs ${bootargs} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
		"${hostname}:${netdev}:off\0" \
	"args_net_dynamic=setenv bootargs ${bootargs} ip=dhcp\0" \
	"args_net=setenv bootargs ${bootargs} " \
		"root=/dev/nfs rw nfsroot=${serverip}:${rootpath},v3,tcp; " \
		"run args_net_${ipmode}\0" \
	"init_net_static=true\0" \
	"init_net_dynamic=dhcp\0" \
	"init_net=run init_net_${ipmode} args_reset args_net args_board args_extra\0" \
	"get_overlay_net=" \
		"fdt address ${fdt_addr};" \
		"fdt resize 0x100000;" \
		"for overlay in ${name_overlay1} ${name_overlay2} ${name_overlay_extra}; do " \
			"nfs ${overlayaddr} ${rootpath}${bootdir}/${overlay}; " \
			"fdt apply ${overlayaddr}; " \
		"done\0" \
	"get_fdt_net=nfs ${fdt_addr} ${rootpath}${bootdir}/${fdtfile}\0" \
	"get_kern_net=nfs ${loadaddr} ${rootpath}${bootdir}/${name_kern}\0" \
	""

#define EXTRA_ENV_TQMA65XX_SETTINGS_UPDATE \
	"sysfw=sysfw.itb\0" \
	"tiboot3=tiboot3.bin\0" \
	"tispl=tispl.bin\0" \
	"u-boot=u-boot.img\0" \
	"update_firmware_mmc=mmc dev ${mmcdev}; " \
		"if tftp ${sysfw}; then " \
			"echo updating sysfw on mmc${mmcdev}...; " \
			"fatwrite mmc ${mmcdev}:${firmwarepart} ${loadaddr} sysfw.itb ${filesize}; " \
		"fi; " \
		"setenv filesize\0" \
	"update_uboot_mmc=mmc dev ${mmcdev}; " \
		"if tftp ${tiboot3}; then " \
			"echo updating tiboot3 on mmc${mmcdev}...; " \
			"fatwrite mmc ${mmcdev}:${firmwarepart} ${loadaddr} tiboot3.bin ${filesize}; " \
		"fi; " \
		"if tftp ${tispl}; then " \
			"echo updating tispl on mmc${mmcdev}...; " \
			"fatwrite mmc ${mmcdev}:${firmwarepart} ${loadaddr} tispl.bin ${filesize}; " \
		"fi; " \
		"if tftp ${u-boot}; then " \
			"echo updating u-boot.img on mmc${mmcdev}...; " \
			"fatwrite mmc ${mmcdev}:${firmwarepart} ${loadaddr} u-boot.img ${filesize}; " \
		"fi; " \
		"setenv filesize\0" \
	"update_firmware_spi=sf probe; " \
		"if tftp ${sysfw}; then " \
			"echo updating sysfw on ospi...; " \
			"sf update ${loadaddr} ospi.sysfw ${filesize}; " \
		"fi; " \
		"setenv filesize\0" \
	"update_uboot_spi=sf probe; " \
		"if tftp ${tiboot3}; then " \
			"echo updating tiboot3 on ospi...; " \
			"sf update ${loadaddr} ospi.tiboot3 ${filesize}; " \
		"fi; " \
		"if tftp ${tispl}; then " \
			"echo updating tispl on ospi...; " \
			"sf update ${loadaddr} ospi.tispl ${filesize}; " \
		"fi; " \
		"if tftp ${u-boot}; then " \
			"echo updating u-boot on ospi...; " \
			"sf update ${loadaddr} ospi.u-boot ${filesize}; " \
		"fi; " \
		"setenv filesize\0" \
	""

/* Incorporate settings into the U-Boot environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	EXTRA_ENV_TQMA65XX_SETTINGS \
	EXTRA_ENV_TQMA65XX_SETTINGS_MMC \
	EXTRA_ENV_TQMA65XX_SETTINGS_UBI \
	EXTRA_ENV_TQMA65XX_SETTINGS_NET \
	EXTRA_ENV_TQMA65XX_SETTINGS_UPDATE \
	EXTRA_ENV_TQMA65XX_SETTINGS_BOARD \
	""

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif
