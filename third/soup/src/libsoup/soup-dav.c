/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-dav.c: DAV client support.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <string.h>

#include "soup-dav.h"
#include "soup-error.h"

SoupDavResponse *
soup_dav_response_new (gchar           *href,
		       guint            response_code,
		       gchar           *response_desc)
{
	SoupDavResponse *ret;
	const gchar *known_reason;

	g_return_val_if_fail (href != NULL, NULL);
	g_return_val_if_fail (response_code != 0, NULL);

	ret = g_new0 (SoupDavResponse, 1);

	ret->href = g_strdup (href);
	ret->response_code = response_code;

	known_reason = soup_error_get_phrase (response_code);
	if (known_reason) 
		ret->response_reason = g_strdup (known_reason);

	if (response_desc)
		ret->response_desc = g_strdup (response_desc);

	return ret;
}

SoupDavResponse *
soup_dav_propstat_new (gchar           *href,
		       GSList          *prop_list,
		       gchar           *response_desc)
{
	SoupDavResponse *ret;

       	g_return_val_if_fail (href != NULL, NULL);
	g_return_val_if_fail (prop_list != NULL, NULL);

	ret = g_new0 (SoupDavResponse, 1);

	ret->href = g_strdup (href);
	ret->prop_list = prop_list;

	if (response_desc)
		ret->response_desc = g_strdup (response_desc);

	return ret;
}

void 
soup_dav_response_free (SoupDavResponse *resp)
{
	g_return_if_fail (resp != NULL);

	soup_dav_prop_list_free (resp->prop_list);

	g_free (resp->href);
	g_free (resp->response_reason);
	g_free (resp->response_desc);
	g_free (resp);
}

SoupDavMultiStatus *
soup_dav_mstat_new (SoupMessage *msg)
{
	SoupDavMultiStatus *ret;

	g_return_val_if_fail (msg != NULL, NULL);

	ret = g_new0 (SoupDavMultiStatus, 1);
	ret->msg = msg;

	return ret;
}

void 
soup_dav_mstat_free (SoupDavMultiStatus *ms)
{
	GSList *iter;

	g_return_if_fail (ms);

	for (iter = ms->responses; iter; iter = iter->next) {
		SoupDavResponse *resp = iter->data;

		soup_dav_response_free (resp);
	}

	g_slist_free (ms->responses);

	g_free (ms);
}

static gint
hash_prop (SoupDavProp *prop)
{
	return prop->response_code + g_str_hash (prop->response_desc);
}

static gboolean
compare_prop (SoupDavProp *prop1, SoupDavProp *prop2)
{
	if (prop1->response_code == prop2->response_code &&
	    !strcmp (prop1->response_desc, prop2->response_desc))
		return TRUE;

	return FALSE;
}

static gboolean 
serialize_proplist (SoupDavProp *key, GSList *prop_list, GString *data)
{
	GSList *iter;

	g_string_sprintfa (data,
			   "    <DAV:propstat>\n"
			   "      <DAV:status>HTTP/1.1 %d %s</DAV:status>\n"
			   "      <DAV:prop>\n",
			   key->response_code ? key->response_code : 200,
			   key->response_reason ? key->response_reason : "OK");

	for (iter = prop_list; iter; iter = iter->next) {
		SoupDavProp *prop = iter->data;

		if (key->response_code) {
			if (key->ns_prefix)
				g_string_sprintfa (
					data,
					"        <%s:%s xmlns:%s=\"%s\"/>\n",
					prop->ns_prefix,
					prop->name,
					prop->ns_prefix,
					prop->ns_uri);
			else 
				g_string_sprintfa (
					data,
					"        <DAV:%s/>\n",
					prop->name);
		} else {
			if (key->ns_prefix)
				g_string_sprintfa (
					data,
					"        <%s:%s xmlns:%s=\"%s\">"
					"%s"
					"</%s:%s>\n",
					prop->ns_prefix,
					prop->name,
					prop->ns_prefix,
					prop->ns_uri,
					prop->content,
					prop->ns_prefix,
					prop->name);
			else
				g_string_sprintfa (
					data,
					"        <DAV:%s>%s</DAV:%s>\n",
					prop->name,
					prop->content,
					prop->name);				
		}

	}

	g_string_append (data, 
			 "      </DAV:prop>\n"
			 "    </DAV:propstat>\n");

	g_slist_free (prop_list);

	return TRUE;
}

static void
serialize_response (SoupDavResponse *resp, GString *data) 
{
	g_string_sprintfa (data, 
			   "  <DAV:response>\n"
			   "    <DAV:href>%s</DAV:href>\n",
			   resp->href);

	if (resp->response_code)
		g_string_sprintfa (
			data, 
			"    <DAV:status>HTTP/1.1 %d %s</DAV:status>\n",
			resp->response_code,
			resp->response_reason);

	if (resp->prop_list) {
		GSList *props;
		GHashTable *prop_hash;

		prop_hash = g_hash_table_new ((GHashFunc) hash_prop, 
					      (GCompareFunc) compare_prop);

		for (props = resp->prop_list; props; props = props->next) {
			SoupDavProp *prop = props->data;
			GSList *match_list;

			match_list = g_hash_table_lookup (prop_hash, prop);

			if (match_list)
				match_list = g_slist_prepend (match_list, prop);
			else
				g_hash_table_insert (prop_hash, 
						     prop, 
						     g_slist_prepend (NULL, 
								      prop));
		}

		g_hash_table_foreach_remove (prop_hash, 
					     (GHRFunc) serialize_proplist, 
					     data);

		g_hash_table_destroy (prop_hash);
	}

	if (resp->response_desc)
		g_string_sprintfa (data,
				   "    <DAV:responsedescription>"
				   "%s"
				   "</DAV:responsedescription>\n",
				   resp->response_desc);

	g_string_append (data, "  </DAV:response>\n");
}

void 
soup_dav_mstat_serialize (SoupDavMultiStatus *ms, SoupDataBuffer *buf)
{
	GString *data;
	GSList *iter;

	g_return_if_fail (ms != NULL);
	g_return_if_fail (buf != NULL);

	data = g_string_new ("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
			     "<DAV:multistatus xmlns:DAV=\"DAV:\">\n");

	for (iter = ms->responses; iter; iter = iter->next) {
		SoupDavResponse *resp = iter->data;
		serialize_response (resp, data);
	}

	g_string_append (data, "</DAV:multistatus>\n");

	buf->owner = SOUP_BUFFER_SYSTEM_OWNED;
	buf->body = data->str;
	buf->length = data->len;

	g_string_free (data, FALSE);
}

void 
soup_dav_mstat_add_response (SoupDavMultiStatus *ms, SoupDavResponse *resp)
{
	g_return_if_fail (ms != NULL);
	g_return_if_fail (resp != NULL);

	ms->responses = g_slist_append (ms->responses, resp);
}

SoupDavProp *
soup_dav_prop_new (const gchar *name, 
		   const gchar *ns_prefix, 
		   const gchar *ns_uri, 
		   const gchar *content)
{
	SoupDavProp *prop;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (ns_prefix != NULL && ns_uri == NULL, NULL);

	prop = g_new0 (SoupDavProp, 1);
	prop->name = g_strdup (name);

	if (ns_prefix && ns_uri) {
		prop->ns_prefix = g_strdup (ns_prefix);
		prop->ns_uri = g_strdup (ns_uri);
	}

	if (content)
		prop->content = g_strdup (content);

	return prop;
}

void 
soup_dav_prop_free (SoupDavProp *prop)
{
	g_return_if_fail (prop != NULL);

	g_free (prop->name);
	g_free (prop->ns_prefix);
	g_free (prop->ns_uri);
	g_free (prop->content);
	g_free (prop->response_reason);
	g_free (prop->response_desc);
	g_free (prop);
}

void 
soup_dav_prop_list_free (GSList *prop_list)
{
	GSList *iter;

	for (iter = prop_list; iter; iter = iter->next) {
		soup_dav_prop_free ((SoupDavProp *) iter->data);
	}

	g_slist_free (prop_list);
}

SoupDavProp *
soup_dav_prop_copy (SoupDavProp *src)
{
	SoupDavProp *prop;

	g_return_val_if_fail (src != NULL, NULL);

	prop = soup_dav_prop_new (src->name,
				  src->ns_prefix,
				  src->ns_uri,
				  src->content);

	if (src->response_code) 
		soup_dav_prop_set_error (prop,
					 src->response_code,
					 src->response_reason,
					 src->response_desc);

	return prop;
}

GSList *
soup_dav_prop_list_copy (GSList *src)
{
	GSList *ret = NULL;

	g_return_val_if_fail (src != NULL, NULL);

	for (; src; src = src->next) {
		SoupDavProp *prop = src->data;
		ret = g_slist_prepend (ret, soup_dav_prop_copy (prop));
	}

	return ret;
}

void 
soup_dav_prop_set_content  (SoupDavProp *prop,
			    const gchar *content)
{
	g_return_if_fail (prop != NULL);

	if (prop->content) g_free (prop->content);

	if (content)
		prop->content = g_strdup (content);
	else
		prop->content = NULL;
}

void 
soup_dav_prop_set_error (SoupDavProp *prop,
			 guint        response_code,
			 const gchar *response_reason,
			 const gchar *response_desc)
{
	g_return_if_fail (prop != NULL);
	g_return_if_fail (response_code != 0);
	g_return_if_fail (response_reason != NULL);

	if (prop->content) g_free (prop->content);

	prop->response_code = response_code;
	prop->response_reason = g_strdup (response_reason);
	
	if (response_desc) 
		prop->response_desc = g_strdup (response_desc);
}
