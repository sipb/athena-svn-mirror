/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/xrevstack.h,v $
 *      $Author: lwvanels $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#ifndef _XREVSTACK_H_
#define _XREVSTACK_H_

#if (!defined(lint) && !defined(SABER))
static char rcsid_xrevstack_h[] = "$Id: xrevstack.h,v 1.3 1992-08-08 23:57:49 lwvanels Exp $";
#endif

#include <zephyr/mit-copyright.h>

extern x_gram *bottom_gram; /* for testing against NULL */
extern x_gram *unlinked;
extern int reverse_stack; /* is reverse stack on? */

extern void add_to_bottom(/* x_gram */);
extern void delete_gram(/* x_gram */);
extern void unlink_gram(/* x_gram */);
extern void pull_to_top(/* x_gram */);
extern void push_to_bottom(/* x_gram */);

#endif /* _XREVSTACK_H_ */
