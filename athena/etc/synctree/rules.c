/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include "synctree.h"
#include <string.h>
#include <stdio.h>
#ifdef SOLARIS
#include <sys/exechdr.h>
#else
#include <a.out.h>
#endif

extern char *date_db_name;
extern rule rules[];
extern unsigned int lastrule;
extern unsigned int rflag;

extern bool nosrcrules;
extern bool nodstrules;

extern char *src_rule_file;
extern char *dst_rule_file;

extern int verbosef;

char srcpath;
char dstpath;

void compute_ifs();

int findrule(pathname,rtype,ftype,srcpath)
     enum rule_type rtype;
     char *pathname, *srcpath;
{
    int i;
    char *p;

    if (verbosef>2) {
	printf("findrule of |%s| rtype = %d\n", pathname, rtype);
    }

    /* First we catch the special file names which we never want to be
     * updated.  They are "." and "..".  If pathname refers to one of
     * these files, we return rule 0 which should always be a "map *".
     * We also map ".reconcile_dates*" files (date_db_name) to nowhere
     * (by using rule 0).
     */

    if (p = strrchr(pathname,'/'))
	p++;
    else
	p = pathname;

    if ((strcmp(p,".") == 0) || (strcmp(p,"..") == 0) ||
	(strncmp(p,date_db_name,strlen(date_db_name)) == 0))
	return
	    (rtype == R_MAP)? 0 :
    (rtype == R_ACTION)? 1 :
    panic("findrule: rtype is unexpected rule type for special case");
  
    /* now search the normal rules */
    for (i = lastrule ; i >= 0 ; i--) {
	switch (rules[i].type) {
	case R_MAP:
	    if ((rtype == R_MAP) &&
		hastype(rules[i].u.u_map.file_types,ftype) &&
		glob_match(rules[i].u.u_map.globexp,pathname))
		return i;
	    break;
	case R_CHASE:
	    if ((rtype == R_CHASE) &&
		(ftype == TYPE_L) &&
		glob_match(rules[i].u.u_chase.globexp,pathname))
		return i;
	    break;
	case R_ACTION:
	    if (verbosef > 2) {
		printf("Checking for a match with rule %d:\n",i);
		printf("\tFile types: 0x%02.2x\t(source type: 0x%02.2x)\n",
		       rules[i].u.u_action.file_types, ftype);
		printf("\tAction type: %d\n",
		       rules[i].u.u_action.type);
	    }	
	    if ((rtype == R_ACTION) &&
		glob_match(rules[i].u.u_action.globexp,pathname))
		{
		    if (verbosef > 2) {
			printf("Rule %d - regexp passed\n", i);
		    }
		    
		    /*
		     * A "virtual" file (one that exists only in the target)
		     * can only have one of a couple actions performed on it:
		     * "delete" and "ignore"
		     */
		    if (hastype(ftype,TYPE_V) &&
			rules[i].u.u_action.type != ACTION_DELETE &&
			rules[i].u.u_action.type != ACTION_IGNORE)
			break;

		    /*
		     * Don't waste time checking for executables if there is
		     * already a match...
		     */
		    if (hastype(rules[i].u.u_action.file_types,ftype)) {
			if (verbosef>2) {
			    printf("findrule %s - matches general mask; rule %d\n", pathname, i);
			}
			return i;
		    }
	  
		    /*
		     * We have been unable to establish a match; check if this
		     * rule applies to executables and if the file in question
		     * is an executable.
		     */
		    if (hastype(rules[i].u.u_action.file_types,TYPE_X) &&
			!hastype(rules[i].u.u_action.file_types,TYPE_R) &&
			(ftype == TYPE_R))
			{
			    /*
			     * At this point, we know:
			     * 1. The pathname matches the pattern supplied.
			     * 2. The pathname should be an executable.
			     */

			    int fd, nbytes;
			    unsigned long	magic;
			    unsigned short	*smagic =
				(unsigned short *)&magic;

#define swabs(a) (((a&0xff00) >> 8) | ((a&0xff) << 8))
#define swabl(a) ((a>>24)&0xff | (((a>>16)&0xff)<<8) | \
		  (((a>>8)&0xff)<<16) | ((a&0xff)<<24))
    
			    if ((fd=open(srcpath,0,0)) < 0)
				break;
			    nbytes = read(fd,&magic,sizeof(magic));
			    close(fd);

			    if (nbytes != sizeof(magic))
				break;

#ifdef ZMAGIC
#define xxx(a) case(a): case(swabl(a)):
			    switch(magic) {
			    xxx(ZMAGIC);
			    xxx(NMAGIC);
			    xxx(OMAGIC);
#ifdef Z0MAGIC
			    xxx(Z0MAGIC);
#endif
				if (verbosef>2) {
				    printf("findrule %s - matches (COFF executable); rule %d\n", pathname, i);
				}
				return i;

			    default:
				break;	/* Not an a.out executable */
			    }
#undef xxx
#endif /* ZMAGIC */

#ifdef F_EXEC
#define xxx(a) \
	if (*smagic == (a) || *smagic == (swabs(a))) { \
	    if (verbosef>2) { \
		printf("findrule %s - matches (COFF executable); rule %d\n", \
		       pathname, i); \
	    } \
	    return i; \
	}

#ifdef B16MAGIC
			    /* Basic-16 */
			    xxx(B16MAGIC);
			    xxx(BTVMAGIC);
#endif
#ifdef X86MAGIC
			    /* x86 */
			    xxx(X86MAGIC);
			    xxx(XTVMAGIC);
#endif
#ifdef I286SMAGIC
			    xxx(I286SMAGIC);
			    xxx(I286LMAGIC);
#endif
#ifdef N3BMAGIC
			    /* n3b */
			    xxx(N3BMAGIC);
			    xxx(NTVMAGIC);
#endif
#ifdef FBOMAGIC
			    /* MAC-32, 3b-5 */
			    xxx(FBOMAGIC);
			    xxx(RBOMAGIC);
			    xxx(MTVMAGIC);
#endif
#ifdef VAXWRMAGIC
			    /* VAX 11/780 and VAX 11/750 */
			    xxx(VAXWRMAGIC);
			    xxx(VAXROMAGIC);
#endif
#ifdef MC68MAGIC
			    /* Motorola 68000 */
			    xxx(MC68MAGIC);
			    xxx(MC68TVMAGIC);
			    xxx(M68MAGIC);
			    xxx(M68TVMAGIC);
#endif
#ifdef M68NSMAGIC
			    /* UniSoft additions for 68000 */
			    xxx(M68NSMAGIC);
#endif
#ifdef AMDWRMAGIC
			    /* Amdahl 470/580 */
			    xxx(AMDWRMAGIC);
			    xxx(AMDROMAGIC);
#endif
#ifdef U370WRMAGIC
			    /* IBM 370 */
			    xxx(U370WRMAGIC);
			    xxx(U370ROMAGIC);
#endif
#ifdef U800WRMAGIC
			    /* IBM RT */
			    xxx(U800WRMAGIC);
			    xxx(U800ROMAGIC);
			    xxx(U800TOCMAGIC);
#endif
#ifdef U802WRMAGIC
			    /* IBM R2 */
			    xxx(U802WRMAGIC);
			    xxx(U802ROMAGIC);
			    xxx(U802TOCMAGIC);
#endif
#ifdef MIPSEBMAGIC
			    /* DEC Mips */
			    xxx(MIPSEBMAGIC);
			    xxx(MIPSELMAGIC);
			    xxx(SMIPSEBMAGIC);
			    xxx(SMIPSELMAGIC);
			    xxx(MIPSEBUMAGIC);
			    xxx(MIPSELUMAGIC);
#endif

#undef xxx
#endif /* F_EXEC */
			    break;	/* Not an executable */
			}
		}
	    break;
	case R_WHEN:
	    if ((rtype == R_WHEN) &&
		hastype(rules[i].u.u_when.file_types,ftype) &&
		glob_match(rules[i].u.u_when.globexp,pathname))
		return i;
	    break;
	case R_IF:
	    if (rules[i].u.u_if.inactive) {
		i = rules[i].u.u_if.first; /* will be decremented in a moment... */
		continue;
	    }
	    break;
	case R_IF_ELSE:
	    if (rules[i].u.u_if.inactive) {
		/* do nothing (we do the else clause and then hit the skip) */
	    } else {
		i = rules[i].u.u_if.first; /* will be decremented in a moment... */
		continue;
	    }
	    break;
	case R_SKIP:
	    i = rules[i].u.u_skip.first;
	    continue;
	case R_FRAMEMARKER:
	    break;
	default:
	    panic("findrule: unknown rule type");
	}
    }
  
    /* we did not find a rule */
    switch(rtype) {
    case R_CHASE:
    case R_WHEN:
	/* the code that looks for these types of rules can deal with
	 * 	    this error return value */
	return -1;
    default:
	panic("findrule: did not find rule");
    }
}


void getrules(src,dst)
     char *src;
     char *dst;
{
  char *srcrules,*dstrules;
  static firstread = 0;
  extern int aflag;
  extern char *afile;

  newrule();
  lstrule.type = R_FRAMEMARKER;

  vtable_push();
  
  srcrules = (char *) alloca(strlen(src) + strlen(src_rule_file) + 2);
  (void) strcpy(srcrules,src);
  (void) strcat(srcrules,"/");
  (void) strcat(srcrules,src_rule_file);
  
  dstrules = (char *) alloca(strlen(dst) + strlen(dst_rule_file) + 2);
  (void) strcpy(dstrules,dst);
  (void) strcat(dstrules,"/");
  (void) strcat(dstrules,dst_rule_file);

  if (nosrcrules == FALSE)
    if (readrules(srcrules,src,dst) != 0)
      { fprintf(stderr,"Error while reading %s\n", srcrules);
	panic("Syntax error in conf file.");
      }

  if (nodstrules == FALSE)
    if (readrules(dstrules,src,dst) != 0)
      { fprintf(stderr,"Error while reading %s\n",dstrules);
        panic("Syntax error in conf file.");
      }

  if (!(firstread++) && aflag && readrules(afile,src,dst) != 0)
    { fprintf(stderr,"Error while reading %s\n",afile);
      panic("Syntax error in conf file.");
    }
  
  compute_ifs();  /* see below */
  freea(dstrules);
  freea(srcrules);
}


void poprules()
{
  vtable_pop();

  for(;;lastrule--) {
    switch(lstrule.type) {
    case R_MAP:
      sfree(lstrule.u.u_map.globexp);
      svecfree(lstrule.u.u_map.dests);
      continue;
    case R_CHASE:
      sfree(lstrule.u.u_chase.globexp);
      continue;
    case R_ACTION:
      sfree(lstrule.u.u_action.globexp);
      continue;
    case R_WHEN:
      sfree(lstrule.u.u_when.globexp);
      svecfree(lstrule.u.u_when.cmds);
      continue;
    case R_IF:
    case R_IF_ELSE:
      bool_free(lstrule.u.u_if.boolexp);
      continue;
    case R_FRAMEMARKER:
      break;
    default:
       panic("poprules: unknown rule type");
    }
    lastrule--;
    break;
  }

#ifdef notdef
  /* This doesn't need to be done here because, reconcile will not call */
  /* findrule after calling this without first calling getrules. If */
  /* this ever changes, this will have to be turned on again. */
  compute_ifs();  /* defined below */
#endif notdef
}

static void compute_ifs()
{
  int i;

  rflag &= ~RFLAG_SCAN_TARGET;
  
  for(i = lastrule; i >= 0; i--) {
    switch(rules[i].type) {
    case R_IF:
    case R_IF_ELSE:
      rules[i].u.u_if.inactive = (bool) ! bool_eval(rules[i].u.u_if.boolexp);
      break;
    case R_CHASE:
    case R_MAP:
    case R_WHEN:
    case R_FRAMEMARKER:
      break;
    case R_ACTION:
      if (rules[i].u.u_action.type == ACTION_DELETE)
	  rflag |= RFLAG_SCAN_TARGET;
      break;
    default:
      panic("compute_ifs: unknown rule type");
    }
  }
}


void newrule()
{
    if (++lastrule >= MAXNRULES)
	panic("newrule: too many rules");
    if (verbosef > 2)
	printf("newrule: lastrule = %d\n",lastrule);
}
    
  
