/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/fssync.h,v 2.1 1990/08/07 19:47:26 hines Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/fssync.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidfssync = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/fssync.h,v 2.1 1990/08/07 19:47:26 hines Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*

	System:		VICE-TWO
	Module:		fssync.h
	Institution:	The Information Technology Center, Carnegie-Mellon University

 */


/* FSYNC commands */

#define FSYNC_ON		1 /* Volume online */
#define FSYNC_OFF		2 /* Volume offline */
#define FSYNC_LISTVOLUMES	3 /* Update local volume list */
#define FSYNC_NEEDVOLUME	4 /* Put volume in whatever mode (offline, or whatever)
				     best fits the attachment mode provided in reason */
#define FSYNC_MOVEVOLUME	5 /* Generate temporary relocation information
				     for this volume to another site, to be used
				     if this volume disappears */
#define	FSYNC_RESTOREVOLUME	6 /* Break all the callbacks on this volume since 				    it is being restored */
#define FSYNC_DONE		7 /* Done with this volume (used after a delete).
				     Don't put online, but remove from list */


/* Reasons (these could be communicated to venus or converted to messages) */

#define FSYNC_WHATEVER		0 /* XXXX */
#define FSYNC_SALVAGE		1 /* volume is being salvaged */
#define FSYNC_MOVE		2 /* volume is being moved */
#define FSYNC_OPERATOR		3 /* operator forced volume offline */


/* Replies (1 byte) */

#define FSYNC_DENIED		0
#define FSYNC_OK		1
