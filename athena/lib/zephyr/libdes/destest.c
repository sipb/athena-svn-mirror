/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/libdes/destest.c,v $
 * $Author: ghudson $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#ifndef	lint
static char rcsid_destest_c[] =
    "$Id: destest.c,v 1.1 1994-10-31 05:54:16 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <des.h>

char clear[] = "eight bytes";
char cipher[8];
char key[8];
Key_schedule schedule;

main()
{
    int i;
    string_to_key("good morning!", key);
    i = key_sched(key, schedule);
    if (i) {
	printf("bad schedule (%d)\n", i);
	exit(1);
    }
    for (i = 1; i <= 10000; i++)
	des_ecb_encrypt(clear, cipher, schedule, i&1);
    return 0;
}
