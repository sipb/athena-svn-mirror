#ifndef EFFECT_H
#define EFFECT_H

struct EffectPluginData
{
	GList *effect_list;
	EffectPlugin *current_effect_plugin;
	/* FIXME: Needed? */
	gboolean playing;
	gboolean paused;
};

GList *get_effect_list(void);
void set_current_effect_plugin(int i);
void effect_about(int i);
void effect_configure(int i);

#endif
