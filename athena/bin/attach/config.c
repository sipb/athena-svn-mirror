/*
 *	$Id: config.c,v 1.11 1999-01-22 23:08:31 ghudson Exp $
 */

#ifndef lint
static char *rcsid_config_c = "$Id: config.c,v 1.11 1999-01-22 23:08:31 ghudson Exp $";
#endif

#include "attach.h"
#include "pwd.h"
#include <string.h>
#include <regex.h>


#define TOKSEP	" \t\r\n"

/*
 * Declare the filesystem tables for nosetuid and allowable mountings
 */
struct filesys_tab {
	struct _tab {
		char	*regexp;
		int	fs_type;
		int	explicit; /* 0 = don't care, 1 = explicit */
				  /* only, -1 = no explicit only */
		char	*tab_data;
	} tab[MAXFILTAB];
	int	idx;		/* Index to next free entry */
} nosuidtab, allowtab, goodmntpt, fsdb, optionsdb;

static char	data_t[] = "#!TRUE";

struct uid_tab {
	int	tab[MAXTRUIDTAB];
	int	idx;
} trusted_uid;			/* trusted uid table */

/*
 * Define the keyword table
 */
static void parse_boolean(), parse_string(), parse_uidlist(), parse_fslist();
static void parse_fslista();
static int get_fstype();

struct key_def {
	char	*key;
	void	(*proc)();
	caddr_t	variable;
	int	arg;
} keyword_list[] = {
	{"verbose",	parse_boolean,	(caddr_t) &verbose,	1 },
	{"debug",	parse_boolean,	(caddr_t) &debug_flag,	1 },
	{"ownercheck",	parse_boolean,	(caddr_t) &owner_check,	1 },
	{"ownerlist",	parse_boolean,	(caddr_t) &owner_list,	1 },
	{"keep-mount",	parse_boolean,	(caddr_t) &keep_mount,	1 },
	{"explicit-mntpt",	parse_boolean,	(caddr_t) &exp_mntpt,	1 },
	{"explicit",	parse_boolean,	(caddr_t) &exp_allow,	1 },
		
	{"nfs-root-hack", parse_boolean, (caddr_t) &nfs_root_hack, 1 },
	{"nfs-mount-dir", parse_string, (caddr_t) &nfs_mount_dir, 0 },

	{"attachtab",	parse_string,	(caddr_t) &attachtab_fn, 0 },
	{"mtab",	parse_string,	(caddr_t) &mtab_fn, 0 },
#ifdef AFS
        {"aklog",       parse_string,   (caddr_t) &aklog_fn, 0 },
	{"afs-mount-dir", parse_string,	(caddr_t) &afs_mount_dir, 0 },
#endif
	{"fsck",	parse_string,	(caddr_t) &fsck_fn, 0 },

	{"trusted",	parse_uidlist,	(caddr_t) &trusted_uid,	0 },
	{"nosetuid",	parse_fslist,	(caddr_t) &nosuidtab, 	1 },
	{"nosuid",	parse_fslist,	(caddr_t) &nosuidtab,	1 },
	{"setuid",	parse_fslist,	(caddr_t) &nosuidtab,	0 },
	{"suid",	parse_fslist,	(caddr_t) &nosuidtab,	0 },
	{"allow",	parse_fslist,	(caddr_t) &allowtab,	1 },
	{"noallow",	parse_fslist,	(caddr_t) &allowtab,	0 },
	{"mountpoint",	parse_fslist,	(caddr_t) &goodmntpt,	1 },
	{"nomountpoint",parse_fslist,	(caddr_t) &goodmntpt,	0 },
	
	{"filesystem",	parse_fslista,	(caddr_t) &fsdb,	0 },
	{"options",	parse_fslista,	(caddr_t) &optionsdb, 0},
	{NULL, 		NULL,		NULL,			0 }
};

static void config_abort()
{
	fprintf(stderr, " Aborting...\n");
	exit(ERR_BADCONF);
}

read_config_file(config_file_name)
	const char *config_file_name;
{
	register FILE	*f;
	static char	buff[256];
	char		*cp;
	register char	*keyword;
	char		*argument;
	struct key_def	*kp;

	allowtab.tab[0].regexp = NULL;
	nosuidtab.tab[0].regexp = NULL;
	goodmntpt.tab[0].regexp = NULL;
 	fsdb.tab[0].regexp = NULL;
	trusted_uid.tab[0] = -1;
	if (debug_flag)
		printf("Reading configuration file: %s\n", config_file_name);
	if (!(f = fopen(config_file_name, "r"))) {
		fprintf(stderr, "Can't read configuration file %s.\n",
			config_file_name);
		config_abort();
	}
	while (fgets(buff, sizeof(buff), f)) {
		cp = buff+strlen(buff)-1;
		while (cp >= buff && *cp < 032)
			*cp-- = '\0';
		cp = buff;
		while (*cp && isspace(*cp)) cp++;
		if (!*cp || *cp == '#')
			continue;
		keyword = strtok(cp, TOKSEP);
		argument = strtok(NULL, TOKSEP);
		for (kp=keyword_list; kp->key; kp++) {
			if (!strcmp(kp->key, keyword)) {
				(kp->proc)(keyword,argument,buff,
					   kp->variable, kp->arg);
				break;
			}
		}
		if (!kp->key) {
			if (debug_flag)
		fprintf(stderr, "%s: bad keyword in config file\n",
					keyword);
		}
	}
	fclose(f);
	return(SUCCESS);
}

static void parse_boolean(keyword, argument, buff, variable, arg)
	char	*keyword, *argument, *buff;
	caddr_t	variable;
	int	arg;
{
	if (argument && !strcmp(argument, "on"))
		* (int *) variable = 1;
	else if (argument && !strcmp(argument, "off"))
		* (int *) variable = 0;
	else {
		if (arg == -1)
			if (argument)
			  fprintf(stderr,
				  "%s: Argument to %s must be on or off!\n",
				  argument, keyword);
			else
			  fprintf(stderr, "%s: Argument required!\n", keyword);
		else
			* (int *) variable = arg;	/* Default */
	}
}

static void parse_string(keyword, argument, buff, variable, arg)
	char	*keyword, *argument, *buff;
	caddr_t	variable;
	int	arg;
{
	if (!argument || !*argument) {
		fprintf(stderr,
			"%s: missing argument in config file!\n",
			keyword);
		config_abort();
	}
	if (* (char **) variable)
		free(* (char **) variable);
	* (char **) variable = strdup(argument);
}

static void parse_fslist(keyword, argument, buff, variable, set_sw)
	char	*keyword, *argument, *buff;
	caddr_t	variable;
	int	set_sw;
{
	register struct filesys_tab	*fslist;
	
	fslist = (struct filesys_tab *) variable;
	while (argument && *argument) {
		if (fslist->idx >= MAXFILTAB) {
			fprintf(stderr, "Exceeded filesystem table for %s!\n",
				keyword);
			config_abort();
		}
		fslist->tab[fslist->idx].fs_type = get_fstype(&argument,
				      &fslist->tab[fslist->idx].explicit);
		fslist->tab[fslist->idx].regexp = strdup(argument);
		fslist->tab[fslist->idx].tab_data = set_sw ? data_t : NULL;
		fslist->idx++;
		argument = strtok(NULL, TOKSEP);
	}
	fslist->tab[fslist->idx].regexp = NULL;
	fslist->tab[fslist->idx].tab_data = NULL;
}

static void parse_fslista(keyword, argument, buff, variable, dummy)
	char	*keyword, *argument, *buff;
	caddr_t	variable;
	int	dummy;
{
	register struct filesys_tab	*fslist;
	char	*cp;
	
	fslist = (struct filesys_tab *) variable;
	if (fslist->idx >= MAXFILTAB) {
		fprintf(stderr, "Exceeded filesystem table for %s!\n",
			keyword);
		config_abort();
	}
	fslist->tab[fslist->idx].fs_type = get_fstype(&argument,
				      &fslist->tab[fslist->idx].explicit);
	fslist->tab[fslist->idx].regexp = strdup(argument);
	cp = strtok(NULL, "");
	while (cp && *cp)
		if (isspace(*cp))
			cp++;
		else
			break;
		
	if (cp && *cp) {
		fslist->tab[fslist->idx].tab_data = strdup(cp);
	} else {
		fprintf(stderr,
			"%s: missing filesystem definition in config file\n");
		config_abort();
	}
	fslist->idx++;
	
	fslist->tab[fslist->idx].regexp = NULL;
	fslist->tab[fslist->idx].tab_data = NULL;
}

/*
 * Parse a filesystem type specification
 *
 * Format:   {nfs,afs}:tytso
 * 	     {^ufs}:.*
 * 
 */
static int get_fstype(cpp, explicit)
	char	**cpp;
	int	*explicit;
{
	register char	*cp, *ptrstart;
	register int	parsing, type;
	int	negate = 0;
	struct _fstypes	*fs;
	int	exp = 0;

	if (explicit)
		*explicit = 0;
	cp = *cpp;
	if (!cp || *cp++ != '{') 
		return(ALL_TYPES);
	if (*cp == '+') {
		exp = 1;
		cp++;
	} else if (*cp == '-') {
		exp = -1;
		cp++;
	}
	if (explicit)
		*explicit = exp;
	if (!strchr(cp, '}')) {
		fprintf(stderr,"Error in filesystem specification:\n{%s\n",
			cp);
		config_abort();
	}
	if (*cp == '^') {
		negate++;
		cp++;
	}
	parsing = 1;
	type = 0;
	while (parsing) {
		ptrstart = cp;
		if (!(cp = strchr(ptrstart,','))) {
			cp = strchr(ptrstart,'}');
			parsing = 0;
		}
		*cp++ = '\0';
		fs = get_fs(ptrstart);
		if (!fs) {
			fprintf(stderr,
				"%s: Illegal filesystem type in config file\n",
				ptrstart);
			config_abort();
		}
		type += fs->type;
	}
	if (negate)
		type = ~type & ALL_TYPES;
	if (*cp == ':')
		cp++;
	*cpp = cp;
	return(type);
}

static void parse_uidlist(keyword, argument, buff, variable, arg)
	char	*keyword, *argument, *buff;
	caddr_t	variable;
	int	arg;
{
	register struct uid_tab		*ul;
	int	uid;
	struct passwd	*pw;
	
	ul = (struct uid_tab *) variable;
	while (argument && *argument) {
		if (ul->idx >= MAXTRUIDTAB) {
			fprintf(stderr, "Exceeded user table for %s!\n",
				keyword);
			config_abort();
		}
		if (isnumber(argument))
			uid = atoi(argument);
		else {
			if ((pw = getpwnam(argument)))
				uid = pw->pw_uid;
			else {
				if (debug_flag) 
					fprintf(stderr,
					"Unknown user %s in config file\n",
						argument);
				argument = strtok(NULL, TOKSEP);
				continue;
			}
		}
		ul->tab[ul->idx] = uid;
		ul->idx++;
		argument = strtok(NULL, TOKSEP);
	}
	ul->tab[ul->idx] = -1;
}

isnumber(s)
	register char	*s;
{
	register char	c;
	
	while (c = *s++)
		if (!isdigit(c))
			return(0);
	return(1);
}

int nosetuid_filsys(name, type)
	register char	*name;
	int	type;
{
	register struct	_tab	*fp;
	char	buf[BUFSIZ];
	regex_t	reg;
	int	status;

	for (fp = nosuidtab.tab; fp->regexp; fp++) {
		if (fp->explicit && ((explicit && fp->explicit == -1) ||
				     (!explicit && fp->explicit == 1)))
			continue;
		if (!(fp->fs_type & type))
			continue;

		status = regcomp(&reg, fp->regexp, REG_NOSUB);
		if (status != 0) {
			regerror(status, &reg, buf, sizeof(buf));
			fprintf(stderr, "Nosetuid config: %s: %s\n",
				buf, fp->regexp);
		}
		status = regexec(&reg, name, 0, NULL, 0);
		regfree(&reg);
		if (status == 0)
			return(fp->tab_data ? 1 : 0);
	}

	return(default_suid);
}

int allow_filsys(name, type)
	register char	*name;
	int	type;
{
	register struct	_tab	*fp;
	char	buf[BUFSIZ];
	regex_t	reg;
	int	status;


	for (fp = allowtab.tab; fp->regexp; fp++) {
		if (fp->explicit && ((explicit && fp->explicit == -1) ||
				     (!explicit && fp->explicit == 1)))
			continue;
		if (!(fp->fs_type & type))
			continue;

		status = regcomp(&reg, fp->regexp, REG_NOSUB);
		if (status != 0) {
			regerror(status, &reg, buf, sizeof(buf));
			fprintf(stderr, "Allow config: %s: %s\n",
				buf, fp->regexp);
		}
		status = regexec(&reg, name, 0, NULL, 0);
		regfree(&reg);
		if (status == 0)
			return(fp->tab_data ? 1 : 0);

	}
	return(1);		/* Default to allow filsystem to be mounted */
}

int check_mountpt(name, type)
	register char	*name;
	int	type;
{
	register struct	_tab	*fp;
	char	buf[BUFSIZ];
	regex_t	reg;
	int	status;


	for (fp = goodmntpt.tab; fp->regexp; fp++) {
		if (fp->explicit && ((explicit && fp->explicit == -1) ||
				     (!explicit && fp->explicit == 1)))
			continue;
		if (!(fp->fs_type & type))
			continue;


		status = regcomp(&reg, fp->regexp, REG_NOSUB);
		if (status != 0) {
			regerror(status, &reg, buf, sizeof(buf));
			fprintf(stderr, "Mountpoint config: %s: %s\n",
				buf, fp->regexp);
		}
		status = regexec(&reg, name, 0, NULL, 0);
		regfree(&reg);
		if (status == 0)
			return(fp->tab_data ? 1 : 0);

	}
	return(1);		/* Default to allow filsystem mounted */
				/* anywhere */
}

/*
 * Return true if uid is of a trusted user.
 */
int trusted_user(uid)
	int	uid;
{
	int	*ip;

	ip = trusted_uid.tab;
	while (*ip >= 0) {
		if (uid == *ip)
			return(1);
		ip++;
	}
	return(uid == 0);	/* Hard code root being trusted */
}	

/*
 * Look up a filesystem name and return a ``hesiod'' entry
 */
char **conf_filsys_resolve(name)
	char	*name;
{
	register struct	_tab	*fp;
	char	**hp;
	static char	*hesptr[32];      /* Limit 32 ``hesiod'' entries */
	
	hp = hesptr;
	for (fp = fsdb.tab; fp->regexp; fp++) {
		if (!strcasecmp(fp->regexp, name))
			*hp++ = strdup(fp->tab_data);
	}
	*hp = NULL;
	return(hesptr);
}

/*
 * Return the options for a particular filesystem.
 */
char *filsys_options(name, type)
	char	*name;
	int	type;
{
	register struct	_tab	*fp;
	char	buf[BUFSIZ];
	regex_t	reg;
	int	status;


	for (fp = optionsdb.tab; fp->regexp; fp++) {
		if (fp->explicit && ((explicit && fp->explicit == -1) ||
				     (!explicit && fp->explicit == 1)))
			continue;
		if (!(fp->fs_type & type))
			continue;


		status = regcomp(&reg, fp->regexp, REG_NOSUB);
		if (status != 0) {
			regerror(status, &reg, buf, sizeof(buf));
			fprintf(stderr, "Mountpoint config: %s: %s\n",
				buf, fp->regexp);
		}
		status = regexec(&reg, name, 0, NULL, 0);
		regfree(&reg);
		if (status == 0)
			return(fp->tab_data);

	}
	return(NULL);
}
