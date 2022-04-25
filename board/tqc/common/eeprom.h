/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2014-2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Authors: Markus Niebel <Markus.Niebel@tq-group.com>
 *          Matthias Schiffer <matthias.schiffer@tq.tq-group.com>
 */

#ifndef __TQ_COMMON_EEPROM_H__
#define __TQ_COMMON_EEPROM_H__

/**
 * Length of an TQ model ID string read from an EEPROM, including
 * NUL termination
 */
#define TQ_ID_STRLEN 0x41

/**
 * Length of an TQ serial number string read from an EEPROM, including
 * NUL termination
 */
#define TQ_SERIAL_STRLEN 9

#define TQ_EE_HRCW_BYTES	0x20
#define VARD_FEATURE_BYTES	8

/**
 * VARD - variant and revision detection
 * must have an exact size of 32 bytes to fit in EEPROM just before
 * the module data
 */
struct tq_vard {
	u16 crc;		/* checksum of vard data - CRC16 XMODEM */
	u8 hwrev;		/* hardware major revision */
	u8 memsize;		/* RAM size */
	u8 memtype;		/* RAM Type + ECC */
	u8 features[VARD_FEATURE_BYTES];	/* feature bitmask */
	u8 eepromsize;		/* user eeprom size (feature EEPROM) */
	u8 eepromtype;		/* user eeprom type (feature EEPROM) */
	u8 rsv[0x11];		/* for future use */
};

_Static_assert(sizeof(struct tq_vard) == TQ_EE_HRCW_BYTES,
	       "struct tq_vard has incorrect size");

/**
 * TQ EEPROM layout (128 bytes)
 */
struct tq_eeprom_data {
	union {
		struct tq_vard vard;
		u8 hrcw_primary[TQ_EE_HRCW_BYTES];
	} tq_hw_data;
	u8 mac[6];		/* 0x20 ... 0x25 */
	u8 rsv1[10];
	u8 serial[8];		/* 0x30 ... 0x37 */
	u8 rsv2[8];
	u8 id[0x40];		/* 0x40 ... 0x7f */
};

/**
 * Reads a tq_eeprom_data from an EEPROM, at a given offset from the
 * start of the EEPROM
 */
int tq_read_eeprom_at(int seq, uint offset, struct tq_eeprom_data *eeprom);

/**
 * Reads a tq_eeprom_data from an EEPROM
 */
static inline int tq_read_eeprom(int seq, struct tq_eeprom_data *eeprom)
{
	return tq_read_eeprom_at(seq, 0, eeprom);
}

/**
 * Extracts the model ID from TQ EEPROM data
 *
 * A buffer of size TQMAXX_ID_STRLEN must be passed.
 *
 * Returns false of no valid ID was found.
 */
bool tq_get_eeprom_id(const struct tq_eeprom_data *eeprom, char *buf);

/**
 * Extracts the serial number from TQ EEPROM data
 *
 * A buffer of size TQMAXX_SERIAL_STRLEN must be passed.
 *
 * Returns false of no valid serial number was found.
 */
bool tq_get_eeprom_serial(const struct tq_eeprom_data *eeprom, char *buf);

/**
 * Extracts the MAC address from TQ EEPROM data
 *
 * A buffer of length 6 must be passed.
 *
 * Returns false of no valid MAC address was found.
 */
bool tq_get_eeprom_mac(const struct tq_eeprom_data *eeprom, uint8_t *macaddr);

/**
 * Prints the information from TQ EEPROM data in a human-readable format
 */
void tq_show_eeprom(const struct tq_eeprom_data *eeprom, const char *id_prefix);

#if defined(CONFIG_TQ_VARD)

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

bool tq_vard_show(const struct tq_vard *vard);

#endif

#endif
