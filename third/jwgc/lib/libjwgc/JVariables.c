#include <sysdep.h>

/*
 *      Parts of this this are from MIT's Zephyr system, and the following
 *      applies to those parts:
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

/* $Id: JVariables.c,v 1.1.1.1 2006-03-10 15:32:48 ghudson Exp $ */

#include "libjwgc.h"

int jVars_fixed[jNumVars];
int jVars_types[jNumVars];
char *jVars_strings[jNumVars];
void *jVars_contents[jNumVars];
static void (*jVars_change_handler[jNumVars]) ();
static int (*jVars_check_handler[jNumVars]) ();
static int (*jVars_show_handler[jNumVars]) ();
static void (*jVars_defaults_handler) ();

void jVars_read_conf(char *file, int locksettings);
void jVars_read_defaults();
void jVars_set_error(char *errorstr);
char *jVars_get_error();

#define max(a,b) ((a > b) ? (a) : (b))
char *varfiledefaults;
char *varfilefixed;
char *varfilepersonal;
char *varerrorstr;

static int 
varline(bfr, var)
	char *bfr;
	char *var;
{
	register char *cp;


	if (!bfr[0] || bfr[0] == '#')
		return (0);

	cp = bfr;
	while (*cp && !isspace(*cp) && (*cp != '='))
		cp++;

	if (strncasecmp(bfr, var, max(strlen(var), cp - bfr)))
		return (0);

	cp = (char *) strchr(bfr, '=');
	if (!cp)
		return (0);
	cp++;
	while (*cp && isspace(*cp))
		cp++;

	return (cp - bfr);
}

int 
jVars_set_personal_var(var, value)
	char *var;
	char *value;
{
	int written;
	FILE *fpin, *fpout;
	char *varfilebackup, varbfr[512];

	written = 0;

	if (!varfilepersonal)
		return 0;

	varfilebackup = (char *)malloc(sizeof(char) *
			(strlen(varfilepersonal) + 7 + 1));
	strcpy(varfilebackup, varfilepersonal);
	strcat(varfilebackup, ".backup");

	if (!(fpout = fopen(varfilebackup, "w")))
		return 0;

	if ((fpin = fopen(varfilepersonal, "r")) != NULL) {
		while (fgets(varbfr, sizeof varbfr, fpin) != (char *) 0) {
			varbfr[511] = '\0';

			if (varline(varbfr, var)) {
				fprintf(fpout, "%s = %s\n", var, value);
				written = 1;
			}
			else
				fprintf(fpout, "%s\n", varbfr);
		}
		fclose(fpin);
	}

	if (!written)
		fprintf(fpout, "%s = %s\n", var, value);

	if (fclose(fpout) == EOF)
		return 0;

	if (rename(varfilebackup, varfilepersonal))
		return 0;

	return 1;
}

int 
jVars_unset_personal_var(var)
	char *var;
{
	FILE *fpin, *fpout;
	char *varfilebackup, varbfr[512];

	varfilebackup = (char *)malloc(sizeof(char) *
			(strlen(varfilepersonal) + 7 + 1));
	strcpy(varfilebackup, varfilepersonal);
	strcat(varfilebackup, ".backup");

	if (!(fpout = fopen(varfilebackup, "w")))
		return 0;

	if ((fpin = fopen(varfilepersonal, "r")) != NULL) {
		while (fgets(varbfr, sizeof varbfr, fpin) != (char *) 0) {
			varbfr[511] = '\0';

			if (!varline(varbfr, var))
				fprintf(fpout, "%s\n", varbfr);
		}
		fclose(fpin);
	}

	if (fclose(fpout) == EOF)
		return 0;

	if (rename(varfilebackup, varfilepersonal))
		return 0;

	return 1;
}

int 
jVars_init()
{
	jVar i;
	struct passwd *pwd;
	char *envptr, *homedir;

	dprintf(dVars, "Initializing variable manager...\n");

	varfiledefaults = NULL;
	varfilefixed = NULL;
	varfilepersonal = NULL;
	varerrorstr = NULL;

	for (i = 0; i < jNumVars; i++) {
		jVars_fixed[i] = 0;
		jVars_contents[i] = NULL;
		jVars_check_handler[i] = NULL;
		jVars_change_handler[i] = NULL;
		jVars_show_handler[i] = NULL;
	}

	if ((envptr = (char *) getenv("HOME"))) {
		homedir = strdup(envptr);
	}
	else {
		if (!(pwd = getpwuid((int) getuid()))) {
			jVars_set_error("Can't find your entry in /etc/passwd.");
			return 0;
		}
		homedir = strdup(pwd->pw_dir);
	}

	varfilepersonal = (char *)malloc(sizeof(char) *
			(strlen(homedir) + 1 + strlen(USRVARS) + 1));
	strcpy(varfilepersonal, homedir);
	strcat(varfilepersonal, "/");
	strcat(varfilepersonal, USRVARS);

	varfilefixed = (char *)malloc(strlen(DATADIR) + strlen(FIXEDVARS) + 2);
	sprintf(varfilefixed, "%s/%s", DATADIR, FIXEDVARS);

	varfiledefaults = (char *)malloc(strlen(DATADIR) + strlen(DEFVARS) + 2);
	sprintf(varfiledefaults, "%s/%s", DATADIR, DEFVARS);

	jVars_strings[jVarUsername] = "username";
	jVars_types[jVarUsername] = jTypeString;

	jVars_strings[jVarPassword] = "password";
	jVars_types[jVarPassword] = jTypeString;

	jVars_strings[jVarServer] = "server";
	jVars_types[jVarServer] = jTypeString;

	jVars_strings[jVarResource] = "resource";
	jVars_types[jVarResource] = jTypeString;

	jVars_strings[jVarPort] = "port";
	jVars_types[jVarPort] = jTypeNumber;

	jVars_strings[jVarPriority] = "priority";
	jVars_types[jVarPriority] = jTypeNumber;

#ifdef USE_SSL
	jVars_strings[jVarUseSSL] = "usessl";
	jVars_types[jVarUseSSL] = jTypeBool;
#endif /* USE_SSL */

	jVars_strings[jVarInitProgs] = "initprogs";
	jVars_types[jVarInitProgs] = jTypeString;

	jVars_strings[jVarJID] = "jid";
	jVars_types[jVarJID] = jTypeString;

	jVars_strings[jVarPresence] = "presence";
	jVars_types[jVarPresence] = jTypeString;

#ifdef USE_GPGME
	jVars_strings[jVarUseGPG] = "usegpg";
	jVars_types[jVarUseGPG] = jTypeBool;

	jVars_strings[jVarGPGPass] = "gpgpass";
	jVars_types[jVarGPGPass] = jTypeString;

	jVars_strings[jVarGPGKeyID] = "gpgkeyid";
	jVars_types[jVarGPGKeyID] = jTypeString;
#endif /* USE_GPGME */

	if (*jVars_defaults_handler) {
		jVars_defaults_handler();
	}
	jVars_read_defaults();

	return 1;
}

void
jVars_set_error(errorstr)
	char *errorstr;
{
	if (varerrorstr) { free(varerrorstr); }
	varerrorstr = strdup(errorstr);
}

char *
jVars_get_error()
{
	if (varerrorstr) {
		return varerrorstr;
	}
	else {
		return NULL;
	}
}

void
jVars_set_change_handler(jvar, handler)
	jVar jvar;
	void (*handler) ();
{
	jVars_change_handler[jvar] = handler;
}

void
jVars_set_check_handler(jvar, handler)
	jVar jvar;
	int (*handler) ();
{
	jVars_check_handler[jvar] = handler;
}

void
jVars_set_show_handler(jvar, handler)
	jVar jvar;
	int (*handler) ();
{
	jVars_show_handler[jvar] = handler;
}

void
jVars_set_defaults_handler(handler)
	void (*handler) ();
{
	jVars_defaults_handler = handler;
}

void *
jVars_get(name)
	jVar name;
{
	if (name >= jNumVars || name < 0) {
		return NULL;
	}

	if (jVars_contents[name]) {
		return jVars_contents[name];
	}
	else {
		return NULL;
	}
}

char *
jVars_show(name)
	jVar name;
{
	if (name >= jNumVars || name < 0) {
		jVars_set_error("Variable unknown.");
		return NULL;
	}

	if (*jVars_show_handler[name]) {
		if (!(jVars_show_handler[name] ())) {
			/* handler is expected to set error */
			return NULL;
		}
	}

	if (jVars_contents[name]) {
		return jVars_contents[name];
	}
	else {
		return (char *)"<not set>";
	}
}

int
jVars_set(name, setting)
	jVar name;
	void *setting;
{
	void *oldsetting;

	if (name < 0 || name >= jNumVars) {
		jVars_set_error("Variable does not exist.");
		return 0;
	}

	if (!setting) {
		jVars_set_error("No setting specified.");
		return 0;
	}

	if (*jVars_check_handler[name]) {
		if (!(jVars_check_handler[name] (setting))) {
			/* handler is expected to set error */
			return 0;
		}
	}

	if (jVars_fixed[name]) {
		jVars_set_error("The variable you've selected can not be changed.");
		return 0;
	}
	else {
		oldsetting = jVars_contents[name];
		if (jVars_types[name] == jTypeString) {
			jVars_contents[name] = strdup(setting);
		}
		else if (jVars_types[name] == jTypeNumber) {
			int *intset;
			intset = (int *)malloc(sizeof(int));
			*intset = atoi(setting);
			jVars_contents[name] = intset;
		}
		else if (jVars_types[name] == jTypeBool) {
			int *intset;
			intset = (int *)malloc(sizeof(int));
			*intset = 0;
			if (!strcmp(setting, "on")
					|| !strcmp(setting, "enable")
					|| !strcmp(setting, "enabled")
					|| !strcmp(setting, "true")
					|| !strcmp(setting, "1")) {
				*intset = 1;
			}
			jVars_contents[name] = intset;
		}

		if (*jVars_change_handler[name]) {
			jVars_change_handler[name] (oldsetting, jVars_contents[name]);
		}

		if (oldsetting != NULL) {
			free(oldsetting);
		}
	}

	return 1;
}

char *
jVars_itos(jvar)
	jVar jvar;
{
	return jVars_strings[jvar];
}

jVar 
jVars_stoi(jvar)
	char *jvar;
{
	jVar i;

	for (i = 0; i < jNumVars; i++) {
		if (!strcmp(jvar, jVars_strings[i])) {
			return i;
		}
	}

	return -1;
}

void
jVars_read_conf(file, locksettings)
	char *file;
	int locksettings;
{
	FILE *fp;
	char *cp, *nameptr, *valptr;
	char varbfr[512];
	jVar curvar;

	dprintf(dVars, "Reading configuration file: %s\n", file);
	fp = fopen(file, "r");
	if (fp) {
		dprintf(dVars, "Successfully opened.\n");
		while (fgets(varbfr, sizeof(varbfr), fp) != (char *) 0) {
			varbfr[511] = '\0';
			cp = varbfr;

			while (*cp && isspace(*cp))
				cp++;

			if (!*cp || *cp == '=' || *cp == '#')
				continue;

			nameptr = cp;
			while (*cp && !isspace(*cp) && (*cp != '='))
				cp++;

			*cp = '\0';
			dprintf(dVars, "Found variable %s.\n", nameptr);
			curvar = jVars_stoi(nameptr);
			if (curvar < 0)
				continue;
			cp++;
			
			while (*cp && (isspace(*cp) || *cp == '='))
				cp++;

			if (!*cp)
				continue;

			valptr = cp;

			while (*cp && (*cp != '\n'))
				cp++;

			if (!*cp)
				continue;

			if (*cp == '\n')
				*cp = '\0';

			if (valptr) {
				jVars_set(curvar, valptr);
				if (locksettings) {
					jVars_fixed[curvar] = 1;
				}
			}
		}
		fclose(fp);
	}
}

void
jVars_read_defaults()
{
	dprintf(dVars, "Reading defaults\n");
	jVars_read_conf(varfilefixed, 1);
	jVars_read_conf(varfiledefaults, 0);
	if (varfilepersonal) {
		jVars_read_conf(varfilepersonal, 0);
	}
}
