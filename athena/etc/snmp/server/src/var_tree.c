#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/var_tree.c,v 1.2 1990-05-26 13:41:48 tom Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  90/04/26  18:36:31  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:43:20  snmpdev
 * Initial revision
 * 
 */

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */

/*
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/var_tree.c,v 1.2 1990-05-26 13:41:48 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains the code to build and manipulate the
 *  SNMP variable tree.  The tree is built as specified in the MIB document.
 *  The tree info structure that is mentioned in the comments coming
 *  up, is declared in var_tree.h and statically defined in ext.c
 */

#include "include.h"

/*
 *  top level routine that sets up and builds the SNMP variable
 *  tree.  First initialize the variable codes and then build
 *  the tree.  Return an error code if it had a problem.
 *  error codes are defined in snmpd.h
 */
int
make_var_tree()
{
	int err;

	if (debuglevel > 3)
		ptreeinfo(var_tree_info);

	if (err = build_snmp_tree(var_tree_info) <= 0) {
		syslog(LOG_ERR, "make_var_tree: build error, code %d", err);
		return(BUILD_ERR);
	}
	if (debuglevel > 3)
		pvartree(top);

	return(BUILD_SUCCESS);
}

/*
 *  initialize a SNMP tree node.  tree info is provided by the
 *  passed in "tree_info" structure.  nodeflags should contain
 *  at least whether the node is to be a LEAF or PARENT.
 *  Return NULL on any error in generating the node.
 */
struct snmp_tree_node *
init_var_tree_node(nodeflags, tree_info)
	int nodeflags;
	struct snmp_tree_info *tree_info;
{
	struct snmp_tree_node *tmp1;
	int node_size = sizeof(struct snmp_tree_node);
	int cnt;

	/*
	 *  get space for the new node.
	 */
	tmp1 = (struct snmp_tree_node *)malloc((unsigned)node_size);
	if (tmp1 == SNMPNODENULL) {
		syslog(LOG_ERR, "init_var_tree: malloc: %m.\n");
		return(SNMPNODENULL);
	}

	/*
	 *  if there is no "tree_info" we can assume it is a
	 *  parent node.  Initialize everything to null.
	 *  if "tree_info" is present and not a parent, we can
	 *  assume leaf node and fill in the information.
	 */
	if ((tree_info == SNMPINFONULL) || (nodeflags & PARENT_NODE)) {
		tmp1->getval = NULL;
		tmp1->setval = NULL;
		tmp1->flags = nodeflags;
		tmp1->offset = 0;
	}
	else {   /*  info is present, must be a leaf node */
		tmp1->getval = tree_info->valfunc;
		tmp1->setval = tree_info->valsetfunc;
		tmp1->flags = nodeflags | tree_info->var_flags;
		tmp1->var_code = tree_info->codestr;
		tmp1->offset = tree_info->nl_def;
	}
	tmp1->from = (struct sockaddr_in *)NULL;

	/*
	 *  initialize all of the child pointers to NULL
	 */
	for (cnt = 0; cnt <= MAXCHILDPERNODE; cnt++)
		tmp1->child[cnt] = SNMPNODENULL;
	/*
	 *  send back the new node
	 */
	return(tmp1);
}

/*
 *  connect a new leaf node (SNMP variable) to
 *  the variable tree.  If not already made, make and add the
 *  parent nodes until we can properly attach the new leaf node
 *  to the tree.  Return an error code if problems.
 */
int
connect_node(newnode)
	struct snmp_tree_node *newnode;
{
	struct snmp_tree_node *tmp1, *tmp2;
	int var_code_size = (newnode->var_code->ncmp - MIB_PREFIX_SIZE);
	int cnt = 1;
	u_long *c = (newnode->var_code->cmp + MIB_PREFIX_SIZE);

	if (top == SNMPNODENULL) {
		syslog(LOG_ERR, "connect_node: top of tree is NULL.\n");
		return(CONNECT_FAIL);
	}
	tmp1 = top;	/* start at top */

	/*
	 *  starting with the first byte of the variable code, try
	 *  to traverse the tree and find the right place for the
	 *  leaf.  If we need a parent node created, do so and
	 *  continue onward, until we have reached the leaf (or
	 *  reached the end of the variable code), same thing!
	 */
	while (cnt <= var_code_size) {
		if (tmp1->child[*c] == SNMPNODENULL) {
			if (cnt == var_code_size) {  /* we are at the leaf */
				tmp1->child[*c] = newnode;
			}
			else {  /* must create a parent node */
				tmp2 = init_var_tree_node(PARENT_NODE, SNMPINFONULL);
				if (tmp2 == SNMPNODENULL) {
					syslog(LOG_ERR, "connect_node: error in getting new parent node");
					return(CONNECT_FAIL);
				}
				tmp1->child[*c] = tmp2;
			}
		}
		else if (cnt == var_code_size) { /* leaf already there */
			tmp1->child[*c]->getval = newnode->getval;
			tmp1->child[*c]->setval = newnode->setval;
			tmp1->child[*c]->flags |= newnode->flags;
			tmp1->child[*c]->offset = newnode->offset;
			if (tmp1->child[*c]->from != NULL)
				free((char *)tmp1->child[*c]->from);
			tmp1->child[*c]->from = newnode->from;
			free((char *)newnode);
		}
		/*
		 *  traverse downward.
		 */
		tmp1 = tmp1->child[*c];
		cnt++;
		c++;	/* next byte in variable code */
	}
	return(CONNECT_SUCCESS);
}

/*
 *  build the SNMP tree given the static information structure
 *  Also, we want each node created to have the first child
 *  (index "0" in the child array) be a dummy node, NULL and a LEAF.
 *
 *  Makes pointer to top of tree.  Then it traverses the tree info
 *  structure, adding each node to the tree by first making a leaf
 *  node and then connecting it to the tree.  Return an error code
 *  on failure.
 */
int
build_snmp_tree(info)
	struct snmp_tree_info info[];
{
	struct snmp_tree_node *tmp1;
	int cnt = 0, errcode, wasinfo = 0;

	top = init_var_tree_node(PARENT_NODE, SNMPINFONULL);
	if (top == SNMPNODENULL) {
		syslog(LOG_ERR, "build_snmp_tree: root of tree could not be made.");
		return(BUILD_ERR);
	}
	/*
	 *  go through the tree info structure, making leaves and
	 *  connecting them.
	 */
	while (info[cnt].codestr != NULL) {
		wasinfo++;
		tmp1 = init_var_tree_node(LEAF_NODE, &info[cnt]);
		if (tmp1 == SNMPNODENULL) {
			syslog(LOG_ERR, "build_snmp_tree: cannot make leaf");
			return(BUILD_ERR);
		}
	 	/*
		 *  connect the leaf to the tree.
		 */
		if (errcode = connect_node(tmp1) != CONNECT_SUCCESS) {
			syslog(LOG_ERR, "build_snmp_tree: error in connecting leaf");
			return(errcode);
		}
		cnt++;		/* go to next variable */
	}
	/*
	 *  if there "wasinfo" and we got this far, return a sucess!
	 *  if not, report it and return a fail.
	 */
	if (wasinfo != 0)
		return(BUILD_SUCCESS);
	else {
		syslog(LOG_ERR,"build_snmp_tree: tree info structure was NULL");
		return(BUILD_ERR);
	}
}

/*
 *  free up memory allocated for string (STR) type values.
 */
varlist_free(vlt)
	var_list_type *vlt;
{
	int cnt = 0;

	/*
	 *  Go through variable bindings searching for STR types.
	 *  Free them if appropriate.
	 */
	while (cnt < vlt->len) {
		if ((vlt->elem[cnt].val.type == STR) &&
		    (vlt->elem[cnt].val.value.str.str != NULL)) {
			free(vlt->elem[cnt].val.value.str.str);
			vlt->elem[cnt].val.value.str.str = NULL;
		}
		cnt++;
	}
}

