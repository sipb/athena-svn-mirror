
#include "stackenv.h"
#include "mktex.h"

int main(int argc, char* argv[])
{
  program_description * program = 0;
  int i, errstatus;
#if defined(WIN32)
  /* if _DEBUG is not defined, these macros will result in nothing. */
   SETUP_CRTDBG;
   /* Set the debug-heap flag so that freed blocks are kept on the
    linked list, to catch any inadvertent use of freed memory */
   SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF );
#endif

#if 0
  extern MKTEXDLL string (* var_lookup)(const_string);
  var_lookup = getval;
#endif

  mktexinit(argc, argv);

  for(i = 0; makedesc[i].name; ++i)
    if (FILESTRCASEEQ(kpse_program_name, makedesc[i].name)) {
      program_number = i;
      progname = makedesc[i].name;
      program = makedesc+i;
      break;
    }
  if (!makedesc[i].name) {
    fprintf(stderr, "This program was incorrectly copied to the name %s\n", 
	    argv[0]);
    return 1;
  }

  /* mktex_opt may modify argc and shift argv */
  argc = mktex_opt(argc, argv, program);

  errstatus = program->prog(argc, argv);

  mt_exit(errstatus);
  return 0;			/* Not reached !!! */
}
