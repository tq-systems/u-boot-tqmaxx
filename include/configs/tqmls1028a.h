/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMLS1028A_H
#define __TQMLS1028A_H

#include "ls1028a_common.h"

#ifdef CONFIG_BAREMETAL
#define CONFIG_MP
#define CONFIG_SYS_DDR_SDRAM_SLAVE_SIZE        (256 * 1024 * 1024)
#define CONFIG_MASTER_CORE                     0
#define CONFIG_SYS_DDR_SDRAM_MASTER_SIZE       (512 * 1024 * 1024)
#endif

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ / 4)

/*  */
#define CONFIG_SYS_RTC_BUS_NUM         0
#define CONFIG_MISC_INIT_R

/* DDR */
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define CONFIG_DIMM_SLOTS_PER_CTLR		1
#define DDR_RAM_SIZE	0x40000000 /* 1GB */

/* EEPROM */
#define I2C_EEPROM_ADDR 0x57
#define I2C_EEPROM_ADDR_LEN 2

/* FlexSPI */
#ifdef CONFIG_NXP_FSPI
#define NXP_FSPI_FLASH_SIZE            SZ_256M
#define NXP_FSPI_FLASH_NUM              1
#endif

#define CONFIG_SYS_UBOOT_BASE 0x20116000
#define CONFIG_SPL_PAD_TO	0x16000

/* Store environment at top of flash */
#ifdef CONFIG_EMU_PXP
#define CONFIG_ENV_SIZE			0x1000
#else
#define CONFIG_ENV_SIZE			0x2000
#endif

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE
#endif

#define CONFIG_QIXIS_I2C_ACCESS
#define CONFIG_SYS_I2C_EARLY_INIT

#define CONFIG_SYS_FLASH_BASE 0x20000000


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
		"512k@0k(RCW),"                                                \
		"512k@512k(PPA),"                                              \
		"3M@1M(U-Boot-PBL),"                                           \
		"64k@4M(DTB),"                                                 \
		"512k@4608k(HDP),"                                             \
		"10M@5M(Linux),"                                               \
		"32M@15M(RootFS)\0"                                            \


/* SATA */
#ifndef SPL_NO_SATA
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
#endif

#define TQMLS1028_SD_RCW_FILE_NAME	"rcw_1300_sd.bin"
#define TQMLS1028_EMMC_RCW_FILE_NAME	"rcw_1300_emmc.bin"
#define TQMLS1028_QSPI_RCW_FILE_NAME	"rcw_1300_spi_nor.bin"
#define TQMLS1028_SD_UBOOT_FILE_NAME	"u-boot-with-spl.bin"
#define TQMLS1028_SD_KERNEL_FILE_NAME	"Image.gz"
#define MAX_RCW_SIZE 1024
#define SD_RCW_OFFSET 0x8 /* Blocks */
#define MAX_UBOOT_SIZE 0x300000
#define SD_UBOOT_OFFSET 0x800 /* Blocks */

#define TQMLS1028_UPDATE_ENV_SETTINGS                                          \
	"rcw_sd_file="TQMLS1028_SD_RCW_FILE_NAME"\0"                           \
	"rcw_max_size="__stringify(MAX_RCW_SIZE)"\0"                           \
	"rcw_sd_offset="__stringify(SD_RCW_OFFSET)"\0"                         \
	"rcw_emmc_file="TQMLS1028_EMMC_RCW_FILE_NAME"\0"                           \
	"rcw_qspi_file="TQMLS1028_QSPI_RCW_FILE_NAME"\0"                           \
	"uboot_sd_file="TQMLS1028_SD_UBOOT_FILE_NAME"\0"                           \
	"uboot_max_size="__stringify(MAX_UBOOT_SIZE)"\0"                           \
	"uboot_sd_offset="__stringify(SD_UBOOT_OFFSET)"\0"                         \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"kernel_file="TQMLS1028_SD_KERNEL_FILE_NAME"\0"\
	"firmwarepart=1\0"                                                     \
	"mmcsddev=0\0"								\
	"mmcdev=1\0"								\
	"update_rcw_sd=run set_getcmd; "                                       \
		"if ${getcmd} ${rcw_sd_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev 0; mmc rescan; "		       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${rcw_max_size}; then "\
					"mmc write ${loadaddr} ${rcw_sd_offset} ${blkc}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_uboot_sd=run set_getcmd; "                                     \
		"if ${getcmd} ${uboot_sd_file}; then "                         \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcsddev}; mmc rescan; "		       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"mmc write ${loadaddr} ${uboot_sd_offset} ${blkc}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_fdt_sd=run set_getcmd; "                                          \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcsddev}; mmc rescan; "              \
				"echo Write fdt image to mmc ${mmcsddev}:${firmwarepart}...; " \
				"save mmc ${mmcsddev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_kernel_sd=run set_getcmd; "                                          \
		"if ${getcmd} ${kernel_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcsddev}; mmc rescan; "              \
				"echo Write kernel image to mmc ${mmcsddev}:${firmwarepart}...; " \
				"save mmc ${mmcsddev}:${firmwarepart} ${loadaddr} " \
					"${kernel_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_rcw_emmc=run set_getcmd; "                                       \
		"if ${getcmd} ${rcw_emmc_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "		       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${rcw_max_size}; then "\
					"mmc write ${loadaddr} ${rcw_sd_offset} ${blkc}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_uboot_emmc=run set_getcmd; "                                     \
		"if ${getcmd} ${uboot_sd_file}; then "                         \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "		       \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"mmc write ${loadaddr} ${uboot_sd_offset} ${blkc}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_fdt_emmc=run set_getcmd; "                                          \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_kernel_emmc=run set_getcmd; "                                          \
		"if ${getcmd} ${kernel_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${kernel_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_rcw_qspi=run set_getcmd; "                                       \
		"if ${getcmd} ${rcw_qspi_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"sf probe; sf update ${loadaddr} RCW ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_uboot_qspi=run set_getcmd; "                                     \
		"if ${getcmd} ${uboot_sd_file}; then "                         \
			"if itest ${filesize} > 0; then "                      \
				"if itest ${filesize} <= ${uboot_max_size}; then "	       \
					"sf probe; sf update ${loadaddr} U-Boot-PBL ${filesize};"\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_fdt_qspi=run set_getcmd; "                                     \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"sf probe; sf update ${loadaddr} DTB ${filesize};"\
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_kernel_qspi=run set_getcmd; "                                  \
		"if ${getcmd} ${kernel_file}; then "                           \
			"if itest ${filesize} > 0; then "                      \
				"sf probe; sf update ${loadaddr} Linux ${filesize};"\
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \

#undef BOOT_ENV_SETTINGS
#define BOOT_ENV_SETTINGS \
	"boot=SD\0" \
	"loadaddr=0x82000000\0 " \
	"fdtaddr=0x88000000\0 " \
	"addtty=setenv bootargs ${bootargs} console=ttyS0,115200\0" \
	"addvideo=setenv bootargs ${bootargs} cma=256M video=1920x1080-32@60\0"	\
	"addsd=setenv bootargs ${bootargs} root=/dev/mmcblk0p2 "               \
		"rootfstype=ext4\0"                                \
	"addemmc=setenv bootargs ${bootargs} root=/dev/mmcblk1p2 "               \
		"rootfstype=ext4\0"                                \
	"resetusb=i2c dev 5; i2c mw 0x25 0x6.1 0xfd; i2c mw 0x25 0x2.1 0xfd; "\
		"sleep 0.1; i2c mw 0x25 0x2.1 0xff;\0 " \
	"resetphy=i2c dev 5; i2c mw 0x70 0x3 0xd5; i2c mw 0x70 0x1 0xd5; " \
		"sleep 0.1; i2c mw 0x70 0x1 0xff;\0" \
	"sdhdpload=fatload mmc 0:1 ${loadaddr} ls1028a-dp-fw.bin; hdp load ${loadaddr};\0" \
	"sdimageload=fatload mmc 0:1 ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr\0" \
	"sdfdtload=fatload mmc 0:1 ${fdtaddr} ls1028a-mbls1028a.dtb;\0" \
	"sdargs=run addsd addtty addvideo\0"                      \
	"sdboot=echo Booting from sd card ...; " \
		"setenv bootargs; " \
		"run sdargs; " \
		"run sdhdpload; " \
		"run sdimageload; "\
		"run sdfdtload;" \
		"booti ${loadaddr} - ${fdtaddr}\0" \
	"emmchdpload=fatload mmc 1:1 ${loadaddr} ls1028a-dp-fw.bin; hdp load ${loadaddr};\0" \
	"emmcimageload=fatload mmc 1:1 ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr\0" \
	"emmcfdtload=fatload mmc 1:1 ${fdtaddr} ls1028a-mbls1028a.dtb;\0" \
	"emmcargs=run addemmc addtty addvideo\0"                      \
	"emmcboot=echo Booting from emmc ...; " \
		"setenv bootargs; " \
		"run emmcargs; " \
		"run emmchdpload; " \
		"run emmcimageload; "\
		"run emmcfdtload;" \
		"booti ${loadaddr} - ${fdtaddr}\0" \
	"rootfs_mtddev=RootFS\0"                                               \
	"addspi=setenv bootargs ${bootargs} root=ubi0_0 rw "                \
		"rootfstype=ubifs ubi.mtd=6\0"                                 \
	"spiargs=run addspi addtty addvideo\0"                                 \
	"spikernelload=sf probe 0; sf read ${fdtaddr} Linux\; "		       \
		"unzip ${fdtaddr} ${loadaddr}\0"			       \
	"spifdtload=sf probe 0; sf read ${fdtaddr} DTB\0"                      \
	"spihdpload=sf probe; sf read ${loadaddr} HDP; hdp load ${loadaddr};\0" \
	"spiboot=echo Booting from SPI NOR flash...; setenv bootargs; "        \
		"run spiargs; run spihdpload spikernelload spifdtload ; "       \
		"booti ${loadaddr} - ${fdtaddr};\0"                            \
	"panicboot=echo No boot device !!! reset\0"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "run sdboot; run emmcboot; run panicboot"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"baudrate=115200\0" \
	"bootdelay=3\0" \
	BOOT_ENV_SETTINGS \
	"ethact=enetc\#1\0" \
	"ethprime=enetc\#1\0" \
	"hwconfig=fsl_ddr:bank_intlv=auto\0" \
	"stderr=serial\0" \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"board=tqmls1028a_mbls1028a\0" \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
		TQMLS1028_UPDATE_ENV_SETTINGS

/*
 * All the defines above are for the TQMLS1028a SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBLS1028A
#include "tqmls1028a_mbls1028a.h"
#else
#error "No baseboard for the TQMLS1028A SOM defined!"
#endif

#endif /* __TQMLS1028A_H */
