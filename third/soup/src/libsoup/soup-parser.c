/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-parser.c: Parser for SOAP requests/responses.
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <string.h>
#include <libxml/parser.h>

#include "soup-parser.h"

struct _SoupParamList {
	GHashTable *table;
};

struct _SoupParser {
	xmlDocPtr      xml_doc;
	xmlNodePtr     xml_root;
	xmlNodePtr     xml_body;
	xmlNodePtr     xml_method;
	SoupParamList *param_list;
	SoupFault     *fault;
};

/*
 * Public functions
 */

SoupParam *
soup_param_new (void)
{
	SoupParam *param;

	param = g_new0(SoupParam, 1);
	param->type = SOUP_PARAM_TYPE_STRING;

	return param;
}

SoupParam *
soup_param_new_full (const gchar *name, SoupParamType type, gpointer value)
{
	SoupParam *param;

	param = soup_param_new();
	soup_param_set_name(param, name);
	soup_param_set_type(param, type);
	soup_param_set_data(param, value);

	return param;
}

const gchar *
soup_param_get_name (SoupParam *param)
{
	g_return_val_if_fail(param != NULL, NULL);
	return (const gchar *) param->name;
}

void
soup_param_set_name (SoupParam *param, const gchar *new_name)
{
	g_return_if_fail(param != NULL);

	if (param->name)
		g_free((gpointer) param->name);
	param->name = g_strdup(new_name);
}

SoupParamType
soup_param_get_type (SoupParam *param)
{
	g_return_val_if_fail(param != NULL, SOUP_PARAM_TYPE_INVALID);

	return param->type;
}

void
soup_param_set_type (SoupParam *param, SoupParamType new_type)
{
	g_return_if_fail(param != NULL);

	if (param->type != new_type) {
		/* changing type, free current data */
		switch (param->type) {
		case SOUP_PARAM_TYPE_STRING :
			g_free(param->data);
			break;
		case SOUP_PARAM_TYPE_STRUCT :
			soup_param_list_free((SoupParamList *) param->data);
			break;
		default :
			return;
		}

		/* now set the new type */
		param->type = new_type;
		param->data = NULL;
	}
}

gpointer
soup_param_get_data (SoupParam *param)
{
	g_return_val_if_fail(param != NULL, NULL);

	return param->data;
}

void
soup_param_set_data (SoupParam *param, gpointer new_value)
{
	g_return_if_fail(param != NULL);

	switch (param->type) {
	case SOUP_PARAM_TYPE_STRING :
		g_free((gpointer) param->data);
		param->data = g_strdup((const gchar *) new_value);
		break;
	case SOUP_PARAM_TYPE_STRUCT :
		soup_param_list_free((SoupParamList *) param->data);
		param->data = (SoupParamList *) new_value;
		break;
	default :
		param->data = NULL;
	}
}

void
soup_param_free (SoupParam *param)
{
	g_return_if_fail(param != NULL);

	if (param->name)
		g_free((gpointer) param->name);

	if (param->type == SOUP_PARAM_TYPE_STRING)
		g_free((gpointer) param->data);
	else if (param->type == SOUP_PARAM_TYPE_STRUCT)
		soup_param_list_free((SoupParamList *) param->data);

	g_free((gpointer) param);
}

SoupParamList *
soup_param_list_new (void)
{
	SoupParamList *plist;

	plist = g_new0(SoupParamList, 1);
	plist->table = g_hash_table_new(g_str_hash, g_str_equal);

	return plist;
}

void
soup_param_list_add (SoupParamList *plist, SoupParam *param)
{
	gchar *old_value;
	gchar *name;

	g_return_if_fail(plist != NULL);
	g_return_if_fail(param != NULL);

	name = (gchar *) soup_param_get_name(param);
	old_value = (gchar *) g_hash_table_lookup(plist->table,
						  (gconstpointer) name);
	if (old_value) {
		return;
	}

	/* create new param */
	g_hash_table_insert(plist->table, (gpointer) name, param);
}

SoupParam *
soup_param_list_get_by_name (SoupParamList *plist, const gchar *name)
{
	SoupParam *param;

	g_return_val_if_fail(plist != NULL, NULL);
	g_return_val_if_fail(name != NULL, NULL);

	param = (SoupParam *) g_hash_table_lookup(plist->table,
						  (gconstpointer) name);
	if (param)
		return param;

	return NULL; /* not found */
}

void
soup_param_list_foreach (SoupParamList *plist,
			 SoupParamListFunc func,
			 gpointer user_data)
{
	g_return_if_fail(plist != NULL);
	g_return_if_fail(func != NULL);
	
	g_hash_table_foreach(plist->table, (GHFunc) func, user_data);
}

static void
destroy_param_in_list (gpointer key, gpointer value, gpointer user_data)
{
	soup_param_free((SoupParam *) value);
}

void
soup_param_list_free (SoupParamList *plist)
{
	g_return_if_fail(plist != NULL);

	g_hash_table_foreach(plist->table,
			     (GHFunc) destroy_param_in_list,
			     NULL);
	g_hash_table_destroy(plist->table);
	g_free((gpointer) plist);
}

static SoupParamList *
get_params_from_node (SoupParser *parser, xmlNodePtr xml_node)
{
	xmlNodePtr tmp;
	SoupParamList *param_list = NULL;

	g_return_val_if_fail(parser != NULL, NULL);
	g_return_val_if_fail(xml_node != NULL, NULL);

	for (tmp = xml_node->xmlChildrenNode; tmp; tmp = tmp->next) {
		SoupParam *param;

		if (!strcmp(tmp->name, "Fault")) {
			xmlNodePtr subnode;
			gchar *code = NULL;
			gchar *string = NULL;
			gchar *actor = NULL;
			gchar *detail = NULL;

			/* retrieve all info about the fault */
			for (subnode = tmp->xmlChildrenNode; 
			     subnode; 
			     subnode = subnode->next) {
				xmlChar *str;

				str = xmlNodeListGetString(parser->xml_doc, 
							   tmp->xmlChildrenNode,
							   1);

				if (!strcmp(subnode->name, "faultcode"))
					code = g_strdup(str);
				else if (!strcmp(subnode->name, "faultstring"))
					string = g_strdup(str);
				else if (!strcmp(subnode->name, "faultactor"))
					actor = g_strdup(str);
				else if (!strcmp(subnode->name, "detail"))
					detail = g_strdup(str); /* FIXME */
			}

			parser->fault = 
				soup_fault_new (code, string, actor, detail);

			/* free memory */
			if (code)
				g_free(code);
			if (string)
				g_free(string);
			if (actor)
				g_free(actor);
			if (detail)
				g_free(detail);

			continue;
		}

		/* regular parameters */
		if (!param_list)
			param_list = soup_param_list_new();

		param = soup_param_new();
		soup_param_set_name(param, tmp->name);
		if (tmp->xmlChildrenNode && tmp->xmlChildrenNode == tmp->last
			&& tmp->xmlChildrenNode->type == XML_TEXT_NODE) {
			soup_param_set_type(param, SOUP_PARAM_TYPE_STRING);
			soup_param_set_data(param, xmlNodeGetContent(tmp));
		}
		else {
			soup_param_set_type(param, SOUP_PARAM_TYPE_STRUCT);
			soup_param_set_data(
				param,
				(gpointer) get_params_from_node (parser, tmp));
		}

		soup_param_list_add(param_list, param);
	}

	return param_list;
}

static SoupParser *
soup_parser_construct (SoupParser *parser)
{
	g_return_val_if_fail(parser != NULL, NULL);

	parser->xml_root = xmlDocGetRootElement(parser->xml_doc);

	/* check the validity of the XML document */
	if (strcmp(parser->xml_root->name, "Envelope") != 0) {
		soup_parser_free(parser);
		return NULL;
	}

	if (parser->xml_root->xmlChildrenNode) {
		parser->xml_body = parser->xml_root->xmlChildrenNode;
		if (strcmp(parser->xml_body->name, "Body") != 0) {
			soup_parser_free(parser);
			return NULL;
		}

		/* retrieve method name */
		parser->xml_method = parser->xml_body->xmlChildrenNode;

		/* read all parameters */
		if (parser->xml_method)
			parser->param_list = 
				get_params_from_node(parser, 
						     parser->xml_method);
	}
	else {
		parser->xml_body = NULL;
		parser->xml_method = NULL;
		parser->param_list = NULL;
	}

	return parser;
}

SoupParser *
soup_parser_new_from_string (const gchar *str)
{
	SoupParser *parser;

	g_return_val_if_fail(str != NULL, NULL);

	/* create the object */
	parser = g_new0(SoupParser, 1);
	parser->xml_doc = xmlParseMemory((char *) str, strlen(str));
	if (!parser->xml_doc) {
		soup_parser_free(parser);
		return NULL;
	}

	return soup_parser_construct(parser);
}

SoupParser *
soup_parser_new_from_data_buffer (SoupDataBuffer *buf)
{
	SoupParser *parser;
	gchar *str;
	
	g_return_val_if_fail(buf != NULL, NULL);

	str = g_strndup (buf->body, buf->length);
	parser = soup_parser_new_from_string (str);
	g_free (str);

	if (!parser)
		return NULL;
	
	return soup_parser_construct(parser);
}

SoupFault *
soup_parser_get_fault (SoupParser *parser)
{
	g_return_val_if_fail(parser != NULL, NULL);

	return parser->fault;
}

const gchar *
soup_parser_get_method_name (SoupParser *parser)
{
	g_return_val_if_fail(parser != NULL, NULL);
	g_return_val_if_fail(parser->xml_method != NULL, NULL);
	return parser->xml_method->name;
}

SoupParamList *
soup_parser_get_param_list (SoupParser *parser)
{
	g_return_val_if_fail(parser != NULL, NULL);
	return parser->param_list;
}

SoupParam *
soup_parser_get_param_by_name (SoupParser *parser, const gchar *name)
{
	g_return_val_if_fail(parser != NULL, NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return soup_param_list_get_by_name(parser->param_list, name);
}

void
soup_parser_free (SoupParser *parser)
{
	g_return_if_fail(parser != NULL);

	if (parser->param_list)
		soup_param_list_free(parser->param_list);

	if (parser->fault)
		soup_fault_free(parser->fault);

	if (parser->xml_doc)
		xmlFreeDoc(parser->xml_doc);

	g_free((gpointer) parser);
}
