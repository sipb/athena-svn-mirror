
#define YPPASSWDPROG ((u_long)100009)
#define YPPASSWDVERS ((u_long)1)
#define YPPASSWDPROC_UPDATE ((u_long)1)
extern int *yppasswdproc_update_1();


struct passwd {
	char *pw_name;
	char *pw_passwd;
	int pw_uid;
	int pw_gid;
	char *pw_gecos;
	char *pw_dir;
	char *pw_shell;
};
typedef struct passwd passwd;
bool_t xdr_passwd();


struct yppasswd {
	char *oldpass;
	passwd newpw;
};
typedef struct yppasswd yppasswd;
bool_t xdr_yppasswd();

