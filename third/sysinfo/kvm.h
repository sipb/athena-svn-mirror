#ifndef __kvm_h__
#define __kvm_h__
/*
 * $Id: kvm.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
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
