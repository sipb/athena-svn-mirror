#ifndef _VAR_H_
#define _VAR_H_

#ifndef _VARP_H_
typedef struct _varlist varlist;
#endif

extern int var_setString(varlist *, char *, char *);
extern int var_getString(varlist *, char *, char **);
extern int var_setValue(varlist *, char *, void *, int);
extern int var_getValue(varlist *, char *, void **, int *);
extern int var_init(varlist **);
extern int var_destroy(varlist *);
extern int var_listVars(varlist *, char ***);
extern int var_freeList(varlist *, char **);

#endif /* _VAR_H_ */
