/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 TQ-Systems GmbH
 */

#ifndef __TQMLS1088A_H__
#define __TQMLS1088A_H__

#include "ls1088a_common.h"

/*
#define DEBUG
*/

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000
#define COUNTER_FREQUENCY_REAL  25000000
#define COUNTER_FREQUENCY       25000000

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#define CONFIG_DIMM_SLOTS_PER_CTLR		1
/* Physical Memory Map */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4
#define CONFIG_NR_DRAM_BANKS			2

#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE           0xdeadbeef
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */

#if defined(CONFIG_QSPI_BOOT)
#define CONFIG_SPL_PAD_TO       0x10000
#endif

#ifndef SPL_NO_IFC
/* IFC */
#define CONFIG_FSL_IFC
#endif

#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS        5000

/* VID */
#define CONFIG_VID_FLS_ENV      "tqmls1088a_vdd_mv"
#define CONFIG_VID
#define VDD_MV_MIN              819
#define VDD_MV_MAX              1212

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

/* EEPROM */
#define CONFIG_SYS_EEPROM_BUS_NUM				0
#define CONFIG_SYS_I2C_EEPROM_ADDR				0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS		3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/* PMIC */
#define CONFIG_POWER
#ifdef CONFIG_POWER
#define CONFIG_POWER_I2C
#endif

#define CONFIG_FSL_MEMAC

/* QSPI device */
#ifndef SPL_NO_QSPI
#ifdef CONFIG_FSL_QSPI
#define CONFIG_SPI_FLASH_MACRONIX
#define FSL_QSPI_FLASH_SIZE			(1 << 26)
#define FSL_QSPI_FLASH_NUM			2
#define CONFIG_SPI_FLASH_BAR
#endif
#endif

#if defined(CONFIG_SD_BOOT)
#define CONFIG_SYS_MMC_ENV_DEV      0
#define CONFIG_ENV_OFFSET           (3 * 1024 * 1024)
#define CONFIG_ENV_SIZE             0x4000
#else
#define CONFIG_ENV_SIZE             0x4000  /* 16KB */
#define CONFIG_ENV_OFFSET           0xd0000 /* 832KB */
#define CONFIG_ENV_SECT_SIZE        0x10000 /* 64KB */
#endif

/*
 * Environment
 */
#ifndef SPL_NO_ENV
#define CONFIG_ENV_OVERWRITE
#if defined(CONFIG_QSPI_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=sf probe 0:0; "	\
	"sf read 0x80000000 0x100000 0x1f0000;"	\
	"sf read 0x80100000 0x0f0000 0x10000;"				\
	"fsl_mc start mc 0x80000000 0x80100000\0"	\
	"mcmemsize=0x20000000\0"
#elif defined(CONFIG_SD_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=mmc rescan; "	\
	"mmc read 0x80000000 0x5000 0x1800;"		\
	"mmc read 0x80100000 0x7000 0x800;"				\
	"fsl_mc start mc 0x80000000 0x80100000\0"	\
	"mcmemsize=0x20000000\0"
#endif

#ifndef SPL_NO_MISC
#undef CONFIG_BOOTCOMMAND
#if defined(CONFIG_QSPI_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run spiboot; run netboot; "	\
							"run panicboot"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run mmcboot; run netboot; "	\
							"run panicboot"
#endif

/* eMMC/SD partitioning */
#define TQMLS1088A_UBOOT_MMC_SECT_START		0x800
#define TQMLS1088A_UBOOT_MMC_SECT_SIZE		0x1000

#ifndef CONFIG_SPL_BUILD
#undef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(SCSI, scsi, 0) \
	func(USB, usb, 0)
#include <config_distro_bootcmd.h>
#endif

#undef CONFIG_EXTRA_ENV_SETTINGS
/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"addmisc=setenv bootargs ${bootargs}\0"	\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"	\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"	\
	"consdev=ttyS1\0"	\
	"addtty=setenv bootargs ${bootargs} earlycon=uart8250,mmio,0x21c0600 "	\
		"console=${consdev},${baudrate}\0"	\
	"mmcblkdev=0\0"	\
	"mmcpart=2\0"	\
	"addmmc=setenv bootargs ${bootargs} "	\
		"root=/dev/mmcblk${mmcblkdev}p${mmcpart} rw rootwait\0"	\
	"mmcargs=run addmmc addtty addmisc\0"	\
	"mmcdev=0\0"	\
	"firmwarepart=1\0"	\
	"loadaddr=0x81000000\0"	\
	"kernel=linuximage\0"	\
	"fdt_addr=0x90000000\0"	\
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"	\
	"loadimage=load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} \0"	\
	"loadfdt=load mmc ${mmcdev}:${firmwarepart} ${fdt_addr} ${fdt_file} \0" \
	"mcdplapply=mmc read 0x80200000 0x6800 0x800 && fsl_mc lazyapply dpl 0x80200000 \0"	\
	MC_INIT_CMD \
	BOOTENV	\
	"mmcboot=echo Booting from mmc ...; "	\
		"setenv bootargs; "	\
		"run mmcargs; "	\
		"run loadimage; "	\
		"run loadfdt; "	\
		"run mcdplapply; "	\
		"booti ${loadaddr} - ${fdt_addr};\0"	\
	"addspi=setenv bootargs ${bootargs} "   \
		"root=ubi0:root rw rootfstype=ubifs ubi.mtd=8\0"	\
	"spiargs=run addspi addtty addmisc\0"	\
	"loadspiimage=sf probe 0; sf read ${loadaddr} kernel\0"	\
	"loadspifdt=sf probe 0; sf read ${fdt_addr} dtb\0"	\
	"mcdplspiapply=sf probe 0; sf read 0x80200000 dpaadpl && fsl_mc lazyapply dpl 0x80200000 \0"	\
	"spiboot=echo Booting from SPI NOR flash ...;"	\
		"setenv bootargs; "	\
		"run spiargs; "	\
		"run loadspiimage; "	\
		"run loadspifdt; "	\
		"run mcdplspiapply; "	\
		"booti ${loadaddr} - ${fdt_addr};\0"	\
	"rootpath=/srv/nfs/tqmls1088a\0"	\
	"addnfs=setenv bootargs ${bootargs} "	\
		"root=/dev/nfs rw "	\
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"	\
	"netdev=eth0\0"	\
	"addip_static=setenv bootargs ${bootargs} "	\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"${hostname}:${netdev}:off\0"	\
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"	\
	"ipmode=static\0"	\
	"addip=if test \"${ipmode}\" != static; then "	\
		"run addip_dynamic; else run addip_static; fi\0"	\
	"set_getcmd=if test \"${ipmode}\" != static; then "	\
		"setenv getcmd dhcp; setenv autoload yes; "	\
		"else setenv getcmd tftp; setenv autoload no; fi\0"	\
	"netargs=run addnfs addip addtty addmisc\0"	\
	"netboot=echo Booting from net ...; "	\
		"run set_getcmd; "	\
		"setenv bootargs; "	\
		"run netargs; "	\
		"${getcmd} 0x80200000 mc-dpl.dtb && fsl_mc lazyapply dpl 0x80200000; "	\
		"if ${getcmd} ${kernel}; then "	\
			"if ${getcmd} ${fdt_addr} ${fdt_file}; then "	\
				"booti ${loadaddr} - ${fdt_addr}; "    \
			"fi; "	\
		"fi; "	\
		"echo ... failed\0"	\
	"panicboot=echo No boot device !!! reset\0"	\
	"uboot_mmc=u-boot-with-spl.bin\0"	\
	"uboot_mmc_start="__stringify(TQMLS1088A_UBOOT_MMC_SECT_START)"\0"	\
	"uboot_mmc_size="__stringify(TQMLS1088A_UBOOT_MMC_SECT_SIZE)"\0"	\
	"uboot_qspi=u-boot-dtb.bin\0"	\
	"update_uboot_mmc=run set_getcmd; "	\
		"if ${getcmd} ${uboot_mmc}; then "	\
			"if itest ${filesize} > 0; then "	\
				"mmc dev ${mmcdev}; mmc rescan; "	\
				"setexpr blkc ${filesize} + 0x1ff; "	\
				"setexpr blkc ${blkc} / 0x200; "	\
				"if itest ${blkc} <= ${uboot_mmc_size}; then "	\
					"mmc write ${loadaddr} ${uboot_mmc_start} ${blkc}; "	\
				"fi; "	\
			"fi; "	\
		"fi;\0"	\
	"update_uboot_qspi=run set_getcmd; "	\
		"if ${getcmd} ${uboot_qspi}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} uboot ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"rcw_mmc=rcw-emmcboot.bin\0"	\
	"rcw_mmc_start=0x8\0"	\
	"rcw_mmc_size=0x7f8\0"	\
	"update_rcw_mmc=run set_getcmd; "	\
		"if ${getcmd} ${rcw_mmc}; then "	\
			"if itest ${filesize} > 0; then "	\
				"mmc dev ${mmcdev}; mmc rescan; "	\
				"setexpr blkc ${filesize} + 0x1ff; "	\
				"setexpr blkc ${blkc} / 0x200; "	\
				"if itest ${blkc} <= ${rcw_mmc_size}; then "	\
					"mmc write ${loadaddr} ${rcw_mmc_start} ${blkc}; "	\
				"fi; "	\
			"fi; "	\
		"fi;\0"	\
	"rcw_qspi=rcw-qspiboot.bin\0"	\
	"update_rcw_qspi=run set_getcmd; "	\
		"if ${getcmd} ${rcw_qspi}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} rcwpbi ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"update_kernel_mmc=run set_getcmd; "   \
		"if ${getcmd} ${kernel}; then " \
			"if itest ${filesize} > 0; then "   \
				"mmc dev ${mmcdev}; mmc rescan; "   \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; "	\
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} ${filesize}; "	\
			"fi; "  \
		"fi;\0" \
	"update_kernel_qspi=run set_getcmd; "	\
		"if ${getcmd} ${kernel}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} kernel ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"update_fdt_mmc=run set_getcmd; "	\
		"if ${getcmd} ${fdt_file}; then " \
			"if itest ${filesize} > 0; then "   \
				"mmc dev ${mmcdev}; mmc rescan; "   \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; "	\
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${fdt_file} ${filesize}; "	\
			"fi; "  \
		"fi;\0" \
	"update_fdt_qspi=run set_getcmd; "	\
		"if ${getcmd} ${fdt_file}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} dtb ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"rootfs-qspi=root.ubi\0"	\
	"init_rootfs_qspi=run set_getcmd; "	\
		"if ${getcmd} ${rootfs-qspi}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf erase rootfs 2800000; "	\
				"sf write ${loadaddr} rootfs ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	""
#endif /* !SPL_NO_MISC */
#endif /* !SPL_NO_ENV */

#include <asm/fsl_secure_boot.h>

#include "tqmls1088a_baseboard.h"

#endif /* __TQMLS1088A_H__ */
