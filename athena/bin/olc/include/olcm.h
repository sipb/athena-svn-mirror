/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the defintions used in the program to receive questions via
 * mail
 *
 *      Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Id: olcm.h,v 1.8 1999-03-06 16:48:28 ghudson Exp $
 */

#include <mit-copyright.h>
#include "olxx_paths.h"

#define DFLT_TOPIC	"test"
#define DFLT_SERVER	"infocalypse.mit.edu"
#define DFLT_USERNAME	"nobody"
#define STOCK_HEADER	OLXX_MAIL_DIR "/olcm_default_header"
#define STOCK_FILE	OLXX_MAIL_DIR "/olcm_default_reply"
#define SRVTAB_LOC	OLXX_CONFIG_DIR "/srvtab"

#define SYSLOG_FACILITY LOG_LOCAL6
