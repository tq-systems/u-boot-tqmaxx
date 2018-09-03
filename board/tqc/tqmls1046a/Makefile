# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2018 TQ-Systems GmbH

obj-y += ddr.o
obj-y += tqmls1046a.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_NET) += eth.o
endif
obj-$(CONFIG_LAYERSCAPE_NS_ACCESS)	+= ../../freescale/common/ns_access.o
obj-$(CONFIG_TQMLS1046A_BB_MBLS10XXA) += tqmls1046a_bb_mbls10xxa.o
