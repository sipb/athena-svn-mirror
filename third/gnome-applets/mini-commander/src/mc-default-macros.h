/*
 * Mini-Commander Applet
 * Copyright (C) 2002 Sun Microsystems Inc.
 *
 * Authors: Mark McLoughlin <mark@skynet.ie>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __MC_DEFAULT_MACROS_H__
#define __MC_DEFAULT_MACROS_H__

#include <glib/gmacros.h>

G_BEGIN_DECLS

typedef struct {
    char *pattern;
    char *command;
} MCDefaultMacro;

/* These are both the defaults stored in the GConfSchema
 * and the fallback defaults that are used if we are
 * having GConf problems.
 *
 * See mc-install-default-macros.c for how they get 
 * installed into the schemas.
 */
static const MCDefaultMacro mc_default_macros [] = {
	{ "^(http://.*)$",       "mozilla \\1" },
	{ "^(ftp://.*)",         "mozilla \\1" },
	{ "^(www\\..*)$",        "mozilla http://\\1" },
	{ "^(ftp\\..*)$",        "mozilla ftp://\\1" },
	{ "^lynx: *(.*)$",       "gnome-terminal -e \"sh -c 'lynx \\1'\"" },
	{ "^term: *(.*)$",       "gnome-terminal -e \"sh -c '\\1'\"" },
	{ "^xterm: *(.*)$",      "xterm -e sh -c '\\1'" },
	{ "^nxterm: *(.*)$",     "nxterm -e sh -c '\\1'" },
	{ "^rxvt: *(.*)$",       "rxvt -e sh -c '\\1'" },
	{ "^t$",                 "gnome-terminal" },
	{ "^nx$",                "nxterm" },
	{ "^n$",                 "netscape" },

	/* altavista search */
	{ "^av: *(.*)$",         "gnome-moz-remote --newwin http://www.altavista.net/cgi-bin/query?pg=q\\&kl=XX\\&q=$(echo '\\1'|sed -e ': p;s/+/%2B/;t p;: s;s/\\ /+/;t s;: q;s/\\\"/%22/;t q')" },

	/* yahoo search */
	{ "^yahoo: *(.*)$",      "gnome-moz-remote --newwin http://ink.yahoo.com/bin/query?p=$(echo '\\1'|sed -e ': p;s/+/%2B/;t p;: s;s/\\ /+/;t s;: q;s/\\\"/%22/;t q')" },

	/* freshmeat search */
	{ "^fm: *(.*)$",         "gnome-moz-remote --newwin http://core.freshmeat.net/search.php3?query=$(echo '\\1'|tr \" \" +)" },

	/* dictionary search */
	{ "^dictionary: *(.*)$", "gnome-moz-remote --newwin http://www.dictionary.com/cgi-bin/dict.pl?term=\\1" },
};

G_END_DECLS

#endif /* __MC_DEFAULT_MACROS_H__ */