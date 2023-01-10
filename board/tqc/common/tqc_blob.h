/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

/*
 * Arbitrary number that doesn't conflict with bloblist_tag_t
 * upstream uses BLOBLISTT_VENDOR_AREA = 0xc000
 */
#define BLOBLISTT_TQ_VARD	0xc000 /* forward VARD EEPROM struct */
#define BLOBLISTT_TQ_RAMSIZE	0xc001 /* forward only memory size info */

struct tq_raminfo {
	phys_size_t memsize;
};
