#include <AL/AL.h>
#include <stdio.h>

#define ERROR() { com_err(argv[0], code, ALcontext(&sess)); exit(1); }

int
main(int argc, char **argv)
{
  ALsessionStruct sess;
  ALut ut;
  long code;

  code = ALinit();
  if (code) ERROR();

  code = ALinitSession(&sess);
  if (code) ERROR();

  code = ALinitUtmp(&sess);
  if (code) ERROR();

  ut.user = "nanny";
  ut.host = "insanity";
  ut.line = "ttyq42";
  ut.id = "";
  ut.type = ALutDEAD_PROC;

  code = ALsetUtmpInfo(&sess, ALutUSER | ALutHOST | ALutLINE
		      /* | ALutID */ | ALutTYPE, &ut);
  if (code) ERROR();

  code = ALputUtmp(&sess);
  if (code) ERROR();
}
