/*
** Copyright (c) 1990 David E. Smyth
**
** This file was derived from work performed by Martin Brunecky at
** Auto-trol Technology Corporation, Denver, Colorado, under the
** following copyright:
**
*******************************************************************************
* Copyright 1990 by Auto-trol Technology Corporation, Denver, Colorado.
*
*                        All Rights Reserved
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted, provided
* that the above copyright notice appears on all copies and that both the
* copyright and this permission notice appear in supporting documentation
* and that the name of Auto-trol not be used in advertising or publicity
* pertaining to distribution of the software without specific, prior written
* permission.
*
* Auto-trol disclaims all warranties with regard to this software, including
* all implied warranties of merchantability and fitness, in no event shall
* Auto-trol be liable for any special, indirect or consequential damages or
* any damages whatsoever resulting from loss of use, data or profits, whether
* in an action of contract, negligence or other tortious action, arising out
* of or in connection with the use or performance of this software.
*******************************************************************************
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation, advertising
** materials, and other materials related to such distribution and use
** acknowledge that the software was developed by David E. Smyth.  The
** name of David E. Smyth may not be used to endorse or promote products
** derived from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*/

/*
* SCCS_data: @(#)MriRegAll.c 1.0 ( 19 June 1990 )
*
*     This module contains registration routine for all Motif
*     widget/gadget constructors and classes.
*
* Module_interface_summary: 
*
*     void MriRegisterMotif ( XtAppContext app )
*
*******************************************************************************
*/

#include "config.h"

#include <Xm/Xm.h>

#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>

void MriRegisterMotif ( app )
    XtAppContext app;
{
#define RCO( name, func  )  WcRegisterConstructor ( app, name, func  )
#define RCN( name, class )  WcRegisterClassName   ( app, name, class )

/* -- register all Motif constructors */
 RCO( "XmCreateInformationDialog",	XmCreateInformationDialog	);
 RCO( "XmCreatePromptDialog",		XmCreatePromptDialog		);
 RCO( "XmCreateScrolledList",		XmCreateScrolledList		);
 RCO( "XmCreateScrolledText",		XmCreateScrolledText		);

/* -- register Motif widget classes */
 RCN("XmForm",				xmFormWidgetClass		);
 RCN("XmLabelGadget",			xmLabelGadgetClass		);
 RCN("XmPanedWindow",			xmPanedWindowWidgetClass	);
 RCN("XmPushButton",			xmPushButtonWidgetClass		);
 RCN("XmText",				xmTextWidgetClass		);

#undef RCO
#undef RCN
}
