.. SPDX-License-Identifier: CC-BY-4.0

.. Copyright (c) 2020-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

U-Boot for the TQ-Systems TQMa7 modules
=======================================

This file contains information for the port of
U-Boot to the TQ-Systems TQMa7 modules.

1. Boot source
--------------

The following boot source is supported:

- SD/eMMC
- QSPI NOR (work in progress)

2. Building
------------

To build U-Boot for the TQ-Systems TQMa7 modules:

.. code-block:: bash

    $ make tqma7<var>_mba7_<boot>_<dram>_defconfig
    $ make

``var`` is a placeholder for the Variant of the System on Module (SoM)

- s - means Solo Core
- d - means Dual Core


``boot`` is a placeholder for the boot device

- mmc - means eMMC
- qspi - mean QSPI NOR

``dram`` is a placeholder for the RAM size

- 512mb - means 512MB
- 1gb - means 1GB
- 2gb - means 2GB

Fastboot
--------

Also there is a sample defconfig available for Fastboot:

tqma7s_mba7_mmc_512mb_fastboot_defconfig

Replace the defconfig mentioned in "2. Building" with this if you want fastboot
to be enabled.
This is just a sample. If you want to use Fastboot on other SoM, bootdevices or
RAM-configs, you have to edit it accordingly.
