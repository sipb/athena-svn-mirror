/****************************************************************/
/*  getflags.c  - command line parser                           */
/****************************************************************/

/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */



#include <stdio.h>
#include <string.h>
#include "idl_base.h"
#include "getflags.h"
#include "sysdep.h"

#define NFLAGS 128
static unsigned char option_count[NFLAGS] = {0};
static int other_count = 0;
static char *(other_flags[NFLAGS]) = {0};

extern void exit __PROTOTYPE((int));
    
int flags_option_count(table, option)
OPTIONS table[];
char *option;
{
    int o;
    if (*option == '-') option++;
    for (o = 0; table[o].option && o < NFLAGS; o++) {
        if(strcmp(option, table[o].option) == 0)
            return((int)option_count[o]);
        }
    return(-1);
}

void flags_incr_count(table, option, delta)
OPTIONS table[];
char *option;
int delta;
{
    int o;
    if (*option == '-') option++;
    for(o = 0; table[o].option && o < NFLAGS; o++) {
        if(strlen(option) != strlen(table[o].option))
            continue;
        if(strcmp(option, table[o].option) == 0) {
            option_count[o] += delta;
            return;
            }
        }
}

int flags_other_count()
{
  return(other_count);
}

char *flags_other(index)
int index;
{
  if(0 <= index && index < other_count)
    return(other_flags[index]);
  else
    return(NULL);
}
      

void getflags(ac, av, table)
int ac;
char **av;
OPTIONS table[];
{
    int o;
    int nflags, type;
    int vflag;
    register char **pstring;
    register char *pchar;
    register int *pint;
    register char *flag;
    register long *plong;
    register double *pfloat;

    while (ac > 0) 
    {
    thisf:
        for (o = 0;  table[o].option;  o++)
        {
            flag = *av;
            if (flag[0] == '-')
                flag++;
            if (strlen(flag) != strlen(table[o].option))
                continue;
            if (strcmp(flag, table[o].option) == 0)
            {
                nflags = (table[o].ftype >> 8) & 0xFF;
                vflag = nflags & VARARGFLAG;
                nflags &= MULTARGMASK;
                if (nflags <= 0)
                    nflags = 1;
                type = table[o].ftype & 0xFF;
                switch (type)
                {
                default:
                    fprintf(stderr,
                            "Illegal option ftype in getflags: %d\n",
                            table[o].ftype);
                    exit(1);
                case INTARG:
                    pint = (int *)table[o].dest;
                    while (nflags-- && (ac > 1))
                    {
                        GETINT(*pint++);
                        if (ac > 0 && vflag && **av == '-') goto thisf;
                        option_count[o]++;
                    }
                    goto nextf;
                case STRARG:
                    pstring = (char **)table[o].dest;
                    if (vflag)
                        pstring += option_count[o];
                    while (nflags-- && (ac > 1))
                    {
                        GETSTR(*pstring);
                        if(ac > 0 && vflag && **av == '-') {
                            *pstring = NULL;
                            goto thisf;
                        }       
                        pstring++;
                        option_count[o]++;
                    }
                    goto nextf;
                case TOGGLEARG:
                    pchar = (char *)table[o].dest;
                    *pchar = ~*pchar;
                    goto nextf;
                case ASSERTARG:
                    pchar = (char *)table[o].dest;
                    *pchar = true;
                    goto nextf;
                case DENYARG:
                    pchar = (char *)table[o].dest;
                    *pchar = false;
                    goto nextf;
                case CHRARG:
                    pchar = (char *)table[o].dest;
                    while (nflags-- && (ac > 1))
                    {
                        GETCH(*pchar++);
                        if(ac > 0 && vflag && **av == '-') goto thisf;
                        option_count[o]++;
                    }
                    goto nextf;
                case FLTARG:
                    pfloat = (double *)table[o].dest;
                    while (nflags-- && (ac > 1))
                    {
                        GETFLT(*pfloat++);
                        if(ac > 0 && vflag && **av == '-') goto thisf;
                        option_count[o]++;
                    }   
                    goto nextf;
                case LONGARG:
                    plong = (long *)table[o].dest;
                    while (nflags-- && (ac > 1))
                    {
                        GETLONG(*plong++);
                        if(ac > 0 && vflag && **av == '-') goto thisf;
                        option_count[o]++;
                    }
                    goto nextf;
                }
            }
        }
        if(**av == '-')
        {
            fprintf(stderr,
                    "Unknown flag (%s) in getflags\n", *av);
            printflags(table);
            exit(1);
        }
        else
        {
            other_flags[other_count++] = *av;
        }
    nextf:
        ac--;
        av++;
    }
}

#define yes_no(x) (x? "Yes" : "No")
#define no_yes(x) (x? "No" : "Yes")
void printflags(table)
OPTIONS table[];
    {
    register int o;
    register int nflags;
    register int type;
    int vflag;
    int *pint;
    char *pchar;
    char **pstring;
    long *plong;
    double *pdouble;
    int option_len;
    option_len = 0;
    for(o = 0; table[o].option; o++)
      if(strlen(table[o].option) > option_len)
         option_len = strlen(table[o].option);
    option_len += 3;
    fprintf(stderr, "Options table:\n");
    for (o = 0;  table[o].option;  o++)
        {
        type = table[o].ftype;
        if(type & HIDARG) continue;
        nflags = (type >> 8) & 0xFF;
        vflag = nflags & VARARGFLAG;
        if (vflag)
          nflags = option_count[o];
        type &= 255;
        fprintf(stderr, "    %-*s", option_len, table[o].option);
        if (!vflag && nflags <= 0)
          nflags = 1;
        switch (type)
          {
          default:
            fprintf(stderr, "\tillegal option in printflags: %d\n",
                table[o].ftype);
            exit(1);
          case INTARG:
            pint = (int *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%d", *pint++);
            fprintf(stderr, "\n");
            break;
          case STRARG:
            pstring = (char **)table[o].dest;
            while (nflags-- > -0)
              fprintf(stderr, "\t%s", *pstring++);
            fprintf(stderr, "\n");
            break;
          case TOGGLEARG:
          case ASSERTARG:
            pchar = (char *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%s", yes_no(*pchar++));
            fprintf(stderr, "\n");
            break;
          case DENYARG:
            pchar = (char *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%s", no_yes(*pchar++));
            fprintf(stderr, "\n");
            break;
          case CHRARG:
            pchar = (char *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%c", *pchar++);
            fprintf(stderr, "\n");
            break;
          case FLTARG:
            pdouble = (double *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%.3f", *pdouble++);
            fprintf(stderr, "\n");
            break;
          case LONGARG:
            plong = (long *)table[o].dest;
            while (nflags-- > 0)
              fprintf(stderr, "\t%ld", *plong++);
            fprintf(stderr, "\n");
            break;
          }
          }
    fprintf(stderr, "\n");
    }
