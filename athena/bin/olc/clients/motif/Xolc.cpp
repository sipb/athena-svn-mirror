!
!  Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
!  For copying and distribution information, see the file "mit-copyright.h".
!
#ifdef _IBMR2
*XmMessageBox.labelFontList:	-adobe-*schoolbook-medium-r-normal--14-*
*XmSelectionBox.labelFontList:	-adobe-*schoolbook-medium-r-normal--14-*
*XmPushButtonGadget.fontList:	-adobe-*schoolbook-medium-r-normal--14-*
*XmPushButton.fontList:		-adobe-*schoolbook-medium-r-normal--14-*
*XmLabelGadget.fontList:	-adobe-*schoolbook-medium-r-normal--12-*
#else
*XmMessageBox.labelFontList:	-adobe-*schoolbook-medium-r-*-*-*-140-*
*XmSelectionBox.labelFontList:	-adobe-*schoolbook-medium-r-*-*-*-140-*
*XmPushButtonGadget.fontList:	-adobe-*schoolbook-medium-r-*-*-*-140-*
*XmPushButton.fontList:		-adobe-*schoolbook-medium-r-*-*-*-140-*
*XmLabelGadget.fontList:	-adobe-*schoolbook-medium-r-*-*-*-120-*
#endif
*XmText.blinkRate:			300
*XmForm.paneMinimum:			150
*XmForm.horizontalSpacing:		5
*XmForm.verticalSpacing:		5
*XmList.listMarginWidth:		5
!*XmList.listMarginHeight:		5
*XmList.listSpacing:			0
*XmLabelGadget.alignment:		ALIGNMENT_CENTER
*fontList:				fixed
*XmDialogShell.borderWidth:		2
!
*title:					On-Line Consulting
*iconName:				OLC
!
xolc.minWidth:			585
xolc.minHeight:			415
xolc.width:			576
xolc.height:			430
xolc.borderWidth:		2
!
!*main.width:			560
!*main.height:			430
!
*main.topAttachment:			ATTACH_FORM
*main.leftAttachment:			ATTACH_FORM
*main.rightAttachment:			ATTACH_FORM
*main.bottomAttachment:			ATTACH_FORM
*main.shadowThickness:			2
*main.resizePolicy:			RESIZE_NONE
!
! Menu buttons
!
*new_ques_btn.topAttachment:		ATTACH_FORM
*new_ques_btn.leftAttachment:		ATTACH_FORM
*new_ques_btn.labelString:		Ask a question
*new_ques_btn.traversalOn:		false
!
*cont_ques_btn.labelString:		Continue your question
*cont_ques_btn.topAttachment:		ATTACH_FORM
*cont_ques_btn.leftAttachment:		ATTACH_FORM
*cont_ques_btn.traversalOn:		false
!
*stock_btn.labelString:			Browse stock answers
*stock_btn.topAttachment:		ATTACH_FORM
*stock_btn.leftAttachment:		ATTACH_WIDGET
!*stock_btn.leftWidget:			cont_ques_btn
*stock_btn.traversalOn:			false
!
*help_btn.labelString:			Help
*help_btn.topAttachment:		ATTACH_FORM
*help_btn.rightAttachment:		ATTACH_FORM
*help_btn.traversalOn:			false
!
*quit_btn.labelString:			Quit
*quit_btn.topAttachment:		ATTACH_FORM
*quit_btn.rightAttachment:		ATTACH_WIDGET
*quit_btn.rightWidget:			help_btn
*quit_btn.traversalOn:			false
!
! Separator between top buttons and main part
!
*buttonSep.separatorType:		SHADOW_ETCHED_IN
*buttonSep.topAttachment:		ATTACH_WIDGET
*buttonSep.topWidget:			help_btn
*buttonSep.leftAttachment:		ATTACH_FORM
*buttonSep.rightAttachment:		ATTACH_FORM
!
!
!
! New question window
!
*new_ques_form.marginWidth:		0
*new_ques_form.marginHeight:		0
*new_ques_form.leftAttachment:		ATTACH_FORM
*new_ques_form.rightAttachment:		ATTACH_FORM
*new_ques_form.bottomAttachment:	ATTACH_FORM
*new_ques_form.topAttachment:		ATTACH_WIDGET
*new_ques_form.topWidget:		buttonSep
!
! New question
!
*nq_pane.marginWidth:			0
*nq_pane.marginHeight:			0
*nq_pane.separatorOn:			TRUE
*nq_pane.leftAttachment:		ATTACH_FORM
*nq_pane.rightAttachment:		ATTACH_FORM
*nq_pane.bottomAttachment:		ATTACH_FORM
*nq_pane.topAttachment:			ATTACH_FORM
!
*top_form.marginHeight:			0
*top_form.marginWidth:			0
!
*top_lbl.labelString:			Choose a topic by clicking on it below.
*top_lbl.topAttachment:			ATTACH_FORM
*top_lbl.leftAttachment:		ATTACH_FORM
*top_lbl.rightAttachment:		ATTACH_FORM
!
*topic_listSW.topAttachment:		ATTACH_WIDGET
*topic_listSW.topWidget:		top_lbl
*topic_listSW.leftAttachment:		ATTACH_FORM
*topic_listSW.rightAttachment:		ATTACH_FORM
*topic_listSW.bottomAttachment:		ATTACH_FORM
!
*topic_list*selectionPolicy:		BROWSE_SELECT
*topic_list*scrollBarDisplayPolicy:	AS_NEEDED
*topic_list*scrollBarPlacement:		BOTTOM_RIGHT
!
*bottom_form.marginWidth:		0
*bottom_form.marginHeight:		0
!
*bottom_lbl.labelString:		Type in the initial text of your question.
*bottom_lbl.topAttachment:		ATTACH_FORM
*bottom_lbl.leftAttachment:		ATTACH_FORM
*bottom_lbl.rightAttachment:		ATTACH_FORM
!
*newqRowCol.leftAttachment:		ATTACH_FORM
*newqRowCol.rightAttachment:		ATTACH_FORM
*newqRowCol.bottomAttachment:		ATTACH_FORM
*newqRowCol.marginHeight:		0
*newqRowCol.marginWidth:		0
*newqRowCol.borderWidth:		0
*newqRowCol.orientation:		HORIZONTAL
*newqRowCol.packing:			PACK_TIGHT
*newqRowCol*traversalOn:		False
!
*send_newq_btn.labelString:		Enter this question in OLC
!
*clear_btn.labelString:			Clear question
!
*newqSW.rightAttachment:		ATTACH_FORM
*newqSW.leftAttachment:			ATTACH_FORM
*newqSW.topAttachment:			ATTACH_WIDGET
*newqSW.topWidget:			bottom_lbl
*newqSW.bottomAttachment:		ATTACH_WIDGET
*newqSW.bottomWidget:			newqRowCol
!
*newq.traversalOn:			True
*newq.editable:				TRUE
*newq.editMode:				MULTI_LINE_EDIT
*newq.wordWrap:				TRUE
*newq.scrollHorizontal:			FALSE
*newq.scrollVertical:			TRUE
!
! Continue question form -contains log in progress & motd
!
*cont_ques_form.marginWidth:		5
*cont_ques_form.marginHeight:		0
*cont_ques_form.leftAttachment:		ATTACH_FORM
*cont_ques_form.rightAttachment:	ATTACH_FORM
*cont_ques_form.bottomAttachment:	ATTACH_FORM
*cont_ques_form.topAttachment:		ATTACH_WIDGET
*cont_ques_form.topWidget:		buttonSep
*cont_ques_form*traversalOn:		false
!
*connect_lbl.labelString:		Status unknown.
*connect_lbl.topAttachment:		ATTACH_FORM
*connect_lbl.leftAttachment:		ATTACH_FORM
!
*topic_lbl.topAttachment:		ATTACH_FORM
*topic_lbl.rightAttachment:		ATTACH_FORM
*topic_lbl.labelString:			unknown
#ifdef _IBMR2
*topic_lbl.fontList:			-adobe-*schoolbook-medium-i-normal--12-*
#else
*topic_lbl.fontList:			-adobe-*schoolbook-medium-i-*-*-*-120-*
#endif
!
*your_topic_lbl.topAttachment:		ATTACH_FORM
*your_topic_lbl.rightAttachment:	ATTACH_WIDGET
*your_topic_lbl.rightWidget:		topic_lbl
*your_topic_lbl.labelString:		Your topic is: 
!
*optionsRowCol.leftAttachment:		ATTACH_FORM
*optionsRowCol.rightAttachment:		ATTACH_FORM
*optionsRowCol.bottomAttachment:	ATTACH_FORM
*optionsRowCol.borderWidth:		0
*optionsRowCol.marginWidth:		0
*optionsRowCol.marginHeight:		0
*optionsRowCol.orientation:		HORIZONTAL
*optionsRowCol.packing:			PACK_TIGHT
*optionsRowCol*traversalOn:		False
!
*send_btn.labelString:			Open "send" window
*done_btn.labelString:			Done
*cancel_btn.labelString:		Cancel
*savelog_btn.labelString:		Save to file...
*motd_btn.labelString:			MOTD
!#ifdef _IBMR2
!*motd_btn.sensitive:			false
!#endif
*update_btn.labelString:		Show new messages
!
*replaySW.rightAttachment:		ATTACH_FORM
*replaySW.leftAttachment:		ATTACH_FORM
*replaySW.topAttachment:		ATTACH_WIDGET
*replaySW.topWidget:			connect_lbl
*replaySW.bottomAttachment:		ATTACH_WIDGET
*replaySW.bottomWidget:			optionsRowCol
*replaySW.marginHeight:			5
*replaySW.marginWidth:			5
!*replaySW.height			300

!
*replay.editable:			FALSE
*replay.editMode:			MULTI_LINE_EDIT
*replay.wordWrap:			TRUE
*replay.scrollHorizontal:		FALSE
*replay.scrollVertical:			TRUE
*replay.insertionPointVisible:		FALSE
!
! Motd form
!
*motd_form.marginWidth:			0
*motd_form.marginHeight:		0
*motd_form.rightAttachment:		ATTACH_FORM
*motd_form.leftAttachment:		ATTACH_FORM
*motd_form.bottomAttachment:		ATTACH_FORM
*motd_form.topAttachment:		ATTACH_WIDGET
*motd_form.topWidget:			buttonSep
!
*motdSW.rightAttachment:		ATTACH_FORM
*motdSW.leftAttachment:			ATTACH_FORM
*motdSW.topWidget:			copyright_lbl
*motdSW.topAttachment:			ATTACH_WIDGET
*motdSW.bottomAttachment:		ATTACH_FORM
!
*motd.traversalOn:			false
*motd.editable:				FALSE
*motd.editMode:				MULTI_LINE_EDIT
*motd.wordWrap:				TRUE
*motd.scrollHorizontal:			FALSE
*motd.scrollVertical:			TRUE
!
#ifdef _IBMR2
*welcome_lbl.fontList:			-adobe-*schoolbook-bold-r-normal--18-*
#else
*welcome_lbl.fontList:			-adobe-*schoolbook-bold-r-*-*-*-180-*
#endif
*welcome_lbl.labelString:		Welcome to On-Line Consulting.
*welcome_lbl.topAttachment:		ATTACH_FORM
*welcome_lbl.leftAttachment:		ATTACH_FORM
*welcome_lbl.rightAttachment:		ATTACH_FORM
*welcome_lbl.recomputeSize:		False
!
*copyright_lbl.labelString:		Copyright © 1992 by the Massachusetts Institute of Technology.
#ifdef _IBMR2
*copyright_lbl.fontList:		-adobe-*schoolbook-bold-r-normal--14-*
#else
*copyright_lbl.fontList:		-adobe-*schoolbook-bold-r-*-*-*-140-*
#endif
*copyright_lbl.rightAttachment:		ATTACH_FORM
*copyright_lbl.leftAttachment:		ATTACH_FORM
*copyright_lbl.topAttachment:		ATTACH_WIDGET
*copyright_lbl.topWidget:		welcome_lbl
*copyright_lbl.recomputeSize:		False
!
!
! Dialogs
!
!
*motd_dlg.dialogStyle:			DIALOG_MODELESS
*motd_dlg.dialogType:			DIALOG_INFORMATION
*motd_dlg.borderWidth:			5
!
*help_dlg.dialogStyle:			DIALOG_MODELESS
*help_dlg.borderWidth:			5
!
*save_dlg.selectionLabelString:		Please enter the name of a file to save in:
!
!
*send_form.marginWidth:			5
*send_form.marginHeight:		5
*send_form.shadowThickness:		2
*send_form.width:			530
*send_form.height:			250
!
*send_lbl.labelString:			Type your message in the box below.
*send_lbl.topAttachment:		ATTACH_FORM
*send_lbl.leftAttachment:		ATTACH_FORM
*send_lbl.rightAttachment:		ATTACH_FORM
!
*sendRowCol.leftAttachment:		ATTACH_FORM
*sendRowCol.rightAttachment:		ATTACH_FORM
*sendRowCol.bottomAttachment:		ATTACH_FORM
*sendRowCol.borderWidth:		0
*sendRowCol.marginHeight:		0
*sendRowCol.marginWidth:		0
*sendRowCol.orientation:		HORIZONTAL
*sendRowCol.packing:			PACK_TIGHT
*sendRowCol*traversalOn:		False
!
*send_msg_btn.labelString:		Send message
!
*clear_msg_btn.labelString:		Clear message
!
*close_msg_btn.labelString:		Close "send" window
!
*sendSW.rightAttachment:		ATTACH_FORM
*sendSW.leftAttachment:			ATTACH_FORM
*sendSW.topAttachment:			ATTACH_WIDGET
*sendSW.topWidget:			send_lbl
*sendSW.bottomAttachment:		ATTACH_WIDGET
*sendSW.bottomWidget:			sendRowCol
!
*send.editMode:			MULTI_LINE_EDIT
*send.editable:			TRUE
*send.wordWrap:			TRUE
*send.scrollHorizontal:		FALSE
*send.scrollVertical:		TRUE

!
*XmPushButton.translations:		#override \
					<Btn1Down>: Arm() \n\
					<Btn2Down>: Arm() \n\
					<Btn3Down>: Arm() \n\
					<Btn1Up>: Activate() Disarm() \n\
					<Btn2Up>: Activate() Disarm() \n\
					<Btn3Up>: Activate() Disarm() \n\
					<Key>F1: Help() \n\
					<Key>Return: Arm() Activate() Disarm()

