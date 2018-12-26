/*
 * syscall reporting example for seccomp
 *
 * Copyright (c) 2012 The Chromium OS Authors <chromium-os-dev@chromium.org>
 * Authors:
 *  Will Drewry <wad@chromium.org>
 *  Kees Cook <keescook@chromium.org>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "syscall-reporter.h"
#include "syscall-names.h"
#include <err.h>

const char * const msg_needed = "Looks like you also need syscall: ";

/* Since "sprintf" is technically not signal-safe, reimplement %d here. */
static void write_uint(char *buf, unsigned int val)
{
	int width = 0;
	unsigned int tens;

	if (val == 0) {
		strcpy(buf, "0");
		return;
	}
	for (tens = val; tens; tens /= 10)
		++ width;
	buf[width] = '\0';
	for (tens = val; tens; tens /= 10)
		buf[--width] = '0' + (tens % 10);
}


static void sig_handler(int signo)
{
	exit(1);
}


