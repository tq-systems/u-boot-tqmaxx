# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2016 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
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
obj-$(CONFIG_TQ_ENVLOC) += tq_envloc.o
obj-$(CONFIG_TQ_RTC) += tq_rtc.o
obj-$(CONFIG_TQ_SCU) += tq_scu.o
endif
