/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-locate.c: Find an element structure, given a name
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>

#include "wsdl-parse.h"
#include "wsdl-locate.h"

/**
 * wsdl_locate_porttype_operation:
 * @name: a string containing the name of the WSDL porttype operation to find
 * @porttype: a pointer to a #wsdl_porttype structure
 *
 * Finds the #wsdl_porttype_operation named @name that is a direct
 * descendant of @porttype.
 *
 * Returns: the #wsdl_porttype_operation named @name that is a direct
 * descendant of @porttype, or NULL if one can't be found.
 */
wsdl_porttype_operation *
wsdl_locate_porttype_operation (guchar * name, wsdl_porttype * porttype)
{
	GSList *iter;

	g_assert (porttype != NULL);

	iter = porttype->operations;
	while (iter != NULL) {
		wsdl_porttype_operation *op = iter->data;

		g_assert (op != NULL);

		if (!g_strcasecmp (op->name, name)) {
			return (op);
		}

		iter = iter->next;
	}

	return (NULL);
}


/**
 * wsdl_locate_message_part:
 * @name: a string containing the name of the WSDL message part to find
 * @message: a pointer to a #wsdl_message structure
 *
 * Finds the #wsdl_message_part named @name that is a direct
 * descendant of @message.
 *
 * Returns: the #wsdl_message_part named @name that is a direct
 * descendant of @message, or NULL if one can't be found.
 */
wsdl_message_part *
wsdl_locate_message_part (guchar * name, wsdl_message * message)
{
	GSList *iter;

	g_assert (message != NULL);

	iter = message->parts;
	while (iter != NULL) {
		wsdl_message_part *part = iter->data;

		g_assert (part != NULL);

		if (!g_strcasecmp (part->name, name)) {
			return (part);
		}

		iter = iter->next;
	}

	return (NULL);
}


/**
 * wsdl_locate_message:
 * @name: a string containing the name of the WSDL message to find
 * @definitions: a pointer to a #wsdl_definitions structure
 *
 * Finds the #wsdl_message named @name that is a direct descendant of
 * @definitions.
 *
 * Returns: the #wsdl_message named @name that is a direct
 * descendant of @definitions, or NULL if one can't be found.
 */
wsdl_message *
wsdl_locate_message (guchar * name, wsdl_definitions * definitions)
{
	GSList *iter;

	g_assert (definitions != NULL);

	iter = definitions->messages;
	while (iter != NULL) {
		wsdl_message *message = iter->data;

		g_assert (message != NULL);

		if (!g_strcasecmp (message->name, name)) {
			return (message);
		}

		iter = iter->next;
	}

	return (NULL);
}


/**
 * wsdl_locate_porttype:
 * @name: a string containing the name of the WSDL porttype to find
 * @definitions: a pointer to a #wsdl_definitions structure
 *
 * Finds the #wsdl_porttype named @name that is a direct descendant of
 * @definitions.
 *
 * Returns: the #wsdl_porttype named @name that is a direct descendant
 * of @definitions, or NULL if one can't be found.
 */
wsdl_porttype *
wsdl_locate_porttype (guchar * name, wsdl_definitions * definitions)
{
	GSList *iter;

	g_assert (definitions != NULL);

	iter = definitions->porttypes;
	while (iter != NULL) {
		wsdl_porttype *binding = iter->data;

		g_assert (binding != NULL);

		if (!g_strcasecmp (binding->name, name)) {
			return (binding);
		}

		iter = iter->next;
	}

	return (NULL);
}


/**
 * wsdl_locate_binding:
 * @name: a string containing the name of the WSDL binding to find
 * @definitions: a pointer to a #wsdl_definitions structure
 *
 * Finds the #wsdl_binding named @name that is a direct descendant of
 * @definitions.
 *
 * Returns: the #wsdl_binding named @name that is a direct descendant
 * of @definitions, or NULL if one can't be found.
 */
wsdl_binding *
wsdl_locate_binding (guchar * name, wsdl_definitions * definitions)
{
	GSList *iter;

	g_assert (definitions != NULL);

	iter = definitions->bindings;
	while (iter != NULL) {
		wsdl_binding *binding = iter->data;

		g_assert (binding != NULL);

		if (!g_strcasecmp (binding->name, name)) {
			return (binding);
		}

		iter = iter->next;
	}

	return (NULL);
}
