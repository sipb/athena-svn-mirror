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

./firstdie $srcdir/testfile $srcdir/testfile2 > firstdie.out

diff -u firstdie.out - <<"EOF"
cuhl = 11, v = 2, o = 0, sz = 4, ncu = 191
cuhl = 11, v = 2, o = 114, sz = 4, ncu = 5617
cuhl = 11, v = 2, o = 412, sz = 4, ncu = 5752
cuhl = 11, v = 2, o = 0, sz = 4, ncu = 2418
cuhl = 11, v = 2, o = 213, sz = 4, ncu = 2521
cuhl = 11, v = 2, o = 267, sz = 4, ncu = 2680
EOF

rm -f firstdie.out

exit 0
