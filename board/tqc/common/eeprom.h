/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2014-2022 TQ-Systems GmbH
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

/**
 * TQ EEPROM layout (128 bytes)
 */
struct tq_eeprom_data {
	u8 hrcw_primary[0x20];
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

#endif
