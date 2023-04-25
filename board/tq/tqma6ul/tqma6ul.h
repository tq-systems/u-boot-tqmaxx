/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

enum tqma6ul_som_type {
	/* unknown */
	tqma6_som_type_unknown,
	/* connector module */
	tqma6_som_type_ca,
	/* LGA Variant */
	tqma6_som_type_lga,
};

const char *check_cpu_variant(void);
enum tqma6ul_som_type  check_tqma6ul_variant(void);
enum tqma6ul_som_type tqma6ul_dt(char *dt, size_t dtsize, const char *mb);
