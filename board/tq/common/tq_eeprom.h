/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2014-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Matthias Schiffer
 */

#ifndef __TQ_EEPROM_H__
#define __TQ_EEPROM_H__

/*
 * static EEPROM layout
 */
#define TQ_EE_HRCW_BYTES		0x20
#define TQ_EE_MAC_BYTES			6
#define TQ_EE_RSV1_BYTES		10
#define TQ_EE_SERIAL_BYTES		8
#define TQ_EE_RSV2_BYTES		8
#define TQ_EE_BDID_BYTES		0x40

struct __packed tq_eeprom_data {
	u8 hrcw_primary[TQ_EE_HRCW_BYTES];
	u8 mac[TQ_EE_MAC_BYTES];		/* 0x20 ... 0x25 */
	u8 rsv1[TQ_EE_RSV1_BYTES];
	u8 serial[TQ_EE_SERIAL_BYTES];		/* 0x30 ... 0x37 */
	u8 rsv2[TQ_EE_RSV2_BYTES];
	u8 id[TQ_EE_BDID_BYTES];		/* 0x40 ... 0x7f */
};

/**
 * Parse MAC address from module EEPROM to buf, respect buf len
 */
int tq_parse_eeprom_mac(const struct tq_eeprom_data *eeprom, u8 buf[6]);

/**
 * Parse BCD coded serial number from module EEPROM to buf,
 * respect buf len
 */
int tq_parse_eeprom_serial(const struct tq_eeprom_data *eeprom, char *buf,
			   size_t len);

/**
 * Parse module name string number from module EEPROM to buf,
 * respect buf len
 */
int tq_parse_eeprom_id(const struct tq_eeprom_data *eeprom, char *buf,
		       size_t len);

/**
 * display the contents of the module EEPROM
 * (MAC, serial number, module name/variant string)
 * report error, if module name doesn't match id string or malformed
 * MAC / serial number
 */
int tq_show_eeprom(const struct tq_eeprom_data *eeprom, const char *id);

/**
 * verify and display contents of the module EEPROM
 * optionally set 'serial' and 'board_type env' if
 * CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG in enabled
 */
int tq_board_handle_eeprom_data(const char *board_name,
				const struct tq_eeprom_data *eeprom);

#if CONFIG_IS_ENABLED(I2C_EEPROM)

/**
 * Reads struct tq_eeprom_data from EEPROM given by seq nr
 * starting at offset in EEPROM array.
 */
int tq_read_eeprom_at(int seq, uint offset,
		      struct tq_eeprom_data *eeprom);

/**
 * Reads struct tq_eeprom_data from EEPROM given by seq nr
 * starting at offset zero in EEPROM array.
 */
static inline int tq_read_eeprom(int seq, struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom_at(seq, 0, eeprom);
}

/**
 * Reads struct tq_eeprom_data from EEPROM with seq nr zero
 * starting at offset zero in EEPROM array.
 */
static inline int tq_read_module_eeprom(struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom(0, eeprom);
}

#else /* !I2C_EEPROM */

/*
 * Older versions, for use in SPL without DM support
 */

int tq_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		      unsigned int addr, int alen,
		      struct tq_eeprom_data *eeprom);

static inline int tq_read_module_eeprom(struct tq_eeprom_data *eeprom)
{
#if defined(CONFIG_SYS_I2C_EEPROM_BUS) && \
		defined(CONFIG_SYS_I2C_EEPROM_ADDR) && \
		defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN)
	return tq_read_eeprom_at(CONFIG_SYS_I2C_EEPROM_BUS,
				 CONFIG_SYS_I2C_EEPROM_ADDR,
				 CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				 0, eeprom);
#else
	return -ENOSYS;
#endif
}

#endif /* !I2C_EEPROM */

#endif
