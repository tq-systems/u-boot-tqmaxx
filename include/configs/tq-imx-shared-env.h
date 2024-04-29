/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQ_IMX_SHARED_ENV_H)
#define __TQ_IMX_SHARED_ENV_H

#ifdef CONFIG_CMD_SF

#ifdef CONFIG_CMD_UBIFS

#define TQ_IMX_SHARED_UBI_ENV_SETTINGS                                 \
	"addubi="                                                      \
		"setenv bootargs ${bootargs} rootfstype=ubifs "        \
		"ubi.mtd=${ubimtdidx} "                                \
		"root=ubi0:${ubirootfsvol} ${rootfsmode} rootwait\0"   \
	"load_spi="                                                    \
		"if sf probe; then "                                   \
			"if ubi part ${ubirootfspart}; then "          \
				"if ubifsmount ubi0:${ubirootfsvol}; then "\
					"ubifsload ${kernel_addr_r} "  \
						"/boot/${image}; "     \
					"ubifsload ${fdt_addr_r} "     \
						"/boot/${fdtfile}; "   \
					"fdt address ${fdt_addr_r}; "  \
					"fdt resize 0x100000; "        \
					"for overlay in ${fdt_overlays}; do "\
						"ubifsload ${overlay_addr_r} "\
						"/boot/${overlay} && " \
						"fdt apply ${overlay_addr_r}; "\
					"done; "                       \
				"fi; "                                 \
				"ubifsumount; "                        \
			"fi; "                                         \
			"ubi detach; "                                 \
		"fi\0"                                                 \
	"prepare_ubi_part="                                            \
		"if sf probe; then "                                   \
			"mtd erase ${ubirootfspart}; "                 \
			"ubi part ${ubirootfspart}; "                  \
			"ubi create ${ubirootfsvol}; ubi detach; "     \
		"fi \0"                                                \
	"ubiargs=run addubi addtty\0"                                  \
	"ubiboot="                                                     \
		"echo Booting from UBI ...; "                          \
		"setenv bootargs; "                                    \
		"run ubiargs; "                                        \
		"if run load_spi; then "                               \
			"run boot_os; "                                \
		"else "                                                \
			"echo ERR: loading kernel; "                   \
		"fi;\0"                                                \
	"ubimtdidx=3\0"                                                \
	"ubirootfs=rootfs.ubifs\0"                                     \
	"ubirootfspart=ubi\0"                                          \
	"ubirootfsvol=rootfs\0"                                        \
	"update_rootfs_spi="                                           \
		"run check_ipaddr; "                                   \
		"setenv filesize; "                                    \
		"if tftp ${ubirootfs}; then "                          \
			"echo Write rootfs image to UBI ...; "         \
			"if sf probe; then "                           \
				"ubi part ${ubirootfspart}; "          \
				"ubi write ${loadaddr} "               \
					"${ubirootfsvol} "             \
					"${filesize}; "                \
				"ubi detach; "                         \
			"fi; "                                         \
		"fi; "                                                 \
		"setenv filesize \0"                                   \

#else
#define TQ_IMX_SHARED_UBI_ENV_SETTINGS
#endif

#define TQ_IMX_SHARED_SPI_ENV_SETTINGS                                 \
	"update_uboot_spi="                                            \
		"run check_ipaddr; "                                   \
		"setenv filesize; "                                    \
		"if tftp ${uboot}; then "                              \
			"if itest ${filesize} >= ${uboot_spi_size}; then "\
				"echo ERROR: size to large ...; "      \
				"exit; "                               \
			"fi; "                                         \
			"echo Write u-boot image to SPI NOR ...; "     \
			"if sf probe; then "                           \
				"run write_uboot_spi; "                \
			"fi; "                                         \
		"fi; "                                                 \
		"setenv filesize \0"                                   \
	TQ_IMX_SHARED_UBI_ENV_SETTINGS                                 \

#define TQ_IMX_SPI_UBOOT_UPDATE                                        \
	"write_uboot_spi="                                             \
		"sf update ${loadaddr} ${uboot_spi_start} "            \
			"${filesize} \0"                               \

#define TQ_IMX_LEGACY_SPI_UBOOT_UPDATE                                 \
	"write_uboot_spi="                                             \
		"if test ${uboot_spi_sector_size} = ''; then "         \
			"echo ERROR: uboot_spi_sector_size; "          \
			"exit; "                                       \
		"fi; "                                                 \
		"setexpr erase_range "                                 \
			"${filesize} + ${uboot_spi_start}; "           \
		"setexpr erase_range "                                 \
			"${erase_range} + "                            \
			"${uboot_spi_sector_size}; "                   \
		"setexpr erase_range "                                 \
			"${erase_range} - 1; "                         \
		"setexpr erase_sector_count "                          \
			"${erase_range} / "                            \
			"${uboot_spi_sector_size}; "                   \
		"setexpr erase_range "                                 \
			"${erase_sector_count} * "                     \
			"${uboot_spi_sector_size}; "                   \
		"sf erase 0 ${erase_range}; "                          \
		"sf write ${loadaddr} ${uboot_spi_start} "             \
			"${filesize}; "                                \
		"setenv erase_range; "                                 \
		"setenv erase_sector_count; \0"                        \

#else
#define TQ_IMX_SHARED_SPI_ENV_SETTINGS
#define TQ_IMX_SPI_UBOOT_UPDATE
#define TQ_IMX_LEGACY_SPI_UBOOT_UPDATE
#endif

#ifdef CONFIG_CMD_MMC

#define TQ_IMX_SHARED_MMC_ENV_SETTINGS                                 \
	"addmmc="                                                      \
		"setenv bootargs ${bootargs} "                         \
		"root=/dev/mmcblk${mmcblkdev}p${mmcrootpart} "         \
			"${rootfsmode} "                               \
		"rootwait\0"                                           \
	"load_mmc=mmc dev ${mmcdev}; mmc rescan; "                     \
		"load mmc ${mmcdev}:${mmcpart} ${kernel_addr_r} "      \
			"${mmcpath}${image}; "                         \
		"load mmc ${mmcdev}:${mmcpart} ${fdt_addr_r} "         \
			"${mmcpath}${fdtfile}; "                       \
		"fdt address ${fdt_addr_r};"                           \
		"fdt resize 0x100000;"                                 \
		"for overlay in ${fdt_overlays}; do "                  \
			"load mmc ${mmcdev}:${mmcpart} "               \
				"${overlay_addr_r} "                   \
				"${mmcpath}/${overlay} && "            \
			"fdt apply ${overlay_addr_r}; "                \
		"done;\0"                                              \
	"mmcargs=run addtty addmmc\0"                                  \
	"mmcboot=echo Booting from mmc ...; "                          \
		"setenv bootargs && "                                  \
		"run mmcargs && "                                      \
		"if run load_mmc; then "                               \
			"run boot_os; "                                \
		"else "                                                \
			"echo ERR: loading from mmc; "                 \
		"fi;\0"                                                \
	"mmcpath=/boot/\0"                                             \
	"mmcpart=2\0"                                                  \
	"mmcrootpart=2\0"                                              \
	"update_uboot_mmc="                                            \
		"run check_ipaddr; "                                   \
		"setenv filesize; "                                    \
		"if tftp ${uboot} ; then "                             \
			"echo Write U-Boot to mmc ${mmcdev} ...; "     \
			"mmc dev ${mmcdev}; mmc rescan; "              \
			"setexpr blkc ${filesize} + 0x1ff; "           \
			"setexpr blkc ${blkc} / 0x200; "               \
			"if itest ${blkc} <= ${uboot_mmc_size}; then " \
				"mmc write ${loadaddr} "               \
					"${uboot_mmc_start} ${blkc}; " \
			"fi; "                                         \
		"fi; "                                                 \
		"setenv filesize; setenv blkc \0"                      \

#else
#define TQ_IMX_SHARED_MMC_ENV_SETTINGS
#endif

#ifdef CONFIG_CMD_NFS

#define TQ_IMX_SHARED_NFS_ENV_SETTINGS                                 \
	"load_nfs="                                                    \
		"echo Booting from NFS ...; "                          \
		"nfs ${kernel_addr_r} ${serverip}:${rootpath}/boot/${image}; "\
		"nfs ${fdt_addr_r} ${serverip}:${rootpath}/boot/${fdtfile}; "\
		"fdt address ${fdt_addr_r};"                           \
		"fdt resize 0x100000;"                                 \
		"for overlay in ${fdt_overlays}; do "                  \
			"nfs ${overlay_addr_r} "                       \
				"${serverip}:${rootpath}/boot/${overlay} && "\
			"fdt apply ${overlay_addr_r}; "                \
		"done;\0"                                              \
	"nfsboot="                                                     \
		"echo Booting from nfs ...; "                          \
		"run check_ipaddr; "                                   \
		"setenv bootargs; run netargs; "                       \
		"if run load_nfs; then "                               \
			"run boot_os; "                                \
		"else "                                                \
			"echo ERR: loading from nfs; "                 \
		"fi;\0"                                                \

#else
#define TQ_IMX_SHARED_NFS_ENV_SETTINGS
#endif

#define TQ_IMX_SHARED_ENV_SETTINGS                                     \
	"addip="                                                       \
		"run check_ipaddr; "                                   \
		"setenv bootargs ${bootargs} "                         \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"    \
			"${hostname}:${netdev}:off\0"                  \
	"addnfs="                                                      \
		"setenv bootargs ${bootargs} "                         \
		"root=/dev/nfs rw "                                    \
		"nfsroot=${serverip}:${rootpath},v3,tcp\0"            \
	"addtty="                                                      \
		"setenv bootargs ${bootargs} "                         \
		"console=${console},${baudrate}\0"                     \
	"check_ipaddr="                                                \
		"if test -n \"${ipaddr}\" && test -n \"${serverip}\"; then " \
			"exit; "                                       \
		"fi; " \
		"echo 'ipaddr or serverip unset, falling back to DHCP...'; " \
		"dhcp\0"                                               \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"                        \
	"load_net="                                                    \
		"tftp ${kernel_addr_r} ${image}; "                     \
		"tftp ${fdt_addr_r} ${fdtfile}; "                      \
		"fdt address ${fdt_addr_r}; "                          \
		"fdt resize 0x100000;"                                 \
		"for overlay in ${fdt_overlays}; do "                  \
			"if tftp ${overlay_addr_r} ${overlay}; then "  \
				"fdt apply ${overlay_addr_r}; "        \
			"else "                                        \
				"exit; "                               \
			"fi; "                                         \
		"done;\0"                                              \
	"netboot="                                                     \
		"echo Booting from net ...; "                          \
		"setenv bootargs; "                                    \
		"run netargs; "                                        \
		"run check_ipaddr; "                                   \
		"if run load_net; then "                               \
			"run boot_os; "                                \
		"else "                                                \
			"echo ERR: loading from TFTP; "                \
		"fi;\0"                                                \
	"netargs=run addnfs addip addtty\0"                            \
	"rootpath=/srv/nfs\0"                                          \
	"rootfsmode=ro\0"                                              \
	TQ_IMX_SHARED_MMC_ENV_SETTINGS                                 \
	TQ_IMX_SHARED_NFS_ENV_SETTINGS                                 \
	TQ_IMX_SHARED_SPI_ENV_SETTINGS                                 \

#if !defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_DISTRO_DEFAULTS)

/* This needs > 32k env size */

#if CONFIG_IS_ENABLED(CMD_USB)
# define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
#endif

#if CONFIG_IS_ENABLED(CMD_PXE)
# define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
# define BOOT_TARGET_PXE(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOT_TARGET_UBIFS(func)	func(UBIFS, ubifs, 0, ubi, rootfs)
#else
#define BOOT_TARGET_UBIFS(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_MMC0(func)	func(MMC, mmc, 0)
#define BOOT_TARGET_MMC1(func)	func(MMC, mmc, 1)
#else
#define BOOT_TARGET_MMC0(func)
#define BOOT_TARGET_MMC1(func)
#endif

#define BOOT_TARGET_DEVICES(func)                                      \
	BOOT_TARGET_DHCP(func)                                         \
	BOOT_TARGET_MMC0(func)                                         \
	BOOT_TARGET_MMC1(func)                                         \
	BOOT_TARGET_PXE(func)                                          \
	BOOT_TARGET_UBIFS(func)                                        \
	BOOT_TARGET_USB(func)

#include <config_distro_bootcmd.h>

#endif /* CONFIG_DISTRO_DEFAULTS */

#endif /* CONFIG_SPL_BUILD */

#if !defined(BOOTENV)
#define BOOTENV
#endif

#endif /* __TQ_IMX_SHARED_ENV_H */
