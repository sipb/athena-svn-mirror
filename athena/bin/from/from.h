/* $Id: from.h,v 1.1 1999-09-21 01:40:07 danw Exp $ */

int pop_init(char *host);
int pop_command(char *fmt, ...);
int pop_stat(int *nmsgs, int *nbytes);
int putline(char *buf, char *err, FILE *f);
int getline(char *buf, int n, FILE *f);
int multiline(char *buf, int n, FILE *f);
