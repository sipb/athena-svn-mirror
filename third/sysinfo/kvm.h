/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef __kvm_h__
#define __kvm_h__
/*
 * $Revision: 1.1.1.3 $
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
