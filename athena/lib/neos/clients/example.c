#include <stdio.h>
#include <fxcl.h>

main(argc, argv)
     int argc;
     char *argv[];
{
  FX *fxp;
  long code;
  Paper p;
  FILE *pipe;

  /* Check command line usage */
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <exchange>\n", argv[0]);
    exit(1);
  }
  /* Open the file exchange */
  fxp = fx_open(argv[1], &code);
  if (!fxp) {
    com_err(argv[0], code, "while opening %s", argv[1]);
    exit(1);
  }
  /* Send the essay.dvi file */
  paper_clear(&p);
  code = fx_send_file(fxp, &p, "essay.dvi");
  if (code) {
    com_err(argv[0], code, "while sending essay.dvi");
    exit(1);
  }
  /* Open a pipe to the dvi/Postscript converter */
  pipe = popen("dvi2ps -r essay.dvi", "r");
  if (!pipe) {
    fprintf(stderr, "%s: Could not run dvi2ps\n", argv[0]);
    exit(1);
  }
  /* Send the essay.PS file */
  p.filename = "essay.PS";
  code = fx_send(fxp, &p, pipe);
  if (code) {
    com_err(argv[0], code, "while sending essay.PS");
    exit(1);
  }
  fx_close(fxp);
  (void) pclose(pipe);
  exit(0);
}
