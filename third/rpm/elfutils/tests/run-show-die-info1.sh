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

./show-die-info1 $srcdir/testfile5 $srcdir/testfile2 > show-die-info1.out

diff -u show-die-info1.out - <<"EOF"
file: testfile5
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 135
     DW_TAG_compile_unit
      Name     : b.c
      Offset   : 11
      CU offset: 11
          DW_TAG_subprogram
           Name     : bar
           Offset   : 104
           CU offset: 104
          DW_TAG_base_type
           Name     : int
           Offset   : 127
           CU offset: 127
New CU: cuhl = 11, v = 2, o = 54, sz = 4, ncu = 270
     DW_TAG_compile_unit
      Name     : f.c
      Offset   : 146
      CU offset: 11
          DW_TAG_subprogram
           Name     : foo
           Offset   : 239
           CU offset: 104
          DW_TAG_base_type
           Name     : int
           Offset   : 262
           CU offset: 127
New CU: cuhl = 11, v = 2, o = 108, sz = 4, ncu = 461
     DW_TAG_compile_unit
      Name     : m.c
      Offset   : 281
      CU offset: 11
          DW_TAG_subprogram
           Name     : main
           Offset   : 374
           CU offset: 104
               DW_TAG_subprogram
                Name     : bar
                Offset   : 402
                CU offset: 132
                    DW_TAG_unspecified_parameters
                     Name     : * NO NAME *
                     Offset   : 419
                     CU offset: 149
               DW_TAG_subprogram
                Name     : foo
                Offset   : 421
                CU offset: 151
                    DW_TAG_unspecified_parameters
                     Name     : * NO NAME *
                     Offset   : 434
                     CU offset: 164
          DW_TAG_base_type
           Name     : int
           Offset   : 437
           CU offset: 167
          DW_TAG_variable
           Name     : a
           Offset   : 444
           CU offset: 174
file: testfile2
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 2418
     DW_TAG_compile_unit
      Name     : b.c
      Offset   : 11
      CU offset: 11
          DW_TAG_subprogram
           Name     : bar
           Offset   : 72
           CU offset: 72
          DW_TAG_base_type
           Name     : int
           Offset   : 95
           CU offset: 95
          DW_TAG_typedef
           Name     : size_t
           Offset   : 102
           CU offset: 102
          DW_TAG_base_type
           Name     : unsigned int
           Offset   : 116
           CU offset: 116
          DW_TAG_typedef
           Name     : __gnuc_va_list
           Offset   : 132
           CU offset: 132
          DW_TAG_array_type
           Name     : __builtin_va_list
           Offset   : 154
           CU offset: 154
               DW_TAG_subrange_type
                Name     : * NO NAME *
                Offset   : 181
                CU offset: 181
          DW_TAG_base_type
           Name     : unsigned int
           Offset   : 188
           CU offset: 188
          DW_TAG_structure_type
           Name     : __va_list_tag
           Offset   : 204
           CU offset: 204
               DW_TAG_member
                Name     : gpr
                Offset   : 226
                CU offset: 226
               DW_TAG_member
                Name     : fpr
                Offset   : 240
                CU offset: 240
               DW_TAG_member
                Name     : overflow_arg_area
                Offset   : 254
                CU offset: 254
               DW_TAG_member
                Name     : reg_save_area
                Offset   : 282
                CU offset: 282
          DW_TAG_base_type
           Name     : unsigned char
           Offset   : 307
           CU offset: 307
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 324
           CU offset: 324
          DW_TAG_typedef
           Name     : __u_char
           Offset   : 326
           CU offset: 326
          DW_TAG_typedef
           Name     : __u_short
           Offset   : 342
           CU offset: 342
          DW_TAG_base_type
           Name     : short unsigned int
           Offset   : 359
           CU offset: 359
          DW_TAG_typedef
           Name     : __u_int
           Offset   : 381
           CU offset: 381
          DW_TAG_typedef
           Name     : __u_long
           Offset   : 396
           CU offset: 396
          DW_TAG_base_type
           Name     : long unsigned int
           Offset   : 412
           CU offset: 412
          DW_TAG_typedef
           Name     : __u_quad_t
           Offset   : 433
           CU offset: 433
          DW_TAG_base_type
           Name     : long long unsigned int
           Offset   : 451
           CU offset: 451
          DW_TAG_typedef
           Name     : __quad_t
           Offset   : 477
           CU offset: 477
          DW_TAG_base_type
           Name     : long long int
           Offset   : 493
           CU offset: 493
          DW_TAG_typedef
           Name     : __int8_t
           Offset   : 510
           CU offset: 510
          DW_TAG_base_type
           Name     : signed char
           Offset   : 526
           CU offset: 526
          DW_TAG_typedef
           Name     : __uint8_t
           Offset   : 541
           CU offset: 541
          DW_TAG_typedef
           Name     : __int16_t
           Offset   : 558
           CU offset: 558
          DW_TAG_base_type
           Name     : short int
           Offset   : 575
           CU offset: 575
          DW_TAG_typedef
           Name     : __uint16_t
           Offset   : 588
           CU offset: 588
          DW_TAG_typedef
           Name     : __int32_t
           Offset   : 606
           CU offset: 606
          DW_TAG_typedef
           Name     : __uint32_t
           Offset   : 623
           CU offset: 623
          DW_TAG_typedef
           Name     : __int64_t
           Offset   : 641
           CU offset: 641
          DW_TAG_typedef
           Name     : __uint64_t
           Offset   : 658
           CU offset: 658
          DW_TAG_typedef
           Name     : __qaddr_t
           Offset   : 676
           CU offset: 676
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 693
           CU offset: 693
          DW_TAG_typedef
           Name     : __dev_t
           Offset   : 699
           CU offset: 699
          DW_TAG_typedef
           Name     : __uid_t
           Offset   : 714
           CU offset: 714
          DW_TAG_typedef
           Name     : __gid_t
           Offset   : 729
           CU offset: 729
          DW_TAG_typedef
           Name     : __ino_t
           Offset   : 744
           CU offset: 744
          DW_TAG_typedef
           Name     : __mode_t
           Offset   : 759
           CU offset: 759
          DW_TAG_typedef
           Name     : __nlink_t
           Offset   : 775
           CU offset: 775
          DW_TAG_typedef
           Name     : __off_t
           Offset   : 792
           CU offset: 792
          DW_TAG_base_type
           Name     : long int
           Offset   : 807
           CU offset: 807
          DW_TAG_typedef
           Name     : __loff_t
           Offset   : 819
           CU offset: 819
          DW_TAG_typedef
           Name     : __pid_t
           Offset   : 835
           CU offset: 835
          DW_TAG_typedef
           Name     : __ssize_t
           Offset   : 850
           CU offset: 850
          DW_TAG_typedef
           Name     : __rlim_t
           Offset   : 867
           CU offset: 867
          DW_TAG_typedef
           Name     : __rlim64_t
           Offset   : 883
           CU offset: 883
          DW_TAG_typedef
           Name     : __id_t
           Offset   : 901
           CU offset: 901
          DW_TAG_structure_type
           Name     : * NO NAME *
           Offset   : 915
           CU offset: 915
               DW_TAG_member
                Name     : __val
                Offset   : 923
                CU offset: 923
          DW_TAG_array_type
           Name     : * NO NAME *
           Offset   : 940
           CU offset: 940
               DW_TAG_subrange_type
                Name     : * NO NAME *
                Offset   : 949
                CU offset: 949
          DW_TAG_typedef
           Name     : __fsid_t
           Offset   : 956
           CU offset: 956
          DW_TAG_typedef
           Name     : __daddr_t
           Offset   : 972
           CU offset: 972
          DW_TAG_typedef
           Name     : __caddr_t
           Offset   : 989
           CU offset: 989
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 1006
           CU offset: 1006
          DW_TAG_base_type
           Name     : char
           Offset   : 1012
           CU offset: 1012
          DW_TAG_typedef
           Name     : __time_t
           Offset   : 1020
           CU offset: 1020
          DW_TAG_typedef
           Name     : __swblk_t
           Offset   : 1036
           CU offset: 1036
          DW_TAG_typedef
           Name     : __clock_t
           Offset   : 1053
           CU offset: 1053
          DW_TAG_typedef
           Name     : __fd_mask
           Offset   : 1070
           CU offset: 1070
          DW_TAG_structure_type
           Name     : * NO NAME *
           Offset   : 1087
           CU offset: 1087
               DW_TAG_member
                Name     : __fds_bits
                Offset   : 1095
                CU offset: 1095
          DW_TAG_array_type
           Name     : * NO NAME *
           Offset   : 1117
           CU offset: 1117
               DW_TAG_subrange_type
                Name     : * NO NAME *
                Offset   : 1126
                CU offset: 1126
          DW_TAG_typedef
           Name     : __fd_set
           Offset   : 1133
           CU offset: 1133
          DW_TAG_typedef
           Name     : __key_t
           Offset   : 1149
           CU offset: 1149
          DW_TAG_typedef
           Name     : __ipc_pid_t
           Offset   : 1164
           CU offset: 1164
          DW_TAG_typedef
           Name     : __blkcnt_t
           Offset   : 1183
           CU offset: 1183
          DW_TAG_typedef
           Name     : __blkcnt64_t
           Offset   : 1201
           CU offset: 1201
          DW_TAG_typedef
           Name     : __fsblkcnt_t
           Offset   : 1221
           CU offset: 1221
          DW_TAG_typedef
           Name     : __fsblkcnt64_t
           Offset   : 1241
           CU offset: 1241
          DW_TAG_typedef
           Name     : __fsfilcnt_t
           Offset   : 1263
           CU offset: 1263
          DW_TAG_typedef
           Name     : __fsfilcnt64_t
           Offset   : 1283
           CU offset: 1283
          DW_TAG_typedef
           Name     : __ino64_t
           Offset   : 1305
           CU offset: 1305
          DW_TAG_typedef
           Name     : __off64_t
           Offset   : 1322
           CU offset: 1322
          DW_TAG_typedef
           Name     : __t_scalar_t
           Offset   : 1339
           CU offset: 1339
          DW_TAG_typedef
           Name     : __t_uscalar_t
           Offset   : 1359
           CU offset: 1359
          DW_TAG_typedef
           Name     : __intptr_t
           Offset   : 1380
           CU offset: 1380
          DW_TAG_structure_type
           Name     : _IO_FILE
           Offset   : 1398
           CU offset: 1398
               DW_TAG_member
                Name     : _flags
                Offset   : 1415
                CU offset: 1415
               DW_TAG_member
                Name     : _IO_read_ptr
                Offset   : 1432
                CU offset: 1432
               DW_TAG_member
                Name     : _IO_read_end
                Offset   : 1455
                CU offset: 1455
               DW_TAG_member
                Name     : _IO_read_base
                Offset   : 1478
                CU offset: 1478
               DW_TAG_member
                Name     : _IO_write_base
                Offset   : 1502
                CU offset: 1502
               DW_TAG_member
                Name     : _IO_write_ptr
                Offset   : 1527
                CU offset: 1527
               DW_TAG_member
                Name     : _IO_write_end
                Offset   : 1551
                CU offset: 1551
               DW_TAG_member
                Name     : _IO_buf_base
                Offset   : 1575
                CU offset: 1575
               DW_TAG_member
                Name     : _IO_buf_end
                Offset   : 1598
                CU offset: 1598
               DW_TAG_member
                Name     : _IO_save_base
                Offset   : 1620
                CU offset: 1620
               DW_TAG_member
                Name     : _IO_backup_base
                Offset   : 1644
                CU offset: 1644
               DW_TAG_member
                Name     : _IO_save_end
                Offset   : 1670
                CU offset: 1670
               DW_TAG_member
                Name     : _markers
                Offset   : 1693
                CU offset: 1693
               DW_TAG_member
                Name     : _chain
                Offset   : 1712
                CU offset: 1712
               DW_TAG_member
                Name     : _fileno
                Offset   : 1729
                CU offset: 1729
               DW_TAG_member
                Name     : _blksize
                Offset   : 1747
                CU offset: 1747
               DW_TAG_member
                Name     : _old_offset
                Offset   : 1766
                CU offset: 1766
               DW_TAG_member
                Name     : _cur_column
                Offset   : 1788
                CU offset: 1788
               DW_TAG_member
                Name     : _vtable_offset
                Offset   : 1810
                CU offset: 1810
               DW_TAG_member
                Name     : _shortbuf
                Offset   : 1835
                CU offset: 1835
               DW_TAG_member
                Name     : _lock
                Offset   : 1855
                CU offset: 1855
               DW_TAG_member
                Name     : _offset
                Offset   : 1871
                CU offset: 1871
               DW_TAG_member
                Name     : _unused2
                Offset   : 1889
                CU offset: 1889
          DW_TAG_structure_type
           Name     : _IO_marker
           Offset   : 1909
           CU offset: 1909
               DW_TAG_member
                Name     : _next
                Offset   : 1928
                CU offset: 1928
               DW_TAG_member
                Name     : _sbuf
                Offset   : 1944
                CU offset: 1944
               DW_TAG_member
                Name     : _pos
                Offset   : 1960
                CU offset: 1960
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 1976
           CU offset: 1976
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 1982
           CU offset: 1982
          DW_TAG_array_type
           Name     : * NO NAME *
           Offset   : 1988
           CU offset: 1988
               DW_TAG_subrange_type
                Name     : * NO NAME *
                Offset   : 1997
                CU offset: 1997
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 2004
           CU offset: 2004
          DW_TAG_array_type
           Name     : * NO NAME *
           Offset   : 2006
           CU offset: 2006
               DW_TAG_subrange_type
                Name     : * NO NAME *
                Offset   : 2015
                CU offset: 2015
          DW_TAG_typedef
           Name     : FILE
           Offset   : 2022
           CU offset: 2022
          DW_TAG_typedef
           Name     : wchar_t
           Offset   : 2034
           CU offset: 2034
          DW_TAG_typedef
           Name     : wint_t
           Offset   : 2050
           CU offset: 2050
          DW_TAG_typedef
           Name     : _G_int16_t
           Offset   : 2065
           CU offset: 2065
          DW_TAG_typedef
           Name     : _G_int32_t
           Offset   : 2083
           CU offset: 2083
          DW_TAG_typedef
           Name     : _G_uint16_t
           Offset   : 2101
           CU offset: 2101
          DW_TAG_typedef
           Name     : _G_uint32_t
           Offset   : 2120
           CU offset: 2120
          DW_TAG_structure_type
           Name     : _IO_jump_t
           Offset   : 2139
           CU offset: 2139
          DW_TAG_typedef
           Name     : _IO_lock_t
           Offset   : 2152
           CU offset: 2152
          DW_TAG_typedef
           Name     : _IO_FILE
           Offset   : 2166
           CU offset: 2166
          DW_TAG_structure_type
           Name     : _IO_FILE_plus
           Offset   : 2182
           CU offset: 2182
          DW_TAG_typedef
           Name     : __io_read_fn
           Offset   : 2198
           CU offset: 2198
          DW_TAG_subroutine_type
           Name     : * NO NAME *
           Offset   : 2219
           CU offset: 2219
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2229
                CU offset: 2229
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2234
                CU offset: 2234
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2239
                CU offset: 2239
          DW_TAG_typedef
           Name     : __io_write_fn
           Offset   : 2245
           CU offset: 2245
          DW_TAG_subroutine_type
           Name     : * NO NAME *
           Offset   : 2267
           CU offset: 2267
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2277
                CU offset: 2277
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2282
                CU offset: 2282
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2287
                CU offset: 2287
          DW_TAG_pointer_type
           Name     : * NO NAME *
           Offset   : 2293
           CU offset: 2293
          DW_TAG_const_type
           Name     : * NO NAME *
           Offset   : 2299
           CU offset: 2299
          DW_TAG_typedef
           Name     : __io_seek_fn
           Offset   : 2304
           CU offset: 2304
          DW_TAG_subroutine_type
           Name     : * NO NAME *
           Offset   : 2325
           CU offset: 2325
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2335
                CU offset: 2335
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2340
                CU offset: 2340
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2345
                CU offset: 2345
          DW_TAG_typedef
           Name     : __io_close_fn
           Offset   : 2351
           CU offset: 2351
          DW_TAG_subroutine_type
           Name     : * NO NAME *
           Offset   : 2373
           CU offset: 2373
               DW_TAG_formal_parameter
                Name     : * NO NAME *
                Offset   : 2383
                CU offset: 2383
          DW_TAG_typedef
           Name     : fpos_t
           Offset   : 2389
           CU offset: 2389
          DW_TAG_typedef
           Name     : off_t
           Offset   : 2403
           CU offset: 2403
New CU: cuhl = 11, v = 2, o = 213, sz = 4, ncu = 2521
     DW_TAG_compile_unit
      Name     : f.c
      Offset   : 2429
      CU offset: 11
          DW_TAG_subprogram
           Name     : foo
           Offset   : 2490
           CU offset: 72
          DW_TAG_base_type
           Name     : int
           Offset   : 2513
           CU offset: 95
New CU: cuhl = 11, v = 2, o = 267, sz = 4, ncu = 2680
     DW_TAG_compile_unit
      Name     : m.c
      Offset   : 2532
      CU offset: 11
          DW_TAG_subprogram
           Name     : main
           Offset   : 2593
           CU offset: 72
               DW_TAG_subprogram
                Name     : bar
                Offset   : 2621
                CU offset: 100
                    DW_TAG_unspecified_parameters
                     Name     : * NO NAME *
                     Offset   : 2638
                     CU offset: 117
               DW_TAG_subprogram
                Name     : foo
                Offset   : 2640
                CU offset: 119
                    DW_TAG_unspecified_parameters
                     Name     : * NO NAME *
                     Offset   : 2653
                     CU offset: 132
          DW_TAG_base_type
           Name     : int
           Offset   : 2656
           CU offset: 135
          DW_TAG_variable
           Name     : a
           Offset   : 2663
           CU offset: 142
EOF

rm -f show-die-info1.out

exit 0
