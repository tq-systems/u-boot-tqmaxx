## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright (c) 2021-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
## D-82229 Seefeld, Germany.
## Author: Markus Niebel
##

obj-y  := tqma6.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TQMA6DL) += tqma6dl.o
obj-$(CONFIG_TQMA6Q) += tqma6q.o
obj-$(CONFIG_TQMA6Q_2GB) += tqma6q.o
obj-$(CONFIG_TQMA6S) += tqma6s.o
obj-$(CONFIG_MBA6) += spl_mba6.o
endif

obj-$(CONFIG_MBA6) += tqma6_mba6.o
obj-$(CONFIG_WRU4) += tqma6_wru4.o
