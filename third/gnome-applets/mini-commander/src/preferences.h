#ifndef PREFERENCES_H__
#define PREFERENCES_H__

#include <sys/types.h>
#include <regex.h>
#include <panel-applet.h>
#include "mini-commander_applet.h"

#define MAX_COMMAND_LENGTH        500
#define MAX_NUM_MACROS            99
#define MAX_NUM_MACRO_PARAMETERS  100
#define MAX_MACRO_PATTERN_LENGTH  25

struct struct_properties
{
    int show_time;
    int show_date;
    int show_handle;
    int show_frame;
    int show_default_theme;
    int auto_complete_history;
    int normal_size_x;
    int normal_size_y;
    int reduced_size_x;
    int reduced_size_y;
    int cmd_line_size_y;
    int flat_layout;
    int cmd_line_color_fg_r;
    int cmd_line_color_fg_g;
    int cmd_line_color_fg_b;
    int cmd_line_color_bg_r;
    int cmd_line_color_bg_g;
    int cmd_line_color_bg_b;
    char *macro_pattern[MAX_NUM_MACROS];
    char *macro_command[MAX_NUM_MACROS];
    regex_t *macro_regex[MAX_NUM_MACROS];
};

properties * load_session(MCData *mcdata);

gint save_session_signal(GtkWidget *widget, const char *privcfgpath, const char *globcfgpath);

void properties_box (BonoboUIComponent *uic,
		     MCData            *mcdata,
		     const char        *verbname);

#endif
