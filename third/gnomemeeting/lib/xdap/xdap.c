/*
 * XML LDAP client
 * $Id: xdap.c,v 1.1.1.1 2004-10-16 17:42:29 ghudson Exp $
 * Copyright 2001 paul666@mailandnews.com
 * Licensed under version 2 or later of the GNU GPL
 */


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "../../config.h"

#include "xdap.h"

static int doldapop (LDAP *, xmlDocPtr, int, xmlNodePtr, int, char *, int,
		     char *, char **, int);
static int addattr (LDAPMod **, int *, char *, char *, int);
static void freeattrs (LDAPMod **);
static int parsedoc (xmlDocPtr);
static char *evalcmd (char *);

#ifndef WIN32
extern char **environ;
#endif


/* For registration of libxml private pointers */
static int nregs = 0;
static struct {
	int refcount;
	void *val;
} regs[REGMAX];

/* free all allocated pointers */
/* only need to use this if using an old version of libxml */
void
xdapfree(void)
{
	int i;
	for (i = 0; i < nregs; i++)
		if (regs[i].refcount != 0) {
			if ((regs[i].val == 0) || (regs[i].val == (void *)0xdeadbeef)) {
				////fprintf(stderr, "BOGUSFREE %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);
                                //
                        }
			else {
#if DEBUGXDAPLEAK
				//fprintf(stderr, "FREE %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);
#endif
				free(regs[i].val);
				regs[i].refcount = 0;
				regs[i].val = (void *)0xdeadbeef;
			}
		}
}

/* check if libxml failed to call our unregister handler on any pointer */
void
xdapleakcheck(void)
{/*
	int i;
	for (i = 0; i < nregs; i++)
		if (regs[i].refcount != 0)
		fprintf(stderr, "LEAK %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);*/
}

/* register a pointer manually */
static void
registerptr(void *ptr)
{
	int i;
	int first = 0; /* reuse old slot */
	if (ptr == 0)
		return;
	for (i = 0; i < nregs; i++)
		if (regs[i].refcount > 0) {
			if (regs[i].val == ptr) {
				/* can't think of a good reason to have multiple references to same ptr */
				/* but, just in case... */
				regs[i].refcount++;
				break;
			}
		} else if (!first)
			first = i;
	if (i >= nregs) {
		if (first) {
			i = first;
			nregs--; /* negate later increment */
		}
		regs[i].refcount = 1;
		regs[i].val = ptr;
		if (i < REGMAX - 3) 
                  nregs++;
	}
#if DEBUGXDAPLEAK
	//fprintf(stderr, "RP %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);
#endif
}

/* this is the new libxml node registration function */
/* however it always gets called before we've filled in _private */
/* so it's not much use for us at the moment */
static void
registerNode(xmlNodePtr node)
{
	registerptr(node->_private);
}

/* deregister a node */
static void
deregisterNode(xmlNodePtr node)
{
	int i;
	if (node->_private == 0)
		return;
	for (i = 0; i < nregs; i++)
		if ((regs[i].refcount > 0) && (regs[i].val == node->_private)) {
			regs[i].refcount--;
			//fprintf(stderr, "DN %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);
			if (regs[i].refcount == 0) {
				free(regs[i].val);
				regs[i].val = (void *)0xdeadbeef;
			}
			break;
		}
	if (i >= nregs) {
		regs[i].refcount = -1;
		regs[i].val = node->_private;
#if DEBUGXDAPLEAK
	//fprintf(stderr, "DN %d %d %lx\n", i, regs[i].refcount, (unsigned long)regs[i].val);
#endif
	}
}

/* Parse an xml file and process it */
/* Returns PFERR if file fails to parse or error from parsedoc() */
int
parsefile (char *filename, xmlEntityPtr (*getent) (void *, const xmlChar *),
	   xmlEntityPtr (**oldgetent) (void *, const xmlChar *))
{
  xmlDocPtr xp;

  if (!(xp = parseonly (filename, getent, oldgetent, 0)))
    return PFERR;
  return parsedoc (xp);
}

/* Parse an xml file and substitute entities */
/* Returns pointer to in memory xml document, or
 * 0 - xml document failed to parse
 */
xmlDocPtr
parseonly (char *filename, xmlEntityPtr (*getent) (void *, const xmlChar *),
	   xmlEntityPtr (**oldgetent) (void *, const xmlChar *), int noerr)
{
  xmlDocPtr xp;

#if HAVE_XMLSAXHANDLERV1
  xmlSAXHandlerV1 sax;
#else
  xmlSAXHandler sax;
#endif

  memset (&sax, 0, sizeof sax);
  sax = xmlDefaultSAXHandler;	/* Copy original handler */
  *oldgetent = sax.getEntity;	/* Substitute entity handling */
  if (noerr) {
    sax.error = NULL;	/* Disable errors and warnings */
    sax.warning = NULL;	/* Substitute to capture error text */
  }
  sax.getEntity = getent;
  D (D_TRACE, fprintf (stderr, "Parsing %s\n", filename));
#if HAVE_XMLREGISTERNODEDEFAULT
  xmlRegisterNodeDefault(registerNode);
  xmlDeregisterNodeDefault(deregisterNode);
#endif

  if (!(xp = xmlSAXParseFile (&sax, filename, 0)))
    return 0;
  return xp;
}

/* Process each element of the xml tree synchronously */
/* Returns:
 * Error code from getldapinfo(), or
 * -1 if ldap_open() fails, or
 * Error from ldaprun(), or
 * Error from ldap_unbind_s()
 * 
 * ldap error code from ldap_bind_s
 */
static int
parsedoc (xmlDocPtr xp)
{
  LDAP *ldap;		/* handle */
  xmlNodePtr current;	/* current node of document */
  char *host;
  int port;
  char *who;
  char *cred;
  ber_tag_t method;
  int rc;
  int op;
  unsigned int ip;

  rc = -1;
  if (!(rc = getldapinfo (xp, &current, &host, &port, &who, &cred,
			  &method))) {
    if ((ldap = ldap_open (host, port)) &&
	!(rc = ldap_bind_s (ldap, who, cred, method))) {
      while (current) {
	rc = ldaprun (ldap, xp, &current, &op, &ip, 1);
	if (rc)
	  break;
      }
      xmlFreeDoc (xp);
      if (rc) {
	(void) ldap_unbind_s (ldap);	/* Discard errors */
	return rc;
      } else
	return ldap_unbind_s (ldap);
    }
    xmlFree (host);
    xmlFree (who);
    xmlFree (cred);

  }
  xmlFreeDoc (xp);
  return rc;

}

/* Retrieve ldap server and binding info from xml tree */
/* Returns:
 * 0 - no error
 * PFEMPTY - Empty document
 * PFBADMETHOD - Bad authentication method
 * PFNOCHILD - No childnodes, i.e. <ldap></ldap> - no point in connecting
 */
int
getldapinfo (xmlDocPtr xp, xmlNodePtr * current, char **hostp, int *portp,
	     char **whop, char **credp, ber_tag_t * methp)
{
  xmlNodePtr root;	/* root node of document */
  /* attributes of root */
  xmlChar *host;		/* ldap host */
  xmlChar *port;		/* ldap port */
  xmlChar *who;		/* DN to bind to */
  xmlChar *cred;		/* credentials */
  xmlChar *method;	/* authentication method symbolic */
  ber_tag_t imethod;	/* authentication method */
  struct edata *ep;	/* per element private structure */

  if (!(root = xmlDocGetRootElement (xp))) {
    xmlFreeDoc (xp);
    return PFEMPTY;
  }
  D (D_SHOWTREE, xmlDocDump (stderr, xp));
  D (D_SHOWELS, fprintf (stderr, "Root element name %s\n", root->name));
  /* process root element attributes */
  /* Ignore root element name */
  if (!(method = xmlGetProp (root, BAD_CAST "method")))
    method = xmlStrdup (BAD_CAST "SIMPLE");
  if (!strcasecmp ((char *) method, "NONE"))
    imethod = LDAP_AUTH_NONE;
  else if (!strcasecmp ((char *) method, "SIMPLE"))
    imethod = LDAP_AUTH_SIMPLE;
  else if (!strcasecmp ((char *) method, "SASL"))
    imethod = LDAP_AUTH_SASL;
  else if (!strcasecmp ((char *) method, "KRBV4"))
    imethod = LDAP_AUTH_KRBV4;
  else if (!strcasecmp ((char *) method, "KRBV41"))
    imethod = LDAP_AUTH_KRBV41;
  else if (!strcasecmp ((char *) method, "KRBV42"))
    imethod = LDAP_AUTH_KRBV42;
  else {
    xmlFree (method);
    return PFBADMETHOD;
  }
  if (!(host = xmlGetProp (root, BAD_CAST "host")))
    host = xmlStrdup (BAD_CAST "ils.seconix.com");
  if (!(port = xmlGetProp (root, BAD_CAST "port")))
    port = xmlStrdup (BAD_CAST "389");
  if (!(who = xmlGetProp (root, BAD_CAST "who")))
    who = xmlStrdup (BAD_CAST "");
  if (!(cred = xmlGetProp (root, BAD_CAST "cred")))
    cred = xmlStrdup (BAD_CAST "");
  D (D_SHOWATTS, fprintf (stderr,
			  "host %s who %s cred %s method %s port %s\n",
			  host, who, cred, method, port));

  if (!(*current = root->xmlChildrenNode)) {
    D (D_NODES, fprintf (stderr, "Empty node %s\n", root->name));
    xmlFree (method);
    xmlFree (host);
    xmlFree (port);
    xmlFree (who);
    xmlFree (cred);
    return PFNOCHILD;
  }
  if (!(ep = (struct edata *) malloc (sizeof (struct edata)))) {
    xmlFree (method);
    xmlFree (host);
    xmlFree (port);
    xmlFree (who);
    xmlFree (cred);
    return PFNOMEM;
  }
  ep->optype = 0;
  ep->fertile = 1;
  ep->modop = 0;
  root->_private = (void *) ep;
  registerptr(ep);
  *hostp = (char *) host;
  *portp = atoi ((char *) port);
  xmlFree (method);
  xmlFree (port);
  *whop = (char *) who;
  *credp = (char *) cred;
  *methp = imethod;

  return 0;
}

/* Process an individual node (ldap operation) of the xml tree */
/* Returns:
 * 0 - No error
 * Local negative error codes:
 *  PFNONODE - No current node
 *  PFTOOMANYATTRS - node has too many attributes
 *  PFUNKOP - Not add, delete, search, modify
 *  PFBADSCOPE - Unknown scope
 *  PFTOOMANYOPS - attribute has too many values
 *  PFNOSUB - Node cannot be a child of parent
 *  PFBADMOD - Bad modify mod_op
 * -1 - ldap error occurred (retrieve with ldap_errno() family)
 * > 0 - ldap msgid
 */
int
ldaprun (LDAP * ldap, xmlDocPtr xp, xmlNodePtr * current, int *opp,
	 unsigned int *ipp, int sync)
{
  int optype;		/* ldap operation */
  xmlChar *base = 0;	/* search base */
  xmlChar *scope = 0;	/* search scope symbolic */
  int iscope = 0;	/* search scope */
  xmlChar *filter = 0;	/* search filter */
  xmlChar *attrlist = 0;	/* attribute list to return */
  char *attra[MAXATTRS];	/* attribute list in array form */
  xmlChar *ignores = 0;	/* errors to ignore */
  int attrcount;		/* number of attributes */
  xmlChar *attrsonly = 0;	/* return attributes not vals */
  int rc;			/* return code */
  unsigned int ignoremask = 0;	/* errors to ignore */
  char *p, *q;
  int l;
  struct edata *ep;	/* per element private structure */

  D (D_NODES, fprintf (stderr, "Node name %s %sns %s\n",
		       (*current)->name,
		       (*current)->ns ? "" : "no",
		       (*current)->ns ? "exists" : ""));
  while (*current && (xmlIsBlankNode (*current) || ((*current)->type !=
						    XML_TEXT_NODE
						    && (*current)->type !=
						    XML_ELEMENT_NODE))) {

    D (D_SHOWBLK, fprintf (stderr, "Blank %s\n", (*current)->name));
    D (D_SHOWBLK, fprintf (stderr, "Type %d\n", (*current)->type));
    *current = (*current)->next;
  }
  if (!(*current))
    return 0;	/* PFNONODE */
  /* Check if parent can have children */

  D (D_TRACE, fprintf (stderr, "PCHK1 %d %d %d\n",
		       (*current)->parent ? 1 : 0,
		       (*current)->parent->_private ? 1 : 0,
		       ((struct edata *) ((*current)->parent->_private))->
		       fertile));
  if ((*current)->parent && (*current)->parent->_private
      && !((struct edata *) ((*current)->parent->_private))->fertile) {
    D (D_SHOWOP,
       fprintf (stderr, "Nochildren %s %s %s\n", (*current)->name,
		xmlNodeListGetString (xp,
				      (*current)->xmlChildrenNode,
				      1),
		(*current)->parent ? (char *) ((*current)->parent)->
		name : "T"));
    return PFNOSUB;
  }
  /* Process common attributes */
  if (!(ignores = xmlGetProp (*current, BAD_CAST "ignore")))
    ignores = xmlStrdup (BAD_CAST "");
  for (p = (char *) ignores; *p;) {
    if (*p == ' ') {
      p++;
      continue;
    }
    q = strchr (p + 1, ' ');
    if (q)
      l = q - p;
    else {
      l = (int) strlen (p);
      q = p;
    }
    D (D_MISC, fprintf (stderr, "p %s q %s l %d\n", p, q, l));
    if (q) {
      if (!strncmp (p, "exists", l > 6 ? l : 6))
	ignoremask |= IGN_EX;
      else if (!strncmp (p, "nonleaf", l > 7 ? l : 7))
	ignoremask |= IGN_NL;
      else if (!strncmp (p, "noobject", l > 8 ? l : 8))
	ignoremask |= IGN_NO;
      else if (!strncmp (p, "noaccess", l > 8 ? l : 8))
	ignoremask |= IGN_NA;
      else {
	xmlFree (ignores);
	*current = (*current)->next;
	return PFBADIGN;
      }
      p += l;
    }
  }
  xmlFree (ignores);
  D (D_MISC, fprintf (stderr, "ignoremask %d\n", ignoremask));
  if (!(attrsonly = xmlGetProp (*current, BAD_CAST "attrsonly")))
    attrsonly = xmlStrdup (BAD_CAST "0");
  if (!strcmp ((char *) ((*current)->name), "add")) {
    if (!(ep = (struct edata *) malloc (sizeof (struct edata))))
      return PFNOMEM;
    ep->optype = optype = LD_ADD;
    ep->fertile = 1;
    ep->modop = 0;
    (*current)->_private = (void *) ep;
    registerptr(ep);
  } else if (!strcmp ((char *) ((*current)->name), "modify")) {
    if (!(ep = (struct edata *) malloc (sizeof (struct edata))))
      return PFNOMEM;
    ep->optype = optype = LD_MOD;
    ep->fertile = 1;
    ep->modop = LDAP_MOD_REPLACE;
    (*current)->_private = (void *) ep;
    registerptr(ep);
  } else if (!strcmp ((char *) ((*current)->name), "delete")) {
    if (!(ep = (struct edata *) malloc (sizeof (struct edata))))
      return PFNOMEM;
    ep->optype = optype = LD_DEL;
    ep->fertile = 1;
    ep->modop = 0;
    (*current)->_private = (void *) ep;
    registerptr(ep);
#ifdef DOSYS
  } else if (!strcmp ((char *) ((*current)->name), "system")) {
    if (!(ep = (struct edata *) malloc (sizeof (struct edata))))
      return PFNOMEM;
    ep->optype = optype = LD_SYS;
    ep->fertile = 0;
    ep->modop = 0;
    (*current)->_private = (void *) ep;
    registerptr(ep);
#endif
  } else if (!strcmp ((char *) ((*current)->name), "search")) {
    if (!(ep = (struct edata *) malloc (sizeof (struct edata))))
      return PFNOMEM;
    ep->optype = optype = LD_SEARCH;
    ep->fertile = 1;
    ep->modop = 0;
    (*current)->_private = (void *) ep;
    registerptr(ep);
    /* Process search attributes */
    if (!(base = xmlGetProp (*current, BAD_CAST "base")))
      base = xmlStrdup (BAD_CAST "");
    if (!(scope = xmlGetProp (*current, BAD_CAST "scope")))
      scope = xmlStrdup (BAD_CAST "SUBTREE");
    if (!(filter = xmlGetProp (*current, BAD_CAST "filter")))
      filter = xmlStrdup (BAD_CAST "(objectClass=*)");
    if (!(attrlist = xmlGetProp (*current, BAD_CAST "attrs")))
      attrlist = 0;
    if (!strcasecmp ((char *) scope, "SUBTREE"))
      iscope = LDAP_SCOPE_SUBTREE;
    else if (!strcasecmp ((char *) scope, "BASE"))
      iscope = LDAP_SCOPE_BASE;
    else if (!strcasecmp ((char *) scope, "ONELEVEL"))
      iscope = LDAP_SCOPE_ONELEVEL;
    else {
      xmlFree (scope);
      xmlFree (base);
      xmlFree (filter);
      xmlFree (attrsonly);
      if (attrlist)
	xmlFree (attrlist);
      *current = (*current)->next;
      return PFBADSCOPE;
    }
    attrcount = 0;
    attra[attrcount++] = (char *) attrlist;
    if (attrlist)
      for (p = (char *) attrlist; *p; p++)
	if (*p == ' ') {
	  *p = '\0';
	  if (*(p + 1) && *(p + 1) != ' ') {
	    if (attrcount >= MAXATTRS) {
	      *current = (*current)
		->next;
	      return PFTOOMANYATTRS;
	    } else
	      attra[attrcount++] =
		p + 1;
	  }
	}
    attra[attrcount++] = 0;
  } else {
    *current = (*current)->next;
    return PFUNKOP;
  }
  *opp = optype;
  *ipp = ignoremask;
  /* TODO - Optionally replace if already exists */
  if ((rc = doldapop (ldap, xp, sync, *current, optype,
		      (char *) base, iscope, (char *) filter, attra,
		      atoi ((char *) attrsonly)))) {
    /*D(D_MISC, fprintf(stderr, "Ignoring error %d\n",
      rc)); */
  }
  if (scope)
    xmlFree (scope);
  if (base)
    xmlFree (base);
  if (filter)
    xmlFree (filter);
  if (attrsonly)
    xmlFree (attrsonly);
  if (attrlist)
    xmlFree (attrlist);
  if (sync)
    if (((ignoremask & IGN_NO) && (rc == LDAP_NO_SUCH_OBJECT)) ||
	((ignoremask & IGN_NA)
	 && (rc == LDAP_INSUFFICIENT_ACCESS)) ||
	((ignoremask & IGN_NL)
	 && (rc == LDAP_NOT_ALLOWED_ON_NONLEAF)) ||
	((ignoremask & IGN_EX) && (rc == LDAP_ALREADY_EXISTS)))
      rc = 0;
  *current = (*current)->next;
  D (D_MISC, fprintf (stderr, "dold rc %d\n", rc));
  return rc;
}

/* Create LDAP attribute structure and perform operation */
/* Returns:
 * 0 - No error
 * Local negative error codes:
 *  PFTOOMANYATTRS - node has too many attributes
 *  PFTOOMANYOPS - attribute has too many values
 *  PFBADMOD - Bad modify mod_op
 * -1 - ldap error occurred (retrieve with ldap_errno() family)
 * > 0 - ldap msgid
 */
static int
doldapop (LDAP * ldap, xmlDocPtr xp, int sync, xmlNodePtr node, int optype,
	  char *base, int iscope, char *filter, char *attra[], int attrsonly)
{
  char *dn = 0;		/* Active DN */
  LDAPMod *attrs[MAXATTRS];	/* attribute pointers */
  LDAPMessage *res;	/* result list from search */
  int nattrs = 0;		/* Number of attributes */
  int rc = 0;
  char *text = 0;		/* shell command */
  xmlChar *modop;		/* attribute mod_op */
  xmlChar *eval;		/* evaluate the element text */
  char *nodeval;		/* node text contents */
  char *nodevalexp;	/* expanded node contents */
  int modval;		/* decimal mod_op */
  struct edata *ep;	/* per element private structure */

  D (D_TRACE, fprintf (stderr, "LDOP %d\n", optype));
  attrs[0] = 0;		/* Just in case there are no attributes */
  if (!(node = node->xmlChildrenNode)) {
    D (D_NODES, fprintf (stderr, "Empty node\n"));
    /*return PFNOCHILD; */
  }
  while (node) {
    D (D_NODES,
       fprintf (stderr, "ld Node name %s type %d %sns %s\n",
		node->name, node->type, node->ns ? "" : "no",
		node->ns ? "exists" : ""));
    if (xmlIsBlankNode (node)) {
      D (D_SHOWBLK,
	 fprintf (stderr, "Blank %s\n", node->name));
    } else if (node->type == XML_TEXT_NODE) {
      text = (char *) xmlNodeGetContent (node);
      D (D_SHOWBLK, fprintf (stderr, "Text %s\n", text));
    } else if (node->type != XML_TEXT_NODE &&
	       node->type != XML_ELEMENT_NODE) {
      /* Not interested in comments etc. */
      D (D_SHOWBLK, fprintf (stderr, "Type %d\n",
			     node->type));
    } else {
      D (D_TRACE, fprintf (stderr, "PCHK2 %d %d %d\n",
			   node->parent ? 1 : 0,
			   node->parent->_private ? 1 : 0,
			   ((struct edata *) (node->parent->
					      _private))->
			   fertile));
      if (node->parent && node->parent->_private
	  && !((struct edata *) (node->parent->_private))->
	  fertile) {
	D (D_SHOWOP,
	   fprintf (stderr, "Nochildren %s %s %s\n",
		    node->name,
		    xmlNodeListGetString (xp,
					  node->
					  xmlChildrenNode,
					  1),
		    node->parent ? (char *) (node->
					     parent)->
		    name : "T"));
	return PFNOSUB;
      }
      D (D_SHOWOP, fprintf (stderr, "xOp %s %s %s\n",
			    node->name,
			    xmlNodeListGetString (xp,
						  node->
						  xmlChildrenNode,
						  1),
			    node->parent ? (char *) (node->
						     parent)->
			    name : "T"));
      /* Setting DN doesn't add attribute */
      if (!strcmp ((char *) (node->name), "dn")) {
	dn = (char *) xmlNodeListGetString (xp,
					    node->
					    xmlChildrenNode,
					    1);
	if (!
	    (ep =
	     (struct edata *)
	     malloc (sizeof (struct edata))))
	  return PFNOMEM;
	ep->optype = ep->fertile = ep->modop =
	  ep->eval = 0;
	node->_private = (void *) ep;
        registerptr(ep);
      } else {
	if (!
	    (ep =
	     (struct edata *)
	     malloc (sizeof (struct edata))))
	  return PFNOMEM;
	ep->optype = ep->fertile = ep->modop =
	  ep->eval = 0;
	node->_private = (void *) ep;
        registerptr(ep);

	/* if no "op" mod_op, inherit from parent */
	if (!(modop = xmlGetProp (node, BAD_CAST "op"))) {
	  if (node->parent
	      && node->parent->_private)
	    modval = ((struct edata *)
		      (node->parent->
		       _private))->modop;
	  else
	    modval = 0;
	} else {
	  if (!strcmp ((char *) modop, "delete"))
	    modval = LDAP_MOD_DELETE;
	  else if (!strcmp
		   ((char *) modop, "add"))
	    modval = LDAP_MOD_ADD;
	  else if (!strcmp
		   ((char *) modop, "replace"))
	    modval = LDAP_MOD_REPLACE;
	  else {
	    xmlFree (modop);
	    return PFBADMODOP;
	  }
	  xmlFree (modop);
	}
	/* Test to see if this element should be backticked */
	if ((eval = xmlGetProp (node, BAD_CAST "eval"))) {
	  D (D_TRACE,
	     fprintf (stderr, "EVALOP %s\n",
		      eval));
	  if (!strcmp ((char *) eval, "yes")
	      || !strcmp ((char *) eval, "y")
	      || !strcmp ((char *) eval, "1"))
	    ep->eval = 1;
	  xmlFree (eval);
	}
	nodeval = (char *) xmlNodeListGetString (xp,
						 node->
						 xmlChildrenNode,
						 1);
	if (ep->eval) {
	  nodevalexp = evalcmd (nodeval);
	  xmlFree (nodeval);
	  nodeval = nodevalexp;
	}
	/* process ldap attributes */
	if ((rc = addattr (attrs, &nattrs,
			   (char *) (node->name),
			   nodeval, modval))) {
	  freeattrs (attrs);
	  return rc;
	}
      }
    }
    node = node->next;
  }
  D (D_TRACE, fprintf (stderr, "LDOP %d\n", optype));
  switch (optype) {
  case LD_ADD:
    if (!dn)
      return PFNODN;
    rc = sync ? ldap_add_s (ldap, dn, attrs) :
      ldap_add (ldap, dn, attrs);
    break;
  case LD_MOD:
    if (!dn)
      return PFNODN;
    rc = sync ? ldap_modify_s (ldap, dn, attrs) :
      ldap_modify (ldap, dn, attrs);
    break;
  case LD_DEL:
    if (!dn)
      return PFNODN;
    rc = sync ? ldap_delete_s (ldap, dn) : ldap_delete (ldap, dn);
    break;
  case LD_SEARCH:
    rc = sync ? ldap_search_s (ldap, base, iscope, filter,
			       attra, attrsonly, &res) :
      ldap_search (ldap, base, iscope, filter, attra, attrsonly);
    /* Dump results if syncronous search */
    if (sync && !rc)
      dumpres (ldap, res);
    break;
#ifdef DOSYS
  case LD_SYS:
    rc = system (text);
    break;
#endif
  }

  freeattrs (attrs);
  D (D_MISC, fprintf (stderr, "dold returning %d\n", rc));
  return rc;
}

/* Append an ldap attribute */
/* Returns:
 * 0 - No error
 * Local negative error codes:
 *  PFTOOMANYATTRS - node has too many attributes
 *  PFTOOMANYOPS - attribute has too many values
 */
static int
addattr (LDAPMod ** attrs, int *nattrs, char *name, char *val, int modval)
{
  int j, k = 0;

  /* if attribute already exists */
  /* tack on the additional value */
  for (j = 0; j < *nattrs; j++)
    if (!strcmp (attrs[j]->mod_type, name)) {
      D (D_MISC, fprintf (stderr, "found %s %d %d\n",
			  attrs[0]->mod_type, j, *nattrs));
      for (k = 0; attrs[j]->mod_values[k]; k++) ;
      break;
    }
  if (j >= MAXATTRS)
    return PFTOOMANYATTRS;
  if (j == *nattrs) {
    if (!
	(attrs[*nattrs] = (LDAPMod *) xmlMalloc (sizeof (LDAPMod))))
      return PFNOMEM;
    attrs[*nattrs]->mod_op = modval;
    attrs[*nattrs]->mod_type = (char *) xmlStrdup (BAD_CAST name);
    attrs[*nattrs]->mod_values =
      (char **) xmlMalloc (MAXOPS * sizeof (char *));
    attrs[++*nattrs] = 0;
  }
  if (k >= MAXOPS)
    return PFTOOMANYOPS;
	
  attrs[j]->mod_values[k] = (char *) xmlStrdup (BAD_CAST val);
  attrs[j]->mod_values[++k] = 0;

  return 0;
}

/* Free memory blocks allocated by addattr */
static void
freeattrs (LDAPMod ** attrs)
{
  LDAPMod **ap;
  char **op;

  for ((ap = attrs); *ap; ap++) {
    D (D_MISC, fprintf (stderr, "AT free %s\n", (*ap)->mod_type));
    for ((op = (*ap)->mod_values); *op; op++) {
      D (D_MISC, fprintf (stderr, "Op free %s\n", *op));
      xmlFree (*op);
    }
    xmlFree ((*ap)->mod_type);
    xmlFree ((*ap)->mod_values);
    xmlFree (*ap);
  }
}

/* Output an xml tree of results from a search */
void
dumpres (LDAP * ldap, LDAPMessage * res)
{
  BerElement *ber;
  LDAPMessage *entry;
  char *attr;
  char **vals;
  int len;
  int i;
  char **v;

  printf ("<?xml version=\"1.0\"?>\n");
  printf ("<!DOCTYPE ils SYSTEM \"ils.dtd\" []>\n");
  printf ("<ils>\n");
  printf (" <results>\n");
  entry = ldap_first_entry (ldap, res);
  while (entry) {
    /*printf("  <entry dn=\"%s\">\n", ldap_get_dn(ldap, entry)); */
    printf ("  <entry>\n   <dn>%s</dn>\n",
	    ldap_get_dn (ldap, entry));
    attr = ldap_first_attribute (ldap, entry, &ber);
    while (attr) {
      vals = ldap_get_values (ldap, entry, attr);
      len = ldap_count_values (vals);
      for (i = 0, v = vals; i < len; i++, v++)
	printf ("   <%s>%s</%s>\n", attr, *v, attr);
      ldap_value_free (vals);
      attr = ldap_next_attribute (ldap, entry, ber);
    }
    printf ("  </entry>\n");
    entry = ldap_next_entry (ldap, entry);
  }
  printf (" </results>\n");
  printf ("</ils>\n");
}

char *
pferrtostring (int err)
{
  switch (err) {
  case PFERR:
    return "Parse failure";
  case PFEMPTY:
    return "Empty document";
  case PFNOCHILD:
    return "No child elements";
  case PFUNKOP:
    return "Unknown LDAP operation";
  case PFTOOMANYATTRS:
    return "Too many attributes";
  case PFTOOMANYOPS:
    return "Too many attribute values";
  case PFBADMETHOD:
    return "Unknown search method";
  case PFNONODE:
    return "No current node";
  case PFBADSCOPE:
    return "Unknown scope";
  case PFNOSUB:
    return "Parent may not have sub-elements";
  case PFBADIGN:
    return "Unknown ignore attribute";
  case PFBADMODOP:
    return "Unknown attribute modify operation";
  case PFNOMEM:
    return "Memory allocation failed";
  case PFNODN:
    return "Mandatory DN not specified";
  default:
    return "Unknown error";
  }
}

/* pipe command and return results */
static char *
evalcmd (char *cmd)
{
#ifndef WIN32
  int pfd[2];
  int pid;
  char *av[4];
  char *buf[200];
  int nc;
  int size = 0;
  char *rb = 0;

  if (pipe (pfd))
    return "NOPIPE";

  switch (pid = fork ()) {
  case -1:
    return "NOFORK";
  case 0:
    close (0);
    open ("/dev/null", O_RDONLY);
    close (1);
    dup (pfd[1]);
    close (2);
    dup (pfd[1]);
    close (pfd[0]);
    close (pfd[1]);
    av[0] = "sh";
    av[1] = "-c";
    av[2] = cmd;
    av[3] = 0;
    execve ("/bin/sh", av, environ);
    return "NOEXEC";
  default:
    close (pfd[1]);
    while (((errno = 0), (nc = read (pfd[0], buf, sizeof buf)) > 0) || (errno == EINTR)) {
      if (nc <= 0)
        continue;
      if (!rb) {
	if (!(rb = (char *) malloc (nc)))
	  return "NOMEM";
      } else {
	if (!(rb = (char *) realloc (rb, size + nc)))
	  return "NOMEM";
      }
      memcpy (rb + size, buf, nc);
      size += nc;
    }
    if ((size > 0) && (rb[size - 1] == '\n'))
      rb[size - 1] = '\0';
    D (D_TRACE, fprintf (stderr, "EVAL %s\n", rb));
    return rb;
  }
#else
  return "NOFORK";
#endif
}
