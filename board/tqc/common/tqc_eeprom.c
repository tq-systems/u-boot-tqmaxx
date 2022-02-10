// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2014 - 2022 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <i2c.h>
#include <linux/ctype.h>
#include <malloc.h>

#include "tqc_eeprom.h"

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tqc_read_eeprom_buf(unsigned int bus, unsigned int i2c_addr,
			unsigned int alen, unsigned int addr,
			size_t bsize, uchar *buf)
{
	int ret;
#ifdef CONFIG_DM_I2C
	struct udevice *dev;
#else
	unsigned int oldbus;
#endif

	if (!buf)
		return -1;

#ifdef CONFIG_DM_I2C
	ret = i2c_get_chip_for_busnum(bus, i2c_addr, alen, &dev);
	if (ret) {
		debug("%s: Cannot find I2C chip for bus %d\n", __func__, bus);
		return ret;
	}

	ret = dm_i2c_read(dev, addr, buf, bsize);
#else
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(bus);
	ret = i2c_read(i2c_addr, addr, alen, buf, bsize);
	i2c_set_bus_num(oldbus);
#endif
	return ret;
}

#if !defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_I2C_EEPROM)
int tq_read_eeprom_at(int seq, uint offset, struct tqc_eeprom_data *eeprom)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C_EEPROM, seq, &dev);
	if (ret) {
		debug("%s: Cannot find i2c_eeprom%d\n", __func__, seq);
		return ret;
	}

	return i2c_eeprom_read(dev, offset, (u8 *)eeprom, sizeof(*eeprom));
}
#endif

int tqc_parse_eeprom_mac(struct tqc_eeprom_data * const eeprom, char *buf,
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

	return !(is_valid_ethaddr(p));
}

int tqc_parse_eeprom_serial(struct tqc_eeprom_data * const eeprom, char *buf,
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

int tqc_parse_eeprom_id(struct tqc_eeprom_data * const eeprom, char *buf,
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
int tqc_show_eeprom(struct tqc_eeprom_data * const eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[(TQC_EE_BDID_BYTES) + 1];

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
int tqc_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		       unsigned int alen, unsigned int addr,
		       struct tqc_eeprom_data *eeprom)
{
	return tqc_read_eeprom_buf(bus, i2c_addr, alen, addr, sizeof(*eeprom),
				   (uchar *)eeprom);
}

#if defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN)
int tqc_read_eeprom(unsigned int bus, unsigned int addr,
		    struct tqc_eeprom_data *eeprom)
{
	return tqc_read_eeprom_at(bus, addr,
				  CONFIG_SYS_I2C_EEPROM_ADDR_LEN, addr, eeprom);
}
#endif

int tqc_board_handle_eeprom_data(const char *board_name,
				 struct tqc_eeprom_data * const eeprom)
{
	char sstring[(TQC_EE_BDID_BYTES) + 1];

	tqc_parse_eeprom_id(eeprom, sstring, ARRAY_SIZE(sstring));

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (strncmp(sstring, board_name, strlen(board_name)) == 0)
		env_set("boardtype", sstring);
	if (tqc_parse_eeprom_serial(eeprom, sstring,
				    ARRAY_SIZE(sstring)) == 0)
		env_set("serial#", sstring);
	else
		env_set("serial#", "???");
#endif

	return tqc_show_eeprom(eeprom, board_name);
}

#endif
