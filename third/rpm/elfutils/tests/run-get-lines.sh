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

./get-lines $srcdir/testfile $srcdir/testfile2 > get-lines.out

cmp get-lines.out - <<"EOF"
cuhl = 11, v = 2, o = 0, sz = 4, ncu = 191
 5 lines
804842c: /home/drepper/gnu/new-bu/build/ttt/m.c:5: is_stmt:1, end_seq:0, bb: 0
8048432: /home/drepper/gnu/new-bu/build/ttt/m.c:6: is_stmt:1, end_seq:0, bb: 0
804844d: /home/drepper/gnu/new-bu/build/ttt/m.c:7: is_stmt:1, end_seq:0, bb: 0
8048458: /home/drepper/gnu/new-bu/build/ttt/m.c:8: is_stmt:1, end_seq:0, bb: 0
804845a: /home/drepper/gnu/new-bu/build/ttt/m.c:8: is_stmt:1, end_seq:1, bb: 0
cuhl = 11, v = 2, o = 114, sz = 4, ncu = 5617
 4 lines
804845c: /home/drepper/gnu/new-bu/build/ttt/b.c:4: is_stmt:1, end_seq:0, bb: 0
804845f: /home/drepper/gnu/new-bu/build/ttt/b.c:5: is_stmt:1, end_seq:0, bb: 0
8048464: /home/drepper/gnu/new-bu/build/ttt/b.c:6: is_stmt:1, end_seq:0, bb: 0
8048466: /home/drepper/gnu/new-bu/build/ttt/b.c:6: is_stmt:1, end_seq:1, bb: 0
cuhl = 11, v = 2, o = 412, sz = 4, ncu = 5752
 4 lines
8048468: /home/drepper/gnu/new-bu/build/ttt/f.c:3: is_stmt:1, end_seq:0, bb: 0
804846b: /home/drepper/gnu/new-bu/build/ttt/f.c:4: is_stmt:1, end_seq:0, bb: 0
8048470: /home/drepper/gnu/new-bu/build/ttt/f.c:5: is_stmt:1, end_seq:0, bb: 0
8048472: /home/drepper/gnu/new-bu/build/ttt/f.c:5: is_stmt:1, end_seq:1, bb: 0
cuhl = 11, v = 2, o = 0, sz = 4, ncu = 2418
 4 lines
10000470: /shoggoth/drepper/b.c:4: is_stmt:1, end_seq:0, bb: 0
1000047c: /shoggoth/drepper/b.c:5: is_stmt:1, end_seq:0, bb: 0
10000480: /shoggoth/drepper/b.c:6: is_stmt:1, end_seq:0, bb: 0
10000490: /shoggoth/drepper/b.c:6: is_stmt:1, end_seq:1, bb: 0
cuhl = 11, v = 2, o = 213, sz = 4, ncu = 2521
 4 lines
10000490: /shoggoth/drepper/f.c:3: is_stmt:1, end_seq:0, bb: 0
1000049c: /shoggoth/drepper/f.c:4: is_stmt:1, end_seq:0, bb: 0
100004a0: /shoggoth/drepper/f.c:5: is_stmt:1, end_seq:0, bb: 0
100004b0: /shoggoth/drepper/f.c:5: is_stmt:1, end_seq:1, bb: 0
cuhl = 11, v = 2, o = 267, sz = 4, ncu = 2680
 5 lines
100004b0: /shoggoth/drepper/m.c:5: is_stmt:1, end_seq:0, bb: 0
100004cc: /shoggoth/drepper/m.c:6: is_stmt:1, end_seq:0, bb: 0
100004e8: /shoggoth/drepper/m.c:7: is_stmt:1, end_seq:0, bb: 0
100004f4: /shoggoth/drepper/m.c:8: is_stmt:1, end_seq:0, bb: 0
10000514: /shoggoth/drepper/m.c:8: is_stmt:1, end_seq:1, bb: 0
EOF

rm -f get-lines.out

exit 0
