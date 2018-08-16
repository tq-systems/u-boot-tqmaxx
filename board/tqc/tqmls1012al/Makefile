#
# Copyright 2014 Freescale Semiconductor, Inc.
# Copyright 2015 TQ Systems GmbH
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += tqmls1012al.o

obj-$(CONFIG_MBLS1012AL) += tqmls1012al_mbls1012al.o

ifdef CONFIG_SECURE_BOOT
obj-$(CONFIG_CMD_ESBC_VALIDATE) += ../../freescale/common/fsl_validate.o cmd_esbc_validate.o
endif
obj-$(CONFIG_CHAIN_OF_TRUST) += ../../freescale/common/fsl_chain_of_trust.o
