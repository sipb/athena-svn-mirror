/*
 * make_commands.c
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/cns/src/util/ss/make_commands.c,v 1.1.1.1 1996-09-06 00:47:58 ghudson Exp $
 * $Locker:  $
 *
 * Copyright 1987 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */
#include "mit-sipb-copyright.h"
#include <stdio.h>
#include <sys/file.h>
#include <strings.h>
#include "ss.h"

static char copyright[] = "Copyright 1987 by MIT Student Information Processing Board";

extern char *malloc();
extern char *last_token;
extern FILE *output_file;

extern FILE *yyin, *yyout;
extern int yylineno;

main(argc, argv)
     int argc;
     char **argv;
{
     char c_file[BUFSIZ];
     char o_file[BUFSIZ];
     char z_file[BUFSIZ];
     char *path, *p;
     int dont_delete;
     
     if (argc == 1 || argc > 4 || (argc == 3 && strcmp("-n", argv[2]))) {
	  fprintf(stderr, "Usage: %s cmdtbl.ct [-n]\n", argv[0]);
	  exit(1);
     }
     
     dont_delete = (argc == 3);
     
     path = malloc(strlen(argv[1])+4); /* extra space to add ".ct" */
     strcpy(path, argv[1]);
     p = rindex(path, '/');
     if (p == (char *)NULL)
	  p = path;
     else
	  p++;
     if (rindex(p, '.') == (char *)NULL)
	  strcat(path, ".ct");
     yyin = fopen(path, "r");
     if (!yyin) {
	  perror(path);
	  exit(1);
     }
     
     {
	  register int pid = getpid();
	  register int len;
	  sprintf(c_file, "/tmp/cmd%d.c", pid);
	  len = strlen(c_file)-1;
	  strcpy(o_file, c_file);
	  strcpy(z_file, c_file);
	  o_file[len] = 'o';
	  z_file[len] = 'z';
     }
     output_file = fopen(c_file, "w+");
     if (!output_file) {
	  perror(c_file);
	  exit(1);
     }
     
     fputs("typedef struct {\n\tchar **names;\n\t", output_file);
     fputs(FUNCTION_TYPE_NAME, output_file);
     fputs(" (*f)();\n\tchar *info;\n\tint flags;\n} ss_request_entry;\n\n",
	   output_file);
     fputs("typedef struct {\n\tint version;\n\tss_request_entry *requests;\n} ss_request_table;\n\n",
	   output_file);
     /* parse it */
     yyparse();
     /* put file descriptors back where they belong */
     fclose(yyin);		/* bye bye input file */
     fclose(output_file);	/* bye bye output file */
     
     /* now compile it */
     if (!vfork()) {
	  chdir("/tmp");
	  execl("/bin/cc", "cc", "-c", "-R", "-O", c_file, 0);
	  perror("/bin/cc");
	  _exit(1);
     }
     else wait(0);
     if (!dont_delete)
	  unlink(c_file);

     /* crunch out extra symbols */
     if (!rename(o_file, z_file)) {
	  if (!vfork()) {
	       chdir("/tmp");
	       execl("/bin/ld", "ld", "-o", o_file+5, "-s", "-r", /* "-n", */
		     z_file+5, 0);
	       perror("/bin/ld");
	       _exit(1);
	  }
	  else wait(0);
	  unlink(z_file);
     }

     /* and move it back */
     p = rindex(path, '/');
     if (p == (char *)NULL) p = path;
     else p++;
     p = rindex(p, '.');
     p++;			/* "c" */
     *p = 'o';
     p++;			/* "t" */
     *p = '\0';
     /* this should be mv but rfs loses sometimes.. */
     if (!vfork()) {
	  execl("/bin/cp", "cp", o_file, path, 0);
	  perror("/bin/cp");
	  _exit(1);
     }
     else wait(0);
     if (!vfork()) {
	  execl("/bin/chmod", "chmod", "a-x", path, 0);
	  perror("/bin/chmod");
	  _exit(1);
     }
     else wait(0);
     unlink (o_file);
     
     exit(0);
}

yyerror(s)
     char *s;
{
     fputs(s, stderr);
     fprintf(stderr, "\nLine %d; last token was '%s'\n",
	     yylineno, last_token);
}
