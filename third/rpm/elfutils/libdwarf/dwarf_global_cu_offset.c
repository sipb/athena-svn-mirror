/* Return offset of header of compile unit containing the global definition.
   Copyright (C) 2000, 2002 Red Hat, Inc.
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

#include <string.h>

#include <libdwarfP.h>


int
dwarf_global_cu_offset (global, return_offset, error)
     Dwarf_Global global;
     Dwarf_Off *return_offset;
     Dwarf_Error *error;
{
  Dwarf_Small *cu_header;
  unsigned int offset_size;

  cu_header =
    ((Dwarf_Small *) global->info->dbg->sections[IDX_debug_info].addr
     + global->info->offset);
  if (read_4ubyte_unaligned_noncvt (cu_header) == 0xffffffff)
    offset_size = 8;
  else
    offset_size = 4;

  *return_offset = global->info->offset + 3 * offset_size - 4 + 3;

  return DW_DLV_OK;
}