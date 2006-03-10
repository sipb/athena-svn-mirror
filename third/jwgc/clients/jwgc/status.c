#include "main.h"

void 
show_status()
{
	char retstr[255];
	char tmpstr[50];
	time_t curtime;
	xode x, y;

	x = xode_new("status");
	y = xode_insert_tag(x, "user");
	xode_insert_cdata(y, jab_c->user->user, strlen(jab_c->user->user) + 1);
	y = xode_insert_tag(x, "server");
	xode_insert_cdata(y, jab_c->user->server, strlen(jab_c->user->server) + 1);
	y = xode_insert_tag(x, "resource");
	xode_insert_cdata(y, jab_c->user->resource, strlen(jab_c->user->resource) + 1);
	y = xode_insert_tag(x, "version");
	xode_insert_cdata(y, VERSION, strlen(VERSION) + 1);
	y = xode_insert_tag(x, "machinetype");
	xode_insert_cdata(y, MACHINE_TYPE, strlen(MACHINE_TYPE) + 1);

	curtime = time(NULL);
	strftime(tmpstr, 50, "%A  %B %e, %Y  %I:%M %p", localtime(&curtime));
	y = xode_insert_tag(x, "localtime");
	xode_insert_cdata(y, tmpstr, strlen(tmpstr) + 1);

	strftime(tmpstr, 50, "%A  %B %e, %Y  %I:%M %p", localtime(&jab_connect_time));
	y = xode_insert_tag(x, "connecttime");
	xode_insert_cdata(y, tmpstr, strlen(tmpstr) + 1);

	y = xode_insert_tag(x, "connectstate");
	xode_insert_cdata(y, jab_contype_to_ascii(jab_c->state),
			  strlen(jab_contype_to_ascii(jab_c->state)) + 1);

	list_agents(x);

	y = xode_insert_tag(x, "bugreport");
	xode_insert_cdata(y, PACKAGE_BUGREPORT, strlen(PACKAGE_BUGREPORT) + 1);

	jwg_servsend(jwg_c, x);
	xode_free(x);
}
