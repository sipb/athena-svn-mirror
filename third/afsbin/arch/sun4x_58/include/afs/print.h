/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_PTINT_
#define	_RXGEN_PTINT_

#ifdef	KERNEL
/* The following 'ifndefs' are not a good solution to the vendor's omission of surrounding all system includes with 'ifndef's since it requires that this file is included after the system includes...*/
#include "../afs/param.h"
#ifdef	UKERNEL
#include "../afs/sysincludes.h"
#include "../rx/xdr.h"
#include "../rx/rx.h"
#include "../rx/rx_globals.h"
#else	/* UKERNEL */
#include "../h/types.h"
#ifndef	SOCK_DGRAM  /* XXXXX */
#include "../h/socket.h"
#endif
#ifndef	DTYPE_SOCKET  /* XXXXX */
#ifdef AFS_DEC_ENV
#include "../h/smp_lock.h"
#endif
#ifndef AFS_LINUX22_ENV
#include "../h/file.h"
#endif
#endif
#ifndef	S_IFMT  /* XXXXX */
#include "../h/stat.h"
#endif
#ifndef	IPPROTO_UDP /* XXXXX */
#include "../netinet/in.h"
#endif
#ifndef	DST_USA  /* XXXXX */
#include "../h/time.h"
#endif
#ifndef AFS_LINUX22_ENV
#include "../rpc/types.h"
#endif /* AFS_LINUX22_ENV */
#ifndef	XDR_GETLONG /* XXXXX */
#ifdef AFS_LINUX22_ENV
#ifndef quad_t
#define quad_t __quad_t
#define u_quad_t __u_quad_t
#endif
#endif
#ifdef AFS_LINUX22_ENV
#include "../rx/xdr.h"
#else /* AFS_LINUX22_ENV */
#include "../rpc/xdr.h"
#endif /* AFS_LINUX22_ENV */
#endif /* XDR_GETLONG */
#endif   /* UKERNEL */
#include "../afsint/rxgen_consts.h"
#include "../afs/afs_osi.h"
#include "../rx/rx.h"
#include "../rx/rx_globals.h"
#else	/* KERNEL */
#include <afs/param.h>
#include <afs/stds.h>
#include <sys/types.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <rx/rx_globals.h>
#include <afs/rxgen_consts.h>
#endif	/* KERNEL */

#ifdef AFS_NT40_ENV
#ifndef AFS_RXGEN_EXPORT
#define AFS_RXGEN_EXPORT __declspec(dllimport)
#endif /* AFS_RXGEN_EXPORT */
#else /* AFS_NT40_ENV */
#define AFS_RXGEN_EXPORT
#endif /* AFS_NT40_ENV */

#define PR_STATINDEX 8
#define PRSUCCESS 0
#define PR_MAXNAMELEN 64
#define PR_MAXGROUPS 5000
#define PR_MAXLIST 5000
#define PRSIZE 10
#define COSIZE 39

struct prdebugentry {
	afs_int32 flags;
	afs_int32 id;
	afs_int32 cellid;
	afs_int32 next;
	afs_int32 reserved[5];
	afs_int32 entries[PRSIZE];
	afs_int32 nextID;
	afs_int32 nextname;
	afs_int32 owner;
	afs_int32 creator;
	afs_int32 ngroups;
	afs_int32 nusers;
	afs_int32 count;
	afs_int32 instance;
	afs_int32 owned;
	afs_int32 nextOwned;
	afs_int32 parent;
	afs_int32 sibling;
	afs_int32 child;
	char name[PR_MAXNAMELEN];
};
typedef struct prdebugentry prdebugentry;
bool_t xdr_prdebugentry();


struct prcheckentry {
	afs_int32 flags;
	afs_int32 id;
	afs_int32 owner;
	afs_int32 creator;
	afs_int32 ngroups;
	afs_int32 nusers;
	afs_int32 count;
	afs_int32 reserved[5];
	char name[PR_MAXNAMELEN];
};
typedef struct prcheckentry prcheckentry;
bool_t xdr_prcheckentry();


struct prlistentries {
	afs_int32 flags;
	afs_int32 id;
	afs_int32 owner;
	afs_int32 creator;
	afs_int32 ngroups;
	afs_int32 nusers;
	afs_int32 count;
	afs_int32 reserved[5];
	char name[PR_MAXNAMELEN];
};
typedef struct prlistentries prlistentries;
bool_t xdr_prlistentries();


struct PrUpdateEntry {
	afs_uint32 Mask;
	afs_int32 flags;
	afs_int32 id;
	afs_int32 owner;
	afs_int32 creator;
	afs_int32 ngroups;
	afs_int32 nusers;
	afs_int32 count;
	afs_int32 reserved[5];
	char name[PR_MAXNAMELEN];
};
typedef struct PrUpdateEntry PrUpdateEntry;
bool_t xdr_PrUpdateEntry();

#define PRUPDATE_NAME 0x0001
#define PRUPDATE_ID 0x0002
#define PRUPDATE_FLAGS 0x0004
#define PRUPDATE_NAMEHASH 0x0008
#define PRUPDATE_IDHASH 0x0010
#define PR_SF_ALLBITS   0xff		/* set all access bits */
#define PR_SF_NGROUPS (1<<31)		/* set field limiting group creation */
#define PR_SF_NUSERS  (1<<30)		/*  "  "  foreign users  "  */

typedef char prname[PR_MAXNAMELEN];
bool_t xdr_prname();


typedef struct namelist {
	u_int namelist_len;
	prname *namelist_val;
} namelist;
bool_t xdr_namelist();


typedef struct idlist {
	u_int idlist_len;
	afs_int32 *idlist_val;
} idlist;
bool_t xdr_idlist();


typedef struct prlist {
	u_int prlist_len;
	afs_int32 *prlist_val;
} prlist;
bool_t xdr_prlist();


typedef struct prentries {
	u_int prentries_len;
	prlistentries *prentries_val;
} prentries;
bool_t xdr_prentries();


/* Opcode-related useful stats for package: PR_ */
#define PR_LOWEST_OPCODE   500
#define PR_HIGHEST_OPCODE	521
#define PR_NUMBER_OPCODES	22

#define PR_NO_OF_STAT_FUNCS	22

AFS_RXGEN_EXPORT
extern const char *PR_function_names[];

#endif	/* _RXGEN_PTINT_ */
