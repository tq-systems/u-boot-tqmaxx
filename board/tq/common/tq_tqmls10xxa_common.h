/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 */
#ifndef __TQ_TQMLS10XXA_COMMON
#define __TQ_TQMLS10XXA_COMMON

void tq_tqmls10xx_checkboard(void);
void tq_tqmls10xxa_set_macaddrs(u8 *macaddr, int count);

#endif /* __TQ_TQMLS10XXA_COMMON */
