#ifdef _VAR_H_
int nanny_exchangeVars(varlist *, varlist **)
#endif

int nanny_getNannyPid(int *);
int nanny_setupUser(char *, char **, char **);
int nanny_loginUser(char ***, char ***, char **);
int nanny_logoutUser(void);
int nanny_getTty(char *, int);
int nanny_setConsoleMode(void);
int nanny_setXConsolePref(int);
