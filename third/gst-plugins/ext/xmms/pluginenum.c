/*  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "xmms.h"

#ifdef HPUX
#include <dl.h>
#else
#include <dlfcn.h>
#endif

#ifdef HPUX
# define SHARED_LIB_EXT ".sl"
#else
# define SHARED_LIB_EXT ".so"
#endif

#ifndef RTLD_NOW
# define RTLD_NOW 0
#endif


gchar *plugin_dir_list[] =
{
	PLUGINSUBS,
	NULL
};

extern struct InputPluginData *ip_data;
extern struct OutputPluginData *op_data;
extern struct EffectPluginData *ep_data;
extern struct GeneralPluginData *gp_data;
extern struct VisPluginData *vp_data;

void scan_plugins(char *dirname);
void add_plugin(gchar * filename);

static int d_iplist_compare(const void *a, const void *b)
{
	return strcmp(((char *) a), ((char *) b));
}

static int inputlist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((InputPlugin *) a)->description, ((InputPlugin *) b)->description);
}

static int outputlist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((OutputPlugin *) a)->description, ((OutputPlugin *) b)->description);
}

static int effectlist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((EffectPlugin *) a)->description, ((EffectPlugin *) b)->description);
}

static int generallist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((GeneralPlugin *) a)->description, ((GeneralPlugin *) b)->description);
}

static int vislist_compare_func(const void *a, const void *b)
{
	return strcasecmp(((VisPlugin *) a)->description, ((VisPlugin *) b)->description);
}

void init_plugins(void)
{
	gchar *dir, *temp, *temp2;
	GList *node, *disabled_iplugin_names = NULL;
	OutputPlugin *op;
	InputPlugin *ip;
	EffectPlugin *ep;
	gint dirsel = 0;
#ifdef NATIVE_XMMS
	if (cfg.disabled_iplugins)
	{
		temp = cfg.disabled_iplugins;
		while ((temp2 = strchr(temp, ',')) != NULL)
		{
			(*temp2) = '\0';
			disabled_iplugin_names = g_list_append(disabled_iplugin_names, g_strdup(temp));
			temp = temp2 + 1;
		}
		disabled_iplugin_names = g_list_append(disabled_iplugin_names, g_strdup(temp));
		g_free(cfg.disabled_iplugins);
		cfg.disabled_iplugins = NULL;
	}
#endif

	ip_data = g_malloc0(sizeof (struct InputPluginData));
	op_data = g_malloc0(sizeof (struct OutputPluginData));
	ep_data = g_malloc0(sizeof (struct EffectPluginData));
	gp_data = g_malloc0(sizeof (struct GeneralPluginData));
	vp_data = g_malloc0(sizeof (struct VisPluginData));


#ifndef DISABLE_USER_PLUGIN_DIR
	dir = g_strconcat(g_get_home_dir(), "/.xmms/Plugins", NULL);
	scan_plugins(dir);
	g_free(dir);

	/*
	 * Having directories below ~/.xmms/Plugins is depreciated and
	 * might be removed at some point.
	 */

	/*
	 * This is in a separate loop so if the user puts them in the
	 * wrong dir we'll still get them in the right order (home dir
	 * first                                                - Zinx
	 */
	while (plugin_dir_list[dirsel])
	{
		dir = g_strconcat(g_get_home_dir(), "/.xmms/Plugins/", plugin_dir_list[dirsel++], NULL);
		scan_plugins(dir);
		g_free(dir);
	}
	dirsel = 0;
#endif

	while (plugin_dir_list[dirsel])
	{
		dir = g_strconcat(PLUGIN_DIR, "/", plugin_dir_list[dirsel++], NULL);
		scan_plugins(dir);
		g_free(dir);
	}

	op_data->output_list = g_list_sort(op_data->output_list, outputlist_compare_func);
	if (!op_data->current_output_plugin && g_list_length(op_data->output_list))
		op_data->current_output_plugin = (OutputPlugin *) op_data->output_list->data;
	ip_data->input_list = g_list_sort(ip_data->input_list, inputlist_compare_func);
	ep_data->effect_list = g_list_sort(ep_data->effect_list, effectlist_compare_func);
	if (!ep_data->current_effect_plugin && g_list_length(ep_data->effect_list))
		ep_data->current_effect_plugin = (EffectPlugin *) ep_data->effect_list->data;
	gp_data->general_list = g_list_sort(gp_data->general_list, generallist_compare_func);
	gp_data->enabled_list = NULL;
	vp_data->vis_list = g_list_sort(vp_data->vis_list, vislist_compare_func);
	vp_data->enabled_list = NULL;
#ifdef NATIVE_XMMS
	general_enable_from_stringified_list(cfg.enabled_gplugins);
	vis_enable_from_stringified_list(cfg.enabled_vplugins);
	if (cfg.enabled_gplugins)
	{
		g_free(cfg.enabled_gplugins);
		cfg.enabled_gplugins = NULL;
	}
#endif

	node = op_data->output_list;
	while (node)
	{
		op = (OutputPlugin *) node->data;
#ifdef NATIVE_XMMS
		if (!strcmp(cfg.outputplugin, op->filename))
			op_data->current_output_plugin = op;
#endif
		if (op->init)
			op->init();
		node = node->next;
	}

	node = ep_data->effect_list;
	while (node)
	{
		ep = (EffectPlugin *) node->data;
#ifdef NATIVE_XMMS
		if (!strcmp(cfg.effectplugin, ep->filename))
		{
			ep_data->current_effect_plugin = ep;
		}
#endif
		if (ep->init)
			ep->init();
		node = node->next;
	}

	node = ip_data->input_list;
	while (node)
	{
		ip = (InputPlugin *) node->data;
		temp = g_basename(ip->filename);
#ifdef NATIVE_XMMS
		if (g_list_find_custom(disabled_iplugin_names, temp, d_iplist_compare))
			disabled_iplugins = g_list_append(disabled_iplugins, ip);
#endif
		if (ip->init)
			ip->init();
		node = node->next;
	}

	node = disabled_iplugin_names;
	while (node)
	{
		g_free(node->data);
		node = node->next;
	}
	g_list_free(disabled_iplugin_names);

}

void add_plugin(gchar * filename)
{
#ifdef HPUX
	shl_t *h;
#else
	void *h;
#endif
	void *(*gpi) (void);

#ifndef DISABLE_USER_PLUGIN_DIR
	/*
	 * erg.. gotta check 'em all, surely there's a better way
	 *                                                 - Zinx
	 */
	{
		GList *l;
		gchar *base_filename = g_basename(filename);

		for (l = ip_data->input_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((InputPlugin*)l->data)->filename)))
				return;
		}

		for (l = op_data->output_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((OutputPlugin*)l->data)->filename)))
				return;
		}

		for (l = ep_data->effect_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((EffectPlugin*)l->data)->filename)))
				return;
		}

		for (l = gp_data->general_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((GeneralPlugin*)l->data)->filename)))
				return;
		}

		for (l = vp_data->vis_list; l; l = l->next)
		{
			if (!strcmp(base_filename, g_basename(((VisPlugin*)l->data)->filename)))
				return;
		}
	}
#endif
#ifdef HPUX
	if ((h = shl_load(filename, BIND_DEFERRED, 0)) != NULL)
		/* use shl_load family of functions on HP-UX 
		   HP-UX does not support dlopen on 32-bit
		   PA-RISC executables */
#else
	if ((h = dlopen(filename, RTLD_NOW)) != NULL)
#endif /* HPUX */
	{
#ifdef HPUX
 		if ((shl_findsym(&h, "get_iplugin_info", TYPE_PROCEDURE, (void*) &gpi)) == 0)
#else
		if ((gpi = dlsym(h, "get_iplugin_info")) != NULL)
#endif
		{
			InputPlugin *p;

			p = (InputPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);
			/*p->get_vis_type = input_get_vis_type; */
			/*p->add_vis_pcm = input_add_vis_pcm; */
#ifdef NATIVE_XMMS
			p->set_info = playlist_set_info;
#endif
			/*p->set_info_text = input_set_info_text; */

			ip_data->input_list = g_list_prepend(ip_data->input_list, p);
		}
#ifdef HPUX
		else if ((shl_findsym(&h, "get_oplugin_info", TYPE_PROCEDURE, (void*) &gpi)) == 0)
#else
		else if ((gpi = dlsym(h, "get_oplugin_info")) != NULL)
#endif
		{
			OutputPlugin *p;

			p = (OutputPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);
			op_data->output_list = g_list_prepend(op_data->output_list, p);
		}
#ifdef HPUX
		else if ((shl_findsym(&h, "get_eplugin_info", TYPE_PROCEDURE, (void *) &gpi)) == 0)
#else
		else if ((gpi = dlsym(h, "get_eplugin_info")) != NULL)
#endif
		{
			EffectPlugin *p;

			p = (EffectPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);
			ep_data->effect_list = g_list_prepend(ep_data->effect_list, p);
		}
#ifdef HPUX
		else if ((shl_findsym(&h, "get_gplugin_info", TYPE_PROCEDURE, (void*) &gpi)) == 0)
#else
		else if ((gpi = dlsym(h, "get_gplugin_info")) != NULL)
#endif 
		{
			GeneralPlugin *p;

			p = (GeneralPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);
#ifdef NATIVE_XMMS
			p->xmms_session = ctrlsocket_get_session_id();
#endif
			gp_data->general_list = g_list_prepend(gp_data->general_list, p);
		}
#ifdef HPUX
		else if ((shl_findsym(&h, "get_vplugin_info", TYPE_PROCEDURE, (void *) &gpi)) == 0)
#else
		else if ((gpi = dlsym(h, "get_vplugin_info")) != NULL)
#endif
		{
			VisPlugin *p;

			p = (VisPlugin *) gpi();
			p->handle = h;
			p->filename = g_strdup(filename);
#ifdef NATIVE_XMMS
			p->xmms_session = ctrlsocket_get_session_id();
#endif
			/*p->disable_plugin = vis_disable_plugin; */
			vp_data->vis_list = g_list_prepend(vp_data->vis_list, p);
		}
		else
		{
#ifdef HPUX
                        shl_unload(h);
#else
			dlclose(h);
#endif
		}
	}
	else
#ifdef HPUX
		perror("Error loading plugin!"); 
#else
		fprintf(stderr, "%s\n", dlerror());
#endif
}

void scan_plugins(char *dirname)
{
	gchar *filename, *ext;
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;

	dir = opendir(dirname);
	if (!dir)
		return;

	while ((ent = readdir(dir)) != NULL)
	{
		filename = g_strdup_printf("%s/%s", dirname, ent->d_name);
		if (!stat(filename, &statbuf) && S_ISREG(statbuf.st_mode) &&
		    (ext = strrchr(ent->d_name, '.')) != NULL)
			if (!strcmp(ext, SHARED_LIB_EXT))
				add_plugin(filename);
		g_free(filename);
	}
	closedir(dir);
}

void cleanup_plugins(void)
{
#ifdef HPUX
        shl_t *h;
#endif

	InputPlugin *ip;
	OutputPlugin *op;
	EffectPlugin *ep;
	GeneralPlugin *gp;
	VisPlugin *vp;
	GList *node, *next;

	if (get_input_playing())
		input_stop();


#ifdef NATIVE_XMMS
	if (disabled_iplugins)
		g_list_free(disabled_iplugins);
#endif
	node = get_input_list();
	while (node)
	{
		ip = (InputPlugin *) node->data;
		if (ip && ip->cleanup)
		{
			ip->cleanup();
			while(g_main_iteration(FALSE));

		}
#ifdef HPUX
                h = ip->handle;
                shl_unload(*h);
#else
		dlclose(ip->handle);
#endif
		node = node->next;
	}
	if (ip_data->input_list)
		g_list_free(ip_data->input_list);
	g_free(ip_data);

	node = get_output_list();
	while (node)
	{
		op = (OutputPlugin *) node->data;
#ifdef HPUX
                h = op->handle;
                shl_unload(*h);
#else
		dlclose(op->handle);
#endif
		node = node->next;
	}
	if (op_data->output_list)
		g_list_free(op_data->output_list);
	g_free(op_data);

	node = get_effect_list();
	while (node)
	{
		ep = (EffectPlugin *) node->data;
		if (ep && ep->cleanup)
		{
			ep->cleanup();
			while(g_main_iteration(FALSE));

		}
#ifdef HPUX
                h = ep->handle;
                shl_unload(*h);
#else
		dlclose(ep->handle);
#endif
		node = node->next;
	}
	if (ep_data->effect_list)
		g_list_free(ep_data->effect_list);
	g_free(ep_data);

	node = get_general_enabled_list();
	while (node)
	{
		gp = (GeneralPlugin *) node->data;
		next = node->next;
		enable_general_plugin(g_list_index(gp_data->general_list, gp), FALSE);
		node = next;
	}
	if (gp_data->enabled_list)
		g_list_free(gp_data->enabled_list);

	while(g_main_iteration(FALSE));
	
	node = get_general_list();
	while (node)
	{
		gp = (GeneralPlugin *) node->data;
#ifdef HPUX
                h = gp->handle;
                shl_unload(*h);
#else
		dlclose(gp->handle);
#endif
		node = node->next;
	}
	if (gp_data->general_list)
		g_list_free(gp_data->general_list);

	node = get_vis_enabled_list();
	while (node)
	{
		vp = (VisPlugin *) node->data;
		next = node->next;
		enable_vis_plugin(g_list_index(vp_data->vis_list, vp), FALSE);
		node = next;
	}
	if (vp_data->enabled_list)
		g_list_free(vp_data->enabled_list);
	
	while(g_main_iteration(FALSE));
	
	node = get_vis_list();
	while (node)
	{
		vp = (VisPlugin *) node->data;
#ifdef HPUX
                h = vp->handle;
                shl_unload(*h);
#else
		dlclose(vp->handle);
#endif
		node = node->next;
	}
	if (vp_data->vis_list)
		g_list_free(vp_data->vis_list);
	g_free(vp_data);

}
