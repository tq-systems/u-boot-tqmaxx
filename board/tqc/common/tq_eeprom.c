// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2022 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 * Alexander Stein <Alexander.Stein@tq-group.com>
 */

#include <common.h>
#include <i2c.h>
#include <malloc.h>
#include <net.h>
#include <linux/ctype.h>

#include "tq_eeprom.h"

void tq_set_ethaddr(struct tq_eeprom_data const *eedat,
		    const char *env_var,
		    size_t additional)
{
	char safe_string[0x41];

	if (tq_parse_eeprom_mac_additional(eedat, safe_string,
					   ARRAY_SIZE(safe_string),
					   additional) == 0) {
		char *ethaddr = env_get(env_var);

		if (ethaddr &&
		    strncmp(safe_string, ethaddr, 18)) {
			printf("\n");
			printf("Warning: MAC addresses for '%s' don't match:\n",
			       env_var);
			printf("Address in EEPROM is      %s\n",
			       safe_string);
			printf("Address in environment is %s\n",
			       ethaddr);
		} else {
			env_set(env_var, safe_string);
		}
	}
}

int tq_parse_eeprom_mac_additional(struct tq_eeprom_data const *eeprom,
				   char *buf, size_t len,
				   size_t additional)
{
	u8 const *p;
	int ret = -1;
	u32 addr;

	if (!buf || !eeprom)
		return -1;

	p = eeprom->mac;

	addr = p[3] << 16 | p[4] << 8 | p[5];
	addr += additional;
	addr = addr & 0x00ffffff;

	ret = snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x",
		       p[0], p[1], p[2], (addr >> 16) & 0xff,
		       (addr >> 8) & 0xff, addr & 0xff);

	if (ret < 0)
		return ret;
	if (ret >= len)
		return ret;

	return !(is_valid_ethaddr(p));
}

int tq_parse_eeprom_mac(struct tq_eeprom_data const *eeprom, char *buf,
			size_t len)
{
	return tq_parse_eeprom_mac_additional(eeprom,
					      buf,
					      len,
					      0);
}

int tq_parse_eeprom_serial(struct tq_eeprom_data *eeprom, char *buf,
			   size_t len)
{
	unsigned int i;

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

int tq_parse_eeprom_id(struct tq_eeprom_data *eeprom, char *buf,
		       size_t len)
{
	unsigned int i;

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
int tq_show_eeprom(struct tq_eeprom_data *eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	if (!eeprom)
		return -1;

	puts(id);
	puts(" EEPROM:\n");
	/* ID */
	tq_parse_eeprom_id(eeprom, safe_string,
			   ARRAY_SIZE(safe_string));
	if (strncmp(safe_string, id, strlen(id)) == 0)
		printf("  ID: %s\n", safe_string);
	else
		puts("  unknown hardware variant\n");

	/* Serial number */
	if (tq_parse_eeprom_serial(eeprom, safe_string,
				   ARRAY_SIZE(safe_string)) == 0)
		printf("  SN: %s\n", safe_string);
	else
		puts("  unknown serial number\n");
	/* MAC address */
	if (tq_parse_eeprom_mac(eeprom, safe_string,
				ARRAY_SIZE(safe_string)) == 0)
		printf("  MAC: %s\n", safe_string);
	else
		puts("  invalid MAC\n");

	return 0;
}

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tq_read_eeprom_at(unsigned int bus, unsigned int addr,
		      struct tq_eeprom_data *eeprom,
		      unsigned int offset)
{
	int ret;
#if IS_ENABLED(CONFIG_DM_I2C)
	struct udevice *dev;
#else
	unsigned int oldbus;
#endif

	if (!eeprom)
		return -1;

#if IS_ENABLED(CONFIG_DM_I2C)
	ret = i2c_get_chip_for_busnum(bus, addr, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				      &dev);
	if (ret) {
		debug("%s: Cannot find I2C chip for bus %d\n", __func__, bus);
		return ret;
	}

	ret = dm_i2c_read(dev, offset, (uchar *)eeprom, sizeof(*eeprom));
#else
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(bus);
	ret = i2c_read(addr, offset, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		       (uchar *)eeprom, sizeof(*eeprom));
	i2c_set_bus_num(oldbus);
#endif
	return ret;
}

int tq_read_eeprom(unsigned int bus, unsigned int addr,
		   struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom_at(bus, addr, eeprom, 0);
}
