// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <common.h>
#include <fs.h>
#include <test/ut.h>

/* Declare a new FS test */
#define LIB_TEST(_name, _flags)		UNIT_TEST(_name, _flags, lib_test)

struct test_path {
	const char *input;
	const char *expected;
};

static int fs_test_path_simplify(struct unit_test_state *uts)
{
	const struct test_path test_paths[] = {
		{"", "/"},
		{"/", "/"},
		{"//", "/"},
		{"/foo", "/foo"},
		{"foo", "/foo"},
		{"/foo/", "/foo"},
		{"foo/", "/foo"},
		{"/foo//", "/foo"},
		{"foo//", "/foo"},
		{".", "/"},
		{"/./foo/./", "/foo"},
		{"/foo/..", "/"},
		{"/../foo", "/foo"},
		{"/../foo", "/foo"},
		{"/foo/bar/..", "/foo"},
		{"/foo/bar/./..", "/foo"},
		{"/foo/bar/../..", "/"},
		{"/foo/../bar/..", "/"},
	};
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(test_paths); i++) {
		const char *input = test_paths[i].input;
		const char *expected = test_paths[i].expected;
		size_t inputlen = strlen(input);
		size_t expectedlen = strlen(expected);
		size_t outputlen;

		for (outputlen = 0; outputlen <= inputlen + 2; outputlen++) {
			char *output = malloc(outputlen);

			ut_assert(output != 0);

			ret = fs_path_simplify(output, input, outputlen);

			/*
			 * If outputlen is smaller than the final output, an error must
			 * be returned
			 */
			if (outputlen < expectedlen + 1)
				ut_asserteq(-EOVERFLOW, ret);
			/* For inputlen+2, the call must always be successful */
			if (outputlen == inputlen + 2)
				ut_assertok(ret);

			/*
			 * For output lengths between expectedlen and inputlen+2, an error may
			 * or may not be returned, depending on the number of ".." in the string.
			 * If no error is returned, the output must match expected
			 */

			if (ret == 0)
				ut_asserteq_str(test_paths[i].expected, output);

			free(output);
		}
	}

	return 0;
}

LIB_TEST(fs_test_path_simplify, 0);
