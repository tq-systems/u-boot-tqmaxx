// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Matthias Schiffer
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <i2c.h>
#include <linux/ctype.h>
#include <net.h>

#include "tq_eeprom.h"

int tq_parse_eeprom_mac(const struct tq_eeprom_data *eeprom, u8 buf[6])
{
	memcpy(buf, eeprom->mac, 6);
	return !is_valid_ethaddr(buf);
}

int tq_parse_eeprom_serial(const struct tq_eeprom_data *eeprom,
			   char *buf, size_t len)
{
	int i;

	if (len < sizeof(eeprom->serial) + 1)
		return -EINVAL;

	memcpy(buf, eeprom->serial, sizeof(eeprom->serial));

	for (i = 0; i < sizeof(eeprom->serial); i++) {
		if (!(isdigit(buf[i]) || isupper(buf[i])))
			break;
	}
	buf[i] = '\0';

	return 0;
}

int tq_parse_eeprom_id(const struct tq_eeprom_data *eeprom,
		       char *buf, size_t len)
{
	int i;

	if (len < sizeof(eeprom->id) + 1)
		return -EINVAL;

	memcpy(buf, eeprom->id, sizeof(eeprom->id));

	for (i = 0; i < sizeof(eeprom->id); i++) {
		if (!(isprint(buf[i]) && isascii(buf[i])))
			break;
	}
	buf[i] = '\0';

	return 0;
}

int tq_show_eeprom(const struct tq_eeprom_data *eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char buf[TQ_EE_BDID_BYTES + 1];
	u8 mac[6];

	printf("%s EEPROM:\n", id);

	/* ID */
	tq_parse_eeprom_id(eeprom, buf, sizeof(buf));
	if (strncmp(buf, id, strlen(id)) == 0)
		printf("  ID: %s\n", buf);
	else
		printf("  unknown hardware variant\n");

	/* Serial number */
	if (tq_parse_eeprom_serial(eeprom, buf, sizeof(buf)) == 0)
		printf("  SN: %s\n", buf);
	else
		printf("  unknown serial number\n");

	/* MAC address */
	if (tq_parse_eeprom_mac(eeprom, mac) == 0)
		printf("  MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	else
		printf("  invalid MAC\n");

	return 0;
}

int tq_board_handle_eeprom_data(const char *board_name,
				const struct tq_eeprom_data *eeprom)
{
	char buf[TQ_EE_BDID_BYTES + 1];

	tq_parse_eeprom_id(eeprom, buf, sizeof(buf));

	if (strncmp(buf, board_name, strlen(board_name)) == 0)
		env_set_runtime("boardtype", buf);

	if (tq_parse_eeprom_serial(eeprom, buf, sizeof(buf)) == 0)
		env_set_runtime("serial#", buf);
	else
		env_set_runtime("serial#", "???");

	return tq_show_eeprom(eeprom, board_name);
}

#if CONFIG_IS_ENABLED(I2C_EEPROM)
int tq_read_eeprom_at(int seq, uint offset, struct tq_eeprom_data *eeprom)
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

#else /* !I2C_EEPROM */
int tq_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		      unsigned int addr, int alen,
		      struct tq_eeprom_data *eeprom)
{
	unsigned int oldbus;
	int ret;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(bus);
	ret = i2c_read(i2c_addr, addr, alen, (u8 *)eeprom, sizeof(eeprom));
	i2c_set_bus_num(oldbus);

	return ret;
}

#endif /* !I2C_EEPROM */
