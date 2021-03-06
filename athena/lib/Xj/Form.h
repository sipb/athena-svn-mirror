/*
 * $Id: Form.h,v 1.2 1999-01-22 23:16:52 ghudson Exp $
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_Form_h
#define _Xj_Form_h

#include "Jets.h"

extern JetClass formJetClass;
extern void setForm();

typedef struct {int littlefoo;} FormClassPart;

typedef struct _FormClassRec {
  CoreClassPart		core_class;
  FormClassPart	form_class;
} FormClassRec;

extern FormClassRec formClassRec;

typedef struct {
  char *form;
  int padding;
} FormPart;

typedef struct _FormRec {
  CorePart	core;
  FormPart	form;
} FormRec;

typedef struct _FormRec *FormJet;
typedef struct _FormClassRec *FormJetClass;

#define XjCForm "Form"
#define XjNform "form"
#define XjCPadding "Padding"
#define XjNpadding "padding"

#endif /* _Xj_Form_h */
