/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 - 2018 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#ifndef __TQC_EEPROM_H__
#define __TQC_EEPROM_H__

int tqc_read_eeprom_buf(unsigned int bus, unsigned int i2c_addr,
			unsigned int alen, unsigned int addr,
			size_t bsize, uchar *buf);

/*
 * static EEPROM layout
 * Bytes 0..31 Variant And Revision Detection
 */
struct __attribute__ ((__packed__)) tqc_eeprom_data {
	uint16_t crc;		/* checksum of vard data */
	u8 hwrev;
	u8 memsize;
	u8 memtype;
	u32 features1;
	u32 features2;
	u8 reserved[0x13];
	u8 mac[6];		/* 0x20 ... 0x25 */
	u8 rsv1[10];
	u8 serial[8];		/* 0x30 ... 0x37 */
	u8 rsv2[8];
	u8 id[0x40];		/* 0x40 ... 0x7f */
};

int tqc_has_hwrev(u8 rev);
int tqc_has_memsize(u8 size);
int tqc_has_memtype(u8 type);
int tqc_has_feature1(u32 mask);
int tqc_has_feature2(u32 mask);

#if !defined(CONFIG_SPL_BUILD)

int tqc_parse_eeprom_mac(struct tqc_eeprom_data *eeprom, char *buf,
			 size_t len);

int tqc_parse_eeprom_serial(struct tqc_eeprom_data *eeprom, char *buf,
			    size_t len);
int tqc_parse_eeprom_id(struct tqc_eeprom_data *eeprom, char *buf,
			size_t len);
int tqc_show_eeprom(struct tqc_eeprom_data *eeprom, const char *id);
int tqc_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		       unsigned int alen, unsigned int addr,
		       struct tqc_eeprom_data *eeprom);
#if defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN)
int tqc_read_eeprom(unsigned int bus, unsigned int i2c_addr,
		    unsigned int addr, struct tqc_eeprom_data *eeprom);
#endif /* CONFIG_SYS_I2C_EEPROM_ADDR_LEN */

#endif /* CONFIG_SPL_BUILD */

#endif
