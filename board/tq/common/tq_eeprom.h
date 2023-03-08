/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2014 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_EEPROM_H__
#define __TQ_EEPROM_H__

#ifdef CONFIG_DM_I2C

int tq_read_eeprom_buf(unsigned int bus, unsigned int i2c_addr,
		       unsigned int alen, unsigned int addr,
		       int bsize, u_int8_t *buf);

#else

int tq_read_eeprom_buf(unsigned int bus, unsigned int i2c_addr,
		       unsigned int alen, unsigned int addr,
		       int bsize, uchar *buf);

#endif

#if defined(CONFIG_TQ_VARD)

#define VARD_FEATURE_BYTES	8

/*
 * VARD - variant and revision detection
 * must have an exact size of 32 bytes to fit in EEPROM just before
 * the module data
 */
struct __packed tq_vard {
	u16 crc;		/* checksum of vard data - CRC16 XMODEM */
	u8 hwrev;		/* hardware major revision */
	u8 memsize;		/* RAM size */
	u8 memtype;		/* RAM Type + ECC */
	u8 features[VARD_FEATURE_BYTES];	/* feature bitmask */
	u8 eepromsize;		/* user eeprom size (feature EEPROM) */
	u8 eepromtype;		/* user eeprom type (feature EEPROM) */
	u8 formfactor;		/* SOM Form factor. mask 0xf0 */
	u8 rsv[0x10];		/* for future use */
};

#define VARD_MEMTYPE_MASK_TYPE		0x7f /* board specific RAM Type */
#define VARD_MEMTYPE_MASK_ECC		0x80 /* extra ECC RAM assembled */
#define VARD_MEMTYPE_DEFAULT		0xff /* use board specific default */

#define VARD_MEMSIZE_MASK_EXP		0x1f /* 2^n MBytes */
#define VARD_MEMSIZE_MASK_FACTOR	0x20 /* x3 */
#define VARD_MEMSIZE_DEFAULT		0xff /* use board specific default */

/* feature is present if bit is zero */
#define VARD_FEATURE_0_RESERVED		0xf0 /* Do not use */
#define VARD_FEATURE_0_EMMC		0x08 /* e-MMC assembled */
#define VARD_FEATURE_0_EEPROM		0x04 /* user EEPROM assembled */
#define VARD_FEATURE_0_SPINOR		0x02 /* [Q,O]SPI-NOR assembled */
#define VARD_FEATURE_0_SECELEM		0x01 /* secure element assembled */

#define VARD_FEATURE_4_RESERVED		0xf0 /* Do not use */
#define VARD_FEATURE_4_RTC		0x08 /* RTC assembled */

#define VARD_EESIZE_MASK_EXP		0x1f /* 2^n Bytes */
#define VARD_EETYPE_DEFAULT		0xff /* use board specific default */
#define VARD_EETYPE_MASK_MFR		0xf0 /* manufacturer / type mask */
#define VARD_EETYPE_MASK_PGSIZE		0x0f /* page size */

#define VARD_FORMFACTOR_MASK_TYPE	0xf0 /* SOM type mask */
#define VARD_FORMFACTOR_TYPE_CONNECTOR	0x00 /* SOM with connector, no board standard */
#define VARD_FORMFACTOR_TYPE_LGA	0x10 /* LGA SOM, no board standard */
#define VARD_FORMFACTOR_TYPE_SMARC2	0x20 /* SOM conforms to SMARC-2 standard */
#define VARD_FORMFACTOR_TYPE_NONE	0xf0 /* unspecified SOM type */
/*
 * read module configuration data from eeprom
 * bus: I2C bus number
 * bus: I2C address of EEPROM
 * This function is intended for SPL usage without DT support
 * All supported modules have an JC42 type EEPROM with one byte offset
 * addressing
 */
static inline
int tq_vard_read(unsigned int bus, unsigned int i2c_addr, struct tq_vard *vard)
{
	return tq_read_eeprom_buf(bus, i2c_addr, 1, 0, sizeof(*vard),
				  (uchar *)vard);
}

/*
 * check CRC - true if CRC is valid
 */
bool tq_vard_valid(const struct tq_vard *vard);

/*
 * all data should only be handled as valid, if CRC is OKAY
 */
static inline
bool tq_vard_has_ramecc(const struct tq_vard *vard)
{
	return (vard->memsize & VARD_MEMTYPE_MASK_ECC);
}

/*
 * Calculate size in byte using byte from vard
 * This works as long as coding for EEPROM / RAM size is the same
 * val - memsize byte from tq_vard structure
 * multiply - multiplier, aka 1 / SZ_1K / SZ_1M
 * tmask - mask for triple factor (use only for RAM sizes)
 *
 * return size in bytes or zero in case the val is equal to VARD_MEMSIZE_DEFAULT
 */
phys_size_t tq_vard_memsize(u8 val, unsigned int multiply, unsigned int tmask);

static inline
phys_size_t tq_vard_ramsize(const struct tq_vard *vard)
{
	return tq_vard_memsize(vard->memsize, SZ_1M, VARD_MEMSIZE_MASK_FACTOR);
}

static inline
size_t tq_vard_eepromsize(const struct tq_vard *vard)
{
	return tq_vard_memsize(vard->eepromsize, 1, 0x0);
}

static inline
size_t tq_vard_eeprom_pgsize(const struct tq_vard *vard)
{
	return 1 << (size_t)(vard->eepromtype & VARD_EETYPE_MASK_PGSIZE);
}

static inline
int tq_vard_has_feature(const struct tq_vard *vard, unsigned int fbyte,
			unsigned int fbit)
{
	if (fbyte < VARD_FEATURE_BYTES && fbit < 8)
		return !(vard->features[fbyte] & BIT(fbit));
	else
		return -ERANGE;
}

static inline
bool tq_vard_has_emmc(const struct tq_vard *vard)
{
	return (tq_vard_has_feature(vard, 0, 3) > 0);
}

static inline
bool tq_vard_has_eeprom(const struct tq_vard *vard)
{
	return (tq_vard_has_feature(vard, 0, 2) > 0);
}

static inline
bool tq_vard_has_spinor(const struct tq_vard *vard)
{
	return (tq_vard_has_feature(vard, 0, 1) > 0);
}

static inline
bool tq_vard_has_secelem(const struct tq_vard *vard)
{
	return (tq_vard_has_feature(vard, 0, 0) > 0);
}

static inline
bool tq_vard_has_rtc(const struct tq_vard *vard)
{
	return (tq_vard_has_feature(vard, 4, 3) > 0);
}

static inline u32 tq_vard_get_formfactor(const struct tq_vard *vard)
{
	return (u32)(vard->formfactor & VARD_FORMFACTOR_MASK_TYPE);
};

static inline
bool tq_vard_is_lga(const struct tq_vard *vard)
{
	return (tq_vard_get_formfactor(vard) == VARD_FORMFACTOR_TYPE_LGA);
}

static inline
bool tq_vard_is_connector(const struct tq_vard *vard)
{
	return (tq_vard_get_formfactor(vard) == VARD_FORMFACTOR_TYPE_CONNECTOR);
}

static inline
bool tq_vard_is_smarc2(const struct tq_vard *vard)
{
	return (tq_vard_get_formfactor(vard) == VARD_FORMFACTOR_TYPE_SMARC2);
}

void tq_vard_show(const struct tq_vard *vard);

struct tq_som_feature_list;

/**
 * fill in presence information from VARD.
 *
 * @param[in] vard pointer to VARD structure from SOM EEPROM
 * @param[in] features SOM specific feature list
 *
 * @return 0 on success

 * Must be called after data was read to vard. The function checks
 * if vard is valid, goes through the list and sets the present flag
 * for each entry depending on the flags in vard.
 */
int tq_vard_detect_features(const struct tq_vard *vard,
			    struct tq_som_feature_list *features);

#endif

#if !defined(CONFIG_SPL_BUILD)

/*
 * static EEPROM layout
 */
#define TQ_EE_HRCW_BYTES		0x20
#define TQ_EE_MAC_BYTES		6
#define TQ_EE_RSV1_BYTES		10
#define TQ_EE_SERIAL_BYTES		8
#define TQ_EE_RSV2_BYTES		8
#define TQ_EE_BDID_BYTES		0x40

struct __packed tq_eeprom_data {
	union {
#if defined(CONFIG_TQ_VARD)
		struct tq_vard vard;
		_Static_assert(sizeof(struct tq_vard) == TQ_EE_HRCW_BYTES, \
			"struct tq_vard has incorrect size");
#endif
		u8 hrcw_primary[TQ_EE_HRCW_BYTES];
	} tq_hw_data;
	u8 mac[TQ_EE_MAC_BYTES];		/* 0x20 ... 0x25 */
	u8 rsv1[TQ_EE_RSV1_BYTES];
	u8 serial[TQ_EE_SERIAL_BYTES];		/* 0x30 ... 0x37 */
	u8 rsv2[TQ_EE_RSV2_BYTES];
	u8 id[TQ_EE_BDID_BYTES];		/* 0x40 ... 0x7f */
};

/**
 * Parse MAC address from module EEPROM to buf, respect buf len
 */
int tq_parse_eeprom_mac(struct tq_eeprom_data * const eeprom, char *buf,
			size_t len);

/**
 * Parse BCD coded serial number from module EEPROM to buf,
 * respect buf len
 */
int tq_parse_eeprom_serial(struct tq_eeprom_data * const eeprom, char *buf,
			   size_t len);

/**
 * Parse module name string number from module EEPROM to buf,
 * respect buf len
 */
int tq_parse_eeprom_id(struct tq_eeprom_data * const eeprom, char *buf,
		       size_t len);

/**
 * display the contents of the module EEPROM
 * (MAC, serial number, module name/variant string)
 * report error, if module name doesn't match id string or malformed
 * MAC / serial number
 */
int tq_show_eeprom(struct tq_eeprom_data *const eeprom, const char *id);

/**
 * verify and display contents of the module EEPROM
 * optionally set 'serial' and 'board_type env' if
 * CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG in enabled
 */
int tq_board_handle_eeprom_data(const char *board_name,
				struct tq_eeprom_data * const eeprom);

#if defined(CONFIG_I2C_EEPROM)

/**
 * Reads struct tq_eeprom_data from EEPROM given by seq nr
 * starting at offset in EEPROM array.
 */
int tq_read_eeprom_at(int seq, uint offset,
		      struct tq_eeprom_data *eeprom);

/**
 * Reads struct tq_eeprom_data from EEPROM given by seq nr
 * starting at offset zero in EEPROM array.
 */
static inline int tq_read_eeprom(int seq, struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom_at(seq, 0, eeprom);
}

/**
 * Reads struct tq_eeprom_data from EEPROM with seq nr zero
 * starting at offset zero in EEPROM array.
 */
static inline int tq_read_module_eeprom(struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom(0, eeprom);
}
#else

/*
 * older versions, shall be replaced after conversion to
 * CONFIG_I2C_EEPROM with the above variants.
 */

int tq_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		      unsigned int alen, unsigned int addr,
		      struct tq_eeprom_data *eeprom);

#if defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN)
int tq_read_eeprom(unsigned int bus, unsigned int addr,
		   struct tq_eeprom_data *eeprom);
#endif /* CONFIG_SYS_I2C_EEPROM_ADDR_LEN */

int tq_read_module_eeprom(struct tq_eeprom_data *eeprom);

#endif

#endif /* CONFIG_SPL_BUILD */

#endif
