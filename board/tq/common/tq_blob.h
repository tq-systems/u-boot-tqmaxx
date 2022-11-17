/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

/*
 * start private blobs at BLOBLISTT_VENDOR_AREA
 * to not conflict with bloblist_tag_t
 */

/* forward VARD EEPROM struct */
#define BLOBLISTT_TQ_VARD	(BLOBLISTT_VENDOR_AREA)
/* forward only memory size info */
#define BLOBLISTT_TQ_RAMSIZE	(BLOBLISTT_VENDOR_AREA + 1)

struct tq_raminfo {
	phys_size_t memsize;
};
