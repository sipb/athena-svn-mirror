/* Common definitions for handling files in memory or only on disk.
   Copyright (C) 1998, 1999, 2000, 2002 Red Hat, Inc.
   Written by Ulrich Drepper <drepper@redhat.com>, 1998.

   This program is Open Source software; you can redistribute it and/or
   modify it under the terms of the Open Software License version 1.0 as
   published by the Open Source Initiative.

   You should have received a copy of the Open Software License along
   with this program; if not, you may obtain a copy of the Open Software
   License version 1.0 from http://www.opensource.org/licenses/osl.php or
   by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
   3001 King Ranch Road, Ukiah, CA 95482.   */

#ifndef _COMMON_H
#define _COMMON_H       1

#include <ar.h>
#include <byteswap.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>


static inline Elf_Kind
determine_kind (void *buf, size_t len)
	/*@*/
{
  /* First test for an archive.  */
  if (len >= SARMAG && memcmp (buf, ARMAG, SARMAG) == 0)
    return ELF_K_AR;

  /* Next try ELF files.  */
  if (len >= EI_NIDENT && memcmp (buf, ELFMAG, SELFMAG) == 0)
    {
      /* Could be an ELF file.  */
      int eclass = (int) ((unsigned char *) buf)[EI_CLASS];
      int data = (int) ((unsigned char *) buf)[EI_DATA];
      int version = (int) ((unsigned char *) buf)[EI_VERSION];

      if (eclass > ELFCLASSNONE && eclass < ELFCLASSNUM
	  && data > ELFDATANONE && data < ELFDATANUM
	  && version > EV_NONE && version < EV_NUM)
	return ELF_K_ELF;
    }

  /* We do not know this file type.  */
  return ELF_K_NONE;
}


/* Allocate an Elf descriptor and fill in the generic information.  */
/*@null@*/
static inline Elf *
allocate_elf (int fildes, /*@null@*/ void *map_address, off_t offset,
	      size_t maxsize, Elf_Cmd cmd, /*@null@*/ Elf *parent,
	      Elf_Kind kind, size_t extra)
	/*@*/
{
  Elf *result = (Elf *) calloc (1, sizeof (Elf) + extra);
  if (result == NULL)
    __libelf_seterrno (ELF_E_NOMEM);
  else
    {
      result->kind = kind;
      result->ref_count = 1;
      result->cmd = cmd;
      result->fildes = fildes;
      result->start_offset = offset;
      result->maximum_size = maxsize;
      result->map_address = map_address;
      result->parent = parent;

      rwlock_init (result->lock);
    }

  return result;
}


/* Acquire lock for the descriptor and all children.  */
static void
libelf_acquire_all (Elf *elf)
	/*@modifies elf @*/
{
  rwlock_wrlock (elf->lock);

  if (elf->kind == ELF_K_AR)
    {
      Elf *child = elf->state.ar.children;

      while (child != NULL)
	{
	  if (child->ref_count != 0)
	    libelf_acquire_all (child);
	  child = child->next;
	}
    }
}

/* Release own lock and those of the children.  */
static void
libelf_release_all (Elf *elf)
	/*@modifies elf @*/
{
  if (elf->kind == ELF_K_AR)
    {
      Elf *child = elf->state.ar.children;

      while (child != NULL)
	{
	  if (child->ref_count != 0)
	    libelf_release_all (child);
	  child = child->next;
	}
    }

  rwlock_unlock (elf->lock);
}


/* Macro to convert endianess in place.  It determines the function it
   has to use itself.  */
#define CONVERT(Var) \
  (Var) = (sizeof (Var) == 1						      \
	   ? (Var)							      \
	   : (sizeof (Var) == 2						      \
	      ? bswap_16 (Var)						      \
	      : (sizeof (Var) == 4					      \
		 ? bswap_32 (Var)					      \
		 : bswap_64 (Var))))

#define CONVERT_TO(Dst, Var) \
  (Dst) = (sizeof (Var) == 1						      \
	   ? (Var)							      \
	   : (sizeof (Var) == 2						      \
	      ? bswap_16 (Var)						      \
	      : (sizeof (Var) == 4					      \
		 ? bswap_32 (Var)					      \
		 : bswap_64 (Var))))


#if __BYTE_ORDER == __LITTLE_ENDIAN
# define MY_ELFDATA	ELFDATA2LSB
#else
# define MY_ELFDATA	ELFDATA2MSB
#endif

#endif	/* common.h */