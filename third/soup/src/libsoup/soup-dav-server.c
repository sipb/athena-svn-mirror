/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-dav-server.h: DAV server support.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <ctype.h>
#include <glib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "soup-dav.h"
#include "soup-dav-server.h"
#include "soup-message.h"

typedef struct {
	SoupDavServerHandlers *handlers;
	gpointer               user_data;
} InternalHandlers;

#define __uri_exists(_ctx, _ih, _path) \
	(*(_ih)->handlers->uri_exists) (_ctx, _path, (_ih)->user_data)

#define __is_collection(_ctx, _ih, _path) \
	(*(_ih)->handlers->is_collection) (_ctx, _path, (_ih)->user_data)

#define __opt_describe_locks(_ctx, _ih, _path) \
	(*(_ih)->handlers->opt_describe_locks) (_ctx, _path, (_ih)->user_data)

#define __opt_lock(_ctx, _ih, _path, _lock) \
	(*(_ih)->handlers->opt_lock) (_ctx, _path, _lock, (_ih)->user_data)

#define __opt_unlock(_ctx, _ih, _path, _lock) \
	(*(_ih)->handlers->opt_unlock) (_ctx, _path, _lock, (_ih)->user_data)

#define __create_collection(_ctx, _ih, _path) \
	(*(_ih)->handlers->create_collection) (_ctx, _path, (_ih)->user_data)

#define __create_doc(_ctx, _ih, _path, _buf) \
	(*(_ih)->handlers->create_doc) (_ctx, _path, _buf, (_ih)->user_data)

#define __delete_collection(_ctx, _ih, _path) \
	(*(_ih)->handlers->delete_collection) (_ctx, _path, (_ih)->user_data)

#define __delete_doc(_ctx, _ih, _path) \
	(*(_ih)->handlers->delete_doc) (_ctx, _path, (_ih)->user_data)

#define __can_delete(_ctx, _ih, _path) \
	(*(_ih)->handlers->can_delete) (_ctx, _path, (_ih)->user_data)

#define __list_contents(_ctx, _ih, _path) \
	(*(_ih)->handlers->list_contents) (_ctx, _path, (_ih)->user_data)

#define __get_content(_ctx, _ih, _path, _buf) \
	(*(_ih)->handlers->get_content) (_ctx, _path, _buf, (_ih)->user_data)

#define __set_content(_ctx, _ih, _path, _buf) \
	(*(_ih)->handlers->set_content) (_ctx, _path, _buf, (_ih)->user_data)

#define __get_dav_prop(_ctx, _ih, _path, _prop) \
	(*(_ih)->handlers->get_dav_prop) (_ctx, _path, _prop, (_ih)->user_data)

#define __set_dav_prop(_ctx, _ih, _path, _prop) \
	(*(_ih)->handlers->set_dav_prop) (_ctx, _path, _prop, (_ih)->user_data)

#define __list_custom_props(_ctx, _ih, _path) \
	(*(_ih)->handlers->list_custom_props) (_ctx, _path, (_ih)->user_data)

#define __get_custom_prop(_ctx, _ih, _path, _prop)             \
	(*(_ih)->handlers->get_custom_prop) (_ctx,             \
					     _path,            \
					     _prop,            \
					     (_ih)->user_data)

#define __set_custom_prop(_ctx, _ih, _path, _prop)             \
	(*(_ih)->handlers->set_custom_prop) (_ctx,             \
					     _path,            \
					     _prop,            \
					     (_ih)->user_data)

#define __delete_custom_prop(_ctx, _ih, _path, _prop)             \
	(*(_ih)->handlers->delete_custom_prop) (_ctx,             \
						_path,            \
						_prop,            \
						(_ih)->user_data)

#define __opt_move(_ctx, _ih, _path, _dest, _ow) \
	(*(_ih)->handlers->opt_move) (_ctx, _path, _dest, _ow, (_ih)->user_data)

#define __opt_copy(_ctx, _ih, _path, _dest, _ow) \
	(*(_ih)->handlers->opt_move) (_ctx, _path, _dest, _ow, (_ih)->user_data)

/*
 * Check the given path is under the registered path
 */
static gboolean
check_path_access (SoupServerContext *ctx, const char *path)
{
	if (!ctx->handler->path) {
		if (soup_server_get_handler (ctx->server, 
					     path) != ctx->handler)
			return FALSE;
		else
			return TRUE;
	} else
		return strncmp (path, 
				ctx->handler->path, 
				strlen (ctx->handler->path)) == 0;
}

/*
 * Check that parent collection exists.
 */
static gboolean 
parent_exists (SoupServerContext *ctx, InternalHandlers *ih, const char *path)
{
	gchar *parent, *iter;
	gboolean ret = TRUE;

	parent = g_strdup (path);

	if (parent [strlen (parent) - 1] == '/') 
		parent [strlen (parent) - 1] = '\0';

	iter = strrchr (parent, '/');
	if (iter) *iter = '\0';
	
	if (!__is_collection (ctx, ih, parent)) 
		ret = FALSE;

	g_free (parent);

	return ret;
}

static inline gboolean
get_overwrite (const gchar *val)
{
	gboolean ret = TRUE;
	gchar *val_cpy;

	if (!val) return TRUE;

	val_cpy = g_strdup (val);
	g_strstrip (val_cpy);

	if (toupper (*val_cpy) == 'F') ret = FALSE;

	g_free (val_cpy);

	return ret;
}

static inline gint
get_depth (const gchar *val)
{
	if (!val) return -1;
	else if (*val == '0') return 0;
	else if (*val == '1') return 1;
	else if (!g_strcasecmp (val, "infinity")) return -1;

	return 0;
}

static gchar *
make_href (SoupServerContext *ctx, const gchar *path)
{
	SoupUri *suri;
	gchar *path_old, *uri;

	suri = (SoupUri *) soup_context_get_uri (ctx->msg->context);
	path_old = suri->path;

	suri->path = (gchar *) path;
	uri = soup_uri_to_string (suri, FALSE);
	suri->path = path_old;

	return uri;
}

#define make_dav_prop(_name) soup_dav_prop_new (_name, NULL, NULL, NULL)

static GSList *
list_dav_props (gboolean is_col)
{
	GSList *ret = NULL;
	SoupDavProp *prop;
	
	prop = make_dav_prop ("creationdate");
	ret = g_slist_prepend (ret, prop);

	prop = make_dav_prop ("displayname");
	ret = g_slist_prepend (ret, prop);

	prop = make_dav_prop ("resourcetype");
	ret = g_slist_prepend (ret, prop);

	prop = make_dav_prop ("supportedlock");
	ret = g_slist_prepend (ret, prop);

	if (!is_col) {
		prop = make_dav_prop ("getcontentlength");
		ret = g_slist_prepend (ret, prop);

		prop = make_dav_prop ("getlastmodified");
		ret = g_slist_prepend (ret, prop);

		prop = make_dav_prop ("getetag");
		ret = g_slist_prepend (ret, prop);

		prop = make_dav_prop ("getcontenttype");
		ret = g_slist_prepend (ret, prop);
	}

	return ret;
}

#if 0
static gboolean
dav_prop_is_writable (const gchar *name)
{
	/* FIXME: Implement */
	return TRUE;
}
#endif

static gboolean
munge_dav_prop (SoupServerContext *ctx, 
		InternalHandlers  *ih, 
		const gchar       *path,
		SoupDavProp       *prop) 
{
	/* FIXME: Implement for resource_type, supportedlock, 
	          getcontentlength. */
	return FALSE;
}

static gboolean
i_delete_ok (SoupServerContext *ctx,
	     InternalHandlers  *ih, 
	     const gchar       *path)
{
	GSList *subitems, *iter;
	gboolean fail = FALSE;

	if (__is_collection (ctx, ih, path)) {
		subitems = __list_contents (ctx, ih, path);

		for (iter = subitems; iter && !fail; iter = iter->next) {
			gchar *sub_path, *child = iter->data;

			sub_path = g_strconcat (path, "/", child, NULL);

			if (!i_delete_ok (ctx, ih, sub_path)) {
				for (; iter; iter = iter->next)
					g_free (iter->data);

				g_free (sub_path);
				g_slist_free (subitems);

				return FALSE;
			}				

			g_free (sub_path);
			g_free (child);
		}

		g_slist_free (subitems);
	}

	return __can_delete (ctx, ih, path);
}

static void 
i_delete (SoupServerContext *ctx,
	  InternalHandlers  *ih, 
	  const gchar       *path)
{
	GSList *subitems, *iter;

	if (!i_delete_ok (ctx, ih, path)) return;

	if (__is_collection (ctx, ih, path)) {
		subitems = __list_contents (ctx, ih, path);

		for (iter = subitems; iter; iter = iter->next) {
			gchar *sub_path, *child = iter->data;

			sub_path = g_strconcat (path, "/", child, NULL);

			i_delete (ctx, ih, sub_path);

			g_free (sub_path);
			g_free (child);
			iter = iter->next;
		}

		g_slist_free (subitems);

		__delete_collection (ctx, ih, path);
	} else
		__delete_doc (ctx, ih, path);
}

static void 
i_copy (SoupServerContext  *ctx, 
	InternalHandlers   *src_ih, 
	InternalHandlers   *dest_ih,
	const gchar        *src,
	const gchar        *dest,
	gint                depth,
	SoupDavMultiStatus *mstat)
{
	SoupDataBuffer   buf;
	gboolean         is_col;
	GSList          *props, *iter, *subitems;
	gchar           *href;
	SoupDavResponse *resp;

	/* FIXME: Support propertybehavior (keepalive | omit) */

	is_col = __is_collection (ctx, src_ih, src);

	if (is_col) {
		if (!__create_collection (ctx, dest_ih, dest)) {
			href = make_href (ctx, dest);
			resp = 
				soup_dav_response_new (
					href, 
					SOUP_ERROR_FORBIDDEN, 
					"Cannot create destination directory");
			g_free (href);

			soup_dav_mstat_add_response (mstat, resp);

			return;
		}
	} else {
		if (!__get_content (ctx, src_ih, src, &buf)) {
			href = make_href (ctx, dest);
			resp = 
				soup_dav_response_new (
					href, 
					SOUP_ERROR_FORBIDDEN, 
					"Unable to get source content");
			g_free (href);

			soup_dav_mstat_add_response (mstat, resp);

			return;
		}

		if (!__create_doc (ctx, dest_ih, dest, &buf)) {
			href = make_href (ctx, dest);
			resp = 
				soup_dav_response_new (
					href, 
					SOUP_ERROR_FORBIDDEN, 
					"Cannot create destination resource");
			g_free (href);

			soup_dav_mstat_add_response (mstat, resp);

			return;
		}
	}

	/*
	 * Copy Dav properties
	 */
	props = list_dav_props (is_col);

	for (iter = props; iter; iter = iter->next) {
		SoupDavProp *prop = iter->data;

		if (!__get_dav_prop (ctx, src_ih, src, prop) &&
		    !munge_dav_prop (ctx, src_ih, src, prop)) {
			soup_dav_prop_free (prop);
			continue;
		}

		__set_dav_prop (ctx, dest_ih, dest, prop);

		soup_dav_prop_free (prop);
	}

	g_slist_free (props);

	/*
	 * Copy custom properties
	 */
	props = __list_custom_props (ctx, src_ih, src);

	for (props = iter; iter; iter = iter->next) {
		SoupDavProp *prop = iter->data;

		__get_custom_prop (ctx, src_ih, src, prop);
		__set_custom_prop (ctx, dest_ih, dest, prop);

		soup_dav_prop_free (prop);
	}

	g_slist_free (props);

	/*
	 * Create children
	 */
	if (is_col && depth != 0) {
		subitems = __list_contents (ctx, src_ih, src);

		for (iter = subitems; iter; iter = iter->next) {
			gchar *schild, *dchild, *child;

			child = iter->data;

			schild = g_strconcat (src, "/", child, NULL);
			dchild = g_strconcat (dest, "/", child, NULL);

			i_copy (ctx, 
				src_ih, 
				dest_ih, 
				schild, 
				dchild, 
				depth, 
				mstat);

			g_free (schild);
			g_free (dchild);
			g_free (child);
		}

		g_slist_free (subitems);
	}
}

static void
move_copy (SoupServerContext *ctx, 
	   InternalHandlers  *src_ih, 
	   gboolean           delete_src)
{
	gboolean overwrite, dest_exists;
	gint depth;
	const gchar *src, *dest, *owh;
	SoupDavMultiStatus *mstat;
	InternalHandlers *dest_ih = src_ih;

	src = ctx->path;

	depth = get_depth (soup_message_get_header (ctx->msg->request_headers,
						    "Depth"));
	owh = soup_message_get_header (ctx->msg->request_headers, 
				       "Overwrite");
	overwrite = get_overwrite (owh);

	dest = soup_message_get_header (ctx->msg->request_headers, 
					"Destination");
	
	if (!check_path_access (ctx, dest)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_CONFLICT);
		return;
	}

	dest_exists = __uri_exists (ctx, dest_ih, dest);

	if (dest_exists) {
		if (!overwrite || !__can_delete (ctx, dest_ih, dest)) {
			soup_message_set_error (ctx->msg, 
						SOUP_ERROR_PRECONDITION_FAILED);
			return;
		}

		i_delete (ctx, dest_ih, dest);
	}

	mstat = soup_dav_mstat_new (ctx->msg);

	i_copy (ctx, src_ih, dest_ih, src, dest, depth, mstat);

	if (delete_src) 
		i_delete (ctx, src_ih, src);

	soup_dav_mstat_serialize (mstat, &ctx->msg->response);
	soup_dav_mstat_free (mstat);	
}

static void 
dav_move (SoupServerContext *ctx, InternalHandlers *ih)
{
	if (ih->handlers->opt_move) {
		const gchar *dest, *owh;
		gboolean ow;

		dest = soup_message_get_header (ctx->msg->request_headers,
						"Destination");
		owh = soup_message_get_header (ctx->msg->request_headers, 
					       "Overwrite");
		ow = get_overwrite (owh);

		__opt_move (ctx, ih, ctx->path, dest, ow);

		return;
	}

	move_copy (ctx, ih, TRUE);
}

static void 
dav_copy (SoupServerContext *ctx, InternalHandlers *ih)
{
	if (ih->handlers->opt_copy) {
		const gchar *dest, *owh;
		gboolean ow;

		dest = soup_message_get_header (ctx->msg->request_headers,
						"Destination");
		owh = soup_message_get_header (ctx->msg->request_headers, 
					       "Overwrite");
		ow = get_overwrite (owh);

		__opt_copy (ctx, ih, ctx->path, dest, ow);

		return;
	}

	move_copy (ctx, ih, FALSE);
}

static void 
dav_delete (SoupServerContext *ctx, InternalHandlers *ih)
{
	i_delete (ctx, ih, ctx->path);
}

static gboolean
parse_propfind (SoupServerContext  *ctx,
		SoupDataBuffer     *buf, 
		GSList            **list, 
		gboolean           *show_content) 
{
        xmlDocPtr xml_doc;
        xmlNodePtr cur;

	LIBXML_TEST_VERSION;
        xmlKeepBlanksDefault (0);

        xml_doc = xmlParseMemory (buf->body, buf->length);
        if (xml_doc == NULL) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_BAD_REQUEST);
                return FALSE;
        }

	cur = xmlDocGetRootElement (xml_doc);
        if (cur == NULL) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_BAD_REQUEST);
                xmlFreeDoc (xml_doc);
                return FALSE;
        }

	if (g_strcasecmp (cur->name, "PROPFIND") != 0) 
		goto PARSE_ERROR;

	cur = cur->xmlChildrenNode;
	if (!cur) goto PARSE_ERROR;

	*list = NULL;

	if (!g_strcasecmp (cur->name, "PROP")) {
		cur = cur->xmlChildrenNode;
		if (!cur) goto PARSE_ERROR;
		
		for (; cur; cur = cur->next) {
			SoupDavProp *prop;
			gboolean is_dav;

			is_dav = g_strcasecmp (cur->ns->href, "DAV:") == 0;

			prop = 
				soup_dav_prop_new (
					cur->name, 
					is_dav ? NULL : cur->ns->prefix,
					is_dav ? NULL : cur->ns->href,
					NULL);

			*list = g_slist_append (*list, prop);
		}

		*show_content = TRUE;
	}
	else if (!g_strcasecmp (cur->name, "ALLPROP")) {
		*show_content = TRUE;
	}
	else if (!g_strcasecmp (cur->name, "PROPNAME")) {
		*show_content = FALSE;
	}
	else goto PARSE_ERROR;

	xmlFreeDoc (xml_doc);
	return TRUE;

 PARSE_ERROR:
	soup_message_set_error (ctx->msg, SOUP_ERROR_DAV_UNPROCESSABLE);
	xmlFreeDoc (xml_doc);
	return FALSE;
}

static void
i_propfind (SoupServerContext  *ctx, 
	    InternalHandlers   *ih, 
	    const gchar        *path, 
	    gint                depth,
	    GSList             *find_list,
	    gboolean            show_content,
	    SoupDavMultiStatus *mstat)
{
	GSList *find_copy;
	SoupDavResponse *resp;
	gboolean is_col;
	gchar *href;

	/* FIXME: on error, use error for all following props */

	is_col = __is_collection (ctx, ih, path);

	if (find_list)
		find_copy = soup_dav_prop_list_copy (find_list);
	else {
		find_copy = list_dav_props (is_col);
		find_copy = g_slist_concat (find_copy,
					    __list_custom_props (ctx, 
								 ih, 
								 path));
	}

	if (show_content) {
		GSList *iter;

		for (iter = find_copy; iter; iter = iter->next) {
			SoupDavProp *prop = iter->data;

			if (!prop->ns_uri)
				__get_dav_prop (ctx, ih, path, prop);
			else
				__get_custom_prop (ctx, ih, path, prop);
		}
	}

	href = make_href (ctx, path);
	resp = soup_dav_propstat_new (href, find_copy, NULL);
	g_free (href);

	soup_dav_mstat_add_response (mstat, resp);

	if (depth == 0) return;

	if (is_col) {
		GSList *contents, *iter;

		contents = __list_contents (ctx, ih, path);

		for (iter = contents; iter; iter = iter->next) {
			gchar *path = iter->data;

			i_propfind (ctx, 
				    ih, 
				    path, 
				    depth == 1 ? 0 : depth, 
				    find_list,
				    show_content,
				    mstat);

			g_free (path);
		}

		g_slist_free (contents);
	}
}

static void 
dav_propfind (SoupServerContext *ctx, InternalHandlers *ih)
{
	gint depth;
	SoupDavMultiStatus *mstat;
	GSList *find_list;
	gboolean show_content;

	if (!__uri_exists (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_NOT_FOUND);
		return;
	}

	if (!parse_propfind (ctx, 
			     &ctx->msg->request, 
			     &find_list, 
			     &show_content))
		return;

	depth = get_depth (soup_message_get_header (ctx->msg->request_headers, 
						    "Depth"));

	mstat = soup_dav_mstat_new (ctx->msg);

	i_propfind (ctx, ih, ctx->path, depth, find_list, show_content, mstat);

	if (find_list)
		soup_dav_prop_list_free (find_list);

	soup_dav_mstat_serialize (mstat, &ctx->msg->response);
	soup_dav_mstat_free (mstat);
}

static gboolean
parse_proppatch (SoupServerContext  *ctx,
		 SoupDataBuffer     *buf, 
		 GSList            **list) 
{
        xmlDocPtr   xml_doc;
        xmlNodePtr  cur;

	LIBXML_TEST_VERSION;
        xmlKeepBlanksDefault (0);

        xml_doc = xmlParseMemory (buf->body, buf->length);
        if (xml_doc == NULL) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_BAD_REQUEST);
                return FALSE;
        }

	cur = xmlDocGetRootElement (xml_doc);
        if (cur == NULL) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_BAD_REQUEST);
                xmlFreeDoc (xml_doc);
                return FALSE;
        }

	if (g_strcasecmp (cur->name, "PROPERTYUPDATE") != 0)
		goto PARSE_ERROR;

	cur = cur->xmlChildrenNode;
	if (!cur) goto PARSE_ERROR;

	*list = NULL;

	for (; cur; cur = cur->next) {
		gboolean is_set;
		xmlNodePtr props;

		if (!g_strcasecmp (cur->name, "SET"))
			is_set = TRUE;
		else if (!g_strcasecmp (cur->name, "REMOVE"))
			is_set = FALSE;
		else goto PARSE_ERROR;

		props = cur->xmlChildrenNode;
		if (!props || g_strcasecmp (props->name, "PROP") != 0)
			goto PARSE_ERROR;

		props = props->xmlChildrenNode;
		if (!props)
			goto PARSE_ERROR;

		for (; props; props = props->next) {
			SoupDavProp *prop;
			gboolean     is_dav;

			is_dav = g_strcasecmp (props->ns->href, "DAV:") == 0;

			prop = 
				soup_dav_prop_new (
					props->name, 
					is_dav ? NULL : props->ns->prefix,
					is_dav ? NULL : props->ns->href,
					NULL);

			if (is_set)
				prop->content = 
					xmlNodeListGetString (xml_doc, 
							      props, 
							      FALSE);
			else
				prop->content = NULL;

			*list = g_slist_append (*list, prop);
		}
	}

	xmlFreeDoc (xml_doc);
	return TRUE;

 PARSE_ERROR:
	soup_message_set_error (ctx->msg, SOUP_ERROR_DAV_UNPROCESSABLE);
	xmlFreeDoc (xml_doc);
	return FALSE;
}

static void 
dav_proppatch (SoupServerContext *ctx, InternalHandlers *ih)
{
	SoupDavMultiStatus *mstat;
	SoupDavResponse *resp;
	GSList *list = NULL, *rollback_list = NULL, *delete_list = NULL, *iter;
	gchar *href;

	if (!__uri_exists (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_NOT_FOUND);
		return;
	}

	if (!parse_proppatch (ctx, &ctx->msg->request, &list))
		return;

	for (iter = list; iter; iter = iter->next) {
		SoupDavProp *prop, *backup;

		prop = iter->data;
		backup = soup_dav_prop_copy (prop);

		if (prop->ns_uri)
			__get_custom_prop (ctx, ih, ctx->path, backup);
		else
			__get_dav_prop (ctx, ih, ctx->path, backup);

		if (prop->content) {
			if (prop->ns_uri) {
				if (!__set_custom_prop (ctx, 
							ih, 
							ctx->path, 
							prop))
					goto ROLLBACK;		
			} else {
				if (!__set_dav_prop (ctx, 
						     ih, 
						     ctx->path, 
						     prop))
					goto ROLLBACK;
			}
		} else {
			if (prop->ns_uri) {
				if (!__delete_custom_prop (ctx, 
							   ih, 
							   ctx->path, 
							   prop))
					goto ROLLBACK;
			} else {
				soup_dav_prop_set_error (
					prop,
					SOUP_ERROR_FORBIDDEN,
					"Forbidden",
					"Cannot delete DAV property.");

				goto ROLLBACK;
			}
		}

		rollback_list = g_slist_prepend (rollback_list, backup);

		continue;

	ROLLBACK:
		soup_dav_prop_free (backup);

		/*
		 * Rollback values from already successful props and set
		 * Failed Depenency
		 */
		for (iter = rollback_list; iter; iter = iter->next) {
			backup = iter->data;

			if (backup->ns_uri)
				__set_custom_prop (ctx, ih, ctx->path, backup);
			else
				__set_dav_prop (ctx, ih, ctx->path, backup);
		}

		/*
		 * Set Failed Depenency for all other props
		 */
		for (iter = list; iter; iter = iter->next) {
			SoupDavProp *failprop = iter->data;

			if (failprop == prop) continue;

			soup_dav_prop_set_error (
				failprop, 
				SOUP_ERROR_DAV_DEPENDENCY_FAILED, 
				"Failed Dependency",
				NULL);
		}

		goto FINISH_REQUEST;
	}

	/*
	 * Success. Create result proplist.
	 */
	for (iter = list; iter; iter = iter->next) {
		SoupDavProp *prop = iter->data;

		if (!prop->content) {
			delete_list = g_slist_prepend (list, iter->data);
			list = g_slist_remove_link (list, iter);
		} 
		else {
			if (prop->ns_uri)
				__get_custom_prop (ctx, ih, ctx->path, prop);
			else
				__get_dav_prop (ctx, ih, ctx->path, prop);
		}
	}

 FINISH_REQUEST:
	soup_dav_prop_list_free (rollback_list);
	soup_dav_prop_list_free (delete_list);

	href = make_href (ctx, ctx->path);
	resp = soup_dav_propstat_new (href, list, NULL);
	g_free (href);

	mstat = soup_dav_mstat_new (ctx->msg);
	soup_dav_mstat_add_response (mstat, resp);

	soup_dav_mstat_serialize (mstat, &ctx->msg->response);
	soup_dav_mstat_free (mstat);
}

static void 
dav_lock (SoupServerContext *ctx, InternalHandlers *ih)
{
	/* FIXME: Implement */

	if (ih->handlers->opt_lock)
		__opt_lock (ctx, ih, ctx->path, NULL);

	// if locked
	//   error
	// else 
	//   lock	
}

static void 
dav_unlock (SoupServerContext *ctx, InternalHandlers *ih)
{
	/* FIXME: Implement */

	if (ih->handlers->opt_unlock)
		__opt_unlock (ctx, ih, ctx->path, NULL);

	// if locked
	//   if auth match
	//     unlock
	//   else
	//     error
	// else
	//   error
}

static void
dav_get (SoupServerContext *ctx, InternalHandlers *ih)
{
	if (!__uri_exists (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_NOT_FOUND);
		return;
	}

	if (!__get_content (ctx, ih, ctx->path, &ctx->msg->response)) {
		if (__is_collection (ctx, ih, ctx->path)) 
			soup_message_set_error (ctx->msg, 
						SOUP_ERROR_METHOD_NOT_ALLOWED);
		else
			soup_message_set_error (ctx->msg, 
						SOUP_ERROR_FORBIDDEN);
		return;
	}

	soup_message_set_error (ctx->msg, SOUP_ERROR_OK);
}

static void
dav_put (SoupServerContext *ctx, InternalHandlers *ih)
{
	/* FIXME: Check for read-only */

	if (!parent_exists (ctx, ih, ctx->path) ||
	    __is_collection (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_CONFLICT);
		return;
	}

	if (!__set_content (ctx, ih, ctx->path, &ctx->msg->request)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_FORBIDDEN);
		return;
	}

	soup_message_set_error (ctx->msg, SOUP_ERROR_OK);
}

static void
dav_mkcol (SoupServerContext *ctx, InternalHandlers *ih)
{
	if (__uri_exists (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, 
					SOUP_ERROR_METHOD_NOT_ALLOWED);
		return;
	}

	if (!parent_exists (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_CONFLICT);
		return;
	}

	if (!__create_collection (ctx, ih, ctx->path)) {
		soup_message_set_error (ctx->msg, SOUP_ERROR_FORBIDDEN);
		return;
	}

	soup_message_set_error (ctx->msg, SOUP_ERROR_CREATED);
}

static gchar *
get_supported_methods (SoupServerContext *ctx, 
		       InternalHandlers  *ih, 
		       const gchar       *path)
{
	/* FIXME: Implement */

	return g_strdup ("LOCK, UNLOCK, GET, PUT, COPY, MOVE, MKCOL, DELETE, "
			 "PROPFIND, PROPPATCH, OPTIONS");
}

static void
dav_options (SoupServerContext *ctx, InternalHandlers *ih)
{
	gchar *methods;

	if (*ctx->path != '*') {
		if (!__uri_exists (ctx, ih, ctx->path)) {
			soup_message_set_error (ctx->msg, SOUP_ERROR_NOT_FOUND);
			return;
		}

		methods = get_supported_methods (ctx, ih, ctx->path);
	} else 
		methods = get_supported_methods (ctx, ih, NULL);

	soup_message_add_header (ctx->msg->response_headers, "DAV", "1,2");
	soup_message_add_header (ctx->msg->response_headers, "Allow", methods);

	g_free (methods);
}

SoupMethodId 
soup_dav_server_process  (SoupServerContext     *ctx,
			  SoupDavServerHandlers *handlers,
			  gpointer               user_data)
{
	InternalHandlers ih;

	ih.handlers = handlers;
	ih.user_data = user_data;

	switch (ctx->method_id) {
	case SOUP_METHOD_ID_GET:
		dav_get (ctx, &ih);
		break;		
	case SOUP_METHOD_ID_COPY:
		dav_copy (ctx, &ih);
		break;
	case SOUP_METHOD_ID_MOVE:
		dav_move (ctx, &ih);
		break;
	case SOUP_METHOD_ID_MKCOL:
		dav_mkcol (ctx, &ih);
		break;
	case SOUP_METHOD_ID_DELETE:
		dav_delete (ctx, &ih);
		break;
	case SOUP_METHOD_ID_PROPFIND:
		dav_propfind (ctx, &ih);
		break;
	case SOUP_METHOD_ID_PROPPATCH:
		dav_proppatch (ctx, &ih);
		break;
	case SOUP_METHOD_ID_PUT:
		dav_put (ctx, &ih);
		break;
	case SOUP_METHOD_ID_LOCK:
		dav_lock (ctx, &ih);
		break;
	case SOUP_METHOD_ID_UNLOCK:
		dav_unlock (ctx, &ih);
		break;
	case SOUP_METHOD_ID_OPTIONS:
		dav_options (ctx, &ih);
		break;
	default:
		{
			gchar *methods;

			methods = get_supported_methods (ctx, &ih, ctx->path);
			soup_message_add_header (ctx->msg->response_headers, 
						 "Allow", 
						 methods);
			g_free (methods);

			return SOUP_METHOD_ID_UNKNOWN;
		}
	}

	return ctx->method_id;
}
