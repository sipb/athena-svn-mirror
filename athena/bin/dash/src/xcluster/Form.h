/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Form.h,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

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
