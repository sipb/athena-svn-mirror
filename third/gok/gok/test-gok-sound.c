/*
* test-gok-sound.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include <gnome.h>
#include "gok-sound.h"

gint main (gint argc, gchar* argv[])
{
    gnome_init ("test-gok-sound", "0.1", argc, argv);
    gok_sound_initialize ();
    gok_sound_play ("goksound1.wav");
    sleep (2);
    gok_sound_play ("goksound2.wav");
    sleep (2);
    gok_sound_play ("goksound1.wav");
    sleep (2);
    gok_sound_play ("goksound2.wav");
    gok_sound_shutdown ();

    return 0;
}
