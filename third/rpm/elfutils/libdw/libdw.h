/* Interfaces for libdw.
   Copyright (C) 2002 Red Hat, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   This program is Open Source software; you can redistribute it and/or
   modify it under the terms of the Open Software License version 1.0 as
   published by the Open Source Initiative.

   You should have received a copy of the Open Software License along
   with this program; if not, you may obtain a copy of the Open Software
   License version 1.0 from http://www.opensource.org/licenses/osl.php or
   by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
   3001 King Ranch Road, Ukiah, CA 95482.   */

#ifndef _LIBDW_H
#define _LIBDW_H	1

#include <gelf.h>


/* Mode for the session.  */
typedef enum
  {
    DWARF_C_READ,		/* Read .. */
    DWARF_C_RDWR,		/* Read and write .. */
    DWARF_C_WRITE,		/* Write .. */
  }
Dwarf_Cmd;


/* Callback results.  */
enum
{
  DWARF_CB_OK = 0,
  DWARF_CB_ABORT
};


/* Type for offset in DWARF file.  */
typedef GElf_Off Dwarf_Off;


/* DIE information.  */
typedef struct
{
  void *addr;
  // XXX We'll see what other information will be needed.
} Dwarf_Die;

/* Global symbol information.  */
typedef struct
{
  Dwarf_Off cu_offset;
  Dwarf_Off die_offset;
  const char *name;
} Dwarf_Global;

/* Handle for debug sessions.  */
typedef struct Dwarf Dwarf;


/* Create a handle for a new debug session.  */
extern Dwarf *dwarf_begin (int fildes, Dwarf_Cmd cmd);

/* Create a handle for a new debug session for an ELF file.  */
extern Dwarf *dwarf_begin_elf (Elf *elf, Dwarf_Cmd cmd, Elf_Scn *scngrp);

/* Retrieve ELF descriptor used for DWARF access.  */
extern Elf *dwarf_get_elf (Dwarf *dwarf);

/* Release debugging handling context.  */
extern int dwarf_end (Dwarf *dwarf);


/* Get the data block for the .debug_info section.  */
extern Elf_Data *dwarf_getscn_info (Dwarf *dwarf);

/* Read the header for the DWARF CU header.  */
extern int dwarf_nextcu (Dwarf *dwarf, Dwarf_Off off, Dwarf_Off *next_off,
			 size_t header_size);


/* Return DIE at given offset.  */
extern Dwarf_Die *dwarf_offdie (Dwarf *dbg, Dwarf_Off offset,
				Dwarf_Die *result);

/* Return string in name attribute of DIE.  */
extern const char *dwarf_diename (Dwarf *dbg, Dwarf_Die *die);


/* Get public symbol information.  */
extern size_t dwarf_get_pubnames (Dwarf *dbg,
				  int (*callback) (Dwarf *, Dwarf_Global *,
						   void *),
				  void *arg, size_t offset);


/* Return error code of last failing function call.  This value is kept
   separately for each thread.  */
extern int dwarf_errno (void);

/* Return error string for ERROR.  If ERROR is zero, return error string
   for most recent error or NULL is none occurred.  If ERROR is -1 the
   behaviour is similar to the last case except that not NULL but a legal
   string is returned.  */
extern const char *dwarf_errmsg (int error);

#endif	/* libdw.h */
