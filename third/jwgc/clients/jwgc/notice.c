#include <sys/types.h>
#include <time.h>
#include "main.h"

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

/****************************************************************************/
/* */
/* Module containing code to extract a notice's fields:             */
/* */
/****************************************************************************/

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "error.h"
#include "variables.h"
#include "notice.h"

/*
 *    int count_nulls(char *data, int length)
 *        Requires: length>=0
 *        Effects: Returns the # of nulls in data[0]..data[length-1]
 */

int 
count_nulls(data, length)
	char *data;
	int length;
{
	int count = 0;

	for (; length; data++, length--)
		if (!*data)
			count++;

	return (count);
}

/*
 *    string get_next_field(char **data_p, int *length_p)
 *        Requires: *length_p >= 0
 *        Modifies: *data_p, *length_p
 *        Effects: Treats (*data_p)[0], (*data_p)[1], ... (*data_p)[length-1]
 *                 as a series of null-seperated fields.  This function
 *                 returns a copy of the first field on the heap.  This
 *                 string must eventually be freed.  Also, *data_p is
 *                 advanced and *length_p decreased so that another
 *                 call to this procedure with the same arguments will
 *                 return the second field.  The next call will return
 *                 the third field, etc.  "" is returned if 0 fields
 *                 remain.  (this is the case when *length_p == 0)
 */

string 
get_next_field(data_p, length_p)
	char **data_p;
	int *length_p;
{
	char *data = *data_p;
	int length = *length_p;
	char *ptr;

	for (ptr = data; length; ptr++, length--)
		if (!*ptr) {
			*data_p = ptr + 1;
			*length_p = length - 1;
			return (string_Copy(data));
		}

	length = *length_p;
	*data_p = ptr;
	*length_p = 0;
	return (string_CreateFromData(data, length));
}

/*
 *    string get_field(char *data, int length, int num)
 *        Requires: length>=0, num>0
 *        Effects: Treats data[0]..data[length-1] as a series of
 *                 null-seperated fields.  This function returns a copy of
 *                 the num'th field (numbered from 1 in this case) on the
 *                 heap.  This string must eventually be freed.  If there
 *                 is no num'th field (because num<1 or num># of fields),
 *                 "" is returned.
 */

string 
get_field(data, length, num)
	char *data;
	int length;
	int num;
{
	/*
         * While num>1 and there are fields left, skip a field & decrement num:
         */
	while (length && num > 1) {
		if (!*data)
			num--;
		length--;
		data++;
	}

	/*
         * If any more fields left, the first field is the one we want.
         * Otherwise, there is no such field as num -- return "".
         */
	if (length)
		return (get_next_field(&data, &length));
	else
		return (string_Copy(""));
}

/*
 *    string convert_nulls_to_newlines(data, length)
 *       Requires: length>=0, malloc never returns NULL
 *       Effects: Takes data[0]..data[length-1], converts all nulls to
 *                newlines ('\n') and returns the result as a null-terminated
 *                string on the heap.  The returned string must eventually
 *                be freed.
 */

string 
convert_nulls_to_newlines(data, length)
	char *data;
	int length;
{
	char *result, *ptr;
	char c;

	result = (char *) malloc(length + 1);
	result[length] = '\0';

	for (ptr = result; length; data++, ptr++, length--)
		*ptr = (c = *data) ? c : '\n';

	return (result);
}

char *get_error_string(x)
	xode x;
{
	xode y;
	char *temp;

	if ((y = xode_get_tag(x, "text")) && (temp = xode_get_data(y))) {
		return temp;
	} else if (xode_get_tag(x, "bad-request")) {
		return "Bad request";
	} else if (xode_get_tag(x, "conflict")) {
		return "Conflict";
	} else if (xode_get_tag(x, "feature-not-implemented")) {
		return "Feature not implemented";
	} else if (xode_get_tag(x, "forbidden")) {
		return "Forbidden";
	} else if ((y = xode_get_tag(x, "gone"))) {
		temp = xode_get_data(y);
		if (temp) {
			return xode_spool_str(x->p, "Recipient gone, use ",
					      temp, x->p);
		} else {
			return "Recipient gone";
		}
	} else if (xode_get_tag(x, "internal-server-error")) {
		return "Internal server error";
	} else if (xode_get_tag(x, "item-not-found")) {
		return "Item not found";
	} else if (xode_get_tag(x, "jid-malformed")) {
		return "Malformed JID";
	} else if (xode_get_tag(x, "not-acceptable")) {
		return "Not acceptable";
	} else if (xode_get_tag(x, "not-allowed")) {
		return "Not allowed";
	} else if (xode_get_tag(x, "not-authorized")) {
		return "Not authorized";
	} else if (xode_get_tag(x, "payment-required")) {
		return "Payment required";
	} else if (xode_get_tag(x, "recipient-unavailable")) {
		return "Recipient temporarily unavailable";
	} else if ((y = xode_get_tag(x, "redirect"))) {
		temp = xode_get_data(y);
		if (temp) {
			return xode_spool_str(x->p, "Redirect to: ", temp,
					      x->p);
		} else {
			return "Redirect";
		}
	} else if (y = xode_get_tag(x, "registration-required")) {
		return "Registration required";
	} else if (y = xode_get_tag(x, "remote-server-not-found")) {
		return "Remote server not found";
	} else if (y = xode_get_tag(x, "remote-server-timeout")) {
		return "Remote server timed out";
	} else if (y = xode_get_tag(x, "resource-constraint")) {
		return "Resource constraint";
	} else if (y = xode_get_tag(x, "service-unavailable")) {
		return "Service unavailable";
	} else if (y = xode_get_tag(x, "subscription-required")) {
		return "Subscription required";
	} else if (y = xode_get_tag(x, "undefined-condition")) {
		return "Undefined condition";
	} else if (y = xode_get_tag(x, "unexpected-request")) {
		return "Unexpected request";
	} else {
		return "Unknown error condition";
	}
}

/*
 *    char *decode_notice(JNotice_t *notice)
 *        Modifies: various description language variables
 *        Effects:
 */

char *
decode_notice(notice)
	struct jabpacket_struct *notice;
{
	char *temp, *c, *d, *from, *ns, *jid;
	xode x, y;
	char timestr[26];
	time_t curtime;

	var_set_variable_then_free_value("type",
		jab_type_to_ascii(notice->type));
	var_set_variable_then_free_value("subtype",
		jab_subtype_to_ascii(notice->subtype));

	dprintf(dXML, "Decoding Notice [%d,%d]:\n%s\n",
		notice->type,
		notice->subtype,
		xode_to_prettystr(notice->x));

	temp = xode_get_attrib(notice->x, "type");
	if (temp) {
		var_set_variable("msgtype", temp);
	}
	else {
		var_set_variable("msgtype", "");
	}

	var_set_variable("from", "");
	var_set_variable("sender", "");
	var_set_variable("server", "");
	var_set_variable("resource", "");
	var_set_variable("nickname", "");
	jid = notice->from->full;
	if (jid) {
		from = strdup(jid);
	}
	else {
		from = strdup(xode_get_attrib(notice->x, "from"));
	}
	if (from) {
		var_set_variable("from", from);
		temp = (char *) find_nickname_from_jid(from);
		if (temp) {
			var_set_variable_then_free_value("nickname", temp);
		}

		c = from;
		d = strchr(c, '@');
		if (d) {
			*d = '\0';
			temp = strdup(c);
			var_set_variable_then_free_value("sender", temp);
			d++;
			c = d;
			d = strchr(c, '/');
			if (d) {
				*d = '\0';
				temp = strdup(c);
				var_set_variable_then_free_value("server",
						temp);
				d++;
				c = d;
				temp = strdup(c);
				var_set_variable_then_free_value("resource",
						temp);
			}
			else {
				var_set_variable("server", c);
				var_set_variable("resource", "");
			}
		}
		else {
			var_set_variable("sender", from);
		}
	}
	else {
		var_set_variable("from", "[unknown]");
	}
	free(from);

	x = xode_get_tag(notice->x, "subject");
	if (x) {
		temp = xode_get_data(x);
		var_set_variable("subject", temp);
	}
	else {
		var_set_variable("subject", "");
	}

	var_set_variable("event", "");
	x = xode_get_tag(notice->x, "x");
	if (x) {
		ns = xode_get_attrib(x, "xmlns");
		if (ns && !strcmp(ns, NS_EVENT)) {
			y = xode_get_tag(x, "composing");
			if (y) {
				var_set_variable("event", "composing");
			}
		}
	}

	x = xode_get_tag(notice->x, "show");
	if (x) {
		char *convtemp;
		temp = xode_get_data(x);
		if (!temp || !unicode_to_str(temp, &convtemp)) {
			convtemp = strdup("");
		}
		var_set_variable_then_free_value("show", convtemp);
	}
	else {
		var_set_variable("show", "");
	}

	x = xode_get_tag(notice->x, "status");
	if (x) {
		char *convtemp;
		temp = xode_get_data(x);
		if (temp) {
			if (!unicode_to_str(temp, &convtemp)) {
				convtemp = strdup("");
			}
			var_set_variable_then_free_value("status", convtemp);
		}
		else {
			var_set_variable("status", "");
		}
	}
	else {
		var_set_variable("status", "");
	}

	x = xode_get_tag(notice->x, "error");
	if (x) {
		var_set_variable("error", get_error_string(x));
	}
	else {
		var_set_variable("error", "");
	}

	curtime = time(NULL);
	strftime(timestr, 25, "%T", localtime(&curtime));
	var_set_variable("time", timestr);

	strftime(timestr, 25, "%r", localtime(&curtime));
	var_set_variable("time12", timestr);

	strftime(timestr, 25, "%x", localtime(&curtime));
	var_set_variable("date", timestr);

	strftime(timestr, 25, "%a %b %e %Y", localtime(&curtime));
	var_set_variable("longdate", timestr);

	return (0);
}
