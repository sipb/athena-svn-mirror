/* Return program header table entry.
   Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
   Written by Ulrich Drepper <drepper@redhat.com>, 1998.

   This program is Open Source software; you can redistribute it and/or
   modify it under the terms of the Open Software License version 1.0 as
   published by the Open Source Initiative.

   You should have received a copy of the Open Software License along
   with this program; if not, you may obtain a copy of the Open Software
   License version 1.0 from http://www.opensource.org/licenses/osl.php or
   by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
   3001 King Ranch Road, Ukiah, CA 95482.   */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gelf.h>
#include <string.h>

#include "libelfP.h"


GElf_Phdr *
gelf_getphdr (Elf *elf, int ndx, GElf_Phdr *dst)
{
  GElf_Phdr *result = NULL;

  if (elf == NULL)
    return NULL;

  if (unlikely (elf->kind != ELF_K_ELF))
    {
      __libelf_seterrno (ELF_E_INVALID_HANDLE);
      return NULL;
    }

  if (dst == NULL)
    {
      __libelf_seterrno (ELF_E_INVALID_OPERAND);
      return NULL;
    }

  rwlock_rdlock (elf->lock);

  if (elf->class == ELFCLASS32)
    {
      /* Copy the elements one-by-one.  */
      Elf32_Phdr *phdr = elf->state.elf32.phdr;

      if (phdr == NULL)
	{
	  phdr = INTUSE(elf32_getphdr) (elf);
	  if (phdr == NULL)
	    /* The error number is already set.  */
	    goto out;
	}

      /* Test whether the index is ok.  */
      if (ndx >= elf->state.elf32.ehdr->e_phnum)
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      /* We know the result now.  */
      result = dst;

      /* Now correct the pointer to point to the correct element.  */
      phdr += ndx;

#define COPY(Name) result->Name = phdr->Name
      COPY (p_type);
      COPY (p_offset);
      COPY (p_vaddr);
      COPY (p_paddr);
      COPY (p_filesz);
      COPY (p_memsz);
      COPY (p_flags);
      COPY (p_align);
    }
  else
    {
      /* Copy the elements one-by-one.  */
      Elf64_Phdr *phdr = elf->state.elf64.phdr;

      if (phdr == NULL)
	{
	  phdr = INTUSE(elf64_getphdr) (elf);
	  if (phdr == NULL)
	    /* The error number is already set.  */
	    goto out;
	}

      /* Test whether the index is ok.  */
      if (ndx >= elf->state.elf64.ehdr->e_phnum)
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      /* We only have to copy the data.  */
      result = memcpy (dst, phdr + ndx, sizeof (GElf_Phdr));
    }

 out:
  rwlock_unlock (elf->lock);

  return result;
}