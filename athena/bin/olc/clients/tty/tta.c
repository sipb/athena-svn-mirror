main()
{
  char *out;
  char * getline();
  int done_gl_init = 0;

  while (1) {
  if (! done_gl_init) {
    /* should really get the terminal line width.. */
    gl_init(80);
    done_gl_init = 1;
  } else {
    gl_char_init();
  }

    out = getline("Hello>",0);
    printf("out = %s\n",out);
    gl_char_cleanup();
    sleep(5);
  }
}
