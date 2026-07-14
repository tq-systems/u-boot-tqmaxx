/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Nora Schiffer
 */

#if !defined(__TQMA8MPXL_MBA8MP_RAS314_H)
#define __TQMA8MPXL_MBA8MP_RAS314_H

#if defined(CONFIG_FEC_MXC)

#define FEC_QUIRK_ENET_MAC

#define CONSOLE_DEV			ttymxc3

#endif /* CONFIG_FEC_MXC */

#endif /* __TQMA8MPXL_MBA8MP_RAS314_H */
