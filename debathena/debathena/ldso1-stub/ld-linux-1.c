#include <unistd.h>

int main() {
#define PRINT(x) write(2, x, sizeof(x))
  PRINT(
"Error:\n"
"  The software you are trying to run is too old for this system. Please\n"
"  ask its maintainer to compile a newer version of the software for\n"
"  Debathena (amd64_deb60). If you have questions, contact <debathena@mit.edu>.\n\n"
"  (ld-linux.so.1 has not been supported on Ubuntu since 2006 or\n"
"  Debian since 2007.)\n");
  return 72; //EX_OSFILE "Some system file does not exist..." from FreeBSD
}
