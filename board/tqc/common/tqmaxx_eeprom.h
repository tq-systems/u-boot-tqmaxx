/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014-2018 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 *
 */

#ifndef __TQMAXX_EEPROM_H__
#define __TQMAXX_EEPROM_H__

/*
 * static EEPROM layout
 */
struct __attribute__ ((__packed__)) tqmaxx_eeprom_data {
	u8 hrcw_primary[0x20];
	u8 mac[6];		/* 0x20 ... 0x25 */
	u8 rsv1[10];
	u8 serial[8];		/* 0x30 ... 0x37 */
	u8 rsv2[8];
	u8 id[0x40];		/* 0x40 ... 0x7f */
};

int tqmaxx_parse_eeprom_mac(struct tqmaxx_eeprom_data *eeprom, char *buf,
			    size_t len);
int tqmaxx_parse_eeprom_mac_additional(struct tqmaxx_eeprom_data *eeprom,
				       char *buf, size_t len,
				       size_t additional, char *format_string);
int tqmaxx_parse_eeprom_serial(struct tqmaxx_eeprom_data *eeprom, char *buf,
			       size_t len);
int tqmaxx_parse_eeprom_id(struct tqmaxx_eeprom_data *eeprom, char *buf,
			   size_t len);
int tqmaxx_show_eeprom(struct tqmaxx_eeprom_data *eeprom, const char *id);
int tqmaxx_read_eeprom_at(unsigned int bus, unsigned int addr,
			  struct tqmaxx_eeprom_data *eeprom,
			  unsigned int offset);
int tqmaxx_read_eeprom(unsigned int bus, unsigned int addr,
		       struct tqmaxx_eeprom_data *eeprom);

#endif
