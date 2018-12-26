/* 
 * Copyright (c) 2015-2017 Contributors as noted in the AUTHORS file
 *
 * This file is part of ukvm, a unikernel monitor.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * ukvm_hv_linux.h: Shared Linux process backend definitions.
 */

#ifndef UKVM_HV_LINUX_H
#define UKVM_HV_LINUX_H

/* 
 * Note: we currently need __UKVM_LINUX__ to be defined in
 * ukvm_guest.h for this backend to be used
 */

#define LINUX_MAP_ADDRESS           _AC(0x40000, UL)
//#define LINUX_HYPERCALL_ADDRESS 0x40000 (defined in ukvm_guest.h) */
#define LINUX_BOOT_INFO_BASE         _AC(0x40100, UL)
#define LINUX_CMDLINE_BASE          _AC(0x40200, UL)

/*
#define AARCH64_MMIO_BASE       _AC(0x100000000, UL)
#define AARCH64_MMIO_SZ         _AC(0x40000000, UL)
#define AARCH64_GUEST_MIN_BASE  _AC(0x100000, UL)
#define AARCH64_PAGE_TABLE      _AC(0x10000, UL)
#define AARCH64_CMDLINE_BASE    _AC(0xC000, UL)
#define AARCH64_CMDLINE_SZ      (AARCH64_PAGE_TABLE - AARCH64_CMDLINE_BASE)
#define AARCH64_BOOT_INFO       _AC(0x1000, UL)
#define AARCH64_BOOT_INFO_SZ    (AARCH64_CMDLINE_BASE - AARCH64_BOOT_INFO)
*/

/*x86
 *                        0x10000 = LINUX_MAP_ADDRESS
 *           |--unmapped--|--|--|-------|--unikernel....
 * hv->mem = 0            0x10000 = LINUX_HYPERCALL_ADDRESS 
 *                           0x10100 = LINUX_BOOT_INFO_BASE
 *                              0x10200 = LINUX_CMDLINE_BASE
 *                                      0x100000 = UNIKERNEL
 */

/*aarch64
 *                        0x40000 = LINUX_MAP_ADDRESS
 *           |--unmapped--|--|--|-------|--unikernel....
 * hv->mem = 0            0x40000 = LINUX_HYPERCALL_ADDRESS 
 *                           0x40100 = LINUX_BOOT_INFO_BASE
 *                              0x40200 = LINUX_CMDLINE_BASE
 *                                      0x100000 = UNIKERNEL
 */

struct ukvm_hvb {
    uint8_t *realmem; /* the guest memory allocation */
    uint64_t entry;
    void *arg;
};

#endif /* UKVM_HV_LINUX_H */
