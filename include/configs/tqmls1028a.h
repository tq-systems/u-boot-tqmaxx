/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2019 NXP
 * Copyright 2019-2020 TQ-Systems GmbH
 *
 * Author: Matthias Schiffer <matthias.schiffer@tq-group.com>
 */

#ifndef __TQMLS1028A_H
#define __TQMLS1028A_H

#include "ls1028a_common.h"

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ / 4)

/* DDR */
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define CONFIG_DIMM_SLOTS_PER_CTLR		1
#define DDR_RAM_SIZE	0x40000000 /* 1GB */

/* EEPROM */
#undef CONFIG_ID_EEPROM
#undef CONFIG_SYS_I2C_EEPROM_ADDR_LEN
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2

#define CONFIG_SYS_I2C_RTC_ADDR	0x51

#define CONFIG_SYS_FLASH_BASE 0x20000000

/* Environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)


/* filesystem on flash */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	2
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE
#define CONFIG_MTD_UBI_WL_THRESHOLD	4096
#define CONFIG_LZO
#define MTDIDS_DEFAULT \
	"nor0=nor0\0"

#define MTDPARTS_DEFAULT \
	"mtdparts=nor0:"                                                       \
		"1M@0M(PBL),"                                                  \
		"4M@1M(BL3),"                                                  \
		"1M@5M(U-Boot-ENV),"                                           \
		"3328k@6M(Reserved1),"                                         \
		"256k@9472k(HDP),"                                             \
		"5632k@9728k(Reserved2),"                                      \
		"1M@15M(DTB),"                                                 \
		"10M@16M(Linux),"                                              \
		"38M@26M(RootFS)\0"


/* SATA */
#ifndef CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT2
#endif
#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
#define SCSI_VEND_ID 0x1b4b
#define SCSI_DEV_ID  0x9170
#define CONFIG_SCSI_DEV_LIST {SCSI_VEND_ID, SCSI_DEV_ID}
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA1                        AHCI_BASE_ADDR1

#undef SD_BOOTCOMMAND
#define SD_BOOTCOMMAND "run mmcboot; run spiboot; run panicboot"
#undef SD2_BOOTCOMMAND
#define SD2_BOOTCOMMAND "run mmcboot; run spiboot; run panicboot"
#undef XSPI_NOR_BOOTCOMMAND
#define XSPI_NOR_BOOTCOMMAND "run spiboot; run mmcboot; run panicboot"

/*
 * All the defines above are for the TQMLS1028a SoM
 *
 * Now include the baseboard specific configuration
 */
#if defined CONFIG_MBLS1028A
#include "tqmls1028a_mbls1028a.h"
#elif defined CONFIG_MBLS1028A_IND
#include "tqmls1028a_mbls1028a_ind.h"
#else
#error "No baseboard for the TQMLS1028A SOM defined!"
#endif

#define TQMLS1028_MMC_PBL_FILE_NAME	"bl2_sd.pbl"
#define TQMLS1028_SPI_PBL_FILE_NAME	"bl2_flexspi_nor.pbl"
#define TQMLS1028_UBOOT_FILE_NAME	"fip_uboot.bin"
#define TQMLS1028_MMC_KERNEL_FILE_NAME	"Image.gz"
#define MAX_PBL_SIZE 1024
#define MMC_PBL_OFFSET 0x8 /* Blocks */
#define MAX_UBOOT_SIZE 0x300000
#define MMC_UBOOT_OFFSET 0x800 /* Blocks */

#define TQMLS1028_UPDATE_ENV_SETTINGS                                          \
	"pbl_mmc_file="TQMLS1028_MMC_PBL_FILE_NAME"\0"                         \
	"pbl_max_size="__stringify(MAX_PBL_SIZE)"\0"                           \
	"pbl_mmc_offset="__stringify(MMC_PBL_OFFSET)"\0"                       \
	"pbl_spi_file="TQMLS1028_SPI_PBL_FILE_NAME"\0"                         \
	"uboot_mmc_file="TQMLS1028_UBOOT_FILE_NAME"\0"                         \
	"uboot_spi_file="TQMLS1028_UBOOT_FILE_NAME"\0"                         \
	"uboot_max_size="__stringify(MAX_UBOOT_SIZE)"\0"                       \
	"uboot_mmc_offset="__stringify(MMC_UBOOT_OFFSET)"\0"                   \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"kernel_file="TQMLS1028_MMC_KERNEL_FILE_NAME"\0"                       \
	"firmwarepart=1\0"                                                     \
	"update_pbl_mmc=run set_getcmd; "                                      \
		"if ${getcmd} ${pbl_mmc_file}; then "                          \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev 0; mmc rescan; "		       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${pbl_max_size}; then "\
					"mmc write ${loadaddr} ${pbl_mmc_offset} ${blkc}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	"update_uboot_mmc=run set_getcmd; "                                    \
		"if ${getcmd} ${uboot_mmc_file}; then "                        \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "	       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"mmc write ${loadaddr} ${uboot_mmc_offset} ${blkc}; "\
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
	"update_pbl_spi=run set_getcmd; "                                      \
		"if ${getcmd} ${pbl_spi_file}; then "                          \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"sf probe; sf update ${loadaddr} PBL ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
	"update_uboot_spi=run set_getcmd; "                                    \
		"if ${getcmd} ${uboot_spi_file}; then "                        \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"sf probe; sf update ${loadaddr} BL3 ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd\0"                \
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
		"setenv filesize; setenv blkc; setenv getcmd\0"

#undef BOOT_ENV_SETTINGS
#define BOOT_ENV_SETTINGS \
	BOOT_ENV_BOARD \
	"loadaddr=0x82000000\0" \
	"fdtaddr=0x88000000\0" \
	"addtty=setenv bootargs ${bootargs} console=${console}\0" \
	"addvideo=setenv bootargs ${bootargs} video=1920x1080-32@60\0"	\
	"addmmc=setenv bootargs ${bootargs} root=/dev/mmcblk${mmcdev}p2 rootwait\0" \
	"firmwarepart=1\0"                                                     \
	"mmchdpload=load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ls1028a-dp-fw.bin; hdp load ${loadaddr};\0" \
	"mmcimageload=load mmc ${mmcdev}:${firmarepart} ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr\0" \
	"mmcfdtload=load mmc ${mmcdev}:${firmwarepart} ${fdtaddr} ${fdt_file}; fdt addr ${fdtaddr}\0" \
	"mmcargs=run addmmc addtty addvideo\0"                                 \
	"mmcboot=echo Booting from MMC ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"run mmchdpload; "                                             \
		"run mmcimageload; "                                           \
		"run mmcfdtload;"                                              \
		"booti ${loadaddr} - ${fdtaddr}\0"                             \
	"rootfs_mtddev=RootFS\0"                                               \
	"addspi=setenv bootargs ${bootargs} root=ubi0_0 rw "                   \
		"rootfstype=ubifs ubi.mtd=7\0"                                 \
	"spiargs=run addspi addtty addvideo\0"                                 \
	"spikernelload=sf probe 0; sf read ${fdtaddr} Linux; "		       \
		"unzip ${fdtaddr} ${loadaddr}\0"			       \
	"spifdtload=sf probe 0; sf read ${fdtaddr} DTB; fdt addr ${fdtaddr}\0" \
	"spihdpload=sf probe; sf read ${loadaddr} HDP; hdp load ${loadaddr};\0" \
	"spiboot=echo Booting from SPI NOR flash...; setenv bootargs; "        \
		"run spiargs; run spihdpload spikernelload spifdtload ; "      \
		"booti ${loadaddr} - ${fdtaddr};\0"                            \
	"panicboot=echo No boot device !!! reset\0"			       \
	""

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"hwconfig=fsl_ddr:bank_intlv=auto\0"                                   \
	"bootdelay=3\0"                                                        \
	BOOT_ENV_SETTINGS                                                      \
	"ethact=enetc-1\0"                                                     \
	"ethprime=enetc-1\0"                                                   \
	"stderr=serial\0"                                                      \
	"stdin=serial\0"                                                       \
	"stdout=serial\0"                                                      \
	"ipmode=static\0"                                                      \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
	TQMLS1028_UPDATE_ENV_SETTINGS

#endif /* __TQMLS1028A_H */
