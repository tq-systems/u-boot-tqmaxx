// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Authors: Markus Niebel <Markus.Niebel@tq-group.com>
 *          Matthias Schiffer <matthias.schiffer@tq.tq-group.com>
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <net.h>
#include <u-boot/crc.h>

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

#if defined(CONFIG_TQ_VARD)

/*
 * checksum is calculated over whole structure but the CRC field
 */
static uint16_t tq_vard_chksum(const struct tq_vard *vard)
{
	const unsigned char *start = (const unsigned char *)(vard) +
		sizeof(vard->crc);

	return crc16_ccitt(0, start, sizeof(*vard) - sizeof(vard->crc));
}

bool tq_vard_valid(const struct tq_vard *vard)
{
	return (vard->crc == tq_vard_chksum(vard));
}

phys_size_t tq_vard_memsize(u8 val, unsigned int multiply, unsigned int tmask)
{
	phys_size_t result = 0;

	if (val != VARD_MEMSIZE_DEFAULT) {
		result = 1 << (val & VARD_MEMSIZE_MASK_EXP);
		if (val & tmask)
			result *= 3;
		result *= multiply;
	}

	return result;
}

bool tq_vard_show(const struct tq_vard *vard)
{
	bool valid = tq_vard_valid(vard);

	printf("  VARD CRC: %04x (calculated %04x) [%s]\n",
	       vard->crc, tq_vard_chksum(vard),
	       valid ? "OKAY" : "FAIL");
	/* display data anyway to support developer */
	printf("  HW REV:   %02uxx\n", vard->hwrev);

	printf("  RAM:      type %u, %lu MiB, %s\n",
	       (vard->memtype & VARD_MEMTYPE_MASK_TYPE),
	       (unsigned long)(tq_vard_ramsize(vard) / SZ_1M),
	       (tq_vard_has_ramecc(vard) ? "ECC" : "no ECC"));

	printf("  RTC:      %s\n", tq_vard_has_rtc(vard) ? "yes" : "no");
	printf("  SPI-NOR:  %s\n", tq_vard_has_spinor(vard) ? "yes" : "no");
	printf("  eMMC:     %s\n", tq_vard_has_emmc(vard) ? "yes" : "no");
	printf("  SE:       %s\n", tq_vard_has_secelem(vard) ? "yes" : "no");

	printf("  EEPROM:   ");
	if (tq_vard_has_eeprom(vard))
		printf("type %u, %lu KiB, pagesize %lu\n",
		       (vard->eepromtype & VARD_EETYPE_MASK_MFR) >> 4,
		       (unsigned long)(tq_vard_eepromsize(vard) / SZ_1K),
		       (unsigned long)tq_vard_eeprom_pgsize(vard));
	else
		printf("no\n");

	printf("\n");

	return valid;
}

#endif
