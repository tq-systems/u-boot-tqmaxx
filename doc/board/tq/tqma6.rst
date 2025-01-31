.. SPDX-License-Identifier: CC-BY-4.0

.. Copyright (c) 2014-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

########################################
U-Boot for the TQ-Systems TQMa6x modules
########################################

This file contains information for the port of
U-Boot to the TQ-Systems TQMa6 modules.

***********
Boot source
***********

The following boot sources are supported:

- SD/eMMC
- SPI-NOR
- USB / SDP

********
Building
********

To build U-Boot for the TQ-Systems TQMa6x modules:

.. code-block:: bash

	make <som>[_<dram>]_mba6[_<boot>]_defconfig
	make

**som**  is a placeholder for the TQMA6 module CPU variant:

+---------+-------------------+
| tqma6s  | i.MX6S            |
+---------+-------------------+
| tqma6dl | i.MX6DL           |
+---------+-------------------+
| tqma6q  | i.MX6D / i.MX6Q   |
+---------+-------------------+
| tqma6qp | i.MX6DP / i.MX6QP |
+---------+-------------------+

**boot** is a placeholder for the boot device:

+------+----------------------+
|      | SD/eMMC and QSPI-NOR |
+------+----------------------+
| mfg  | USB/SDP              |
+------+----------------------+

**dram** is a placeholder for the RAM size:

+-------+----------------+
|       | 512MB  or 1 GB |
+-------+----------------+
| 2gb   | 2GB            |
+-------+----------------+

The default build artifact is named ``u-boot-with-spl.imx``.

*****************************************
Serial Download Protocol (SDP) / USB boot
*****************************************

The complete system image can be programmed with ``uuu``
(https://github.com/NXPmicro/mfgtools.git) to eMMC.

Serial Download Protocol is supported on the Micro-B USB port (X8) of MBa6x.
The command ``fastboot usb 0`` is used to enable the fastboot gadget.

Build SDP enabled U-Boot image
==============================

.. code-block:: bash

	make <som>[_<dram>]_mba6_mfg_defconfig
	make

Booting
=======

With mfgtools

.. code-block:: bash

	<path to MFG Tool/>uuu/uuu -b spl <UUU U-Boot image>

************
Support Wiki
************

See `TQ Embedded Wiki for TQMa6x <https://support.tq-group.com/en/arm/tqma6x>`_.
