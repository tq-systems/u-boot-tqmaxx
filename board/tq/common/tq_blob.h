/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

/*
 * start private blobs at BLOBLISTT_PRIVATE_AREA
 * to not conflict with bloblist_tag_t
 */

/* forward VARD EEPROM struct */
#define BLOBLISTT_TQ_VARD	(BLOBLISTT_PRIVATE_AREA + 0x20)
/* forward only memory size info */
#define BLOBLISTT_TQ_RAMSIZE	(BLOBLISTT_PRIVATE_AREA + 0x21)

struct tq_raminfo {
	phys_size_t memsize;
};
