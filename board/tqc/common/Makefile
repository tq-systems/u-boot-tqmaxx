#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# Copyright (C) 2017, Markus Niebel <Markus.Niebel@tq-group.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else

ifdef CONFIG_ARM
obj-$(CONFIG_DEEP_SLEEP)		+= arm_sleep.o
endif

obj-$(CONFIG_LS102XA_STREAM_ID)	+= ls102xa_stream_id.o
obj-$(CONFIG_LAYERSCAPE_NS_ACCESS)	+= ns_access.o
endif

obj-y += tqmaxx_eeprom.o
obj-$(CONFIG_TQC_EMMC) += tqc_emmc.o
