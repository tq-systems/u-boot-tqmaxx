/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 TQ Systems GmbH
 *
 * Author: Matthias Schiffer <matthias.schiffer@tq-group.com>
 *
 */

#ifndef __TQMAXX_H__
#define __TQMAXX_H__

#include <common.h>
#include <env.h>

static inline void tqmaxx_env_set(const char *varname, const char *value)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set(varname, value);
#endif
}

#endif /* __TQMAXX_H__ */
