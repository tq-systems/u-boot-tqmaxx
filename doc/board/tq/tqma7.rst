.. SPDX-License-Identifier: CC-BY-4.0

.. Copyright (c) 2020-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

########################################
U-Boot for the TQ-Systems TQMa7x modules
########################################

This file contains information for the port of
U-Boot to the TQ-Systems TQMa7 modules.

***********
Boot source
***********

The following boot sources are supported:

- SD/eMMC
- QSPI-NOR
- USB / SDP

********
Building
********

To build U-Boot for the TQ-Systems TQMa7x modules:

.. code-block:: bash

	make <som>_<dram>_mba7_<boot>_defconfig
	make

**som**  is a placeholder for the module CPU variant:

+--------+-------------------+
| tqma7s | TQMA7 with i.MX7S |
+--------+-------------------+
| tqma7d | TQMA7 with i.MX7D |
+--------+-------------------+

**boot** is a placeholder for the boot device:

+------+----------+
| mmc  | SD/eMMC  |
+------+----------+
| qspi | QSPI-NOR |
+------+----------+
| mfg  | USB/SDP  |
+------+----------+

**dram** is a placeholder for the RAM size:

+-------+-------+
| 512mb | 512MB |
+-------+-------+
| 1gb   | 1GB   |
+-------+-------+
| 2gb   | 2GB   |
+-------+-------+

The default build artifact is named ``u-boot-dtb.imx``.

*****************************************
Serial Download Protocol (SDP) / USB boot
*****************************************

The complete system image can be programmed with ``uuu``
(https://github.com/NXPmicro/mfgtools.git) to eMMC.

Serial Download Protocol is supported on the Micro-B USB port (X5) of MBa7x.
The command ``fastboot usb 0`` is used to enable the fastboot gadget.

Build SDP enabled U-Boot image
==============================

.. code-block:: bash

	make <som>_<dram>_mba7_mfg_defconfig
	make

Booting
=======

With mfgtools

.. code-block:: bash

	<path to MFG Tool/>uuu/uuu <UUU U-Boot image>

************
Support Wiki
************

See `TQ Embedded Wiki for TQMa7x <https://support.tq-group.com/en/arm/tqma7x>`_.
