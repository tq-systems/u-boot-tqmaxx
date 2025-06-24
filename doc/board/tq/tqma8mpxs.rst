.. SPDX-License-Identifier: GPL-2.0+

TQ-Systems GmbH TQMa8MPxS SoM Family
====================================

The TQ-Systems TQMa8MPxS are System-on-Module in SMARC-2 form factor based on
the NXP i.MX8MP SoC family.

Module variants
---------------

Currently, following TQMa8MPxS variants with LPDDR4 RAM are supported:

* 1 GiB
* 2 GiB
* 4 GiB
* 8 GiB

The RAM configuration, form factor and optional features (SPI-NOR,
Secure Element, ...) are detected automatically based on VARD
(Variant and Revision Detection) data read from an EEPROM.

Base boards / configurations
----------------------------

The following combinations of base board and SOM are supported:

* TQ-Systems TQMa8MPxS (REV.010x prototypes) on MB-SMARC-2 (REV.010x).

Boot sequence
-------------

The boot firmware on the i.MX8MP is loaded in multiple stages. Two of these stages
are provided by this U-Boot repository:

* i.MX8MP U-Boot SPL
* i.MX8MP U-Boot (`u-boot.img`, combined with DTB)

The SPL performs the following actions:

* Initialize CPU
* Initialize DDR controller
* Load U-Boot and DTB into DDR RAM
* Boot TF-A

The next stage is handled by the TF-A. It will optionally start the OP-TEE and
the full A52 U-Boot from fitImage container.

Finally, the full U-Boot runs, which can provide a command line interface
and boot into the actual OS depending on its configuration.

Build
-----

The SPL and full U-Boot are both built from the given defconfigs. The build
requires an ARM-v8/AArch64 cross compiler.

Boot from SD and eMMC
---------------------

**Attention**: partition in this context means the eMMC hardware partitions. This is unrelated to MBR or
GPT partitions in eMMC user partition.

The following table applies:

+------------+-----------------+-------------------+
|   Device   |     offset      | Block (512 Bytes) |
+============+=================+===================+
|  SD-Card   | 32 KiB (0x8000) |     64 / 0x40     |
+------------+-----------------+-------------------+
| eMMC USER  | 32 KiB (0x8000) |     64 / 0x40     |
+------------+-----------------+-------------------+
| eMMC BOOT  | 0 KiB  (0x0000) |      0 / 0x00     |
+------------+-----------------+-------------------+

.. code-block:: bash

    # assign bstart with start block number in hex
    # use correct device
    # eMMC:                0
    # SD-Card:             1
    # use correct partition
    # SD-Card:             0
    # eMMC user partition: 0
    # eMMC boot partition: 1 or 2
    # assign partnum with partition number to use

    setenv devnum <0|1>
    setenv bstart <block number>
    setenv partnum <part number>
    tftp <bootstream>
    setexpr bsz ${filesize} + 1ff
    setexpr bsz ${bsz} / 200
    printenv bsz
    mmc dev ${devnum} ${partnum}
    mmc write ${loadaddr} ${bstart} ${bsz}
    mmc dev ${devnum} 0

MB-SMARC-2 DIP settings for boot sources
----------------------------------------------------------

* DIP switch: S3

+-------------------+-------------------+-----+-----+-----+-----+
| Function          | BOOT MODE         | 1   | 2   | 3   | 4   |
+===================+===================+=====+=====+=====+=====+
| Force Recovery    | Serial Downloader |  x  | x   |     | on  |
+-------------------+-------------------+-----+-----+-----+-----+
| e-MMC (USDHC3)    | 0010              | on  | off | off | off |
+-------------------+-------------------+-----+-----+-----+-----+
| SD Card (USDHC2)  | 0011              | off | on  | on  | off |
+-------------------+-------------------+-----+-----+-----+-----+
| FlexSPI / 3B Read | 0110              | on  | on  | off | off |
+-------------------+-------------------+-----+-----+-----+-----+

Memory usage in SPL
-------------------

SRAM (OCRAM)

+------------+------------+------+---------------------------------------+
| Start      | End        | Size | Note                                  |
+============+============+======+=======================================+
| 0x00180000 | 0x00188FFF |  36K | SAVED_DRAM_TIMING_BASE (OCRAM_S)      |
+------------+------------+------+---------------------------------------+
| 0x00900000 | 0x00917FFF |  96K | ROM reserved                          |
+------------+------------+------+---------------------------------------+
| 0x00918000 | 0x0091ffff |  32K | TBD.                                  |
+------------+------------+------+---------------------------------------+
| 0x00920000 | 0x00945FFF | 152K | SPL (SPL_TEXT_BASE + SPL_MAX_SIZE)    |
+------------+------------+------+---------------------------------------+
| 0x0096DFF0 |            |      | SPL_STACK before DRAM                 |
+------------+------------+------+---------------------------------------+
| 0x0096E000 | 0x0096FFFF |   8K | SPL_BSS_START_ADDR + SPL_BSS_MAX_SIZE |
+------------+------------+------+---------------------------------------+
| 0x00970000 | 0x0098FFFF | 128K | TF-A (BL31_BASE + BL31_LIMIT)         |
+------------+------------+------+---------------------------------------+

DRAM

+------------+------------+------+----------------------------------------------------+
| Start      | End        | Size | Note                                               |
+============+============+======+====================================================+
| 0x40000000 | 0x400003ff |   1K | BLOBLIST                                           |
+------------+------------+------+----------------------------------------------------+
| 0x40200000 |            |   2M | PLAT_NS_IMAGE_OFFSET + PLAT_NS_IMAGE_SIZE (TF-A)   |
|            |            | 512K | CONFIG_TEXT_BASE + CONFIG_SYS_MONITOR_LEN (U-Boot) |
+------------+------------+------+----------------------------------------------------+
| 0x56000000 | 0x57FFFFFF |  32M | BL32_BASE + BL32_SIZE (optee)                      |
+------------+------------+------+----------------------------------------------------+
