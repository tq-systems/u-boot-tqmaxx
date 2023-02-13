/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

/**
 * Initialize USB gadget at port 0 (primary used for SDP / Fastboot)
 *
 * @return 0 on success, negative errno otherwise
 * @param[in] maximum_speed one of the predefined USB_SPEED_xyz
 *
 * The function verifies maximum_speed and tries to initialize USB
 * port 0 with the requested speed. In SPL only USB_SPEED_HIGH is
 * allowed.
 */
int tqma8mpxl_usb_dwc3_gadget_init(enum usb_device_speed maximum_speed);
