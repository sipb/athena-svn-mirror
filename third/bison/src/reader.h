/* Input parser for bison
   Copyright 2000 Free Software Foundation, Inc.

   This file is part of Bison, the GNU Compiler Compiler.

   Bison is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bison is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bison; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef READER_H_
# define READER_H_

/* Read in the grammar specification and record it in the format
   described in gram.h.  All guards are copied into the FGUARD file
   and all actions into FACTION, in each case forming the body of a C
   function (YYGUARD or YYACTION) which contains a switch statement to
   decide which guard or action to execute.  */

extern void reader PARAMS ((void));


extern void reader_output_yylsp PARAMS ((struct obstack *));

extern int lineno;
extern char **tags;
extern short *user_toknums;

#endif /* !READER_H_ */
