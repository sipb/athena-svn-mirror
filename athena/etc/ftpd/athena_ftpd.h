#include <sys/types.h>

extern uid_t athena_setuid;
extern gid_t athena_setgid;

extern int athena;
extern int athena_login;

#define LOGIN_NONE 0
#define LOGIN_LOCAL 1
#define LOGIN_KERBEROS 2

extern struct passwd *athena_getpwnam();
extern char *athena_authenticate();
extern char *athena_attachhomedir();
extern char *athena_attach();
