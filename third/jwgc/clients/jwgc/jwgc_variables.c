#include "main.h"
#include <libjwgc.h>

int
jwgc_show_password_event_handler()
{
	jVars_set_error("Displaying password is not permitted.");
	return 0;
}

#define PRESENCE_AVAILABLE      "available"
#define PRESENCE_NORMAL         "normal"
#define PRESENCE_AWAY           "away"
#define PRESENCE_XA             "xa"
#define PRESENCE_DND            "dnd"
#define PRESENCE_CHAT           "chat"

int
jwgc_check_presence_event_handler(setting)
	char *setting;
{
	if (!strcasecmp(setting, PRESENCE_AVAILABLE))
		return 1;
	else if (!strcasecmp(setting, PRESENCE_NORMAL))
		return 1;
	else if (!strcasecmp(setting, PRESENCE_AWAY))
		return 1;
	else if (!strcasecmp(setting, PRESENCE_XA))
		return 1;
	else if (!strcasecmp(setting, PRESENCE_DND))
		return 1;
	else if (!strcasecmp(setting, PRESENCE_CHAT))
		return 1;
	else {
		jVars_set_error("The presence setting must be one of:\n\
PRESENCE_AVAILABLE, PRESENCE_NORMAL, PRESENCE_AWAY, PRESENCE_XA, PRESENCE_DND, PRESENCE_CHAT.");
		return 0;                             
	}
}

void
jwgc_change_presence_event_handler(oldval, newval)
	char *oldval;
	char *newval;
{
	xode out, x, y;

	dprintf(dExecution, "Old presence = %s, New = %s\n", oldval, newval);
	if (newval && (!oldval || strcmp(oldval, newval))) {
		if (jab_c) {
			dprintf(dExecution, "Setting presence to %s\n", newval);
			if (strcmp(newval, "available")) {
				out = jabutil_presnew(JABPACKET__AVAILABLE,
					NULL,
					newval,
					*(int *)jVars_get(jVarPriority));
				x = xode_insert_tag(out, "show");
				y = xode_insert_cdata(x, oldval,
						  strlen(oldval));
			}
			else {
				out = jabutil_presnew(JABPACKET__AVAILABLE,
					NULL,
					NULL,
					*(int *)jVars_get(jVarPriority));
			}
			jab_send(jab_c, out);
			xode_free(out);
		}
	}
}

void
jwgc_change_resource_event_handler(oldval, newval)
	char *oldval;
	char *newval;
{
	if (!newval) { return; }

	if (!strncmp(newval, "HOSTNAME", 8)) {
		char hostname[1024];
		if (!gethostname(hostname, sizeof(hostname))) {
			hostname[1023] = '\0'; /* paranoia */
			if (!strncmp(newval, "HOSTNAMETRIMMED", 15)) {
				char *pos;
				pos = strchr(hostname, '.');
				if (pos) {
					*pos = '\0';
				}
			}
			jVars_set(jVarResource, hostname);
		}
	}
}

void setup_jwgc_variable_handlers()
{
	jVars_set_show_handler(jVarPassword, jwgc_show_password_event_handler);
	jVars_set_change_handler(jVarPresence, jwgc_change_presence_event_handler);
	jVars_set_change_handler(jVarResource, jwgc_change_resource_event_handler);
	jVars_set_check_handler(jVarPresence, jwgc_check_presence_event_handler);
}

void
jwg_set_defaults_handler()
{
	jVars_set(jVarPresence, DEFPRESENCE);
	jVars_set(jVarPriority, DEFPRIORITY);
	jVars_set(jVarServer, DEFSERVER);
	jVars_set(jVarResource, DEFRESOURCE);
	jVars_set(jVarPort, DEFPORT);
#ifdef USE_SSL
	jVars_set(jVarUseSSL, DEFUSESSL);
#endif /* USE_SSL */
#ifdef USE_GPGME
	jVars_set(jVarUseGPG, DEFUSEGPG);
#endif /* USE_GPGME */

        setup_jwgc_variable_handlers();
}
