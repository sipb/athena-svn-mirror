/* ACME
 * Copyright (C) 2001 Bastien Nocera <hadess@hadess.net>
 *
 * acme.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 * USA.
 */

#define DIALOG_TIMEOUT 1000	/* dialog timeout in ms */
#define VOLUME_STEP 6		/* percents for one volume button press */

#if defined(__powerpc__) && defined (__linux__)
#define USE_FBLEVEL
#else
#undef USE_FBLEVEL
#endif

enum {
	MUTE_KEY,
	VOLUME_DOWN_KEY,
	VOLUME_UP_KEY,
	POWER_KEY,
	EJECT_KEY,
	MEDIA_KEY,
	PLAY_KEY,
	PAUSE_KEY,
	STOP_KEY,
	PREVIOUS_KEY,
	NEXT_KEY,
	HOME_KEY,
	REFRESH_KEY,
	SEARCH_KEY,
	EMAIL_KEY,
	SLEEP_KEY,
	SCREENSAVER_KEY,
	FINANCE_KEY,
	HELP_KEY,
	WWW_KEY,
	GROUPS_KEY,
	CALCULATOR_KEY,
	RECORD_KEY,
	CLOSE_WINDOW_KEY,
	SHADE_WINDOW_KEY,
#ifdef USE_FBLEVEL
	BRIGHT_DOWN_KEY,
	BRIGHT_UP_KEY,
#endif
	HANDLED_KEYS,
};


static struct {
	int key_type;
	const char *key_config;
	const char *description;
	int key_code;
} keys[HANDLED_KEYS] = {
	{ MUTE_KEY, "/apps/acme/mute_key",
		N_("Mute key"), 166 },
	{ VOLUME_DOWN_KEY, "/apps/acme/volume_down_key",
		N_("Volume down key"), 165 },
	{ VOLUME_UP_KEY, "/apps/acme/volume_up_key",
		N_("Volume up key"), 158 },
	{ POWER_KEY, "/apps/acme/power_key",
		N_("Power key"), 222 },
	{ EJECT_KEY, "/apps/acme/eject_key",
		N_("Eject key"), 116 },
	{ MEDIA_KEY, "/apps/acme/media_key",
		N_("Media key"), 0 },
	{ PLAY_KEY, "/apps/acme/play_key",
		N_("Play key"), -1 },
	{ PAUSE_KEY, "/apps/acme/pause_key",
		N_("Pause key"), -1 },
	{ STOP_KEY, "/apps/acme/stop_key",
		N_("Stop (Audio) key"), -1 },
	{ PREVIOUS_KEY, "/apps/acme/previous_key",
		N_("Previous (Audio) key"), -1 },
	{ NEXT_KEY, "/apps/acme/next_key",
		N_("Next (Audio) key"), -1 },
	{ HOME_KEY, "/apps/acme/home_key",
		N_("My Home key"), -1 },
	{ REFRESH_KEY, "/apps/acme/refresh_key",
		N_("Refresh key"), -1 },
	{ SEARCH_KEY, "/apps/acme/search_key",
		N_("Search key"), -1 },
	{ EMAIL_KEY, "/apps/acme/email_key",
		N_("E-Mail key"), -1 },
	{ SLEEP_KEY, "/apps/acme/sleep_key",
		N_("Sleep key"), -1 },
	{ SCREENSAVER_KEY, "/apps/acme/screensaver_key",
		N_("Screensaver key"), -1 },
	{ FINANCE_KEY, "/apps/acme/finance_key",
		N_("Finance key"), -1 },
	{ HELP_KEY, "/apps/acme/help_key",
		N_("Help key"), -1 },
	{ WWW_KEY, "/apps/acme/www_key",
		N_("WWW key"), -1 },
	{ GROUPS_KEY, "/apps/acme/groups_key",
		N_("Groups key"), -1 },
	{ CALCULATOR_KEY, "/apps/acme/calculator_key",
		N_("Calculator key"), -1 },
	{ RECORD_KEY, "/apps/acme/record_key",
		N_("Record key"), -1 },
	{ CLOSE_WINDOW_KEY, "/apps/acme/close_window_key",
		N_("Close Window key"), -1 },
	{ SHADE_WINDOW_KEY, "/apps/acme/shade_window_key",
		N_("Shade Window key"), -1 },
#ifdef USE_FBLEVEL
	{ BRIGHT_DOWN_KEY, "/apps/acme/brightness_down",
		N_("Brightness down key"), 101 },
	{ BRIGHT_UP_KEY, "/apps/acme/brightness_up",
		N_("Brightness up key"), 212 },
#endif
};

