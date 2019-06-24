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
/* #define CONFIG_SYS_DDR_RAW_TIMING */
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

#undef BOOT_ENV_SETTINGS
#ifdef CONFIG_SD_BOOT
#define BOOT_ENV_SETTINGS \
	"boot=SD\0" \
	"loadaddr=0x82000000\0 " \
	"fdtaddr=0x88000000\0 " \
	"bootcmd=setenv bootargs root=/dev/mmcblk0p2 rootwait rw earlycon=uart8250,0x21c0500 console=ttyS0,115200 cma=256M video=1920x1080-32@60; fatload mmc 0:1 ${loadaddr} ls1028a-dp-fw.bin; hdp load ${loadaddr}; fatload mmc 0:1 ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr; fatload mmc 0:1 ${fdtaddr} ls1028a-mbls1028a.dtb; booti ${loadaddr} - ${fdtaddr}\0"
#elif CONFIG_EMMC_BOOT
#define BOOT_ENV_SETTINGS \
	"boot=MMC\0" \
	"loadaddr=0x82000000\0 " \
	"fdtaddr=0x88000000\0 " \
	"bootcmd=setenv bootargs root=/dev/mmcblk1p2 rootwait rw earlycon=uart8250,0x21c0500 console=ttyS0,115200 cma=256M video=1920x1080-32@60; fatload mmc 1:1 ${fdtaddr} Image.gz; unzip $fdtaddr $loadaddr; fatload mmc 1:1 ${fdtaddr} ls1028a-mbls1028a.dtb; booti ${loadaddr} - ${fdtaddr}\0"
#else
#define BOOT_ENV_SETTINGS
#endif

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
	"board=tqmls1028a_mbls1028a\0"

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
