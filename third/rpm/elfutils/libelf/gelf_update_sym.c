/* Update symbol information in symbol table at the given index.
   Copyright (C) 2000, 2001, 2002 Red Hat, Inc.
   Written by Ulrich Drepper <drepper@redhat.com>, 2000.

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
#include <stdlib.h>
#include <string.h>

#include "libelfP.h"


int
gelf_update_sym (Elf_Data *data, int ndx, GElf_Sym *src)
{
  Elf_Data_Scn *data_scn = (Elf_Data_Scn *) data;
  Elf_Scn *scn;
  int result = 0;

  if (data == NULL)
    return 0;

  if (unlikely (ndx < 0))
    {
      __libelf_seterrno (ELF_E_INVALID_INDEX);
      return 0;
    }

  if (unlikely (data_scn->d.d_type != ELF_T_SYM))
    {
      /* The type of the data better should match.  */
      __libelf_seterrno (ELF_E_DATA_MISMATCH);
      return 0;
    }

  scn = data_scn->s;
  rwlock_wrlock (scn->elf->lock);

  if (scn->elf->class == ELFCLASS32)
    {
      Elf32_Sym *sym;

      /* There is the possibility that the values in the input are
	 too large.  */
      if (unlikely (src->st_value > 0xffffffffull)
	  || unlikely (src->st_size > 0xffffffffull))
	{
	  __libelf_seterrno (ELF_E_INVALID_DATA);
	  goto out;
	}

      /* Check whether we have to resize the data buffer.  */
      if (unlikely ((ndx + 1) * sizeof (Elf32_Sym) > data_scn->d.d_size))
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      sym = &((Elf32_Sym *) data_scn->d.d_buf)[ndx];

#define COPY(name) \
      sym->name = src->name
      COPY (st_name);
      COPY (st_value);
      COPY (st_size);
      /* Please note that we can simply copy the `st_info' element since
	 the definitions of ELFxx_ST_BIND and ELFxx_ST_TYPE are the same
	 for the 64 bit variant.  */
      COPY (st_info);
      COPY (st_other);
      COPY (st_shndx);
    }
  else
    {
      /* Check whether we have to resize the data buffer.  */
      if (unlikely ((ndx + 1) * sizeof (Elf64_Sym) > data_scn->d.d_size))
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      ((Elf64_Sym *) data_scn->d.d_buf)[ndx] = *src;
    }

  result = 1;

  /* Mark the section as modified.  */
  scn->flags |= ELF_F_DIRTY;

 out:
  rwlock_unlock (scn->elf->lock);

  return result;
}