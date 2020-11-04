/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#ifndef __TQMLS1012AL_CONFIG_H__
#define __TQMLS1012AL_CONFIG_H__

#include "ls1012a_common.h"
#include <linux/sizes.h>


#undef CONFIG_DISPLAY_BOARDINFO_LATE

#undef CONFIG_SYS_I2C
#define TQMLS1012AL_I2C_EEPROM1_ADDR	0x51

/* CONFIG_SYS_MALLOC_LEN >= 512k for UBI! */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_1M)

/*
 * A lot of RAM is reserved for the BL31 or used by U-Boot itself, so we
 * can test only the lower 368 MiB of RAM
 */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x97000000

/* FLASH */
#define CONFIG_SYS_FSL_QSPI_AHB

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1

/* Environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)

#define MTDIDS_DEFAULT "nor0=nor0\0"
#define MTDPARTS_DEFAULT \
	"mtdparts=nor0:"                                                       \
		"1M@0M(PBL),"                                                  \
		"4M@1M(BL3),"                                                  \
		"1M@5M(U-Boot-ENV),"                                           \
		"4M@6M(Reserved1),"                                            \
		"3M@10M(PFE),"                                                 \
		"2M@13M(Reserved2),"                                           \
		"1M@15M(DTB),"                                                 \
		"10M@16M(Linux),"                                              \
		"38M@26M(RootFS)\0"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "run mmcboot; run panicboot"
#undef QSPI_NOR_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND CONFIG_BOOTCOMMAND

/*
 * All the defines above are for the TQMLS1012al SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBLS1012AL
#include "tqmls1012al_mbls1012al.h"
#else
#error "No baseboard for the TQMLS1012AL SOM defined!"
#endif

#define TQMLS1012_SPI_PBL_FILE_NAME	"bl2_qspi.pbl"
#define TQMLS1012_UBOOT_FILE_NAME	"fip_uboot.bin"
#define TQMLS1012_MMC_KERNEL_FILE_NAME	"Image.gz"
#define MAX_PBL_SIZE 0x100000
#define MAX_UBOOT_SIZE 0x400000

#define TQMLS1012AL_UPDATE_ENV_SETTINGS                                        \
	"pbl_max_size="__stringify(MAX_PBL_SIZE)"\0"                           \
	"pbl_spi_file="TQMLS1012_SPI_PBL_FILE_NAME"\0"                         \
	"uboot_spi_file="TQMLS1012_UBOOT_FILE_NAME"\0"                         \
	"uboot_max_size="__stringify(MAX_UBOOT_SIZE)"\0"                       \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"kernel_file="TQMLS1012_MMC_KERNEL_FILE_NAME"\0"                       \
	"update_pbl=run set_getcmd; "                                          \
		"if ${getcmd} ${pbl_spi_file}; then "                          \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${pbl_max_size}; then " \
					"sf probe; sf update ${loadaddr} PBL ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	"update_uboot=run set_getcmd; "                                        \
		"if ${getcmd} ${uboot_spi_file}; then "                        \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${uboot_max_size}; then " \
					"sf probe; sf update ${loadaddr} BL3 ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	"update_fdt_mmc=run set_getcmd; "                                      \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd\0"                             \
	"update_kernel_mmc=run set_getcmd; "                                   \
		"if ${getcmd} ${kernel_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${kernel_file} ${filesize}; "         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd\0"                             \
	"update_fdt_spi=run set_getcmd; "                                      \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"sf probe; sf update ${loadaddr} DTB ${filesize};"\
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	"update_kernel_spi=run set_getcmd; "                                   \
		"if ${getcmd} ${kernel_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"sf probe; sf update ${loadaddr} Linux ${filesize};"\
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	""

#undef BOOT_ENV_SETTINGS
#define BOOT_ENV_SETTINGS                                                      \
	"loadaddr=0x84000000\0"                                                \
	"fdtaddr=0x8a000000\0"                                                 \
	"addtty=setenv bootargs ${bootargs} console=${console}\0"              \
	"addmmc=setenv bootargs ${bootargs} root=/dev/mmcblk${mmcdev}p2 rootwait\0" \
	"mmcdev=0\0"                                                           \
	"firmwarepart=1\0"                                                     \
	"mmckernelload=load mmc ${mmcdev}:${firmarepart} ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr\0" \
	"mmcfdtload=load mmc ${mmcdev}:${firmwarepart} ${fdtaddr} ${fdt_file}; fdt addr ${fdtaddr}\0" \
	"mmcargs=run addmmc addtty\0"                                 \
	"mmcboot=echo Booting from MMC ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs && "                                              \
		"run mmckernelload && "                                        \
		"run mmcfdtload && "                                           \
		"pfe stop && "                                                 \
		"booti ${loadaddr} - ${fdtaddr}\0"                             \
	"addspi=setenv bootargs ${bootargs} root=ubi0_0 rw "                   \
		"rootfstype=ubifs ubi.mtd=7\0"                                 \
	"spiargs=run addspi addtty\0"                                          \
	"spikernelload=sf probe 0; sf read ${fdtaddr} Linux; "		       \
		"unzip ${fdtaddr} ${loadaddr}\0"			       \
	"spifdtload=sf probe 0; sf read ${fdtaddr} DTB; fdt addr ${fdtaddr}\0" \
	"spiboot=echo Booting from SPI-NOR flash ...; "                        \
		"setenv bootargs; "                                            \
		"run spiargs && "                                              \
		"run spikernelload && "                                        \
		"run spifdtload && "                                           \
		"pfe stop && "                                                 \
		"booti ${loadaddr} - ${fdtaddr}\0"                             \
	"panicboot=echo No boot device !!! reset\0"                            \
	""

/*
 * loadaddr and fdtaddr are chosen differently from other TQ modules to avoid
 * collision with the area reserved for PFE (0x83400000-0x83ffffff)
 */
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	BOOT_ENV_SETTINGS                                                      \
	"ipmode=static\0"                                                      \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
	TQMLS1012AL_UPDATE_ENV_SETTINGS                                        \
	BOOT_ENV_BOARD                                                         \
	""

#endif /* __TQMLS1012AL_CONFIG_H__ */
