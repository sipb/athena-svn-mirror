/**********************************************************************
 * testpw.c -- a test program for adding people to the passwd file
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <stdio.h>

main(argc, argv)
     int argc;
     char *argv[];
{
  long code;
  ALsessionStruct sessionp;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s username\n", argv[0]);
      exit(1);
    }

#define ERROR() { com_err(argv[0], code, ALcontext(&sessionp)); exit(1); }

  code = ALsetUser(&sessionp, argv[1], ALflagNone);
  if (code) ERROR();

  code = ALgetGroups(&sessionp);
  if (code) ERROR();

  code = ALmodifyLinesOfFile(&sessionp, "group.test", "group.test.tmp",
			     ALmodifyGroupAdd, ALappendGroups);
  if (code) ERROR();

  exit(0);
}
