/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_SOM_FEATURES__
#define __TQ_SOM_FEATURES__

/**
 * enumeration of known features for TQ-Systems SOM
 *
 * These features are optional which means not all boards have them
 * soldered.
 */
enum som_feature {
	FEATURE_EEPROM,		/**< user EEPROM */
	FEATURE_EMMC,		/**< eMMC on SOM */
	FEATURE_RTC,		/**< discrete RTC */
	FEATURE_SECELEM,	/**< secure element */
	FEATURE_SPINOR,		/**< [Q]SPI NOR */
};

/**
 * description of an optional SOM specific feature
 *
 * describes a feature that can be detected and enabled / disabled in
 * DT before booting the OS
 * Feature present flag should be checked using SOM specific
 * implementation of tq_board_detect_features
 */
struct tq_som_feature {
	enum som_feature feature; /**< one of the above feature ID */
	const char *dt_path; /**< dt path for device behind the feature */
	bool present; /**< true if feature is present */
};

/**
 * board feature list
 *
 * List of detectable features for the SOM
 * This should be implemented by SOM specific code and shall be filled
 * in tq_board_detect_features
 */
struct tq_som_feature_list {
	struct tq_som_feature *list; /**< list of detectable features */
	size_t entries; /**< count of entries in list */
};

/**
 * board specific feature detection
 *
 * @return filled feature list or NULL on error
 *
 * The function shall be implemented in SOM specific part and can be
 * called by mainboard code. The returned list can be used with
 * tq_ft_fixup_features. The implementation shall use VARD in the vendor
 * EEPROM to fill the list. See tq_vard_detect_features
 */
struct tq_som_feature_list *tq_board_detect_features(void);

/**
 * board specific feature fixup in devicetree
 *
 * @param[in] blob pointer to in memory devicetree
 * @param[in] list of features
 *
 * The function can be used to enable / disable features in the feature
 * list based on its present flag and should be called from mainboard
 * specific part of ft_board_setup.
 * If a feature has no given dt_path in its tq_som_feature entry
 * it will be left untouched in the device tree.
 *
 * The feature list is either returned by tq_board_detect_features
 * or implemented otherwise by board code.
 */
void tq_ft_fixup_features(void *blob,
			  const struct tq_som_feature_list *list);

#endif
