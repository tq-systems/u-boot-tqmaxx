# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
# Copyright (C) 2020-2022 TQ-Systems GmbH
#

obj-y := tqma65xx.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_TQMA65XX_BOARD_MBA65XX) += \
	tqma65xx_mba65xx.o tqma65xx_mba65xx_clocksynth.o
endif
