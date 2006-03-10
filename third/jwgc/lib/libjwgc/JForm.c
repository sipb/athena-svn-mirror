/* $Id: JForm.c,v 1.1.1.1 2006-03-10 15:32:48 ghudson Exp $ */

#include <sysdep.h>
#include "include/libjwgc.h"

#define SEPARATOR "\
----------------------------------------------------------------------------\n\
"

xode set_filled_form(xode filledform, char *variable, char *setting)
{
	xode x, entry, value;
	char *var;

	if (!variable) {
		return filledform;
	}

	entry = NULL;
	x = xode_get_firstchild(filledform);
	while (x) {
		var = xode_get_attrib(x, "var");
		if (!var) {
			continue;
		}

		if (!strcmp(var, variable)) {
			entry = x;
			break;
		}
		
		x = xode_get_nextsibling(x);
	}

	if (entry) {
		xode_hide(entry);
	}

	if (!setting) {
		return filledform;
	}

	entry = xode_insert_tag(filledform, "field");
	xode_put_attrib(entry, "var", variable);
	value = xode_insert_tag(entry, "value");
	xode_insert_cdata(value, setting, strlen(setting));

	return filledform;
}

char *get_filled_form(xode filledform, char *variable)
{
	char *value, *var;
	xode entry, x;

	if (!variable) {
		return NULL;
	}

	entry = NULL;
	x = xode_get_firstchild(filledform);
	while (x) {
		var = xode_get_attrib(x, "var");
		if (!var) {
			continue;
		}

		if (!strcmp(var, variable)) {
			entry = x;
			break;
		}
		
		x = xode_get_nextsibling(x);
	}

	if (!entry) {
		return NULL;
	}

	value = xode_get_tagdata(entry, "value");
	return value;
}

xode set_filled_nonform(xode filledform, char *variable, char *setting)
{
	xode entry;

	if (!variable) {
		return filledform;
	}

	entry = xode_get_tag(filledform, variable);
	if (entry) {
		xode_hide(entry);
	}

	if (!setting) {
		return filledform;
	}

	entry = xode_insert_tag(filledform, variable);
	xode_insert_cdata(entry, setting, strlen(setting));

	return filledform;
}

char *get_filled_nonform(xode filledform, char *variable)
{
	char *entry;

	if (!variable) {
		return NULL;
	}

	entry = xode_get_tagdata(filledform, variable);
	return entry;
}

xode display_field_text_single(xode field, xode filledform, char id)
{
	char *value, *label, *var, *setting;

	var = xode_get_attrib(field, "var");
	if (!var) {
		return filledform;
	}

	label = xode_get_attrib(field, "label");
	value = xode_get_tagdata(field, "value");
	setting = get_filled_form(filledform, var);
	if (!setting && value) {
		setting = value;
		filledform = set_filled_form(filledform, var, setting);
	}

	printf("%c.  %-20.20s = %s\n",
			id,
			label ? label : var,
			setting ? setting : "<not set>");

	return filledform;
}

xode display_field_text_private(xode field, xode filledform, char id)
{
	char *value, *label, *var, *setting, *tmpstr;
	int i;

	var = xode_get_attrib(field, "var");
	if (!var) {
		return filledform;
	}

	label = xode_get_attrib(field, "label");
	value = xode_get_tagdata(field, "value");
	setting = get_filled_form(filledform, var);
	if (!setting && value) {
		setting = value;
		filledform = set_filled_form(filledform, var, setting);
	}

	if (setting) {
		tmpstr = (char *)malloc(sizeof(char) * (strlen(setting) + 1));
		for (i = 0; i < strlen(setting); i++) {
			tmpstr[i] = '*';
		}
		tmpstr[strlen(setting)+1] = '\0';
	}

	printf("%c.  %-20.20s = %s\n",
			id,
			label ? label : var,
			setting ? tmpstr : "<not set>");

	free(tmpstr);

	return filledform;
}

xode display_field_text_multi(xode field, xode filledform, char id)
{
	/* unsupported */
	return display_field_text_single(field, filledform, id);
}

char *find_list_label(xode field, char *target)
{
	xode option;
	char *value;

	option = xode_get_tag(field, "option");
	while (option) {
		value = xode_get_tagdata(option, "value");
		if (!value) {
			continue;
		}

		if (!strcmp(value, target)) {
			char *label;

			label = xode_get_attrib(option, "label");
			if (label) {
				return label;
			}
			else {
				return target;
			}
		}
		
		option = xode_get_nextsibling(option);
	}

	return NULL;
}

char *find_list_id(xode field, int choice)
{
	xode option;
	int id = 0;

	option = xode_get_tag(field, "option");
	while (option) {
		char *label, *value;

		value = xode_get_tagdata(option, "value");
		if (!value) {
			continue;
		}

		label = xode_get_attrib(option, "label");

		id++;
		if (id == choice) {
			return (label ? label : value);
		}
		
		option = xode_get_nextsibling(option);
	}

	return NULL;
}

int show_list_labels(xode field)
{
	xode option;
	int id = 0;

	option = xode_get_tag(field, "option");
	while (option) {
		char *label, *value;

		value = xode_get_tagdata(option, "value");
		if (!value) {
			continue;
		}

		label = xode_get_attrib(option, "label");
		printf("%d. %s\n", id++, label ? label : value);
		
		option = xode_get_nextsibling(option);
	}

	return --id;
}

xode display_field_list_single(xode field, xode filledform, char id)
{
	char *value, *label, *var, *setting;

	var = xode_get_attrib(field, "var");
	if (!var) {
		return filledform;
	}

	label = xode_get_attrib(field, "label");
	value = xode_get_tagdata(field, "value");
	setting = get_filled_form(filledform, var);
	if (!setting && value) {
		setting = value;
		filledform = set_filled_form(filledform, var, setting);
	}

	setting = find_list_label(field, setting);

	printf("%c:  %-20.20s = %s\n",
			id,
			label ? label : var,
			setting ? setting : "<not set>");

	return filledform;
}

xode display_field_list_multi(xode field, xode filledform, char id)
{
	/* unsupported */
	return display_field_text_single(field, filledform, id);
}

xode display_field_boolean(xode field, xode filledform, char id)
{
	char *value, *label, *var, *setting;

	var = xode_get_attrib(field, "var");
	if (!var) {
		return filledform;
	}

	label = xode_get_attrib(field, "label");
	value = xode_get_tagdata(field, "value");
	setting = get_filled_form(filledform, var);
	if (!setting && value) {
		setting = value;
		filledform = set_filled_form(filledform, var, setting);
	}

	printf("%c:  %-20.20s = %s\n",
			id,
			label ? label : var,
			setting ?
				((!strcmp(setting, "1")) ||
				 (!strcmp(setting, "true")) ||
				 (!strcmp(setting, "yes")) ? "true" : "false")
				: "<not set>");

	return filledform;
}

xode display_field_fixed(xode field, xode filledform)
{
	char *value, *label, *var, *setting;

	var = xode_get_attrib(field, "var");
	if (!var) {
		return filledform;
	}

	label = xode_get_attrib(field, "label");
	value = xode_get_tagdata(field, "value");
	setting = get_filled_form(filledform, var);
	if (!setting && value) {
		setting = value;
		filledform = set_filled_form(filledform, var, setting);
	}

	setting = find_list_label(field, setting);

	printf("--:  %-20.20s = %s\n",
			label ? label : var,
			setting ? setting : "<not set>");

	return filledform;
}

xode display_field_hidden(xode field, xode filledform)
{
	char *variable, *setting;

	variable = xode_get_attrib(field, "var");
	if (!variable) {
		return filledform;
	}

	setting = xode_get_tagdata(field, "value");

	filledform = set_filled_form(filledform, variable, setting);

	return filledform;
}

xode display_field_jid_single(xode field, xode filledform, char id)
{
	/* unsupported */
	return display_field_text_single(field, filledform, id);
}

xode display_field_jid_multi(xode field, xode filledform, char id)
{
	/* unsupported */
	return display_field_text_single(field, filledform, id);
}

xode display_field(xode field, xode filledform, char *id)
{
	char *type;

	type = xode_get_attrib(field, "type");
	if (!type) {
		return filledform;
	}

	if (!strcmp(type, JFORM_FIELD_TEXT_SINGLE)) {
		filledform = display_field_text_single(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_TEXT_PRIVATE)) {
		filledform = display_field_text_private(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_TEXT_MULTI)) {
		filledform = display_field_text_multi(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_LIST_SINGLE)) {
		filledform = display_field_list_single(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_LIST_MULTI)) {
		filledform = display_field_list_multi(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_BOOLEAN)) {
		filledform = display_field_boolean(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_FIXED)) {
		filledform = display_field_fixed(field, filledform);
	}
	else if (!strcmp(type, JFORM_FIELD_HIDDEN)) {
		filledform = display_field_hidden(field, filledform);
	}
	else if (!strcmp(type, JFORM_FIELD_JID_SINGLE)) {
		filledform = display_field_jid_single(field, filledform, (*id)++);
	}
	else if (!strcmp(type, JFORM_FIELD_JID_MULTI)) {
		filledform = display_field_jid_multi(field, filledform, (*id)++);
	}
	else {
		/* We don't know this type, fallback to TEXT_SINGLE */
		filledform = display_field_text_single(field, filledform, (*id)++);
	}

	return filledform;
}

xode display_nonform_field(xode field, xode filledform, char id)
{
	char *var, *setting, *current;

	var = xode_get_name(field);
	if (!var) {
		return filledform;
	}

	if (!strcmp(var, "key")) {
		setting = xode_get_data(field);
		filledform = set_filled_form(filledform, var, setting);
	}
	else {
		setting = xode_get_data(field);
		current = get_filled_nonform(filledform, var);
		if (current) {
			setting = current;
		}

		printf("%c:  %-20.20s = %s\n",
			id,
			var,
			setting ? setting : "<not set>");
	}

	return filledform;
}

xode
display_form(xode form, xode filledform)
{
	xode x;
	char *type = NULL;
	char id = 'A';

	x = xode_get_tag(form, "x");
	if (x) {
		type = xode_get_attrib(x, "type");
	}

	if (type && !strcmp(type, "form")) {
		xode y;
		char *title, *instructions;

		printf(SEPARATOR);
		title = xode_get_tagdata(x, JFORM_TITLE);
		if (title) {
			printf("%s\n", title);
		}

		instructions = xode_get_tagdata(x, JFORM_INSTRUCTIONS);
		if (instructions) {
			printf("Instructions: %s\n", instructions);
		}
		printf(SEPARATOR);

		y = xode_get_firstchild(x);
		while (y) {
			char *tag;

			tag = xode_get_name(y);
			if (!tag || strcmp(tag, JFORM_FIELD)) {
				y = xode_get_nextsibling(y);
				continue;
			}

			filledform = display_field(y, filledform, &id);

			y = xode_get_nextsibling(y);
		}
	}
	else {
		xode y;
		char *title, *instructions;

		printf(SEPARATOR);
		title = xode_get_tagdata(form, JFORM_TITLE);
		if (title) {
			printf("%s\n", title);
		}

		instructions = xode_get_tagdata(form, JFORM_INSTRUCTIONS);
		if (instructions) {
			printf("Instructions: %s\n", instructions);
		}
		printf(SEPARATOR);

		y = xode_get_firstchild(form);
		while (y) {
			char *tag;

			tag = xode_get_name(y);
			if (!tag
				 || !strcmp(tag, JFORM_INSTRUCTIONS)
				 || !strcmp(tag, JFORM_TITLE)
								) {
				y = xode_get_nextsibling(y);
				continue;
			}

			filledform = display_nonform_field(y, filledform, id++);

			y = xode_get_nextsibling(y);
		}
	}

	return filledform;
}

xode edit_field_text_single(xode field, xode filledform, char *id)
{
	char *name, *setting, *desc;
	char bfr[256];

	name = xode_get_attrib(field, "var");
	if (!name) {
		return filledform;
	}

	desc = xode_get_tagdata(field, "desc");

	setting = get_filled_form(filledform, name);

	printf(SEPARATOR);
	printf("%s. %s: %s\n",
			id,
			name,
			setting ? setting : "<not set>");
	if (desc) {
		printf("%s\n", desc);
	}
	printf(SEPARATOR);
	printf("Enter setting: ");
	fflush(stdout);

	if (!fgets(bfr, sizeof(bfr), stdin)) {
		return filledform;
	}

	bfr[strlen(bfr) - 1] = '\0';

	if (!strcmp(bfr, "")) {
		filledform = set_filled_form(filledform, name, NULL);
	}
	else {
		filledform = set_filled_form(filledform, name, bfr);
	}

	printf("\n");

	return filledform;
}

xode edit_field_text_private(xode field, xode filledform, char *id)
{
	/* unsupported */
	return edit_field_text_single(field, filledform, id);
}

xode edit_field_text_multi(xode field, xode filledform, char *id)
{
	/* unsupported */
	return edit_field_text_single(field, filledform, id);
}

xode edit_field_list_single(xode field, xode filledform, char *id)
{
	char *name, *setting, *desc;
	char bfr[256];
	int maxid, choiceid;

	name = xode_get_attrib(field, "var");
	if (!name) {
		return filledform;
	}

	desc = xode_get_tagdata(field, "desc");

	setting = get_filled_form(filledform, name);

	printf(SEPARATOR);
	printf("%s. %s: %s\n",
			id,
			name,
			setting ?
				((!strcmp(setting, "1")) ||
				 (!strcmp(setting, "true")) ||
				 (!strcmp(setting, "yes")) ? "true" : "false")
				: "<not set>");
	if (desc) {
		printf("%s\n", desc);
	}
	printf(SEPARATOR);
	maxid = show_list_labels(field);
	printf(SEPARATOR);
	printf("Enter setting: ");
	fflush(stdout);

	if (!fgets(bfr, sizeof(bfr), stdin)) {
		return filledform;
	}

	bfr[strlen(bfr) - 1] = '\0';
	choiceid = atoi(bfr);

	if (!strcmp(bfr, "")) {
		filledform = set_filled_form(filledform, name, NULL);
	}
	else if (isdigit(bfr[0]) && (choiceid <= maxid) && (choiceid >= 0)) {
		filledform = set_filled_form(filledform, name, bfr);
	}
	else {
		printf("Illegal choice specified.\n");
		return edit_field_list_single(field, filledform, id);
	}

	printf("\n");

	return filledform;
}

xode edit_field_list_multi(xode field, xode filledform, char *id)
{
	/* unsupported */
	return edit_field_text_single(field, filledform, id);
}

xode edit_field_boolean(xode field, xode filledform, char *id)
{
	char *name, *setting, *desc;
	char bfr[256];

	name = xode_get_attrib(field, "var");
	if (!name) {
		return filledform;
	}

	desc = xode_get_tagdata(field, "desc");

	setting = get_filled_form(filledform, name);

	printf(SEPARATOR);
	printf("%s. %s: %s\n",
			id,
			name,
			setting ?
				((!strcmp(setting, "1")) ||
				 (!strcmp(setting, "true")) ||
				 (!strcmp(setting, "yes")) ? "true" : "false")
				: "<not set>");
	if (desc) {
		printf("%s\n", desc);
	}
	printf(SEPARATOR);
	printf("0. false\n");
	printf("1. true\n");
	printf(SEPARATOR);
	printf("Enter setting: ");
	fflush(stdout);

	if (!fgets(bfr, sizeof(bfr), stdin)) {
		return filledform;
	}

	bfr[strlen(bfr) - 1] = '\0';

	if (!strcmp(bfr, "")) {
		filledform = set_filled_form(filledform, name, NULL);
	}
	else if (!strcmp(bfr, "0") || !strcmp(bfr, "false") ||
						!strcmp(bfr, "no")) {
		filledform = set_filled_form(filledform, name, "0");
	}
	else if (!strcmp(bfr, "1") || !strcmp(bfr, "true") ||
						!strcmp(bfr, "yes")) {
		filledform = set_filled_form(filledform, name, "1");
	}
	else {
		printf("Illegal setting specified.\n");
		return edit_field_boolean(field, filledform, id);
	}

	printf("\n");

	return filledform;
}

xode edit_field_jid_single(xode field, xode filledform, char *id)
{
	/* unsupported */
	return edit_field_text_single(field, filledform, id);
}

xode edit_field_jid_multi(xode field, xode filledform, char *id)
{
	/* unsupported */
	return edit_field_text_single(field, filledform, id);
}

xode
edit_form_entry(char *id, xode entry, xode filledform)
{
	char *type;

	type = xode_get_attrib(entry, "type");
	if (!type) {
		return filledform;
	}

	if (!strcmp(type, JFORM_FIELD_TEXT_SINGLE)) {
		filledform = edit_field_text_single(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_TEXT_PRIVATE)) {
		filledform = edit_field_text_private(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_TEXT_MULTI)) {
		filledform = edit_field_text_multi(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_LIST_SINGLE)) {
		filledform = edit_field_list_single(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_LIST_MULTI)) {
		filledform = edit_field_list_multi(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_BOOLEAN)) {
		filledform = edit_field_boolean(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_JID_SINGLE)) {
		filledform = edit_field_jid_single(entry, filledform, id);
	}
	else if (!strcmp(type, JFORM_FIELD_JID_MULTI)) {
		filledform = edit_field_jid_multi(entry, filledform, id);
	}
	else {
		/* Unsupported, punt to text_single */
		filledform = edit_field_text_single(entry, filledform, id);
	}

	return filledform;
}

xode
process_form_choice(char *choice, xode form, xode filledform)
{
	xode x, y;
	char id = 'A';
	char idstr[2];
	idstr[1] = '\0';

	x = xode_get_firstchild(form);
	while (x) {
		char *tag;

		tag = xode_get_name(x);
		if (!tag || strcmp(tag, JFORM_FIELD)) {
			x = xode_get_nextsibling(x);
			continue;
		}

		idstr[0] = id++;
		if (!strncasecmp(idstr, choice, 1)) {
			filledform = edit_form_entry(idstr, x, filledform);
			return filledform;
		}

		x = xode_get_nextsibling(x);
	}

	return filledform;
}

xode
edit_nonform_entry(char *id, xode entry, xode filledform)
{
	char *name, *setting;
	char bfr[256];

	name = xode_get_name(entry);
	if (!name) {
		return filledform;
	}

	setting = get_filled_nonform(filledform, name);

	printf(SEPARATOR);
	printf("%s. %s: %s\n",
			id,
			name,
			setting ? setting : "<not set>");
	printf(SEPARATOR);
	printf("Enter setting: ");
	fflush(stdout);

	if (!fgets(bfr, sizeof(bfr), stdin)) {
		return filledform;
	}

	bfr[strlen(bfr) - 1] = '\0';

	if (!strcmp(bfr, "")) {
		filledform = set_filled_nonform(filledform, name, NULL);
	}
	else {
		filledform = set_filled_nonform(filledform, name, bfr);
	}

	printf("\n");

	return filledform;
}

xode
process_nonform_choice(char *choice, xode form, xode filledform)
{
	xode x;
	char id = 'A';
	char idstr[2];
	idstr[1] = '\0';

	x = xode_get_firstchild(form);
	while (x) {
		char *tag;

		tag = xode_get_name(x);
		if (!tag
			 || !strcmp(tag, JFORM_INSTRUCTIONS)
			 || !strcmp(tag, JFORM_TITLE)
			 || !strcmp(tag, "key")
							) {
			x = xode_get_nextsibling(x);
			continue;
		}

		idstr[0] = id++;
		if (!strncasecmp(idstr, choice, 1)) {
			filledform = edit_nonform_entry(idstr, x, filledform);
			return filledform;
		}

		x = xode_get_nextsibling(x);
	}

	return filledform;
}

xode
process_choice(char *choice, xode form, xode filledform)
{
	xode x;
	char *type;

	if (!choice) {
		return filledform;
	}

	x = xode_get_tag(form, "x");
	if (x) {
		type = xode_get_attrib(x, "type");
	}

	if (type && !strcmp(type, "form")) {
		filledform = process_form_choice(choice, x, filledform);
	}
	else {
		filledform = process_nonform_choice(choice, form, filledform);
	}

	return filledform;
}

xode
JFormHandler(xode form)
{
	xode baseform, filledform, query, x;
	char *ns, *type;
	char bfr[100];

	ns = xode_get_attrib(form, "xmlns");
	if (!ns) {
		printf("Invalid form returned, no query namespace.\n");
		return NULL;
	}

	baseform = xode_new("query");
	xode_put_attrib(baseform, "xmlns", ns);

	type = ns = NULL;

	x = xode_get_tag(form, "x");
	if (x) {
		type = xode_get_attrib(x, "type");
	}

	if (type && !strcmp(type, "form")) {
		filledform = xode_insert_tag(baseform, "x");
		xode_put_attrib(filledform, "xmlns", NS_DATA);
		xode_put_attrib(filledform, "type", "submit");
	}
	else {
		filledform = baseform;
	}

	for (;;) {
		filledform = display_form(form, filledform);
		printf("\n");
		printf("Enter a letter id to modify, 'submit' to submit, or 'cancel' to cancel.\n");
		printf("Command: ");
		fflush(stdout);

		if (!fgets(bfr, sizeof(bfr), stdin)) {
			break;
		}

		printf("\n");
		bfr[strlen(bfr) - 1] = '\0';

		if (!strncmp(bfr, "cancel", 6)) {
			return NULL;
		}
		else if (!strncmp(bfr, "submit", 6)) {
			break;
		}
		else if (strlen(bfr) > 1) {
			continue;
		}
		else {
			filledform = process_choice(bfr, form, filledform);
		}
	}

	return baseform;
}

void
display_form_results(xode form)
{
	xode legend, x, item;
	char *title;
	int cnt;

	title = xode_get_tagdata(form, "title");
	printf(SEPARATOR);
	if (title) {
		printf("%s\n", title);
	}
	else {
		printf("Results\n");
	}
	printf(SEPARATOR);

	legend = xode_new("legend");
	x = xode_get_tag(form, "reported");
	if (x) {
		x = xode_get_firstchild(x);
		while (x) {
			char *var, *label;

			var = xode_get_attrib(x, "var");
			label = xode_get_attrib(x, "label");
			if (var && label) {
				xode tag;

				tag = xode_insert_tag(legend, var);
				xode_insert_cdata(tag, label, strlen(label));
			}
			x = xode_get_nextsibling(x);
		}
	}

	cnt = 0;
	item = xode_get_tag(form, "item");
	while (item) {
		xode field;

		cnt++;
		field = xode_get_tag(item, "field");
		while (field) {
			char *label, *var, *value;

			var = xode_get_attrib(field, "var");
			if (!var) {
				field = xode_get_nextsibling(field);
				continue;
			}

			label = xode_get_tagdata(legend, var);
			value = xode_get_tagdata(field, "value");

			printf("%-20.20s = %s\n",
				label ? label : var,
				value ? value : "");
			field = xode_get_nextsibling(field);
		}
		item = xode_get_nextsibling(item);
		printf(SEPARATOR);
	}

	printf("%d results returned.\n", cnt);
}

void
display_nonform_results(xode form)
{
	printf("Non-Form results:\n%s\n", xode_to_prettystr(form));
}

void
JDisplayForm(xode form)
{
	xode x;
	char *type;

	x = xode_get_tag(form, "x");
	if (x) {
		return display_form_results(x);
	}
	else {
		return display_nonform_results(form);
	}
}
