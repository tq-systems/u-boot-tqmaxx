.. SPDX-License-Identifier: CC-BY-4.0

.. Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

###############################################################################
U-Boot for the TQ-Systems TQMa6ULx, TQMa6ULxL, TQMa6ULLx and TQMa6ULLxL modules
###############################################################################

The main difference beween the form factors is the power supply concept. The
different module variants share the same hardware, use the same peripherals and
can be used on the same starter kit.

The following hardware revisions are supported:

Modules:

+------------+------------+
| TQMa6ULx   | / REV.030x |
+------------+------------+
| TQMa6ULLx  | / REV.030x |
+------------+------------+
| TQMa6ULxL  | / REV.020x |
+------------+------------+
| TQMa6ULLxL | / REV.020x |
+------------+------------+

Mainboards:

+----------+------------+
| MBa6ULx  | / REV.020x |
+----------+------------+
| MBa6ULxL | / REV.020x |
+----------+------------+

Hardware on modules TQMa6ULx, TQMa6ULxL, TQMa6ULLx and TQMa6ULLxL:

- eMMC
- RTC (I2C)
- PMIC
- QSPI-NOR
- EEPROM
- Temperature sensor
- 256 / 512 MiB RAM

Supported hardware on Starterkit MBa6ULx:

- 2 Ethernet PHY connected to FEC0 / FEC1 (usage depends on CPU)
- SD-card slot
- UART
- USB

Note: To change the Ethernet port to use for networking functionality, use the
U-Boot generic environment variable ``ethact``.

.. code-block:: bash

    setenv ethact <FEC>

***************
Important notes
***************

IMX6ULL Errata ERR010450: MMC: EMMC can only run below or equal to 150 MHz
- MMC_HS200_SUPPORT is not set and should not be enabled

***********
Boot source
***********

The following boot sources are supported:

- SD/eMMC
- QSPI-NOR

********
Building
********

To build U-Boot for the TQ-Systems TQMa6UL[L]x[L] modules:

.. code-block:: bash

	make <som>_<baseboard>_<boot>_defconfig
	make

**som**  is a placeholder for the module base variant:

+------------+------------+----------+
| tqma6ulx   | TQMa6ULx   | i.MX6UL  |
+------------+------------+----------+
| tqma6ullx  | TQMa6ULLx  | i.MX6ULL |
+------------+------------+----------+
| tqma6ulxl  | TQMa6ULxL  | i.MX6UL  |
+------------+------------+----------+
| tqma6ullxl | TQMa6ULLxL | i.MX6ULL |
+------------+------------+----------+

**baseboard** is a placeholder for the mainboard to compile for:

+----------+----------+
| mba6ul   | MBa6ULx  |
+----------+----------+
| mba6ulxl | MBa6ULxL |
+----------+----------+

**boot** is a placeholder for the boot device:

+------+----------+
| mmc  | SD/eMMC  |
+------+----------+
| qspi | QSPI-NOR |
+------+----------+

The default build artifact is named ``u-boot-with-spl.imx``.
The QSPI header is added during updating qspi image with prepared update script
``run update_uboot_spi``

***********************************
Serial Download Protocol (with uuu)
***********************************

The complete system image can be programmed with ``uuu``
(https://github.com/NXPmicro/mfgtools.git) to eMMC.

Serial Download Protocol is used on the Micro-B USB port (X9).

Build UUU U-Boot image
======================

.. code-block:: bash

	make <som>_<baseboard>_mmc_mfg_defconfig
	make

Booting
=======

With mfgtools

.. code-block:: bash

	<path to MFG Tool/>uuu/uuu SDP: boot -f <UUU U-Boot image>
	<path to MFG Tool/>uuu/uuu SDPV: write -f <UUU U-Boot image> -addr 0x877eefc0
	<path to MFG Tool/>uuu/uuu SDPV: jump -addr 0x8fbfffc0

Please note that the address above is calculated based on TEXT_BASE address:

0x8fbfffc0 = 0x8fc00000 (TEXT_BASE) - 0x40 (U-Boot proper Header size)

Fastboot gadget
===============

The command `fastboot usb 0` is used to enable the fastboot gadget on the
Micro-B USB port (X9). The `fastboot` and `uuu` utilities can be used to erase
and flash SD/eMMC partitions, for example:

With mfgtools

.. code-block:: bash

	<path to MFG Tool/>uuu/uuu FB: flash -raw2sparse all <SD / eMMC system images>
