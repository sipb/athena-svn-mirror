/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: kvmlib.c,v 1.1.1.1 1996-10-07 20:16:50 ghudson Exp $";
#endif

#include "defs.h"

#if defined(NEED_KVM)

#include <stdio.h>
#include <sys/errno.h>
#include "kvm.h"

#ifndef SYSFAIL
#define SYSFAIL -1
#endif

#if defined(DEBUG) && !defined(SYSERR)
extern int errno;
extern char sys_errlist[];
#define SYSERR sys_errlist[errno]
#endif

char *strdup();

#if	!defined(NAMELIST)
#define NAMELIST	"/vmunix"
#endif	/* NAMELIST */
#if	!defined(KMEMFILE)
#define KMEMFILE	"/dev/kmem"
#endif	/* KMEMFILE */

/*
 * Close things down.
 */
extern int kvm_close(kd)
    kvm_t 		       *kd;
{
    if (!kd)
	return(-1);

    if (kd->kmemd)
	close(kd->kmemd);
    if (kd->namelist)
	free(kd->namelist);
    if (kd->vmfile)
	free(kd->vmfile);

    free(kd);

    return(0);
}

/*
 * Open things up.
 */
extern kvm_t *kvm_open(NameList, CoreFile, SwapFile, Flag, ErrStr)
    char 		       *NameList;
    char 		       *CoreFile;
    char 		       *SwapFile;
    int 			Flag;
    char 		       *ErrStr;
{
    kvm_t *kd;

    if ((kd = (kvm_t *) malloc(sizeof(kvm_t))) == NULL) {
#ifdef DEBUG
	fprintf(stderr, "kvm_open() malloc %d bytes failed!\n", sizeof(kvm_t));
#endif
	return((kvm_t *) NULL);
    }

    if (NameList)
	kd->namelist = strdup(NameList);
    else
	kd->namelist = strdup(NAMELIST);

    if (CoreFile)
	kd->vmfile = strdup(CoreFile);
    else
	kd->vmfile = strdup(KMEMFILE);

    if ((kd->kmemd = open(kd->vmfile, Flag, 0)) == SYSFAIL) {
#ifdef DEBUG
	fprintf(stderr, "kvm_open() open '%s' failed: %s.\n", kd->vmfile, 
		SYSERR);
#endif
	return((kvm_t *) NULL);
    }

    return(kd);
}

/*
 * KVM read function
 */
extern int kvm_read(kd, Addr, Buf, NBytes)
    kvm_t 		       *kd;
    OFF_T_TYPE	 		Addr;
    char 		       *Buf;
    size_t 			NBytes;
{
    size_t 			RetVal;

    if (!kd) {
#ifdef DEBUG
	fprintf(stderr, "kvm_read(): invalid kd param\n");
#endif
	return(SYSFAIL);
    }

    if (lseek(kd->kmemd, Addr, 0) == SYSFAIL) {
#ifdef DEBUG
	fprintf(stderr, "kvm_read(): lseek failed (desc %d addr 0x%x): %s\n",
		kd->kmemd, Addr, SYSERR);
#endif
	return(SYSFAIL);
    }

    if ((RetVal = read(kd->kmemd, Buf, NBytes)) != NBytes) {
#ifdef DEBUG
	fprintf(stderr, 
		"kvm_read(): read failed (desc %d buf 0x%x size %d): %s\n",
		kd->kmemd, Buf, NBytes, SYSERR);
#endif
	return(SYSFAIL);
    }

    return(RetVal);
}

/*
 * KVM write function
 */
extern int kvm_write(kd, Addr, Buf, NBytes)
     kvm_t 		       *kd;
     unsigned long 		Addr;
     char 		       *Buf;
     unsigned 			NBytes;
{
    unsigned 			ret;

    if (!kd) {
	return(SYSFAIL);
    }

    if (lseek(kd->kmemd, Addr, 0) == SYSFAIL) {
	return(SYSFAIL);
    }

    if ((ret = write(kd->kmemd, Buf, NBytes)) != NBytes) {
	return(SYSFAIL);
    }

    return(ret);
}

/*
 * Perform an nlist()
 */
#if	defined(HAVE_NLIST)
extern int kvm_nlist(kd, nl)
     kvm_t 		       *kd;
     nlist_t	 	       *nl;
{
     int status;

    if (!kd)
	return(SYSFAIL);

    return(NLIST_FUNC(kd->namelist, nl));
}
#endif	/* HAVE_NLIST */

#endif /* NEED_KVM */
