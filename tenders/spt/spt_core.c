/*
 * Copyright (c) 2015-2018 Contributors as noted in the AUTHORS file
 *
 * This file is part of Solo5, a sandboxed execution environment.
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
 * spt_core.c: Core functionality.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <err.h>
#include <libgen.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <seccomp.h>

#include "spt.h"

/*
 * TODO: This is copied from 'hvt' in a rather quick and dirty fashion to get
 * a working "net" and "block"; needs a re-think.
 */

struct spt_module spt_module_core;

struct spt_module *spt_core_modules[] = {
    &spt_module_core,
    &spt_module_net,
    &spt_module_block,
    NULL,
};
#define NUM_MODULES ((sizeof spt_core_modules / sizeof (struct spt_module *)) - 1)

/*
 * TODO: Split up the functions in this module better, and introduce something
 * similar to hvt_gpa_t for clarity.
 */
struct spt *spt_init(size_t mem_size)
{
    struct spt *spt = malloc(sizeof (struct spt));
    if (spt == NULL)
        err(1, "malloc");
    memset(spt, 0, sizeof (struct spt));

    /*
     * spt->mem is addressed starting at 0, however we cannot actually map it
     * at 0 due to restrictions on mapping low memory addresses present in
     * modern Linux kernels (vm.mmap_min_addr sysctl). Therefore, we map
     * spt_mem at SPT_HOST_MEM_BASE, adjusting the returned pointer and region
     * size appropriately.
     */
    spt->mem = mmap((void *)SPT_HOST_MEM_BASE, mem_size - SPT_HOST_MEM_BASE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (spt->mem == MAP_FAILED)
        err(1, "Error allocating guest memory");
    assert(spt->mem == (void *)SPT_HOST_MEM_BASE);
    spt->mem -= SPT_HOST_MEM_BASE;
    spt->mem_size = mem_size;

    spt->sc_ctx = seccomp_init(SCMP_ACT_KILL);

    return spt;
}

void spt_bi_init(struct spt *spt, uint64_t p_end, char **cmdline)
{
    spt->bi = (struct spt_boot_info *)(spt->mem + SPT_BOOT_INFO_BASE);
    memset(spt->bi, 0, sizeof (struct spt_boot_info));
    spt->bi->cmdline = (char *)spt->mem + SPT_CMDLINE_BASE;
    spt->bi->mem_size = spt->mem_size;
    spt->bi->kernel_end = p_end;

    *cmdline = (char *)spt->mem + SPT_CMDLINE_BASE;
}

/*
 * Defined in spt_lauch_<arch>.S.
 */
extern void spt_launch(uint64_t stack_start, void (*fn)(void *), void *arg);

void spt_run(struct spt *spt, uint64_t p_entry)
{
    typedef void (*start_fn_t)(void *arg);
    start_fn_t start_fn = (start_fn_t)(spt->mem + p_entry);
#if defined(__x86_64__)
    uint64_t sp = spt->mem_size - 0x8;
#elif defined(__aarch64__)
    uint64_t sp = spt->mem_size - 16;
#else
#error Unsupported architecture
#endif

    seccomp_load(spt->sc_ctx);

    spt_launch(sp, start_fn, spt->mem + SPT_BOOT_INFO_BASE);

    abort(); /* spt_launch() does not return */
}

static int setup(struct spt *spt)
{
    seccomp_rule_add(spt->sc_ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
            SCMP_A0(SCMP_CMP_EQ, 1));
    seccomp_rule_add(spt->sc_ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(spt->sc_ctx, SCMP_ACT_ALLOW, SCMP_SYS(ppoll), 0);
    seccomp_rule_add(spt->sc_ctx, SCMP_ACT_ALLOW, SCMP_SYS(clock_gettime), 1,
            SCMP_A0(SCMP_CMP_EQ, CLOCK_MONOTONIC));
    seccomp_rule_add(spt->sc_ctx, SCMP_ACT_ALLOW, SCMP_SYS(clock_gettime), 1,
            SCMP_A0(SCMP_CMP_EQ, CLOCK_REALTIME));
    
    return 0;
}

struct spt_module spt_module_core = {
    .name = "core",
    .setup = setup
};
