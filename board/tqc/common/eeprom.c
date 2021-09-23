// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2014-2022 TQ-Systems GmbH
 * Authors: Markus Niebel <Markus.Niebel@tq-group.com>
 *          Matthias Schiffer <matthias.schiffer@tq.tq-group.com>
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <net.h>

#include "eeprom.h"

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

bool tq_get_eeprom_id(const struct tq_eeprom_data *eeprom, char *buf)
{
	int i;

	for (i = 0; i < sizeof(eeprom->id); i++) {
		if (!(isprint(eeprom->id[i]) && isascii(eeprom->id[i])))
			break;
	}
	if (i == 0)
		return false;

	memcpy(buf, eeprom->id, i);
	buf[i] = '\0';

	return true;
}

bool tq_get_eeprom_serial(const struct tq_eeprom_data *eeprom, char *buf)
{
	int i;

	for (i = 0; i < sizeof(eeprom->serial); i++) {
		if (!isdigit(eeprom->serial[i]))
			break;
	}
	if (i == 0)
		return false;

	memcpy(buf, eeprom->serial, i);
	buf[i] = '\0';

	return true;
}

bool tq_get_eeprom_mac(const struct tq_eeprom_data *eeprom, uint8_t *macaddr)
{
	if (!is_valid_ethaddr(eeprom->mac))
		return false;

	memcpy(macaddr, eeprom->mac, 6);
	return true;
}

void tq_show_eeprom(const struct tq_eeprom_data *eeprom, const char *id_prefix)
{
	char id[TQ_ID_STRLEN], serial[TQ_SERIAL_STRLEN];
	u8 macaddr[6];

	printf("EEPROM:\n");

	if (tq_get_eeprom_id(eeprom, id))
		printf("  ID: %s\n", id);
	else
		printf("  unknown hardware variant\n");

	if (strncmp(id_prefix, id, strlen(id_prefix)) != 0)
		printf("  Warning: EEPROM ID does not match expected module type %s\n",
		       id_prefix);

	if (tq_get_eeprom_serial(eeprom, serial))
		printf("  SN: %s\n", serial);
	else
		printf("  unknown serial number\n");

	if (tq_get_eeprom_mac(eeprom, macaddr))
		printf("  MAC: %pM\n", macaddr);
	else
		printf("  invalid MAC\n");
}
