// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <i2c.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <net.h>
#include <u-boot/crc.h>

#include "tq_eeprom.h"
#include "tq_som_features.h"

/**
 * Reads buffer from EEPROM given by seq nr starting at offset
 */
int tq_read_eeprom_buffer(int seq, uint offset, int buf_size, u_int8_t *buf)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C_EEPROM, seq, &dev);
	if (ret) {
		debug("%s: Cannot find i2c_eeprom%d\n", __func__, seq);
		return ret;
	}

	return i2c_eeprom_read(dev, offset, (u8 *)buf, buf_size);
};

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
		result = 1 << (size_t)(val & VARD_MEMSIZE_MASK_EXP);
		if (val & tmask)
			result *= 3;
		result *= multiply;
	}

	return result;
}

void tq_vard_show(const struct tq_vard *vard)
{
	printf("CRC\t%04x (calculated %04x) [%s]\n",
	       (unsigned int)vard->crc, (unsigned int)tq_vard_chksum(vard),
	       tq_vard_valid(vard) ? "OKAY" : "FAIL");
	/* display data anyway to support developer */
	printf("HW\tREV.%02uxx\n", (unsigned int)vard->hwrev);
	printf("RAM\ttype %u, %lu MiB, %s\n",
	       (unsigned int)(vard->memtype & VARD_MEMTYPE_MASK_TYPE),
	       (unsigned long)(tq_vard_ramsize(vard) / (SZ_1M)),
	       (tq_vard_has_ramecc(vard) ? "ECC" : "no ECC"));
	printf("RTC\t%c\nSPINOR\t%c\ne-MMC\t%c\nSE\t%c\nEEPROM\t%c\n",
	       (tq_vard_has_rtc(vard) ? 'y' : 'n'),
	       (tq_vard_has_spinor(vard) ? 'y' : 'n'),
	       (tq_vard_has_emmc(vard) ? 'y' : 'n'),
	       (tq_vard_has_secelem(vard) ? 'y' : 'n'),
	       (tq_vard_has_eeprom(vard) ? 'y' : 'n'));
	if (tq_vard_has_eeprom(vard))
		printf("EEPROM\ttype %u, %lu KiB, page %lu\n",
		       (unsigned int)(vard->eepromtype & VARD_EETYPE_MASK_MFR) >> 4,
		       (unsigned long)(tq_vard_eepromsize(vard) / (SZ_1K)),
		       tq_vard_eeprom_pgsize(vard));
	puts("FORMFACTOR: ");
	switch (tq_vard_get_formfactor(vard)) {
	case VARD_FORMFACTOR_TYPE_LGA:
		puts("LGA\n");
		break;
	case VARD_FORMFACTOR_TYPE_CONNECTOR:
		puts("CONNECTOR\n");
		break;
	case VARD_FORMFACTOR_TYPE_SMARC2:
		puts("SMARC-2\n");
		break;
	case VARD_FORMFACTOR_TYPE_NONE:
		/*
		 * applies to boards with no variants or older boards
		 * where this field is not written
		 */
		puts("UNSPECIFIED\n");
		break;
	default:
		/*
		 * generic fall trough
		 * unhandled form factor or invalid data
		 */
		puts("UNKNOWN\n");
		break;
	}
}

int tq_vard_detect_features(const struct tq_vard *vard,
			    struct tq_som_feature_list *features)
{
	size_t i;

	if (!vard || !features) {
		pr_err("%s: invalid parameter\n", __func__);
		return -EINVAL;
	}

	if (!tq_vard_valid(vard)) {
		pr_err("%s: VARD invalid\n", __func__);
		return -ENODATA;
	}

	for (i = 0; i < features->entries; ++i) {
		switch (features->list[i].feature) {
		case FEATURE_EEPROM:
			features->list[i].present = tq_vard_has_eeprom(vard);
			break;
		case FEATURE_EMMC:
			features->list[i].present = tq_vard_has_emmc(vard);
			break;
		case FEATURE_IMU:
			features->list[i].present = tq_vard_has_imu(vard);
			break;
		case FEATURE_RTC:
			features->list[i].present = tq_vard_has_rtc(vard);
			break;
		case FEATURE_SECELEM:
			features->list[i].present = tq_vard_has_secelem(vard);
			break;
		case FEATURE_SPINOR:
			features->list[i].present = tq_vard_has_spinor(vard);
			break;
		default:
			pr_warn("%s: unknown feature %d\n", __func__,
				features->list[i].feature);
		}
	}

	return 0;
}

#endif

#if !defined(CONFIG_SPL_BUILD)

/**
 * Reads struct tq_eeprom_data from EEPROM given by seq nr
 * starting at offset in EEPROM array.
 */
int tq_read_eeprom_at(int seq, uint offset, struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom_buffer(seq, offset, sizeof(*eeprom),
				      (u_int8_t *)eeprom);
}

int tq_parse_eeprom_mac(struct tq_eeprom_data * const eeprom, char *buf,
			size_t len)
{
	int ret;
	u8 *p;

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

static int tq_parse_eeprom_serial_new(struct tq_eeprom_data * const eeprom,
				        char *buf)
{
	unsigned int i;

	for (i = 0; i < (sizeof(eeprom->serial)) &&
	     (isdigit(eeprom->serial[i]) || isupper(eeprom->serial[i])); i++)
		buf[i] = eeprom->serial[i];
	buf[i] = '\0';
	if (sizeof(eeprom->serial) != strlen(buf))
		return -EINVAL;

	return 0;
}

static int tq_parse_eeprom_serial_old(struct tq_eeprom_data * const eeprom,
				        char *buf)
{
	unsigned int i;

	for (i = 0; i < (sizeof(eeprom->serial)) && isdigit(eeprom->serial[i]);
	     i++)
		buf[i] = eeprom->serial[i];
	buf[i] = '\0';
	if (sizeof(eeprom->serial) != strlen(buf))
		return -EINVAL;

	return 0;
}

int tq_parse_eeprom_serial(struct tq_eeprom_data * const eeprom,
			    char *buf, size_t len)
{
	if (!buf || !eeprom)
		return -EINVAL;
	if (len < (sizeof(eeprom->serial) + 1))
		return -EINVAL;

	if (eeprom->serial[0] == 'T' && eeprom->serial[1] == 'Q')
		return tq_parse_eeprom_serial_new(eeprom, buf);
	else
		return tq_parse_eeprom_serial_old(eeprom, buf);

	return -EINVAL;
}

int tq_parse_eeprom_id(struct tq_eeprom_data * const eeprom, char *buf,
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

int tq_show_eeprom(struct tq_eeprom_data * const eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[(TQ_EE_BDID_BYTES) + 1];

	if (!eeprom)
		return -1;

	puts(id);
	puts(" EEPROM:\n");
	/* ID */
	tq_parse_eeprom_id(eeprom, safe_string, ARRAY_SIZE(safe_string));
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

int tq_board_handle_eeprom_data(const char *board_name,
				 struct tq_eeprom_data * const eeprom)
{
	char sstring[(TQ_EE_BDID_BYTES) + 1];

	tq_parse_eeprom_id(eeprom, sstring, ARRAY_SIZE(sstring));

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		if (strncmp(sstring, board_name, strlen(board_name)) == 0)
			env_set("boardtype", sstring);
		if (tq_parse_eeprom_serial(eeprom, sstring,
					   ARRAY_SIZE(sstring)) == 0)
			env_set("serial#", sstring);
		else
			env_set("serial#", "???");
	}

	return tq_show_eeprom(eeprom, board_name);
}

#endif
