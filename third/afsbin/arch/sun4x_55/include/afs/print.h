/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_PTINT_
#define	_RXGEN_PTINT_

#ifdef	KERNEL
/* The following 'ifndefs' are not a good solution to the vendor's omission of surrounding all system includes with 'ifndef's since it requires that this file is included after the system includes...*/
#include "../afs/param.h"
#include "../h/types.h"
#ifndef	SOCK_DGRAM  /* XXXXX */
#include "../h/socket.h"
#endif
#ifndef	DTYPE_SOCKET  /* XXXXX */
#ifdef AFS_DEC_ENV
#include "../h/smp_lock.h"
#endif
#include "../h/file.h"
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
#include "../rpc/types.h"
#ifndef	XDR_GETLONG /* XXXXX */
#include "../rpc/xdr.h"
#endif
#include "../afsint/rxgen_consts.h"
#include "../afs/osi.h"
#include "../rx/rx.h"
#else	/* KERNEL */
#include <sys/types.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <afs/rxgen_consts.h>
#endif	/* KERNEL */

#define PRSUCCESS 0
#define PR_MAXNAMELEN 64
#define PR_MAXGROUPS 5000
#define PR_MAXLIST 5000
#define PRSIZE 10
#define COSIZE 39

struct prdebugentry {
	int32 flags;
	int32 id;
	int32 cellid;
	int32 next;
	int32 reserved[5];
	int32 entries[PRSIZE];
	int32 nextID;
	int32 nextname;
	int32 owner;
	int32 creator;
	int32 ngroups;
	int32 nusers;
	int32 count;
	int32 instance;
	int32 owned;
	int32 nextOwned;
	int32 parent;
	int32 sibling;
	int32 child;
	char name[PR_MAXNAMELEN];
};
typedef struct prdebugentry prdebugentry;
bool_t xdr_prdebugentry();


struct prcheckentry {
	int32 flags;
	int32 id;
	int32 owner;
	int32 creator;
	int32 ngroups;
	int32 nusers;
	int32 count;
	int32 reserved[5];
	char name[PR_MAXNAMELEN];
};
typedef struct prcheckentry prcheckentry;
bool_t xdr_prcheckentry();


struct PrUpdateEntry {
	u_int32 Mask;
	int32 flags;
	int32 id;
	int32 owner;
	int32 creator;
	int32 ngroups;
	int32 nusers;
	int32 count;
	int32 reserved[5];
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
	int32 *idlist_val;
} idlist;
bool_t xdr_idlist();


typedef struct prlist {
	u_int prlist_len;
	int32 *prlist_val;
} prlist;
bool_t xdr_prlist();


/* Opcode-related useful stats for package: PR_ */
#define PR_LOWEST_OPCODE   500
#define PR_HIGHEST_OPCODE	520
#define PR_NUMBER_OPCODES	21

#endif	/* _RXGEN_PTINT_ */
