/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 */

#ifndef __TQMLS10XXA_COMMON_H__
#define __TQMLS10XXA_COMMON_H__
#include <config_distro_bootcmd.h>

#define CONFIG_MEM_INIT_VALUE           0xdeadbeef

#define CONFIG_DIMM_SLOTS_PER_CTLR     1
#define CONFIG_CHIP_SELECTS_PER_CTRL   4

/* EEPROM */
#define CONFIG_SYS_EEPROM_BUS_NUM				0
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS		3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/* eMMC/SD partitioning */
#define TQMLS10xxA_PBL_MMC_SECT_START		0x8
#define TQMLS10xxA_PBL_MMC_SECT_SIZE		0x7f8
#define TQMLS10xxA_UBOOT_MMC_SECT_START		0x800
#define TQMLS10xxA_UBOOT_MMC_SECT_SIZE		0x1800
#define TQMLS104xA_FMUCODE_MMC_SECT_START	0x4800
#define TQMLS104xA_FMUCODE_MMC_SECT_SIZE	0x3800

#define CONFIG_SF_DEFAULT_BUS	0

#ifndef CONFIG_SPL_BUILD
#undef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0)
#include <config_distro_bootcmd.h>
#endif

#ifdef CONFIG_TFABOOT
#undef QSPI_NOR_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND "run distro_bootcmd; run spiboot; "
#undef SD_BOOTCOMMAND
#define SD_BOOTCOMMAND "run distro_bootcmd; run mmcboot; "
#endif

#define TQMLS10XXA_COMMON_ENV \
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
	"rootpath=/srv/nfs/\0"		\
	"firmwarepart=1\0"	\
	"firmwarepath=/\0"	\
	"kernel=Image\0"	\
	"kernel_addr_r=0x81000000\0"	\
	"fdt_addr_r=0x90000000\0"	\
	"ubirootfspart=rootfs\0"                                          \
	"ubirootfsvol=root\0"                                        \
	"ubirootfs=root.ubifs\0"	\
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"	\
	"loadimage=load mmc ${mmcdev}:${firmwarepart} ${kernel_addr_r} ${firmwarepath}/${kernel} \0"	\
	"loadfdt=load mmc ${mmcdev}:${firmwarepart} ${fdt_addr_r} ${firmwarepath}/${fdtfile} \0" \
	BOOTENV	\
	"mmcboot=echo Booting from mmc ...; "	\
		"setenv bootargs; "	\
		"run mmcargs; "	\
		"run loadimage; "	\
		"run loadfdt; "	\
		"booti ${kernel_addr_r} - ${fdt_addr_r};\0"	\
	"addspi=setenv bootargs ${bootargs} "   \
		"root=ubi0_0 rw rootfstype=ubifs ubi.mtd=3\0"	\
	"spiargs=run addspi addtty addmisc\0"	\
	"loadspiimage=sf probe 0; ubi part ${ubirootfspart}; " \
		"ubifsmount ubi0:${ubirootfsvol}; "\
		"ubifsload ${kernel_addr_r} /boot/${kernel}; " \
		"ubifsumount; ubi detach\0"	\
	"loadspifdt=sf probe 0; ubi part ${ubirootfspart}; " \
		"ubifsmount ubi0:${ubirootfsvol}; "\
		"ubifsload ${fdt_addr_r} /boot/${fdtfile}; " \
		"ubifsumount; ubi detach\0"	\
	"spiboot=echo Booting from SPI NOR flash ...;"	\
		"setenv bootargs; "	\
		"run spiargs; "	\
		"run loadspiimage; "	\
		"run loadspifdt; "	\
		"booti ${kernel_addr_r} - ${fdt_addr_r};\0"	\
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
		"if ${getcmd} ${kernel}; then "	\
			"if ${getcmd} ${fdt_addr_r} ${fdtfile}; then "	\
				"booti ${kernel_addr_r} - ${fdt_addr_r}; "    \
			"fi; "	\
		"fi; "	\
		"echo ... failed\0"	\
	"panicboot=echo No boot device !!! reset\0"	\
	"uboot=fip_uboot.bin\0"	\
	"uboot_mmc_start=" __stringify(TQMLS10xxA_UBOOT_MMC_SECT_START) "\0"	\
	"uboot_mmc_size=" __stringify(TQMLS10xxA_UBOOT_MMC_SECT_SIZE) "\0"	\
	"pbl_mmc_start=" __stringify(TQMLS10xxA_PBL_MMC_SECT_START) "\0"	\
	"pbl_mmc_size=" __stringify(TQMLS10xxA_PBL_MMC_SECT_SIZE) "\0"	\
	"pbl_mmc=bl2_sd.pbl\0"						\
	"pbl_qspi=bl2_qspi.pbl\0"						\
	"update_pbl_mmc=run set_getcmd; "	\
		"if ${getcmd} ${pbl_mmc}; then "	\
			"if itest ${filesize} > 0; then "	\
				"mmc dev ${mmcdev}; mmc rescan; "	\
				"setexpr blkc ${filesize} + 0x1ff; "	\
				"setexpr blkc ${blkc} / 0x200; "	\
				"if itest ${blkc} <= ${pbl_mmc_size}; then "	\
					"mmc write ${loadaddr} ${pbl_mmc_start} ${blkc}; "	\
				"fi; "	\
			"fi; "	\
		"fi;\0"	\
	"update_uboot_mmc=run set_getcmd; "	\
		"if ${getcmd} ${uboot}; then "	\
			"if itest ${filesize} > 0; then "	\
				"mmc dev ${mmcdev}; mmc rescan; "	\
				"setexpr blkc ${filesize} + 0x1ff; "	\
				"setexpr blkc ${blkc} / 0x200; "	\
				"if itest ${blkc} <= ${uboot_mmc_size}; then "	\
					"mmc write ${loadaddr} ${uboot_mmc_start} ${blkc}; "	\
				"fi; "	\
			"fi; "	\
		"fi;\0"	\
	"update_pbl_qspi=run set_getcmd; "	\
		"if ${getcmd} ${pbl_qspi}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} pbl ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"update_uboot_qspi=run set_getcmd; "	\
		"if ${getcmd} ${uboot}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} u-boot ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"fmucode_mmc_start=" __stringify(TQMLS104xA_FMUCODE_MMC_SECT_START) "\0"	\
	"fmucode_mmc_size=" __stringify(TQMLS104xA_FMUCODE_MMC_SECT_SIZE) "\0"	\
	"update_fmucode_mmc=run set_getcmd; "	\
		"if ${getcmd} ${fmucode}; then "	\
			"if itest ${filesize} > 0; then "	\
				"mmc dev ${mmcdev}; mmc rescan; "	\
				"setexpr blkc ${filesize} + 0x1ff; "	\
				"setexpr blkc ${blkc} / 0x200; "	\
				"if itest ${blkc} <= ${fmucode_mmc_size}; then "	\
					"mmc write ${loadaddr} ${fmucode_mmc_start} ${blkc}; "	\
				"fi; "	\
			"fi; "	\
		"fi;\0"	\
	"update_fmucode_qspi=run set_getcmd; "	\
		"if ${getcmd} ${fmucode}; then "	\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "	\
				"sf update ${loadaddr} fmucode ${filesize}; "	\
			"fi; "	\
		"fi; "	\
		"setenv filesize;\0"	\
	"update_kernel_mmc=run set_getcmd; "   \
		"if ${getcmd} ${kernel}; then " \
			"if itest ${filesize} > 0; then "   \
				"mmc dev ${mmcdev}; mmc rescan; "   \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; "	\
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} ${filesize}; "\
			"fi; "  \
		"fi;\0" \
	"update_fdt_mmc=run set_getcmd; "	\
		"if ${getcmd} ${fdtfile}; then " \
			"if itest ${filesize} > 0; then "   \
				"mmc dev ${mmcdev}; mmc rescan; "   \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; "	\
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${fdtfile} ${filesize}; "\
			"fi; "  \
		"fi;\0" \
	"prepare_ubi_part=if sf probe; then "                          \
			"mtd erase ${ubirootfspart}; "                 \
			"ubi part ${ubirootfspart}; "		       \
			"ubi create ${ubirootfsvol}; "		       \
			"ubi detach; "				       \
		"fi \0"						       \
	"update_rootfs_spi=run set_getcmd; "			       \
		"if ${getcmd} ${ubirootfs}; then "                     \
			"if itest ${filesize} > 0; then "              \
				"echo Write rootfs image to UBI ...; " \
				"if sf probe; then "                   \
					"ubi part ${ubirootfspart}; "  \
					"ubi write ${loadaddr} "       \
						"${ubirootfsvol} "     \
						"${filesize}; "        \
					"ubi detach; "                 \
				"fi; "                                 \
			"fi; "                                         \
		"fi; "                                                 \
		"setenv filesize \0"				       \
	""

#endif /* __TQMLS10XXA_COMMON_H__ */
