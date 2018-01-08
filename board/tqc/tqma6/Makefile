#
# Copyright (C) 2014-2016, Markus Niebel <Markus.Niebel@tq-group.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y  := tqma6.o \
	tqma6_eeprom.o

obj-$(CONFIG_SPL_BUILD) += \
	tqma6_spl.o \
	tqma6s.o \
	tqma6dl.o \
	tqma6q.o \
	tqma6qp.o

obj-$(CONFIG_MBA6) += tqma6_mba6.o
obj-$(CONFIG_WRU4) += tqma6_wru4.o
