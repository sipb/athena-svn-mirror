/* $Id: regexp.h,v 1.1.1.1 2001-01-16 15:25:48 ghudson Exp $
 *
 * We need this little header to help distinguish whether to use the REGEX
 * library installed in the system, or to include our own version (the GNU
 * version to be exact.)
 *
 * Copyright (C) 2000  Robert James Kaes (rjkaes@flarenet.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifdef USE_GNU_REGEX
#  include "gnuregex.h"
#else
#  include <regex.h>
#endif
