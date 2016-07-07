/*
 * Copyright (C) 2014 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMA6_EEPROM_H__
#define __TQMA6_EEPROM_H__

/*
 * static EEPROM layout
 */
struct __attribute__ ((__packed__)) tqma6_eeprom_data {
	u8 hrcw_primary[0x20];
	u8 mac[6];		/* 0x20 ... 0x25 */
	u8 rsv1[10];
	u8 serial[8];		/* 0x30 ... 0x37 */
	u8 rsv2[8];
	u8 id[0x40];		/* 0x40 ... 0x7f */
};

int tqma6_parse_eeprom_mac(struct tqma6_eeprom_data *eeprom, char *buf,
			   size_t len);

int tqma6_parse_eeprom_serial(struct tqma6_eeprom_data *eeprom, char *buf,
			      size_t len);
int tqma6_parse_eeprom_id(struct tqma6_eeprom_data *eeprom, char *buf,
			  size_t len);
int tqma6_show_eeprom(struct tqma6_eeprom_data *eeprom, const char *id);
int tqma6_read_eeprom(unsigned int bus, unsigned int addr,
		      struct tqma6_eeprom_data *eeprom);


#endif
