#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif


/* etale.c */
void initialize_ale_error_table P_((void));

/* etalw.c */
void initialize_alw_error_table P_((void));

/* setuser.c */
long ALsetUser P_((ALsession session, char *uname, ALflag_t initial_flags));

/* passwd.c */
long ALincRefCount P_((ALsession session));
void ALgetErrorContextFromFile P_((ALsession session, char *filename));
long ALaddPasswdEntry P_((ALsession session));
long ALremovePasswdEntry P_((ALsession session));

/* modify.c */
int ALopenLockFile P_((char *filename));
long ALmodifyRemoveUser P_((ALsession session, char buf[]));
long ALmodifyAppendPasswd P_((ALsession session, int fd));
long ALmodifyLinesOfFile P_((ALsession session, char *filename, char *lockfilename, long (*modify )(ALsession, char[]), long (*append )(ALsession, int )));

/* group.c */
char *ALcopySubstring P_((char *string, char buffer[], int delimiter));
long ALgetGroups P_((ALsession session));
long ALmodifyGroupAdd P_((ALsession session, char groupline[]));
long ALappendGroups P_((ALsession session, int fd));
long ALaddToGroupsFile P_((ALsession session));

/* start.c */
long ALstart P_((ALsession session, char *ttyname));
long ALend P_((ALsession session));

/* homedir.c */
int ALisRemoteDir P_((char *dir));
int ALhomedirOK P_((char *dir));
long ALgetHomedir P_((ALsession session));

/* get_phost.c */
char *krb_get_phost P_((char *alias));

/* getrealm.c */

#undef P_
