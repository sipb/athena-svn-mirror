/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: os.h,v 1.13 1999-01-22 23:13:44 ghudson Exp $
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/resource.h>
