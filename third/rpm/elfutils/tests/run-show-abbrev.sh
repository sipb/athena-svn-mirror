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

./show-abbrev $srcdir/testfile $srcdir/testfile2 > show-abbrev.out

diff -u show-abbrev.out - <<"EOF"
abbrev[0]: code = 1, tag = 17, children = 1
abbrev[0]: attr[0]: code = 16, form = 6, offset = 0
abbrev[0]: attr[1]: code = 18, form = 1, offset = 2
abbrev[0]: attr[2]: code = 17, form = 1, offset = 4
abbrev[0]: attr[3]: code = 3, form = 8, offset = 6
abbrev[0]: attr[4]: code = 27, form = 8, offset = 8
abbrev[0]: attr[5]: code = 37, form = 8, offset = 10
abbrev[0]: attr[6]: code = 19, form = 11, offset = 12
abbrev[19]: code = 2, tag = 46, children = 1
abbrev[19]: attr[0]: code = 1, form = 19, offset = 19
abbrev[19]: attr[1]: code = 63, form = 12, offset = 21
abbrev[19]: attr[2]: code = 3, form = 8, offset = 23
abbrev[19]: attr[3]: code = 58, form = 11, offset = 25
abbrev[19]: attr[4]: code = 59, form = 11, offset = 27
abbrev[19]: attr[5]: code = 39, form = 12, offset = 29
abbrev[19]: attr[6]: code = 73, form = 19, offset = 31
abbrev[19]: attr[7]: code = 17, form = 1, offset = 33
abbrev[19]: attr[8]: code = 18, form = 1, offset = 35
abbrev[19]: attr[9]: code = 64, form = 10, offset = 37
abbrev[44]: code = 3, tag = 46, children = 1
abbrev[44]: attr[0]: code = 1, form = 19, offset = 44
abbrev[44]: attr[1]: code = 63, form = 12, offset = 46
abbrev[44]: attr[2]: code = 3, form = 8, offset = 48
abbrev[44]: attr[3]: code = 58, form = 11, offset = 50
abbrev[44]: attr[4]: code = 59, form = 11, offset = 52
abbrev[44]: attr[5]: code = 73, form = 19, offset = 54
abbrev[44]: attr[6]: code = 60, form = 12, offset = 56
abbrev[63]: code = 4, tag = 24, children = 0
abbrev[68]: code = 5, tag = 46, children = 1
abbrev[68]: attr[0]: code = 63, form = 12, offset = 68
abbrev[68]: attr[1]: code = 3, form = 8, offset = 70
abbrev[68]: attr[2]: code = 58, form = 11, offset = 72
abbrev[68]: attr[3]: code = 59, form = 11, offset = 74
abbrev[68]: attr[4]: code = 73, form = 19, offset = 76
abbrev[68]: attr[5]: code = 60, form = 12, offset = 78
abbrev[85]: code = 6, tag = 36, children = 0
abbrev[85]: attr[0]: code = 3, form = 8, offset = 85
abbrev[85]: attr[1]: code = 11, form = 11, offset = 87
abbrev[85]: attr[2]: code = 62, form = 11, offset = 89
abbrev[96]: code = 7, tag = 52, children = 0
abbrev[96]: attr[0]: code = 3, form = 8, offset = 96
abbrev[96]: attr[1]: code = 58, form = 11, offset = 98
abbrev[96]: attr[2]: code = 59, form = 11, offset = 100
abbrev[96]: attr[3]: code = 73, form = 19, offset = 102
abbrev[96]: attr[4]: code = 63, form = 12, offset = 104
abbrev[96]: attr[5]: code = 2, form = 10, offset = 106
abbrev[0]: code = 1, tag = 17, children = 1
abbrev[0]: attr[0]: code = 16, form = 6, offset = 0
abbrev[0]: attr[1]: code = 18, form = 1, offset = 2
abbrev[0]: attr[2]: code = 17, form = 1, offset = 4
abbrev[0]: attr[3]: code = 3, form = 8, offset = 6
abbrev[0]: attr[4]: code = 27, form = 8, offset = 8
abbrev[0]: attr[5]: code = 37, form = 8, offset = 10
abbrev[0]: attr[6]: code = 19, form = 11, offset = 12
abbrev[19]: code = 2, tag = 46, children = 0
abbrev[19]: attr[0]: code = 63, form = 12, offset = 19
abbrev[19]: attr[1]: code = 3, form = 8, offset = 21
abbrev[19]: attr[2]: code = 58, form = 11, offset = 23
abbrev[19]: attr[3]: code = 59, form = 11, offset = 25
abbrev[19]: attr[4]: code = 39, form = 12, offset = 27
abbrev[19]: attr[5]: code = 73, form = 19, offset = 29
abbrev[19]: attr[6]: code = 17, form = 1, offset = 31
abbrev[19]: attr[7]: code = 18, form = 1, offset = 33
abbrev[19]: attr[8]: code = 64, form = 10, offset = 35
abbrev[42]: code = 3, tag = 36, children = 0
abbrev[42]: attr[0]: code = 3, form = 8, offset = 42
abbrev[42]: attr[1]: code = 11, form = 11, offset = 44
abbrev[42]: attr[2]: code = 62, form = 11, offset = 46
abbrev[53]: code = 4, tag = 22, children = 0
abbrev[53]: attr[0]: code = 3, form = 8, offset = 53
abbrev[53]: attr[1]: code = 58, form = 11, offset = 55
abbrev[53]: attr[2]: code = 59, form = 11, offset = 57
abbrev[53]: attr[3]: code = 73, form = 19, offset = 59
abbrev[66]: code = 5, tag = 1, children = 1
abbrev[66]: attr[0]: code = 1, form = 19, offset = 66
abbrev[66]: attr[1]: code = 3, form = 8, offset = 68
abbrev[66]: attr[2]: code = 73, form = 19, offset = 70
abbrev[77]: code = 6, tag = 33, children = 0
abbrev[77]: attr[0]: code = 73, form = 19, offset = 77
abbrev[77]: attr[1]: code = 47, form = 11, offset = 79
abbrev[86]: code = 7, tag = 19, children = 1
abbrev[86]: attr[0]: code = 1, form = 19, offset = 86
abbrev[86]: attr[1]: code = 3, form = 8, offset = 88
abbrev[86]: attr[2]: code = 11, form = 11, offset = 90
abbrev[86]: attr[3]: code = 58, form = 11, offset = 92
abbrev[86]: attr[4]: code = 59, form = 11, offset = 94
abbrev[101]: code = 8, tag = 13, children = 0
abbrev[101]: attr[0]: code = 3, form = 8, offset = 101
abbrev[101]: attr[1]: code = 58, form = 11, offset = 103
abbrev[101]: attr[2]: code = 59, form = 11, offset = 105
abbrev[101]: attr[3]: code = 73, form = 19, offset = 107
abbrev[101]: attr[4]: code = 56, form = 10, offset = 109
abbrev[116]: code = 9, tag = 15, children = 0
abbrev[116]: attr[0]: code = 11, form = 11, offset = 116
abbrev[123]: code = 10, tag = 15, children = 0
abbrev[123]: attr[0]: code = 11, form = 11, offset = 123
abbrev[123]: attr[1]: code = 73, form = 19, offset = 125
abbrev[132]: code = 11, tag = 19, children = 1
abbrev[132]: attr[0]: code = 1, form = 19, offset = 132
abbrev[132]: attr[1]: code = 11, form = 11, offset = 134
abbrev[132]: attr[2]: code = 58, form = 11, offset = 136
abbrev[132]: attr[3]: code = 59, form = 11, offset = 138
abbrev[145]: code = 12, tag = 1, children = 1
abbrev[145]: attr[0]: code = 1, form = 19, offset = 145
abbrev[145]: attr[1]: code = 73, form = 19, offset = 147
abbrev[154]: code = 13, tag = 22, children = 0
abbrev[154]: attr[0]: code = 3, form = 8, offset = 154
abbrev[154]: attr[1]: code = 58, form = 11, offset = 156
abbrev[154]: attr[2]: code = 59, form = 5, offset = 158
abbrev[154]: attr[3]: code = 73, form = 19, offset = 160
abbrev[167]: code = 14, tag = 19, children = 0
abbrev[167]: attr[0]: code = 3, form = 8, offset = 167
abbrev[167]: attr[1]: code = 60, form = 12, offset = 169
abbrev[176]: code = 15, tag = 22, children = 0
abbrev[176]: attr[0]: code = 3, form = 8, offset = 176
abbrev[176]: attr[1]: code = 58, form = 11, offset = 178
abbrev[176]: attr[2]: code = 59, form = 11, offset = 180
abbrev[187]: code = 16, tag = 21, children = 1
abbrev[187]: attr[0]: code = 1, form = 19, offset = 187
abbrev[187]: attr[1]: code = 39, form = 12, offset = 189
abbrev[187]: attr[2]: code = 73, form = 19, offset = 191
abbrev[198]: code = 17, tag = 5, children = 0
abbrev[198]: attr[0]: code = 73, form = 19, offset = 198
abbrev[205]: code = 18, tag = 38, children = 0
abbrev[205]: attr[0]: code = 73, form = 19, offset = 205
EOF

rm -f show-abbrev.out

exit 0
