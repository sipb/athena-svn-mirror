/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 * U U I D _ G E N 
 *
 * A simple program that generates and prints a new UUID.
 */

#include "std.h"

#ifdef DSEE
#include "$(nbase.idl).h"
#include "$(uuid.idl).h"
#else
#include "nbase.h"
#include "uuid.h"
#endif 
#define version "uuid_gen/nck version 1.5.1"
#define FALSE 0
#define TRUE !FALSE
boolean match_command(key,str,min_len)
   char *key;
   char *str;
   int min_len;
{
    int i = 0;

    if (*key) while (*key == *str) {
        i++;
        key++;
        str++;
        if (*str == '\0' || *key == '\0')
            break;
    }
    if (*str == '\0' && i >= min_len)
        return TRUE;
    return FALSE;
}

main(argc, argv)
int argc;
char *argv[];
{
    uuid_$t uuid;
    uuid_$string_t buff;
    status_$t st;

    uuid_$gen(&uuid);
    uuid_$encode(&uuid, buff);

    if (argc-- > 1) {
		while (argc--) {
            argv++; 
            if (match_command("-version", *argv, 2))
            {
                printf("%s\n", version);
                exit(0);
            }

            if (strcmp(*argv, "-p") == 0)
                printf("%%pascal\n[\nuuid(%s),\nversion(1)\n]\ninterface INTERFACENAME;\n\nend;\n", buff);
            else if (strcmp(*argv, "-c") == 0)
                printf("%%c\n[\nuuid(%s),\nversion(1)\n]\ninterface INTERFACENAME {\n\n}\n", buff);
            else if (strcmp(*argv, "-t") == 0)
                printf("%s\n", buff);
            else if (strcmp(*argv, "-C") == 0)
                printf("\
= { 0x%8.8x,\n\
    0x%4.4x,\n\
    0x%4.4x,\n\
    0x%2.2x,\n\
    {0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x} };\n",
                        uuid.time_high, uuid.time_low, uuid.reserved, uuid.family,
                        (unsigned char) uuid.host[0], (unsigned char) uuid.host[1],
                        (unsigned char) uuid.host[2], (unsigned char) uuid.host[3],
                        (unsigned char) uuid.host[4], (unsigned char) uuid.host[5], 
                        (unsigned char) uuid.host[6]);
            else if (strcmp(*argv, "-P") == 0)
                printf("\
:= [\n\
    time_high := 16#%8.8x,\n\
    time_low := 16#%4.4x,\n\
    reserved := 16#%4.4x,\n\
    family := chr(16#%2.2x),\n\
    host := [chr(16#%2.2x), chr(16#%2.2x), chr(16#%2.2x), chr(16#%2.2x), chr(16#%2.2x), chr(16#%2.2x), chr(16#%2.2x)]\n\
    ]\n",
                        uuid.time_high, uuid.time_low, uuid.reserved, uuid.family,
                        (unsigned char) uuid.host[0], (unsigned char) uuid.host[1],
                        (unsigned char) uuid.host[2], (unsigned char) uuid.host[3],
                        (unsigned char) uuid.host[4], (unsigned char) uuid.host[5], 
                        (unsigned char) uuid.host[6]);

            else {
                fprintf(stderr, "usage: uuid_gen {[-p -c -P -C- t] | [-v[ersion]]}\n");
                break;
            }
        }
    } else {
        printf("%s\n", buff);
    }
}
