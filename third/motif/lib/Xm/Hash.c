/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/***********************************************************
Copyright 1987, 1988, 1990 by Digital Equipment Corporation, Maynard,
Massachusetts, and the Massachusetts Institute of Technology, Cambridge,
Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <Xm/HashP.h>

#define HASH(tab,sig) ((sig) & tab->mask)
#define REHASHVAL(tab, idx) ((((idx) % tab->rehash) + 2) | 1)
#define REHASH(tab,idx,rehash) ((idx + rehash) & tab->mask)
#define RELEASE_KEY(tab, entry, key) \
{\
   if (tab->types[entry->hash.type]->hash.releaseKeyProc) \
     (*  (tab->types[entry->hash.type]->hash.releaseKeyProc)) \
       (entry, key); \
     }

#define GETKEY(key, func, entry, clientData) \
{ \
    if (func == _XmGetEmbeddedKey) \
      key = ((XmHashKey)(((char *)entry) + (int)clientData)); \
    else if (func == _XmGetIndirectKey) \
      key = *(XmHashKey *) (((char *)entry) + (int)clientData); \
    else \
      key = ((*func)(entry, clientData)); \
      }

#define GETKEYFUNC(tab, entry) \
  tab->types[entry->hash.type]->hash.getKeyFunc
#define GETCLIENTDATA(tab, entry) \
  tab->types[entry->hash.type]->hash.getKeyClientData


typedef unsigned long 	Signature;

typedef struct _XmHashTableRec {
    unsigned int mask;		/* size of hash table - 1 */
    unsigned int rehash;	/* mask - 2 */
    unsigned int occupied;	/* number of occupied entries */
    unsigned int fakes;		/* number occupied by XMPHASHfake */
    XmHashEntryType *types;	/* lookup methods for key	*/
    unsigned short numTypes;    /* number of lookup methods	*/
    Boolean	 keyIsString;	/* whether the hash key is a string */
    XmHashEntry *entries;	/* the entries */
}XmpHashTableRec;


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static unsigned int GetTableIndex() ;
static void ExpandHashTable() ;

#else

static unsigned int GetTableIndex( 
                        register XmHashTable tab,
                        register XmHashKey key,
#if NeedWidePrototypes
                        register int new) ;
#else
                        register Boolean new) ;
#endif /* NeedWidePrototypes */
static void ExpandHashTable( 
                        register XmHashTable tab) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static XmHashEntryRec XmpHashfake;	/* placeholder for deletions */

static unsigned int 
#ifdef _NO_PROTO
GetTableIndex( tab, key, new )
        register XmHashTable tab ;
        register XmHashKey key ;
        register Boolean new ;
#else
GetTableIndex(
	      register XmHashTable tab,
	      register XmHashKey key,
#if NeedWidePrototypes
	      register int new)
#else
              register Boolean new)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    register XmHashEntry	*entries = tab->entries;
    register int		len, idx, i, rehash = 0;
    register char 		c;
    register Signature 		sig = 0;
    register XmHashEntry	entry;
    String			s1, s2;
    XmHashKey			compKey;
    XmGetHashKeyFunc           keyFunc;
    XtPointer                   clientData;

    if (tab->keyIsString) {
	s1 = (String)key;
	for (s2 = (char *)s1; c = *s2++; )
	  sig = (sig << 1) + c;
	len = s2 - s1 - 1;
    }
    else
      sig = (Signature)key;
    
    idx = HASH(tab, sig);
    while (entry = entries[idx]) {
	keyFunc = GETKEYFUNC(tab, entry);
	clientData = GETCLIENTDATA(tab, entry);

	if (entries[idx] == &XmpHashfake) {
	    if (new)
	      return idx;
	    else
	      goto nomatch;
	}
	
	GETKEY(compKey, keyFunc, entry, clientData);

	if (tab->keyIsString) {
	    for (i = len, s1 = (String)key, s2 = (String) compKey;
		 --i >= 0; ) {
		if (*s1++ != *s2++)
		  goto nomatch;
	    }
	}
	else {
	  
	    if (compKey != key)
	      s2 = " ";
	    else
	      s2 = NULL;
	}
	
	if (*s2) {
nomatch:    
	    RELEASE_KEY(tab, entry, compKey);
	    if (!rehash)
	      rehash = REHASHVAL(tab, idx);
	    idx = REHASH(tab, idx, rehash);
	    continue;
	}
	else
	  RELEASE_KEY(tab, entry, compKey);
	break;
    }
    return idx;
}



void 
#ifdef _NO_PROTO
_XmRegisterHashEntry( tab, key, entry )
        register XmHashTable tab ;
        register XmHashKey key ;
        register XmHashEntry entry ;
#else
_XmRegisterHashEntry(
        register XmHashTable tab,
        register XmHashKey key,
        register XmHashEntry entry )
#endif /* _NO_PROTO */
{
    unsigned int idx;

    if ((tab->occupied + (tab->occupied >> 2)) > tab->mask)
	ExpandHashTable(tab);

    idx = GetTableIndex(tab, key, True);
    if (tab->entries[idx] == &XmpHashfake)
      tab->fakes--;
    tab->occupied++;
    tab->entries[idx] = entry;
}

void 
#ifdef _NO_PROTO
_XmUnregisterHashEntry( tab, entry )
        register XmHashTable tab ;
        register XmHashEntry entry ;
#else
_XmUnregisterHashEntry(
        register XmHashTable tab,
        register XmHashEntry entry )
#endif /* _NO_PROTO */
{
    register int 		idx;
    register XmHashEntry	*entries = tab->entries;
    XmGetHashKeyFunc           keyFunc = GETKEYFUNC(tab, entry);
    XtPointer                   clientData = GETCLIENTDATA(tab, entry);
    XmHashKey			key;

    GETKEY(key, keyFunc, entry, clientData);

    idx = GetTableIndex(tab, key, False);
    RELEASE_KEY(tab, entry, key);
    entries[idx] = &XmpHashfake;
    tab->fakes++;
    tab->occupied--;
}


static void 
#ifdef _NO_PROTO
ExpandHashTable( tab )
        register XmHashTable tab ;
#else
ExpandHashTable(
        register XmHashTable tab )
#endif /* _NO_PROTO */
{
    unsigned int oldmask;
    register XmHashEntry *oldentries, *entries;
    register int oldidx, newidx;
    register XmHashEntry entry;
    register XmHashKey key;

    oldmask = tab->mask;
    oldentries = tab->entries;
    tab->fakes = 0;
    if ((tab->occupied + (tab->occupied >> 2)) > tab->mask) {
	tab->mask = (tab->mask << 1) + 1;
	tab->rehash = tab->mask - 2;
    }
    entries = tab->entries = (XmHashEntry *) XtCalloc(tab->mask+1, sizeof(XmHashEntry));
    for (oldidx = 0; oldidx <= oldmask; oldidx++) {
      if ((entry = oldentries[oldidx]) && entry != &XmpHashfake) {
	XmGetHashKeyFunc           keyFunc = GETKEYFUNC(tab, entry);
	XtPointer                   clientData = GETCLIENTDATA(tab, entry);
	
	GETKEY(key, keyFunc, entry, clientData);
	newidx = GetTableIndex(tab, key, True);
	RELEASE_KEY(tab, entry, key);
	entries[newidx] = entry;
      }
    }
    XtFree((char *)oldentries);
}


XmHashEntry 
#ifdef _NO_PROTO
_XmEnumerateHashTable( tab, enumFunc, clientData )
        register XmHashTable tab ;
        register XmHashEnumerateFunc enumFunc;
        register XtPointer clientData;
#else
_XmEnumerateHashTable(
        register XmHashTable tab,
	register XmHashEnumerateFunc enumFunc,
        register XtPointer clientData )
#endif /* _NO_PROTO */
{
    register unsigned int i;

    for (i = 0; i <= tab->mask; i++)
      if (tab->entries[i] && 
	  tab->entries[i] != &XmpHashfake &&
	  ((*enumFunc) (tab->entries[i], clientData)))
	return tab->entries[i];
    return NULL;
}


XmHashEntry 
#ifdef _NO_PROTO
_XmKeyToHashEntry( tab, key )
        register XmHashTable tab ;
        register XmHashKey key ;
#else
_XmKeyToHashEntry(
        register XmHashTable tab,
        register XmHashKey key )
#endif /* _NO_PROTO */
{
    register int idx;
    register XmHashEntry  *entries = tab->entries;

    if (!key) return NULL;
    idx = GetTableIndex(tab, key, False);
    return entries[idx];
}

XmHashTable 
#ifdef _NO_PROTO
_XmAllocHashTable( hashEntryTypes, numHashEntryTypes, keyIsString )
    XmHashEntryType	*hashEntryTypes;
    Cardinal		numHashEntryTypes;
    Boolean 		keyIsString ;
#else
_XmAllocHashTable(XmHashEntryType	*hashEntryTypes,
		   Cardinal		numHashEntryTypes,
#if NeedWidePrototypes
		   int 			keyIsString)
#else
                   Boolean 		keyIsString)
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    register XmHashTable tab;

    tab = (XmHashTable) XtMalloc(sizeof(struct _XmHashTableRec));
    tab->types = hashEntryTypes;
    tab->numTypes = numHashEntryTypes;
    tab->keyIsString = keyIsString;
    tab->mask = 0x7f;
    tab->rehash = tab->mask - 2;
    tab->entries = (XmHashEntry *) XtCalloc(tab->mask+1, sizeof(XmHashEntry));
    tab->occupied = 0;
    tab->fakes = 0;
    return tab;
}

void 
#ifdef _NO_PROTO
_XmFreeHashTable( hashTable )
        XmHashTable hashTable ;
#else
_XmFreeHashTable(
        XmHashTable hashTable )
#endif /* _NO_PROTO */
{
    XtFree((char *)hashTable->entries);
    XtFree((char *)hashTable);
}



XmHashKey
#ifdef _NO_PROTO
_XmGetEmbeddedKey( entry, clientData)
  XmHashEntry	entry;
  XtPointer	clientData;
#else
_XmGetEmbeddedKey(
  XmHashEntry	entry,
  XtPointer	clientData)
#endif /* _NO_PROTO */
{
    return (XmHashKey)(((char *)entry) + (int)clientData);
}

XmHashKey
#ifdef _NO_PROTO
_XmGetIndirectKey( entry, clientData)
  XmHashEntry	entry;
  XtPointer	clientData;
#else
_XmGetIndirectKey(
  XmHashEntry	entry,
  XtPointer	clientData)
#endif /* _NO_PROTO */
{
    return *(XmHashKey *) (((char *)entry) + (int)clientData);
}

