/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Tree.h,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass treeJetClass;

typedef struct {int littlefoo;} TreeClassPart;

typedef struct _TreeClassRec {
  CoreClassPart		core_class;
  TreeClassPart		tree_class;
} TreeClassRec;

extern TreeClassRec treeClassRec;

typedef struct {
  char *tree;
} TreePart;

typedef struct _TreeRec {
  CorePart	core;
  TreePart	tree;
} TreeRec;

typedef struct _TreeRec *TreeJet;
typedef struct _TreeClassRec *TreeJetClass;

#define XjCTree "Tree"
#define XjNtree "tree"
