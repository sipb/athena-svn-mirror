/* Copyright (C) 1991 QMS, Inc */
#define PRINTER_ADDR "125.0.0.3"

#define PRINTER_PORT 35
#define PRINTER_TCP_PORT  PRINTER_PORT 
#define PRINTER_UDP_PORT  PRINTER_PORT 

#define MAX_STRING_SIZE 512
#define MAX_REPLY_SIZE 256

#ifdef SYSV
#ifdef sparc
#define TCP_PROVIDER "/dev/tcp"
#define UDP_PROVIDER "/dev/udp"
#else
#define TCP_PROVIDER "/dev/inet/tcp"
#define UDP_PROVIDER "/dev/inet/udp"
#endif
#define BUFSIZE       4096
#define POLL_TIMEOUT (99 * 10) /* in msecs */
#else
#define BUFSIZE       16384
#endif

#define UDP_TIMEOUT 99 /* in 1/100th of a second */
#define SLEEPTIME   10
#define NUMRETRY   10

#define STATFILE "./statfile"
#define STATUSINTERVAL  20  /* msecs */

/* Status file codes */

#define STABASE  512
#define STAJSTART (STABASE + 0)
#define STAFSTART (STABASE + 4)
#define STANCOMP  (STABASE + 8)
#define STARETRY  (STABASE + 12)
#define STAACOMP  (STABASE + 16)
#define STASTERM  (STABASE + 20)
#define STAJCOMP  (STABASE + 24)


struct printer_pkt{
	unsigned short pr_length;
	unsigned char  pr_status;
	unsigned char  pr_spare;
	unsigned long  pr_age;
	unsigned long  pr_supportmask;
	unsigned long  pr_acceptmask;
	unsigned short pr_stroffset;
	unsigned short pr_strlength;
};

struct file_list {
	char filename[MAX_STRING_SIZE];
	struct file_list *next;
};

struct command_opt {
	short input_stdin;
	char hostname[MAX_STRING_SIZE];
	char statfile[MAX_STRING_SIZE];
	struct file_list *flist; /* List of filenames to be printed */
	int cpid; /* it's not really a command argument. But it's a good
		   * (though not neat) way of stashing way the child process's
		   * pid thus obviating the need to pass it as an argument from
		   * zillion different places.
		   */
};


/*
 * Printer status.
 */
#define PRINTER_READY 1


#ifdef DEBUG
#define DBGPRT(a) printf a;
#else
#define DBGPRT(a) 
#endif

#ifndef TRUE
#define TRUE 1
#endif




