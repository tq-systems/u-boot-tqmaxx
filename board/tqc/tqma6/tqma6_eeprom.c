/*
 * Copyright (C) 2014 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <malloc.h>
#include <linux/ctype.h>

#include "tqma6_eeprom.h"

int tqma6_parse_eeprom_mac(struct tqma6_eeprom_data *eeprom, char *buf,
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

int tqma6_parse_eeprom_serial(struct tqma6_eeprom_data *eeprom, char *buf,
			      size_t len)
{
	unsigned i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->serial) + 1))
		return -1;

	for (i = 0; i < (sizeof(eeprom->serial)) &&
		isdigit(eeprom->serial[i]); i++)
		buf[i] = eeprom->serial[i];
	buf[i] = '\0';
	if (sizeof(eeprom->serial) != strlen(buf))
		return -1;

	return 0;
}

int tqma6_parse_eeprom_id(struct tqma6_eeprom_data *eeprom, char *buf,
			  size_t len)
{
	unsigned i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->id) + 1))
		return -1;

	for (i = 0; i < sizeof(eeprom->id) &&
		isprint(eeprom->id[i]) && isascii(eeprom->id[i]); ++i)
		buf[i] = eeprom->id[i];
	buf[i] = '\0';

	return 0;
}

/*
 * show_eeprom - display the contents of the module EEPROM
 */
int tqma6_show_eeprom(struct tqma6_eeprom_data *eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	if (!eeprom)
		return -1;

	puts(id);
	puts(" EEPROM:\n");
	/* ID */
	tqma6_parse_eeprom_id(eeprom, safe_string,
			      ARRAY_SIZE(safe_string));
	if (0 == strncmp(safe_string, id, strlen(id)))
		printf("  ID: %s\n", safe_string);
	else
		puts("  unknown hardware variant\n");

	/* Serial number */
	if (0 == tqma6_parse_eeprom_serial(eeprom, safe_string,
					   ARRAY_SIZE(safe_string)))
		printf("  SN: %s\n", safe_string);
	else
		puts("  unknown serial number\n");
	/* MAC address */
	if (0 == tqma6_parse_eeprom_mac(eeprom, safe_string,
					ARRAY_SIZE(safe_string)))
		printf("  MAC: %s\n", safe_string);
	else
		puts("  invalid MAC\n");

	return 0;
}

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tqma6_read_eeprom(unsigned int bus, unsigned int addr,
		      struct tqma6_eeprom_data *eeprom)
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

