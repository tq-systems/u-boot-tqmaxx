# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 TQ-Systems GmbH

obj-y += ddr.o
obj-y += tqmls1088a.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_NET) += eth.o
endif
obj-$(CONFIG_TQMLS1088A_BB_MBLS10XXA) += tqmls1088a_bb_mbls10xxa.o
