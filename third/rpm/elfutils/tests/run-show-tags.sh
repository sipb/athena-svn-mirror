#! /bin/sh
# Copyright (C) 1999, 2000, 2002 Red Hat, Inc.
# Written by Ulrich Drepper <drepper@redhat.com>, 1999.
#
# This program is Open Source software; you can redistribute it and/or
# modify it under the terms of the Open Software License version 1.0 as
# published by the Open Source Initiative.
#
# You should have received a copy of the Open Software License along
# with this program; if not, you may obtain a copy of the Open Software
# License version 1.0 from http://www.opensource.org/licenses/osl.php or
# by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
# 3001 King Ranch Road, Ukiah, CA 95482.
set -e

./show-tags $srcdir/testfile5 $srcdir/testfile2 > show-tags.out

diff -u show-tags.out - <<"EOF"
file: testfile5
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 135
     DW_TAG_compile_unit
          DW_TAG_subprogram
          DW_TAG_base_type
New CU: cuhl = 11, v = 2, o = 54, sz = 4, ncu = 270
     DW_TAG_compile_unit
          DW_TAG_subprogram
          DW_TAG_base_type
New CU: cuhl = 11, v = 2, o = 108, sz = 4, ncu = 461
     DW_TAG_compile_unit
          DW_TAG_subprogram
               DW_TAG_subprogram
                    DW_TAG_unspecified_parameters
               DW_TAG_subprogram
                    DW_TAG_unspecified_parameters
          DW_TAG_base_type
          DW_TAG_variable
file: testfile2
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 2418
     DW_TAG_compile_unit
          DW_TAG_subprogram
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_array_type
               DW_TAG_subrange_type
          DW_TAG_base_type
          DW_TAG_structure_type
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
          DW_TAG_base_type
          DW_TAG_pointer_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_pointer_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_structure_type
               DW_TAG_member
          DW_TAG_array_type
               DW_TAG_subrange_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_pointer_type
          DW_TAG_base_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_structure_type
               DW_TAG_member
          DW_TAG_array_type
               DW_TAG_subrange_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_structure_type
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
          DW_TAG_structure_type
               DW_TAG_member
               DW_TAG_member
               DW_TAG_member
          DW_TAG_pointer_type
          DW_TAG_pointer_type
          DW_TAG_array_type
               DW_TAG_subrange_type
          DW_TAG_pointer_type
          DW_TAG_array_type
               DW_TAG_subrange_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_structure_type
          DW_TAG_typedef
          DW_TAG_typedef
          DW_TAG_structure_type
          DW_TAG_typedef
          DW_TAG_subroutine_type
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
          DW_TAG_typedef
          DW_TAG_subroutine_type
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
          DW_TAG_pointer_type
          DW_TAG_const_type
          DW_TAG_typedef
          DW_TAG_subroutine_type
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
               DW_TAG_formal_parameter
          DW_TAG_typedef
          DW_TAG_subroutine_type
               DW_TAG_formal_parameter
          DW_TAG_typedef
          DW_TAG_typedef
New CU: cuhl = 11, v = 2, o = 213, sz = 4, ncu = 2521
     DW_TAG_compile_unit
          DW_TAG_subprogram
          DW_TAG_base_type
New CU: cuhl = 11, v = 2, o = 267, sz = 4, ncu = 2680
     DW_TAG_compile_unit
          DW_TAG_subprogram
               DW_TAG_subprogram
                    DW_TAG_unspecified_parameters
               DW_TAG_subprogram
                    DW_TAG_unspecified_parameters
          DW_TAG_base_type
          DW_TAG_variable
EOF

rm -f show-tags.out

exit 0
