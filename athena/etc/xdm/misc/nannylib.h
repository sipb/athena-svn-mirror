#ifdef _VAR_H_
int nanny_exchangeVars(varlist *, varlist **)
#endif

int nanny_setupUser(char *, int, char **, char **);
int nanny_loginUser(char ***, char ***, char **);
int nanny_logoutUser(void);
int nanny_getTty(char *, int);
int nanny_setConsoleMode(void);
