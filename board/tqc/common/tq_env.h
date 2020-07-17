/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 TQ-Systems GmbH
 *
 * Author: Matthias Schiffer <matthias.schiffer@tq-group.com>
 *
 */

#ifndef __TQ_ENV_H__
#define __TQ_ENV_H__

#include <env.h>

static inline void tq_env_set(const char *varname, const char *value)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set(varname, value);
#endif
}

#endif /* __TQ_ENV_H__ */
