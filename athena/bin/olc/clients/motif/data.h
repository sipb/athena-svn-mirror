/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the constant defintions for xolc
 *
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (c) 1991 by the Massachusetts Institute of Technology
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/data.h,v $
 *	$Id: data.h,v 1.7 1991-08-12 13:44:59 lwvanels Exp $
 *      $Author: lwvanels $
 */

#include <mit-copyright.h>

/* File locations */

#define HELP_PATH "/usr/athena/lib/olc/motif_help/"
#define HELP_INIT_NEWQ	"olc.init.newq"
#define HELP_INIT_CONTQ	"olc.init.contq"
#define HELP_ASK	"olc.ask"
#define HELP_REPLAY	"olc.replay"

#define SA_ARGV0	"xbrowser"

/*
 *  Messages used in various dialog boxes.
 */

#define  QUIT_MESSAGE		"`Quit' means that you simply want to get out of this program.\nTo continue this question, just type `xolc' again.\n\nRemember, your question is still active until you mark it\n`done' or `cancel' it.  Otherwise, it will remain active until a\nconsultant can answer it.\n\nIf you logout, a consultant will send you mail.\n\nDo you wish to quit OLC at this time?"
#define  CANCEL_MESSAGE		"`Cancel' means that you want to withdraw your question.  You may\nwish to cancel your question if you have found the answer elsewhere.\n\nIf you do not cancel your question, OLC will store it until a consultant\ncan answer it.\n\nIf you simply want to get out of this program, but want to leave your\nquestion in OLC, use the `Quit' button instead.\n\nAre you sure that you wish to cancel this question?"
#define  DONE_MESSAGE		"`Done' means that you have received a satisfactory answer from the consultant.\n\nIf you have not received a satisfactory answer yet, but simply want to get out\nof this program, use the `Quit' button, and OLC will store your question until a\nconsultant can answer it.\n\nIf you wish to withdraw your question, use the `cancel' button instead.\n\nAre you sure that you wish to mark this question `done'?"
