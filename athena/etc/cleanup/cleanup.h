#define MAXUSERS 1024
#define MAXPROCS 1024
#define ROOTUID 0
#define DAEMONUID 1

#define MDEBUG 0
#define LOGGED_IN 1
#define PASSWD 2

struct cl_proc {
    int pid;
    int uid;
};
