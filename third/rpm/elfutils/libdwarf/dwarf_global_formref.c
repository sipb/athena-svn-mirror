/* Return .debug_info section global offset of reference associated with form.
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

#include <dwarf.h>

#include <libdwarfP.h>


int
dwarf_global_formref (attr, return_offset, error)
     Dwarf_Attribute attr;
     Dwarf_Off *return_offset;
     Dwarf_Error *error;
{
  switch (attr->form)
    {
    case DW_FORM_ref1:
      *return_offset = *attr->valp + attr->cu->offset;
      break;

    case DW_FORM_ref2:
      *return_offset = (read_2ubyte_unaligned (attr->cu->dbg, attr->valp)
			+ attr->cu->offset);
      break;

    case DW_FORM_ref4:
      *return_offset = (read_4ubyte_unaligned (attr->cu->dbg, attr->valp)
			+ attr->cu->offset);
      break;

    case DW_FORM_ref8:
      *return_offset = (read_8ubyte_unaligned (attr->cu->dbg, attr->valp)
			+ attr->cu->offset);
      break;

    case DW_FORM_ref_udata:
      {
	Dwarf_Off off;
	Dwarf_Small *attrp = attr->valp;
	get_uleb128 (off, attrp);
	*return_offset = off + attr->cu->offset;
      }
      break;

    case DW_FORM_ref_addr:
      if (attr->cu->address_size == 4)
	*return_offset = read_4ubyte_unaligned (attr->cu->dbg, attr->valp);
      else
	*return_offset = read_8ubyte_unaligned (attr->cu->dbg, attr->valp);
      break;

    default:
      __libdwarf_error (attr->cu->dbg, error, DW_E_NO_REFERENCE);
      return DW_DLV_ERROR;
    }

  return DW_DLV_OK;
}