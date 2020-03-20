/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#ifndef __TQMLS1012AL_CONFIG_H__
#define __TQMLS1012AL_CONFIG_H__

#include <linux/sizes.h>

#include "ls1012a_common.h"

#undef CONFIG_SYS_I2C

#undef CONFIG_DISPLAY_BOARDINFO_LATE

#define TQMLS1012AL_I2C_BUS_NAME	"i2c@2180000"
#define TQMLS1012AL_I2C_EEPROM1_ADDR	0x51

/* CONFIG_SYS_MALLOC_LEN >= 512k for UBI! */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(512u * 1024)
/* CONFIG_SYS_TEXT_BASE points to start of U-Boot */
#undef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x40010000

#define CONFIG_MISC_INIT_R

/* FLASH */
#define CONFIG_MTD_PARTITIONS
#define FSL_QSPI_QUAD_MODE
#define CONFIG_SYS_FSL_QSPI_AHB

/* Watchdog
 * timeout 15 seconds
 */
#define CONFIG_IMX_WATCHDOG
#define CONFIG_HW_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	15000
#define CONFIG_WATCHDOG_INIT_MSECS	1500

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000

/* RAM
 * TQMLS1012AL-PROTO1: 256 MB
 * TQMLS1012AL-PROTO2: 512 MB
 */
#if defined(CONFIG_TQMLS1012AL_256MB)
#define CONFIG_SYS_SDRAM_SIZE		SZ_256M
#define CONFIG_SYS_MEMTEST_END		0x8b7fffff /* size 184M*/
#elif defined(CONFIG_TQMLS1012AL_512MB)
#define CONFIG_SYS_SDRAM_SIZE		SZ_512M
#define CONFIG_SYS_MEMTEST_END		0x9b7fffff /* size 440M*/
#endif

/* use ASR auto mode */
#define RAM_ARS_AUTO_MODE

#define MTDIDS_DEFAULT "nor0=nor0\0"
#define MTDPARTS_DEFAULT \
	"mtdparts=nor0:"                                               \
		"64K@0M(RCW),"                                         \
		"2M@64K(U-Boot),"                                      \
		"1M(U-Boot env),"                                      \
		"2M@4M(PPA FIT image),"                                \
		"64K@10M(PFE),"                                        \
		"1M@15M(DTB),"                                         \
		"48M@16M(Linux),"                                      \

#define TQMLS1012AL_FDT_ADDRESS		0x00f00000

#if defined(CONFIG_DEFAULT_FDT_FILE)
#define TQMLS1012AL_FDT_FILE_ENV	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"
#else
#define TQMLS1012AL_FDT_FILE_ENV
#endif

#define TQMLS1012AL_EXTRA_UPDATE_ENV_SETTINGS                                  \
	"loadimage=sf probe 0; sf read ${loadaddr} ${kernel_mtdpart}\0"        \
	"loadfdt=sf probe 0; sf read ${fdt_addr} ${fdt_mtdpart}\0"             \
	"mmcblkdev=0\0"                                                        \
	"update_uboot=run set_getcmd; if ${getcmd} ${uboot}; then "            \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; "                                         \
			"sf update ${loadaddr} ${uboot_mtdpart} ${filesize}; " \
		"fi; fi; "                                                     \
		"setenv filesize; setenv getcmd \0"                            \
	"update_kernel=run set_getcmd; "                                       \
		"if ${getcmd} ${kernel}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"sf probe 0; "                                 \
				"sf update ${loadaddr} ${kernel_mtdpart} "     \
					"${filesize};"                         \
		"fi; fi; "                                                     \
		"setenv filesize; setenv getcmd; setenv kernel \0"             \
	"update_fdt=run set_getcmd; if ${getcmd} ${fdt_file}; then "           \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; "                                         \
			"sf update ${loadaddr} ${fdt_mtdpart} ${filesize}; "   \
		"fi; fi; "                                                     \
		"setenv filesize; setenv getcmd \0"                            \
	"usb_update_uboot=run set_getcmd; usb reset; "                         \
		"load usb 0 ${loadaddr} ${uboot}; "                            \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; "                                         \
			"sf update ${loadaddr} ${uboot_mtdpart} ${filesize}; " \
		"fi; fi; "                                                     \
		"setenv filesize; setenv getcmd \0"                            \

#define TQMLS1012AL_EXTRA_BOOT_ENV_SETTINGS                                    \
	"kernel_addr_r=0x80080000\0"                                           \
	"fdt_addr_r=0x83000000\0"                                              \
	"ramfs_addr_r=0x81ffffb0\0"                                            \
	"mmcboot=echo Booting from mmc ...; "                                  \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"if load mmc 0:1 ${kernel_addr_r} ${kernel}; then "            \
			"if load mmc 0:1 ${fdt_addr_r} ${fdt_file}; then "     \
				"pfe stop; "                                   \
				"booti ${kernel_addr_r} - ${fdt_addr_r};"      \
			"fi; "                                                 \
		"fi;\0"                                                        \
	"qspiboot=echo Booting from qspi ...; "                                \
		"setenv bootargs; "                                            \
		"run qspiargs; "                                               \
		"if run loadimage; then "                                      \
			"if run loadfdt; then "                                \
				"echo boot device tree kernel ...; "           \
				"pfe stop; "                                   \
				"booti ${loadaddr} - ${fdt_addr}; "            \
			"fi; "                                                 \
		"else "                                                        \
			"pfe stop; "                                           \
			"bootm; "                                              \
		"fi;\0"                                                        \
		"setenv bootargs \0"                                           \
	"ramboot=echo Booting from tftp ...; "                                 \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run ramargs; "                                                \
		"tftp ${kernel_addr_r} ${kernel}; "                            \
		"if itest ${filesize} > 0; then "                              \
			"setenv filesize; "                                    \
			"tftp ${fdt_addr_r} ${fdt_file}; "                     \
			"if itest ${filesize} > 0; then "                      \
				"setenv filesize; "                            \
				"tftp ${ramfs_addr_r} ${ramfs}; "              \
				"if itest ${filesize} > 0; then "              \
					"pfe stop; "                           \
					"booti ${kernel_addr_r} "              \
					      "${ramfs_addr_r} ${fdt_addr_r}; "\
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; \0"                                          \
	"usbboot=echo Booting from usb ...; "                                  \
		"run set_getcmd; "                                             \
		"usb reset; "                                                  \
		"setenv bootargs; "                                            \
		"run usbargs; "                                                \
		"if load usb 0:1 ${kernel_addr_r} ${kernel}; then "            \
			"if load usb 0:1 ${fdt_addr_r} ${fdt_file}; then "     \
				"pfe stop; "                                   \
				"booti ${kernel_addr_r} - ${fdt_addr_r};"      \
			"fi; "                                                 \
		"fi;\0"                                                        \
	"panicboot=echo No boot device !!! reset\0"                            \

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"verify=no\0"                                                          \
	"loadaddr=0x82000000\0"                                                \
	"fdt_high=0xffffffffffffffff\0"                                        \
	"initrd_high=0xffffffffffffffff\0"                                     \
	"kernel_addr=0x01000000\0"                                             \
	"kernel_start=0x01000000\0"                                            \
	"kernel_load=0xa0000000\0"                                             \
	"kernel_size=0x2800000\0"                                              \
	"board=tqmls1012al\0"                                                  \
	"fdt_addr=" __stringify(TQMLS1012AL_FDT_ADDRESS)"\0"                   \
	"kernel=Image.bin\0"                                                   \
	"uboot=u-boot.imx\0"                                                   \
	"netdev=eth0\0"                                                        \
	"ipmode=static\0"                                                      \
	"rootfsmode=ro\0"                                                      \
	"rootpath=/srv/nfs/tqmls1012al\0"                                      \
	"rootfs_mtddev=RootFS\0"                                               \
	"fdt_mtdpart=DTB\0"                                                    \
	"kernel_mtdpart=Linux\0"                                               \
	"uboot_mtdpart=U-Boot\0"                                               \
	"mmcargs=run addtty addmmc econ\0"                                     \
	"ramargs= run addtty econ\0"                                           \
	"usbargs=run addtty addusb econ\0"                                     \
	"ramfs=rd.img\0"                                                       \
	"addip_static=setenv bootargs ${bootargs} "                            \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"            \
		"${hostname}:${netdev}:off\0"                                  \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                  \
	"addip=if test \"${ipmode}\" != static; then "                         \
		"run addip_dynamic; else run addip_static; fi\0"               \
	"addqspi=setenv bootargs ${bootargs} root=ubi0:root ${rootfsmode} "    \
		"rootfstype=ubifs ubi.mtd=${rootfs_mtddev}\0"                  \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate} "   \
		"consoleblank=0\0"                                             \
	"addmmc=setenv bootargs ${bootargs} root=/dev/mmcblk0p2 "              \
		"rootfstype=ext4\0"                                            \
	"addusb=setenv bootargs ${bootargs} root=/dev/sda2 "                   \
		"rootfstype=ext4 rootdelay=5\0"                                \
	"qspiargs=run addqspi addtty econ\0"                                   \
	"econ=setenv bootargs ${bootargs} earlycon=uart8250,mmio,0x21c0500\0"  \
	"console=ttyS0\0"                                                      \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
	TQMLS1012AL_FDT_FILE_ENV                                               \
	TQMLS1012AL_EXTRA_UPDATE_ENV_SETTINGS                                  \
	TQMLS1012AL_EXTRA_BOOT_ENV_SETTINGS                                    \

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "sf probe; run mmcboot; run panicboot"

#ifdef CONFIG_TQMLS1012AL_EMMC
#define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls1012al/ls1012a_rcw_qspi_emmc.cfg
#else
#define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls1012al/ls1012a_rcw_qspi_sd.cfg
#endif

#define CONFIG_SYS_FSL_PBL_PBI	board/tqc/tqmls1012al/ls1012a_pbi_qspi.cfg
#define CONFIG_SPL_PAD_TO		0x10000

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

#endif /* __TQMLS1012AL_CONFIG_H__ */
