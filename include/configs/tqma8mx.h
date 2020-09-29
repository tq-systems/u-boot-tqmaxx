/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 - 2020 TQ Systems GmbH
 */

#ifndef __TQMA8MX_H
#define __TQMA8MX_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>
#include "imx_env.h"

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CSF_SIZE			0x2000 /* 8K region */
#endif

#define CONFIG_SPL_TEXT_BASE		0x7E1000
#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#if defined(CONFIG_SD_BOOT)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#else
#error
#endif

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/
#define CONFIG_SPL_WATCHDOG_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x187FF0
#define CONFIG_SPL_BSS_START_ADDR	0x00180000
#define CONFIG_SPL_BSS_MAX_SIZE		0x2000	/* 8 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000	/* 512 KB */
#define CONFIG_SYS_SPL_PTE_RAM_BASE	0x41580000
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x182000

/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#undef CONFIG_DM_MMC
#undef CONFIG_BLK
#undef CONFIG_DM_PMIC
#undef CONFIG_DM_PMIC_PFUZE100

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR 0x08

#else

/*
 * TODO: DM stuff here to prevent SPL messing with missing stuff
 * - pmic support
 * - gpio expander support via dt
 * - i2c eeprom support via dt
 */

#endif

#define CONFIG_REMAKE_ELF

#undef CONFIG_CMD_EXPORTENV
#undef CONFIG_CMD_IMPORTENV
#undef CONFIG_CMD_IMLS

#undef CONFIG_BOOTM_NETBSD

#define CONFIG_MFG_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS_DEFAULT \
	"initrd_addr=0x43800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"emmc_dev=0\0"\
	"sd_dev=1\0" \

/* Initial environment variables */
#define TQMA8MX_MODULE_ENV_SETTINGS		\
	CONFIG_MFG_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=Image\0" \
	"fdt_addr=0x43000000\0"			\
	"fdt_high=0xffffffffffffffff\0"		\
	"boot_fdt=try\0" \
	"initrd_addr=0x43800000\0"		\
	"initrd_high=0xffffffffffffffff\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcpath=/\0" \
	"mmcautodetect=yes\0" \
	"loadbootscript=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${mmcpath}${image}\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} " \
		"${mmcpath}${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"setenv bootargs; " \
		"run mmcargs; " \
		"run loadimage; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"echo wait for boot; " \
		"fi;\0" \
	"netboot=echo Booting from net ...; " \
		"setenv bootargs; " \
		"run netargs;  " \
		"run set_getcmd; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"booti; " \
		"fi;\0" \
	"update_kernel=run set_getcmd; "                                       \
		"if ${get_cmd} ${image}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"echo Write kernel image to mmc ${mmcdev}:${mmcpart}...; " \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "   \
					"${mmcpath}${image} ${filesize}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv get_cmd \0"                           \
	"update_fdt=run set_getcmd; "                                          \
		"if ${get_cmd} ${fdt_file}; then "                             \
			"if itest ${filesize} > 0; then "                      \
				"echo Write fdt image to mmc ${mmcdev}:${mmcpart}...; " \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "   \
					"${mmcpath}${fdt_file} ${filesize}; "  \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv get_cmd \0"                           \
	"uboot_start=0x42\0"                                                   \
	"uboot_size=0xfbe\0"                                                   \
	"uboot=bootstream.bin\0"                                               \
	"update_uboot=run set_getcmd; if ${get_cmd} ${uboot}; then "           \
		"if itest ${filesize} > 0; then "                              \
			"echo Write u-boot image to mmc ${mmcdev} ...; "       \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} + 0x1ff; "                   \
			"setexpr blkc ${blkc} / 0x200; "                       \
			"if itest ${blkc} <= ${uboot_size}; then "             \
				"mmc write ${loadaddr} ${uboot_start} "        \
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
	"addtty=setenv bootargs ${bootargs} console=${console}\0"              \
	"mmcrootpart=2\0"                                                      \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcrootpart} ${rootfsmode} "   \
		"rootwait\0"                                                   \
	"mmcargs=run addtty addmmc\0"                                          \
	"netargs=run addnfs addip addtty\0"                                    \
	"addnfs=setenv bootargs ${bootargs} "                                  \
		"root=/dev/nfs rw "                                            \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"                    \
	"rootpath=/srv/nfs\0"                                                  \
	"netdev=eth0\0"                                                        \
	"ipmode=static\0"                                                      \
	"addip_static=setenv bootargs ${bootargs} "                            \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"            \
		"${hostname}:${netdev}:off\0"                                  \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                  \
	"addip=if test \"${ipmode}\" != static; then "                         \
		"run addip_dynamic; else run addip_static; fi\0"

#if !defined(CONFIG_BOOTCOMMAND)
#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "else run netboot; " \
			   "fi; " \
		   "fi; " \
	   "else booti ${loadaddr} - ${fdt_addr}; fi"
#endif

/* Link Definitions */
#define CONFIG_LOADADDR			0x40480000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x80000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#if defined(CONFIG_SD_BOOT)
#define CONFIG_ENV_OFFSET		(64 * SZ_64K)
#define CONFIG_ENV_SIZE			SZ_16K
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_SYS_MMC_ENV_DEV		-1   /* invalid */
#elif defined(CONFIG_QSPI_BOOT)
#error
#else
#error
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		((CONFIG_ENV_SIZE + (2*1024) + (16*1024)) * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000

#if defined(CONFIG_TQMA8MX_RAM_4G)
#define PHYS_SDRAM_SIZE			0xc0000000ull /* 3GB LPDDR4 */
#define PHYS_SDRAM_2			0x100000000ull
#define PHYS_SDRAM_2_SIZE		0x40000000ull /* 1GB */
#elif defined(CONFIG_TQMA8MX_RAM_2G)
#define PHYS_SDRAM_SIZE			0x80000000ull /* 2GB LPDDR4 */
#elif defined (CONFIG_TQMA8MX_RAM_1G)
#define PHYS_SDRAM_SIZE			0x40000000ull /* 1GB LPDDR4 */
#else
#error
#endif

#if defined(CONFIG_CMD_MEMTEST)
/*
 * Use alternative / extended memtest,
 * leave 128 MiB free at start
 * U-Boot is loaded to 0x40200000 (offset 2 MiB)
 * and relocated at end of configured RAM
 * for total ram size > 0xc0000000 we limit to the first
 * bank (0x40000000ull ... 0x100000000ull) since this it what we can use in
 * U-Boot without hassle. See get_effective_memsize for this board.
 */
#if defined(CONFIG_SYS_ALT_MEMTEST)
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + SZ_128M)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (PHYS_SDRAM_SIZE / 4) * 3)
#define CONFIG_SYS_MEMTEST_SCRATCH CONFIG_SYS_MEMTEST_END

#endif /* CONFIG_SYS_ALT_MEMTEST */

#endif /* CONFIG_CMD_MEMTEST */

#define CONFIG_BAUDRATE			115200

/* Monitor Command Prompt */
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"u-boot=> "
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#if defined(CONFIG_IMX_BOOTAUX)

#define TQMA8MX_CM4_ENV_SETTINGS						\
	"m4_image=test.bin\0"							\
	"m4_loadaddr=0x7e0000\0"						\
	"load_m4=load mmc ${mmcdev}:${mmcpart} ${m4_loadaddr} ${m4_image}\0"	\
	"boot_m4=run load_m4; bootaux ${m4_loadaddr}\0"				\
	"update_m4=run set_getcmd; "                                            \
		"if ${get_cmd} ${m4_image}; then "                              \
			"if itest ${filesize} > 0; then "                       \
				"echo Write M4 image to mmc ${mmcdev}:${mmcpart}...; " \
				"save mmc ${mmcdev}:${mmcpart} ${loadaddr} "   \
					"${m4_image} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv get_cmd \0"                           \

#else

#define TQMA8MX_CM4_ENV_SETTINGS

#endif

#if defined(CONFIG_FSL_ESDHC)

#define CONFIG_FSL_USDHC

#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#endif

#define CONFIG_SUPPORT_EMMC_BOOT	/* eMMC specific */
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#ifdef CONFIG_FSL_QSPI

#define FSL_QSPI_FLASH_SIZE		(SZ_64M)
#define FSL_QSPI_FLASH_NUM		1

/* needed for UBI support */
/* #define CONFIG_CMD_MTDPARTS */
/* #define CONFIG_MTD_PARTITIONS */
#define CONFIG_MTD_DEVICE

#endif

#define CONFIG_MXC_OCOTP

#ifndef CONFIG_SPL_BUILD

#endif

/* USB configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2

#define CONFIG_USBD_HS

#if defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_SPL_USB_GADGET)
#undef CONFIG_SPL__DM_USB_GADGET
#undef CONFIG_DM_USB_GADGET
#define CONFIG_USB_DWC3_GADGET
#undef CONFIG_USB_DWC3_HOST
#endif /* CONFIG_SPL_USB_GADGET */

#endif

#if defined(CONFIG_TQMA8MX_BB_MBA8MX)
#include "tqma8mx-mba8mx.h"
#else
#error
#endif

#define CONFIG_EXTRA_ENV_SETTINGS		\
	TQMA8MX_CM4_ENV_SETTINGS		\
	TQMA8MX_MODULE_ENV_SETTINGS		\
	BB_ENV_SETTINGS

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0)
#include <config_distro_bootcmd.h>
#endif

#endif
