#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>

int main (int argc, char **argv) {
  struct passwd *pw;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s command-to-wrap [args]\n", argv[0]);
    exit(255);
  }
  pw = getpwuid(getuid());
  if (pw == NULL) {
    perror("getpwuid() failed");
    exit(1);
  }
  if (initgroups(pw->pw_name, pw->pw_gid) == -1) {
    perror("initgroups() failed");
    exit(1);
  }
  if (setgid(pw->pw_gid)) {
    perror("setgid() failed");
    exit(1);
  }
  if (setuid(pw->pw_uid)) {
    perror("setuid() failed");
    exit(1);
  }
  execvp(argv[1], &argv[1]);
  perror("execvp() failed");
  exit(1);
}
