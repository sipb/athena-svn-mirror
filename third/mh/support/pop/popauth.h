/* popauth.h - POP authorization DB definitions */
/* @(#)$Id: popauth.h,v 1.1.1.1 1996-10-07 07:14:06 ghudson Exp $ */


struct authinfo {
    char    auth_secret[16];
    int	    auth_secretlen;
};
