#ifndef INTERNAL__H
#define INTERNAL__H

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#define PATH_SESSIONS		AL_PATH_SESSIONS
#define PATH_TMPDIRS		"/var/athena/tmphomedir"
#define PATH_TMPPROTO		"/usr/athena/lib/prototype_tmpuser"
#define PATH_ATTACH		"/bin/athena/attach"
#define PATH_DETACH		"/bin/athena/detach"
#define PATH_NOLOGIN		"/etc/nologin"
#define PATH_NOREMOTE		"/etc/noremote"
#define PATH_NOCREATE		"/etc/nocreate"
#define PATH_NOATTACH		"/etc/noattach"
#define PATH_NOCRACK		"/etc/nocrack"

#define PATH_GROUP		"/etc/group"
#define PATH_GROUP_TMP		"/etc/gtmp"
#define PATH_GROUP_LOCAL	"/etc/group.local"
#define PATH_GROUP_LOCK		"/var/athena/group.lock"
#ifdef HAVE_MASTER_PASSWD
#define PATH_PASSWD		"/etc/master.passwd"
#else
#define PATH_PASSWD		"/etc/passwd"
#endif
#define PATH_PASSWD_TMP		"/etc/ptmp"
#ifdef HAVE_SHADOW
#define PATH_SHADOW		"/etc/shadow"
#define PATH_SHADOW_TMP		"/etc/stmp"
#endif

struct passwd;

struct al_record {
  FILE *fp;
  sigset_t mask;
  int exists;
  int passwd_added;
  int attached;
  char *old_homedir;
  gid_t *groups;
  int ngroups;
  pid_t *pids;
  int npids;
};

/* session.c */
int al__get_session_record(const char *username, struct al_record *record);
int al__put_session_record(struct al_record *record);

/* passwd.c */
int al__add_to_passwd(const char *username, struct al_record *record,
		      const char *cryptpw);
int al__remove_from_passwd(const char *username, struct al_record *record);
int al__change_passwd_homedir(const char *username, const char *homedir);
int al__update_cryptpw(const char *username, struct al_record *record,
		       const char *cryptpw);

/* group.c */
int al__add_to_group(const char *username, struct al_record *record);
int al__remove_from_group(const char *username, struct al_record *record);

/* homedir.c */
int al__setup_homedir(const char *username, struct al_record *record,
		      int havecred, int tmphomedir);
int al__revert_homedir(const char *username, struct al_record *record);

/* util.c */
struct passwd *al__getpwnam(const char *username);
struct passwd *al__getpwuid(uid_t uid);
void al__free_passwd(struct passwd *pwd);
int al__read_line(FILE *fp, char **buf, int *bufsize);

#endif
