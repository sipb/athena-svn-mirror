/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

#ifndef __kvm_h__
#define __kvm_h__
/*
 * $Id: kvm.h,v 1.1.1.2 1998-02-12 21:32:25 ghudson Exp $
 */

struct _kvmd {
    int			kmemd;
    char	       *namelist;
    char	       *vmfile;
};
typedef struct _kvmd kvm_t;

extern kvm_t	       *kvm_open();
extern int		kvm_close();
extern int		kvm_nlist();
extern int		kvm_read();
extern int		kvm_write();

#endif /* __kvm_h__ */
