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
*XmFrame.shadowThickness:		0
*XmFrame.borderWidth:			0
*XmForm.shadowThickness:		2
*XmForm.borderWidth:			0
*XmForm.paneMinimum:			183
*XmForm.horizontalSpacing:		5
*XmForm.verticalSpacing:		5
*XmList.listMarginWidth:		5
*XmList.listMarginHeight:		5
*fontList:				fixed
*title:					On-Line Consulting
*iconName:				OLC
!
xolc.width:			561
xolc.height:			430
xolc.resizable:			FALSE
!
! *main.width:			560
! *main.height:			430
!
*main.topAttachment:			ATTACH_FORM
*main.leftAttachment:			ATTACH_FORM
*main.rightAttachment:			ATTACH_FORM
*main.bottomAttachment:			ATTACH_FORM
!
! Menu buttons
!
*new_ques_btn.topAttachment:		ATTACH_FORM
*new_ques_btn.leftAttachment:		ATTACH_FORM
*new_ques_btn.labelString:		Ask a question
!
*cont_ques_btn.labelString:		Continue your question
*cont_ques_btn.topAttachment:		ATTACH_FORM
*cont_ques_btn.leftAttachment:		ATTACH_FORM
!
*stock_btn.labelString:			Browse stock answers
*stock_btn.topAttachment:		ATTACH_FORM
*stock_btn.leftAttachment:		ATTACH_WIDGET
*stock_btn.leftWidget:			cont_ques_btn
!
*help_btn.labelString:			Help
*help_btn.topAttachment:		ATTACH_FORM
*help_btn.rightAttachment:		ATTACH_FORM
!
*quit_btn.labelString:			Quit
*quit_btn.topAttachment:		ATTACH_FORM
*quit_btn.rightAttachment:		ATTACH_WIDGET
*quit_btn.rightWidget:			help_btn
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
*new_ques_form.width:			540
*new_ques_form.height:			400
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
*list_frame.topAttachment:		ATTACH_WIDGET
*list_frame.topWidget:			top_lbl
*list_frame.leftAttachment:		ATTACH_FORM
*list_frame.rightAttachment:		ATTACH_FORM
*list_frame.bottomAttachment:		ATTACH_FORM
!
*topic_list*selectionPolicy:		SINGLE_SELECT
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
!
*send_newq_btn.labelString:		Enter this question in OLC
!
*clear_btn.labelString:			Clear question
!
*newq_frame.rightAttachment:		ATTACH_FORM
*newq_frame.leftAttachment:		ATTACH_FORM
*newq_frame.topAttachment:		ATTACH_WIDGET
*newq_frame.topWidget:			bottom_lbl
*newq_frame.bottomAttachment:		ATTACH_WIDGET
*newq_frame.bottomWidget:		newqRowCol
!
*newq_scrl.editable:			TRUE
*newq_scrl.editMode:			MULTI_LINE_EDIT
*newq_scrl.wordWrap:			TRUE
*newq_scrl.scrollHorizontal:		FALSE
*newq_scrl.scrollVertical:		TRUE
!
! Continue question form -containts log in progress & motd
!
*cont_ques_form.marginWidth:		5
*cont_ques_form.marginHeight:		0
*cont_ques_form.leftAttachment:		ATTACH_FORM
*cont_ques_form.rightAttachment:	ATTACH_FORM
*cont_ques_form.bottomAttachment:	ATTACH_FORM
*cont_ques_form.topAttachment:		ATTACH_WIDGET
*cont_ques_form.topWidget:		buttonSep
*cont_ques_form.width:			540
*cont_ques_form.height:			400
!
*status_form.marginWidth:		0
*status_form.marginHeight:		0
*status_form.topAttachment:		ATTACH_FORM
*status_form.leftAttachment:		ATTACH_FORM
*status_form.rightAttachment:		ATTACH_FORM
!
*connect_lbl.labelString:		Status unknown.
*connect_lbl.topAttachment:		ATTACH_FORM
*connect_lbl.leftAttachment:		ATTACH_FORM
*connect_lbl.bottomAttachment:		ATTACH_FORM
!
*topic_lbl.topAttachment:		ATTACH_FORM
*topic_lbl.bottonAttachment:		ATTACH_FORM
*topic_lbl.rightAttachment:		ATTACH_FORM
*topic_lbl.labelString:			unknown
#ifdef _IBMR2
*topic_lbl.fontList:			-adobe-*schoolbook-medium-i-normal--12-*
#else
*topic_lbl.fontList:			-adobe-*schoolbook-medium-i-*-*-*-120-*
#endif
!
*your_topic_lbl.topAttachment:		ATTACH_FORM
*your_topic_lbl.bottomAttachment:	ATTACH_FORM
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
!
*send_btn.labelString:			Open `send' window
*done_btn.labelString:			Done
*cancel_btn.labelString:		Cancel
*savelog_btn.labelString:		Save to file...
*motd_btn.labelString:			MOTD
*update_btn.labelString:		Show new messages
!
*replay_frame.rightAttachment:		ATTACH_FORM
*replay_frame.leftAttachment:		ATTACH_FORM
*replay_frame.topAttachment:		ATTACH_WIDGET
*replay_frame.topWidget:		status_form
*replay_frame.bottomAttachment:		ATTACH_WIDGET
*replay_frame.bottomWidget:		optionsRowCol
*replay_frame.marginHeight:		5
*replay_frame.marginWidth:		5
*replay_frame.height			300

!
*replay_scrl.editable:			FALSE
*replay_scrl.editMode:			MULTI_LINE_EDIT
*replay_scrl.wordWrap:			TRUE
*replay_scrl.scrollHorizontal:		FALSE
*replay_scrl.scrollVertical:		TRUE
*replay_scrl.insertionPointVisible:	FALSE
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
*motd_form.width:			540
*motd_form.height:			400
!
*motd_frame.rightAttachment:		ATTACH_FORM
*motd_frame.leftAttachment:		ATTACH_FORM
*motd_frame.topWidget:			copyright_lbl
*motd_frame.topAttachment:		ATTACH_WIDGET
*motd_frame.bottomAttachment:		ATTACH_FORM
!
*motd_scrl.editable:			FALSE
*motd_scrl.editMode:			MULTI_LINE_EDIT
*motd_scrl.wordWrap:			TRUE
*motd_scrl.scrollHorizontal:		FALSE
*motd_scrl.scrollVertical:		TRUE
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
!
*copyright_lbl.labelString:		Copyright 1991 by the Massachusetts Institute of Technology.
#ifdef _IBMR2
*copyright_lbl.fontList:		-adobe-*schoolbook-bold-r-normal--14-*
#else
*copyright_lbl.fontList:		-adobe-*schoolbook-bold-r-*-*-*-140-*
#endif
*copyright_lbl.rightAttachment:		ATTACH_FORM
*copyright_lbl.leftAttachment:		ATTACH_FORM
*copyright_lbl.topAttachment:		ATTACH_WIDGET
*copyright_lbl.topWidget:		welcome_lbl
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
*send_form.marginWidth:			0
*send_form.marginHeight:		0
*send_form.borderWidth:			5
*send_form.width:		530
*send_form.height:		250
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
!
*send_msg_btn.labelString:		Send message
!
*clear_msg_btn.labelString:		Clear message
!
*close_msg_btn.labelString:		Close `send' window
!
*send_frame.rightAttachment:		ATTACH_FORM
*send_frame.leftAttachment:		ATTACH_FORM
*send_frame.topAttachment:		ATTACH_WIDGET
*send_frame.topWidget:			send_lbl
*send_frame.bottomAttachment:		ATTACH_WIDGET
*send_frame.bottomWidget:		sendRowCol
!
*send_scrl.editMode:			MULTI_LINE_EDIT
*send_scrl.editable:			TRUE
*send_scrl.wordWrap:			TRUE
*send_scrl.scrollHorizontal:		FALSE
*send_scrl.scrollVertical:		TRUE

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

