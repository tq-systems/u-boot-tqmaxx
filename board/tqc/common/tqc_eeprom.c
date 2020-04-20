// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 - 2020 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <i2c.h>
#include <malloc.h>
#include <linux/ctype.h>

#include "tqc_eeprom.h"

int tqc_parse_eeprom_mac(struct tqc_eeprom_data *eeprom, char *buf,
			 size_t len)
{
	u8 *p;
	int ret;

	if (!buf || !eeprom)
		return -1;
	/* MAC address */
	p = eeprom->mac;
	ret = snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x",
		       p[0], p[1], p[2], p[3], p[4], p[5]);
	if (ret < 0)
		return ret;
	if (ret >= len)
		return ret;

	return 0;
}

int tqc_parse_eeprom_serial(struct tqc_eeprom_data *eeprom, char *buf,
			    size_t len)
{
	unsigned int i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->serial) + 1))
		return -1;

	for (i = 0; i < (sizeof(eeprom->serial)) && isdigit(eeprom->serial[i]);
	     i++)
		buf[i] = eeprom->serial[i];
	buf[i] = '\0';
	if (sizeof(eeprom->serial) != strlen(buf))
		return -1;

	return 0;
}

int tqc_parse_eeprom_id(struct tqc_eeprom_data *eeprom, char *buf,
			size_t len)
{
	unsigned int i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->id) + 1))
		return -1;

	for (i = 0; i < sizeof(eeprom->id) && isprint(eeprom->id[i]) &&
	     isascii(eeprom->id[i]); ++i)
		buf[i] = eeprom->id[i];
	buf[i] = '\0';

	return 0;
}

/*
 * show_eeprom - display the contents of the module EEPROM
 */
int tqc_show_eeprom(struct tqc_eeprom_data *eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	if (!eeprom)
		return -1;

	puts(id);
	puts(" EEPROM:\n");
	/* ID */
	tqc_parse_eeprom_id(eeprom, safe_string, ARRAY_SIZE(safe_string));
	if (strncmp(safe_string, id, strlen(id)) == 0)
		printf("  ID: %s\n", safe_string);
	else
		puts("  unknown hardware variant\n");

	/* Serial number */
	if (tqc_parse_eeprom_serial(eeprom, safe_string,
				    ARRAY_SIZE(safe_string)) == 0)
		printf("  SN: %s\n", safe_string);
	else
		puts("  unknown serial number\n");
	/* MAC address */
	if (tqc_parse_eeprom_mac(eeprom, safe_string,
				 ARRAY_SIZE(safe_string)) == 0)
		printf("  MAC: %s\n", safe_string);
	else
		puts("  invalid MAC\n");

	return 0;
}

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tqc_read_eeprom(unsigned int bus, unsigned int addr,
		    struct tqc_eeprom_data *eeprom)
{
	int ret;
	unsigned int oldbus;

	if (!eeprom)
		return -1;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(bus);
	ret = i2c_read(addr, 0, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		       (uchar *)eeprom, sizeof(*eeprom));
	i2c_set_bus_num(oldbus);
	return ret;
}

