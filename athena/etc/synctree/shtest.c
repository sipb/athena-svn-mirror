#include <stdio.h>
main()
{
  
  int pid, pipefds[2];
  FILE *f;

  pipe(pipefds);
  pid = vfork();

  if (pid == 0) {
    dup2(pipefds[0], 0);
    execl("/bin/sh", "/bin/sh", "-s", 0);
  } else {
    f = fdopen(pipefds[1],"w");
    fprintf(f,"/bin/echo testing\n");
    fprintf(f,"kill $$\n");
    fclose(f);
  }

  (void) wait(0);
}
