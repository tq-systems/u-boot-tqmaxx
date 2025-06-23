.. SPDX-License-Identifier: GPL-2.0-or-later

TQMa62xx[L]
###########

Base boards / configurations
----------------------------

Supported combinations of base board and module:

- TQ-Systems MBa62xx (REV.010x) with TQMa62xx[L] (REV.010x)
    - tqma62xx_mba62xx_r5_defconfig
    - tqma62xx_mba62xx_a53_defconfig

Supported RAM configurations (LPDDR4):

- 1 GiB
- 2 GiB

Memory layout for R5 SPL
------------------------

At the time of writing, the R5 SPL for the TQMa62xx uses a different memory
layout than the TI EVMs to allow for larger program images:

.. code-block:: none

    HSM SRAM

        0x43c00000+--------------------------+
                  | SPL program image (248K) |
        0x43c3d800+--------------------------+
                  | DM firmware config data  |
        0x43c3e000+--------------------------+
                  | Reserved for ROM         |
        0x43c40000+--------------------------+

    R5 TCM

        0x41010000+--------------------------+
                  | Stack (20K)              |
        0x41014ff0+--------------------------+
                  | Heap (36K)               |
        0x4101dff0+--------------------------+
                  | Reserved                 |
        0x4101e000+--------------------------+
                  | BSS (8K)                 |
        0x41020000+--------------------------+


The layout used by the EVMs is described in the `Processor SDK AM62x Documentation
<https://software-dl.ti.com/processor-sdk-linux/esd/AM62X/latest/exports/docs/linux/Foundational_Components/U-Boot/UG-General-Info.html#sram-memory-layout-during-initial-bootloader-stage>`_.

Support Wiki
------------

See also: `TQMa62xx in the TQ-Embedded Support Wiki
<https://support.tq-group.com/en/arm/tqma62xx>`_
