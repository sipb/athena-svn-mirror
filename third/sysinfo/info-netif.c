/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */
#ifndef lint
static char *RCSid = "$Id: info-netif.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * This file contains information specific to network interfaces that
 * will need periodic updating.
 */

#include "defs.h"
#include <sys/types.h>
#include <sys/socket.h>

/*
 * Address family table
 */
NetIF_t *GetNetifINET();
NetIF_t *GetNetifUnknown();

AddrFamily_t AddrFamily[] = {
#ifdef AF_INET
    {  AF_INET,		"Internet",		GetNetifINET },
#endif
#ifdef AF_UNSPEC
    {  AF_UNSPEC,	"Unspecified",		GetNetifUnknown },
#endif
#ifdef AF_DECnet
    {  AF_DECnet,	"DECnet",		GetNetifUnknown },
#endif
#ifdef AF_LAT
    {  AF_LAT,		"LAT",			GetNetifUnknown },
#endif
#ifdef AF_GOSIP
    {  AF_GOSIP,	"GOSIP",		GetNetifUnknown },
#endif
#ifdef AF_PUP
    {  AF_PUP,		"PUP",			GetNetifUnknown },
#endif
#ifdef AF_CHAOS
    {  AF_CHAOS,	"CHAOS",		GetNetifUnknown },
#endif
#ifdef AF_NS
    {  AF_NS,		"XEROX NS",		GetNetifUnknown },
#endif
#ifdef AF_NBS
    {  AF_NBS,		"NBS",			GetNetifUnknown },
#endif
#ifdef AF_ECMA
    {  AF_ECMA,		"ECMA",			GetNetifUnknown },
#endif
#ifdef AF_DATAKIT
    {  AF_DATAKIT,	"DATAKIT",		GetNetifUnknown },
#endif
#ifdef AF_CCITT
    {  AF_CCITT,	"CCITT",		GetNetifUnknown },
#endif
#ifdef AF_LYLINK
    {  AF_LYLINK,	"LYLINK",		GetNetifUnknown },
#endif
#ifdef AF_APPLETALK
    {  AF_APPLETALK,	"APPLETALK",		GetNetifUnknown },
#endif
#ifdef AF_BSC
    {  AF_BSC,		"BSC",			GetNetifUnknown },
#endif
#ifdef AF_DSS
    {  AF_DSS,		"DSS",			GetNetifUnknown },
#endif
#ifdef AF_OSI
    {  AF_OSI,		"OSI",			GetNetifUnknown },
#endif
#ifdef AF_NETMAN
    {  AF_NETMAN,	"NETMAN",		GetNetifUnknown },
#endif
#ifdef AF_X25
    {  AF_X25,		"X25",			GetNetifUnknown },
#endif
    {  0 },
};

