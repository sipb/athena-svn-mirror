/* Update REL relocation information at given index.
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

#include "libelfP.h"


int
gelf_update_rel (Elf_Data *dst, int ndx, GElf_Rel *src)
{
  Elf_Data_Scn *data_scn = (Elf_Data_Scn *) dst;
  Elf_Scn *scn;
  int result = 0;

  if (dst == NULL)
    return 0;

  if (unlikely (ndx < 0))
    {
      __libelf_seterrno (ELF_E_INVALID_INDEX);
      return 0;
    }

  if (unlikely (data_scn->d.d_type != ELF_T_REL))
    {
      /* The type of the data better should match.  */
      __libelf_seterrno (ELF_E_DATA_MISMATCH);
      return 0;
    }

  scn = data_scn->s;
  rwlock_wrlock (scn->elf->lock);

  if (scn->elf->class == ELFCLASS32)
    {
      Elf32_Rel *rel;

      /* There is the possibility that the values in the input are
	 too large.  */
      if (unlikely (src->r_offset > 0xffffffffull)
	  || unlikely (GELF_R_SYM (src->r_info) > 0xffffff)
	  || unlikely (GELF_R_TYPE (src->r_info) > 0xff))
	{
	  __libelf_seterrno (ELF_E_INVALID_DATA);
	  goto out;
	}

      /* Check whether we have to resize the data buffer.  */
      if (unlikely ((ndx + 1) * sizeof (Elf32_Rel) > data_scn->d.d_size))
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      rel = &((Elf32_Rel *) data_scn->d.d_buf)[ndx];

      rel->r_offset = src->r_offset;
      rel->r_info = ELF32_R_INFO (GELF_R_SYM (src->r_info),
				  GELF_R_TYPE (src->r_info));
    }
  else
    {
      /* Check whether we have to resize the data buffer.  */
      if (unlikely ((ndx + 1) * sizeof (Elf64_Rel) > data_scn->d.d_size))
	{
	  __libelf_seterrno (ELF_E_INVALID_INDEX);
	  goto out;
	}

      ((Elf64_Rel *) data_scn->d.d_buf)[ndx] = *src;
    }

  result = 1;

  /* Mark the section as modified.  */
  scn->flags |= ELF_F_DIRTY;

 out:
  rwlock_unlock (scn->elf->lock);

  return result;
}
