/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#if !defined(__TQMA8MPXS_MBA8MPXS_H)
#define __TQMA8MPXS_MBA8MPXS_H

#if defined(CONFIG_FEC_MXC)

#define FEC_QUIRK_ENET_MAC

#endif /* CONFIG_FEC_MXC */

#define CONSOLE_DEV			ttymxc2

#endif /* __TQMA8MPXS_MBA8MPXS_H */
