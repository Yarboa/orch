/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "parse-util.h"

bool parse_long(const char *in, long *ret) {
        const int base = 10;
        char *x = NULL;

        if (in == NULL || strlen(in) == 0) {
                return false;
        }

        errno = 0;
        *ret = strtol(in, &x, base);

        if (errno > 0 || !x || x == in || *x != 0) {
                return false;
        }

        return true;
}

bool parse_port(const char *in, uint16_t *ret) {
        if (in == NULL || strlen(in) == 0) {
                return false;
        }

        bool r = false;
        long l = 0;
        r = parse_long(in, &l);

        if (!r || l < 0 || l > USHRT_MAX) {
                return false;
        }

        *ret = (uint16_t) l;
        return true;
}
