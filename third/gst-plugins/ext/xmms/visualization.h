#ifndef VISUALIZATION_H
#define VISUALIZATION_H

struct VisPluginData
{
	GList *vis_list;
	GList *enabled_list;
	gboolean playback_started;	
};

GList *get_vis_list(void);
GList *get_vis_enabled_list(void);
void enable_vis_plugin(int i, gboolean enable);
void vis_disable_plugin(VisPlugin *vp);
void vis_about(int i);
void vis_configure(int i);
void vis_playback_start(void);
void vis_playback_stop(void);
gboolean vis_enabled(int i);
gchar *vis_stringify_enabled_list(void);
void vis_enable_from_stringified_list(gchar * list);
void vis_send_data(gint16 pcm_data[2][512], gint nch);

#endif
