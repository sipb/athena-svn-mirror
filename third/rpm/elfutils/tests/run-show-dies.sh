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

./show-dies $srcdir/testfile5 $srcdir/testfile2 > show-dies.out

diff -u show-dies.out - <<"EOF"
file: testfile5
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 135
     die
          die
          die
New CU: cuhl = 11, v = 2, o = 54, sz = 4, ncu = 270
     die
          die
          die
New CU: cuhl = 11, v = 2, o = 108, sz = 4, ncu = 461
     die
          die
               die
                    die
               die
                    die
          die
          die
file: testfile2
New CU: cuhl = 11, v = 2, o = 0, sz = 4, ncu = 2418
     die
          die
          die
          die
          die
          die
          die
               die
          die
          die
               die
               die
               die
               die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
               die
          die
               die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
               die
          die
               die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
               die
          die
               die
               die
               die
          die
          die
          die
               die
          die
          die
               die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
          die
               die
               die
               die
          die
          die
               die
               die
               die
          die
          die
          die
          die
               die
               die
               die
          die
          die
               die
          die
          die
New CU: cuhl = 11, v = 2, o = 213, sz = 4, ncu = 2521
     die
          die
          die
New CU: cuhl = 11, v = 2, o = 267, sz = 4, ncu = 2680
     die
          die
               die
                    die
               die
                    die
          die
          die
EOF

rm -f show-dies.out

exit 0
