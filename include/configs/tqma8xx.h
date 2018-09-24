/*
 * Copyright 2018 TQ Systemss GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMA8XX_H
#define __TQMA8XX_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_REMAKE_ELF

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ARCH_MISC_INIT

/* Flat Device Tree Definitions */
/* #define CONFIG_OF_BOARD_SETUP */

#undef CONFIG_CMD_EXPORTENV
#undef CONFIG_CMD_IMPORTENV
#undef CONFIG_CMD_IMLS

#undef CONFIG_CMD_CRC32
#undef CONFIG_BOOTM_NETBSD

#define CONFIG_ENV_OVERWRITE


#define CONFIG_FSL_HSIO
#ifdef CONFIG_FSL_HSIO
#
#endif

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* FUSE command */
#define CONFIG_CMD_FUSE

/* GPIO configs */
#define CONFIG_MXC_GPIO

#define XEN_ENV \
	"xen_addr=0x80200000\0" \
	"xen_file=xen\0" \
	"xenargs=setenv bootargs console=dtuart dtuart=/serial@5a060000 dom0_mem=1024M \0" \
	"loadxen=fatload mmc ${mmcdev}:${mmcpart} ${xen_addr} ${xen_file}\0" \
	"xenboot=setenv loadaddr 0x80a00000; setenv fdt_file fsl-imx8qxp-mek-dom0.dtb; "\
	"setenv bootargs console=dtuart dtuart=/serial@5a060000 dom0_mem=1024M; " \
	"run loadfdt; run loadxen; run loadimage; fdt addr ${fdt_addr}; "\
	"fdt set /chosen/module@0 reg <0x00000000 ${loadaddr} 0x00000000 0x${filesize}>; " \
	"booti ${xen_addr} - ${fdt_addr} \0" \

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	XEN_ENV \
	"script=boot.scr\0" \
	"image=Image\0" \
	"panel=NULL\0" \
	"console=ttyLP0,115200 earlycon=lpuart32,0x5a060000,115200\0" \
	"fdt_addr=0x83000000\0"			\
	"fdt_high=0xffffffffffffffff\0"		\
	"boot_fdt=try\0" \
	"fdt_file=fsl-imx8qxp-mek.dtb\0" \
	"initrd_addr=0x83800000\0"		\
	"initrd_high=0xffffffffffffffff\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} root=${mmcroot} " \
	"video=imxdpufb5:off video=imxdpufb6:off video=imxdpufb7:off\0" \
	"loadbootscript=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"echo wait for boot; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp " \
		"video=imxdpufb5:off video=imxdpufb6:off video=imxdpufb7:off\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs;  " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"else " \
			"booti; " \
		"fi;\0"

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

/* Link Definitions */
#define CONFIG_LOADADDR			0x80280000
#define CONFIG_SYS_TEXT_BASE		0x80020000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_SYS_INIT_SP_ADDR         0x80200000


/* Default environment is in SD */
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_IS_NOWHERE

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		((CONFIG_ENV_SIZE + (32*1024)) * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_NR_DRAM_BANKS		3
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
#define PHYS_SDRAM_1_SIZE		0x80000000	/* 2 GB */
/* LPDDR4 board total DDR is 3GB */
#define PHYS_SDRAM_2_SIZE		0x40000000	/* 1 GB */

/* Serial */
#define CONFIG_BAUDRATE			115200

/* Monitor Command Prompt */
#define CONFIG_SYS_LONGHELP
/* #define CONFIG_HUSH_PARSER */
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE              1024
#define CONFIG_SYS_MAXARGS             64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_CMDLINE_EDITING

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		8000000	/* 8MHz */

#define CONFIG_IMX_SMMU


/* #define CONFIG_OF_SYSTEM_SETUP */

#define BOOTAUX_RESERVED_MEM_BASE 0x88000000
#define BOOTAUX_RESERVED_MEM_SIZE 0x08000000 /* Reserve from second 128MB */

#endif /* __TQMA8XX_H */
