// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <common.h>
#include <display_options.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <i2c_eeprom.h>
#include <linux/ctype.h>
#include <linux/sizes.h>
#include <log.h>
#include <net.h>
#include <sysinfo/tq_eeprom.h>
#include <u-boot/crc.h>

#define VARD_FEATURE_BYTES	8

#define VARD_MEMTYPE_MASK_TYPE		0x7f /* board specific RAM Type */
#define VARD_MEMTYPE_MASK_ECC		0x80 /* extra ECC RAM assembled */
#define VARD_MEMTYPE_DEFAULT		0xff /* use board specific default */

#define VARD_MEMSIZE_MASK_EXP		0x1f /* 2^n MBytes */
#define VARD_MEMSIZE_MASK_FACTOR	0x20 /* x3 */
#define VARD_MEMSIZE_DEFAULT		0xff /* use board specific default */

#define VARD_FEATURE(byte, bit)		(((byte) << 4) | (bit))
#define VARD_FEATURE_BYTE(val)		((val) >> 4)
#define VARD_FEATURE_BIT(val)		((val) & 0xf)

/* Feature is present if bit is zero */
#define VARD_FEATURE_SECELEM		VARD_FEATURE(0, 0)
#define VARD_FEATURE_SPINOR		VARD_FEATURE(0, 1)
#define VARD_FEATURE_EEPROM		VARD_FEATURE(0, 2)
#define VARD_FEATURE_EMMC		VARD_FEATURE(0, 3)
/* RESERVED - do not assign		VARD_FEATURE(0, 4) */
/* RESERVED - do not assign		VARD_FEATURE(0, 5) */
/* RESERVED - do not assign		VARD_FEATURE(0, 6) */
/* RESERVED - do not assign		VARD_FEATURE(0, 7) */
#define VARD_FEATURE_RTC		VARD_FEATURE(4, 3)
/* RESERVED - do not assign		VARD_FEATURE(4, 4) */
/* RESERVED - do not assign		VARD_FEATURE(4, 5) */
/* RESERVED - do not assign		VARD_FEATURE(4, 6) */
/* RESERVED - do not assign		VARD_FEATURE(4, 7) */

#define VARD_EESIZE_MASK_EXP		0x1f /* 2^n Bytes */
#define VARD_EETYPE_DEFAULT		0xff /* use board specific default */
#define VARD_EETYPE_MASK_MFR		0xf0 /* manufacturer / type mask */
#define VARD_EETYPE_MASK_PGSIZE		0x0f /* page size */

#define VARD_FORMFACTOR_MASK_TYPE	0xf0 /* SOM type mask */
#define VARD_FORMFACTOR_SHIFT_TYPE	4    /* SOM type shift */

enum tq_formfactor_type {
	VARD_FORMFACTOR_TYPE_CONNECTOR,      /* SOM with connector, no board standard */
	VARD_FORMFACTOR_TYPE_LGA,            /* LGA SOM, no board standard */
	VARD_FORMFACTOR_TYPE_SMARC2,         /* SOM conforms to SMARC-2 standard */
	VARD_FORMFACTOR_TYPE_NONE = 0xf,     /* unspecified SOM type */
};

#define TQ_EE_RSV1_BYTES		10
#define TQ_EE_SERIAL_BYTES		8
#define TQ_EE_RSV2_BYTES		8
#define TQ_EE_BDID_BYTES		0x40

/* VARD - variant and revision detection */
struct tq_vard {
	u16 crc;		/* checksum of vard data - CRC16 XMODEM */
	u8 hwrev;		/* hardware major revision */
	u8 memsize;		/* RAM size */
	u8 memtype;		/* RAM Type + ECC */
	u8 features[VARD_FEATURE_BYTES]; /* feature bitmask */
	u8 eepromsize;		/* user eeprom size (feature EEPROM) */
	u8 eepromtype;		/* user eeprom type (feature EEPROM) */
	u8 formfactor;		/* SOM Form factor. mask 0xf0 */
	u8 rsv[0x10];		/* for future use */
};

_Static_assert(sizeof(struct tq_vard) == 0x20,
	       "struct tq_vard has incorrect size");

struct tq_eeprom_data {
	struct tq_vard vard;
	u8 mac[ETH_ALEN];		/* 0x20 ... 0x25 */
	u8 rsv1[TQ_EE_RSV1_BYTES];
	u8 serial[TQ_EE_SERIAL_BYTES];		/* 0x30 ... 0x37 */
	u8 rsv2[TQ_EE_RSV2_BYTES];
	u8 id[TQ_EE_BDID_BYTES];		/* 0x40 ... 0x7f */
};

_Static_assert(sizeof(struct tq_eeprom_data) == 0x80,
	       "struct tq_eeprom_data has incorrect size");

struct ram_variant_info {
	const char *name;
	u64 ram_size;
	u32 ram_type;
};

/**
 * struct sysinfo_tq_eeprom_priv - sysinfo private data
 */
struct sysinfo_tq_eeprom_priv {
	struct udevice *i2c_eeprom;
	int offset;
	bool has_vard;
	size_t n_ram_variants;
	struct ram_variant_info *ram_variants;

	/* Reserve extra space for \0 in id and serial */
	char id[TQ_EE_BDID_BYTES + 1];
	char serial[TQ_EE_SERIAL_BYTES + 1];
	u8 mac[ETH_ALEN];
	const struct ram_variant_info *ram_variant;
	struct tq_vard vard;
};

static uint16_t tq_vard_chksum(const struct tq_vard *vard)
{
	const unsigned char *start = (const unsigned char *)vard +
		sizeof(vard->crc);

	return crc16_ccitt(0, start, sizeof(*vard) - sizeof(vard->crc));
}

static bool tq_vard_valid(const struct tq_vard *vard)
{
	return vard->crc == tq_vard_chksum(vard);
}

static void tq_eeprom_parse_id(struct udevice *dev, const struct tq_eeprom_data *data)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < sizeof(data->id); i++) {
		if (!(isprint(data->id[i]) && isascii(data->id[i])))
			break;
	}

	if (i == 0)
		dev_warn(dev, "no valid model name in EEPROM\n");

	snprintf(priv->id, sizeof(priv->id), "%.*s", i, data->id);
}

static int tq_eeprom_serial_len_old(const struct tq_eeprom_data *data)
{
	int i;

	for (i = 0; i < sizeof(data->serial); i++) {
		if (!isdigit(data->serial[i]))
			break;
	}

	return i;
}

static int tq_eeprom_serial_len_new(const struct tq_eeprom_data *data)
{
	int i;

	for (i = 0; i < sizeof(data->serial); i++) {
		if (!(isdigit(data->serial[i]) || isupper(data->serial[i])))
			break;
	}

	return i;
}

static void tq_eeprom_parse_serial(struct udevice *dev, const struct tq_eeprom_data *data)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);
	int len;

	if (data->serial[0] == 'T' && data->serial[1] == 'Q')
		len = tq_eeprom_serial_len_new(data);
	else
		len = tq_eeprom_serial_len_old(data);

	/* For now, only serial numbers with the exact size of the field are accepted */
	if (len != sizeof(data->serial)) {
		dev_warn(dev, "no valid serial number in EEPROM\n");
		len = 0;
	}

	snprintf(priv->serial, sizeof(priv->serial), "%.*s", len, data->serial);
}

static unsigned int tq_vard_decode_size(u8 val, u8 tmask)
{
	unsigned int result;

	if (val == VARD_MEMSIZE_DEFAULT)
		return 0;

	result = 1u << (val & VARD_MEMSIZE_MASK_EXP);
	if (val & tmask)
		result *= 3;

	return result;
}

static unsigned int tq_vard_ram_size(const struct tq_vard *vard)
{
	return tq_vard_decode_size(vard->memsize, VARD_MEMSIZE_MASK_FACTOR);
}

static bool tq_vard_ram_has_ecc(const struct tq_vard *vard)
{
	return vard->memsize & VARD_MEMTYPE_MASK_ECC;
}

static unsigned int tq_vard_eeprom_size(const struct tq_vard *vard)
{
	return tq_vard_decode_size(vard->eepromsize, 0);
}

static unsigned int tq_vard_eeprom_pgsize(const struct tq_vard *vard)
{
	return 1u << (vard->eepromtype & VARD_EETYPE_MASK_PGSIZE);
}

static enum tq_formfactor_type tq_vard_formfactor(const struct tq_vard *vard)
{
	return (vard->formfactor & VARD_FORMFACTOR_MASK_TYPE) >> VARD_FORMFACTOR_SHIFT_TYPE;
};

static bool tq_vard_has_feature(const struct tq_vard *vard, unsigned int feature)
{
	unsigned int byte = VARD_FEATURE_BYTE(feature), bit = VARD_FEATURE_BIT(feature);

	if (byte >= sizeof(vard->features))
		return false;

	return !(vard->features[byte] & BIT(bit));
}

static const struct ram_variant_info *
find_ram_variant(const struct ram_variant_info *variants, size_t count,
		 u64 ram_size, u32 ram_type)
{
	int i;

	for (i = 0; i < count; i++) {
		const struct ram_variant_info *variant = &variants[i];

		if (variant->ram_size == ram_size && variant->ram_type == ram_type)
			return variant;
	}

	return NULL;
}

static void print_ram_variants(const struct ram_variant_info *variants, size_t count)
{
	int i;

	for (i = 0; i < count; i++) {
		const struct ram_variant_info *variant = &variants[i];

		printf("%d) %s (", i + 1, variant->name);
		print_size(variant->ram_size, " RAM)\n");
	}
}

static const struct ram_variant_info *
choose_ram_variant(const struct ram_variant_info *variants, size_t count)
{
	int idx;

	/* Only one supported configuration, so we can return it immediately */
	if (count == 1)
		return &variants[0];

	puts("Please enter the index of the configuration to use from the following list:\n\n");

	print_ram_variants(variants, count);

	/* Flush input */
	while (tstc())
		getchar();

	puts("\nSelection: ");

	while (true) {
		idx = getchar() - '1';

		if (idx >= 0 && idx < count)
			break;
	}

	printf("%d\n\n", idx + 1);

	return &variants[idx];
}

const struct ram_variant_info *get_ram_variant(const struct sysinfo_tq_eeprom_priv *priv)
{
	const struct ram_variant_info *variant;

	variant = find_ram_variant(priv->ram_variants, priv->n_ram_variants,
				   (u64)SZ_1M * tq_vard_ram_size(&priv->vard),
				   priv->vard.memtype);

	if (!variant) {
		puts("\nWarning: Failed to find a supported RAM size in EEPROM.\n");

		/*
		 * Do not fall back to manual variant selection unless we are in
		 * SPL (so boot cannot continue without RAM selection)
		 */
		if (!IS_ENABLED(CONFIG_SPL_BUILD))
			return NULL;

		variant = choose_ram_variant(priv->ram_variants, priv->n_ram_variants);
	}

	printf("Selected configuration for ");
	print_size(variant->ram_size, " RAM\n");

	return variant;
}

static void tq_vard_show_feature(const struct tq_vard *vard, const char *name, unsigned int feature)
{
	bool has_feature = tq_vard_has_feature(vard, feature);

	printf("  %-13s %c\n", name, has_feature ? 'y' : 'n');
}

static void tq_vard_dump(const struct tq_vard *vard)
{
	enum tq_formfactor_type formfactor = tq_vard_formfactor(vard);

	printf("VARD DATA:\n");

	printf("  HW REV:\t%02uxx\n", vard->hwrev);

	printf("  RAM:\t\ttype %u, %u MiB, %s\n",
	       vard->memtype & VARD_MEMTYPE_MASK_TYPE,
	       tq_vard_ram_size(vard),
	       tq_vard_ram_has_ecc(vard) ? "ECC" : "no ECC");

	tq_vard_show_feature(vard, "RTC:", VARD_FEATURE_RTC);
	tq_vard_show_feature(vard, "SPI-NOR:", VARD_FEATURE_SPINOR);
	tq_vard_show_feature(vard, "e-MMC:", VARD_FEATURE_EMMC);
	tq_vard_show_feature(vard, "SE:", VARD_FEATURE_SECELEM);

	if (tq_vard_has_feature(vard, VARD_FEATURE_EEPROM))
		printf("  EEPROM:\ttype %u, %u KiB, pagesize %u\n",
		       (vard->eepromtype & VARD_EETYPE_MASK_MFR) >> 4,
		       tq_vard_eeprom_size(vard) / SZ_1K,
		       tq_vard_eeprom_pgsize(vard));
	else
		printf("  EEPROM:\tn\n");

	printf("  FORMFACTOR:\t");
	switch (formfactor) {
	case VARD_FORMFACTOR_TYPE_LGA:
		printf("LGA\n");
		break;
	case VARD_FORMFACTOR_TYPE_CONNECTOR:
		printf("CONNECTOR\n");
		break;
	case VARD_FORMFACTOR_TYPE_SMARC2:
		printf("SMARC-2\n");
		break;
	case VARD_FORMFACTOR_TYPE_NONE:
		/*
		 * applies to boards with no variants or older boards
		 * where this field is not written
		 */
		printf("UNSPECIFIED\n");
		break;
	default:
		/*
		 * generic fallback
		 * unhandled form factor or invalid data
		 */
		printf("UNKNOWN (0x%x)\n", formfactor);
	}
}

static int tq_eeprom_dump(const struct sysinfo_tq_eeprom_priv *priv)
{
	printf("TQ EEPROM:\n");
	printf("  ID:  %s\n", priv->id[0] ? priv->id : "<unknown>");
	printf("  SN:  %s\n", priv->serial[0] ? priv->serial : "<unknown>");
	printf("  MAC: ");
	if (is_valid_ethaddr(priv->mac))
		printf("%pM\n", priv->mac);
	else
		printf("<invalid>\n");

	if (priv->has_vard)
		tq_vard_dump(&priv->vard);

	return 0;
}

static int sysinfo_tq_eeprom_detect(struct udevice *dev)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);
	struct tq_eeprom_data data;
	int ret;

	ret = i2c_eeprom_read(priv->i2c_eeprom, priv->offset,
			      (u8 *)&data, sizeof(data));
	if (ret < 0) {
		dev_err(dev, "EEPROM read failed: %d\n", ret);
		return -EIO;
	}

	if (priv->has_vard) {
		priv->has_vard = tq_vard_valid(&data.vard);
		if (priv->has_vard)
			priv->vard = data.vard;
		else
			dev_warn(dev, "VARD invalid (CRC calculated %04x expected %04x)\n",
				 tq_vard_chksum(&data.vard), data.vard.crc);
	}

	tq_eeprom_parse_id(dev, &data);
	tq_eeprom_parse_serial(dev, &data);
	memcpy(priv->mac, data.mac, ETH_ALEN);

	if (priv->n_ram_variants)
		priv->ram_variant = get_ram_variant(priv);

	if (!IS_ENABLED(CONFIG_SPL_BUILD))
		tq_eeprom_dump(priv);

	return 0;
}

static int sysinfo_tq_eeprom_get_uint64(struct udevice *dev, int id, uint64_t *val)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_RAM_SIZE:
		if (!priv->ram_variant)
			return -ENODATA;

		*val = priv->ram_variant->ram_size;
		return 0;

	default:
		return -EINVAL;
	}
}

static int sysinfo_tq_eeprom_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_RAM_VARIANT:
		if (!priv->ram_variant)
			return -ENODATA;

		strlcpy(val, priv->ram_variant->name, size);
		return 0;

	case SYSINFO_ID_TQ_MODEL:
		if (!priv->id[0])
			return -ENODATA;

		strlcpy(val, priv->id, size);
		return 0;

	case SYSINFO_ID_TQ_SERIAL:
		if (!priv->serial[0])
			return -ENODATA;

		strlcpy(val, priv->serial, size);
		return 0;

	default:
		return -EINVAL;
	}
}

static int sysinfo_tq_eeprom_get_binary(struct udevice *dev, int id, size_t size, void *val)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_TQ_MAC_ADDR:
		if (size != ETH_ALEN)
			return -EINVAL;

		if (!is_valid_ethaddr(priv->mac))
			return -ENODATA;

		memcpy(val, priv->mac, ETH_ALEN);

		return 0;

	default:
		return -EINVAL;
	}
}

static const struct sysinfo_ops sysinfo_tq_eeprom_ops = {
	.detect = sysinfo_tq_eeprom_detect,
	.get_uint64 = sysinfo_tq_eeprom_get_uint64,
	.get_str = sysinfo_tq_eeprom_get_str,
	.get_binary = sysinfo_tq_eeprom_get_binary,
};

static int read_ram_variants(struct sysinfo_tq_eeprom_priv *priv, ofnode ram_variant_node)
{
	struct ram_variant_info *variant;
	ofnode node;
	size_t i;

	ofnode_for_each_subnode(node, ram_variant_node) {
		if (!ofnode_is_enabled(node))
			continue;

		priv->n_ram_variants++;
	}

	priv->ram_variants = calloc(priv->n_ram_variants, sizeof(*priv->ram_variants));

	i = 0;
	ofnode_for_each_subnode(node, ram_variant_node) {
		if (!ofnode_is_enabled(node))
			continue;

		variant = &priv->ram_variants[i];

		variant->name = ofnode_read_string(node, "variant-name");
		if (!variant->name)
			return -EINVAL;

		if (ofnode_read_u64(node, "ram-size", &variant->ram_size))
			return -EINVAL;
		if (ofnode_read_u32(node, "ram-type", &variant->ram_type))
			return -EINVAL;

		i++;
	}

	return 0;
}

static int sysinfo_tq_eeprom_probe(struct udevice *dev)
{
	struct sysinfo_tq_eeprom_priv *priv = dev_get_priv(dev);
	ofnode ram_variant_node;
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_I2C_EEPROM, dev, "i2c-eeprom",
					   &priv->i2c_eeprom);
	if (ret) {
		dev_err(dev, "i2c-eeprom backing device not found: %d\n", ret);
		return ret;
	}

	priv->offset = dev_read_u32_default(dev, "offset", 0);
	priv->has_vard = dev_read_bool(dev, "tq,vard");

	ram_variant_node = dev_read_subnode(dev, "ram-variants");
	if (ofnode_valid(ram_variant_node)) {
		ret = read_ram_variants(priv, ram_variant_node);
		if (ret) {
			dev_err(dev,
				"failed to read RAM variants from Device Tree: %d\n",
				ret);
			return ret;
		}
	}

	return 0;
}

static const struct udevice_id sysinfo_tq_eeprom_ids[] = {
	{ .compatible = "tq,eeprom-sysinfo" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysinfo_tq_eeprom) = {
	.name           = "sysinfo_tq_eeprom",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_tq_eeprom_ids,
	.ops		= &sysinfo_tq_eeprom_ops,
	.priv_auto	= sizeof(struct sysinfo_tq_eeprom_priv),
	.probe          = sysinfo_tq_eeprom_probe,
};
