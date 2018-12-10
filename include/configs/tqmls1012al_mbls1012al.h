/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the TQ Systems MBLS1012AL Carrier board
 * for TQMLS1012AL
 *
 * Copyright (C) 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#ifndef __CONFIG_TQMLS1012AL_MBLS1012AL_H__
#define __CONFIG_TQMLS1012AL_MBLS1012AL_H__

/* e-MMC / SD */
#define CONFIG_FSL_ESDHC

#ifdef CONFIG_TQMLS1012AL_EMMC
#define ESDHC_FIXED_HOSTCAPBLT_VS18
#endif

/* Ethernet */
#define MBLS1012AL_KSZ_I2C_ADDR	0x5f
#define MBLS1012AL_KSZ_ADDR		0x7301
#define MBLS1012AL_KSZ_VALUE		0x18

#ifdef CONFIG_FSL_PFE
#define EMAC1_PHY_ADDR          0x9
#define EMAC2_PHY_ADDR          0x1
#define DEFAULT_PFE_MDIO_NAME "PFE_MDIO"
#define DEFAULT_PFE_MDIO1_NAME "PFE_MDIO1"
#endif

#ifdef CONFIG_PHY_FIXED
#define FIXED_LINK_ADDR        0x1
#define FIXED_LINK_INTERFACE   PHY_INTERFACE_MODE_RGMII
#define FIXED_LINK_SPEED       1000
#define FIXED_LINK_FULL_DUPLEX 1
#define FIXED_LINK_PAUSE       1
#define FIXED_LINK_ASYM_PAUSE  1
#endif

/* SATA */
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA			AHCI_BASE_ADDR
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)

/* PCIE */
#define CONFIG_PCIE1		/* PCIE controller 1 */

#define CONFIG_PCI_SCAN_SHOW

#endif /* __CONFIG_TQMLS1012AL_MBLS1012AL_H__ */
