#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif


/* etale.c */
void initialize_ale_error_table P_((void));

/* etalw.c */
void initialize_alw_error_table P_((void));

/* init.c */
long ALinit P_((void));
long ALinitAL P_((ALsession session, ALflag_t initial_flags));
long ALinitSession P_((ALsession session));

/* setuser.c */
long ALinitUser P_((ALsession session, ALflag_t initial_flags));
long ALsetUser P_((ALsession session, char *uname));

/* passwd.c */
long ALincRefCount P_((ALsession session));
int ALlockPasswdFile P_((ALsession session));
int ALunlockPasswdFile P_((ALsession session));
void ALgetErrorContextFromFile P_((ALsession session, char *filename));
long ALaddPasswdEntry P_((ALsession session));
long ALremovePasswdEntry P_((ALsession session));

/* modify.c */
char *ALlockFile P_((int lockfile));
int ALopenLockFile P_((ALsession session, int lockfile));
int ALcloseLockFile P_((ALsession session, int lockfile));
long ALmodifyRemoveUser P_((ALsession session, char buf[]));
long ALmodifyAppendPasswd P_((ALsession session, int fd));
long ALmodifyLinesOfFile P_((ALsession session, char *filename, int lockfile, long (*modify )(ALsession, char[]), long (*append )(ALsession, int )));

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

/* utmp.c */
long ALinitUtmp P_((ALsession session));
long ALsetUtmpInfo P_((ALsession session, ALflag_t flags, ALut *ut));
long ALsetTtyFd P_((ALsession sess, int fd));
long ALputUtmp P_((ALsession sess));

#undef P_
