/* $Id: asn1-common.h,v 1.1.1.1 2002-12-13 20:42:06 zacheiss Exp $ */

#include <stddef.h>
#include <time.h>

#ifndef __asn1_common_definitions__
#define __asn1_common_definitions__

typedef struct octet_string {
    size_t length;
    void *data;
} octet_string;

typedef char *general_string;

typedef struct oid {
    size_t length;
    unsigned *components;
} oid;

#endif