/*
 * This file is part of the Omega project, which
 * is based in the web2c distribution of TeX.
 *
 * Copyright (c) 1994--1998 John Plaice and Yannis Haralambous
 */

#define YYSTYPE yystype
typedef union {
	int yint;
	char* ystring ;
	list ylist;
	left yleft;
	llist ylleft;
} yystype;
