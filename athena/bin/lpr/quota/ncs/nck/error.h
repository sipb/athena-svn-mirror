/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 * Definitions of types/constants for error text database routines.
 *
 * The database consists of two major pieces -- a header and a text
 * section.  The text section consists of a collection of "module tables".
 * The text section begins immediately after the header.
 *
 * The header is a table mapping from [subsystem, module] into the offset
 * from the beginning of the file (into the text section) of the
 * subsystem/module's module table.  Only valid [subsystem, module]
 * pairs appear in the table.
 *
 * A module table contains the number of the lowest and highest error
 * codes for that module, the text name of the module and the text name
 * of the subsystem.  (Yes, the same subsystem name thus appears in
 * multiple places.)  Following this information is the text of all
 * the module's error codes, in order separated by NULL characters.
 *
 * Note that all integers are stored in big-endian (most significant byte
 * at lower offset) format.
 *
 * Format of error database:
 *
 *              |<------ 16 ----->|
 *      
 *              +-----------------+                      
 *              |                 |
 *              +     version     +
 *              |                 |
 *              +-----------------+
 *              |                 |
 *              +  header size    +   Number of header entries that follow
 *              |                 |
 *              +--------+--------+
 *              | subsys   module |   Header entry #0
 *              +--------+--------+
 *              |       pad       |
 *              +-----------------+                      
 *              |                 |
 *              +      offset     +
 *              |                 |
 *              +-----------------+ 
 *              | subsys   module |   Header entry #1
 *              +--------+--------+
 *              |       pad       |
 *              +-----------------+    
 *              |                 |
 *              +      offset     +
 *              |                 |
 *              +-----------------+ 
 *              | subsys   module |   Header entry #2
 *              +--------+--------+
 *              |       pad       |
 *              +-----------------+
 *              |                 |
 *              +      offset     +
 *              |                 |
 *              +-----------------+     
 *      
 *              :                 :
 *      
 *              :                 :
 *      
 *              +-----------------+     
 *              |    min code     |   Module table #0
 *              +-----------------+
 *              |    max code     |
 *              +-----------------+
 *              |                 |
 *              :   subsys name   :
 *              |   (64 bytes)    |
 *              +-----------------+
 *              |                 |
 *              :   module name   :
 *              |   (64 bytes)    |
 *              +-----------------+
 *              |                 |
 *              |   error text    |
 *              :    strings      :
 *              :    sep'd by     :
 *              :     nulls       :
 *              |                 |
 *              +-----------------+
 *              |    min code     |   Module table #2
 *              +-----------------+
 *              |    max code     |
 *              +-----------------+
 *              |                 |
 *              :   subsys name   :
 *              |   (64 bytes)    |
 *              +-----------------+
 *              |                 |
 *              :   module name   :
 *              |   (64 bytes)    |
 *              +-----------------+
 *              |                 |
 *              |   error text    |
 *              :    strings      :
 *              :    sep'd by     :
 *              :     nulls       :
 *              |                 |
 *              +-----------------+
 *
 */


/*
 * Where the binary error data base lives.
 */

#define ERROR_VERSION 1

#if ! defined(UNIX) && (defined(apollo) || defined(BSD) || defined(SYS5))
#  define UNIX
#endif

#if 0
#ifdef UNIX
#  ifdef apollo
#    define ERROR_DATABASE "/usr/apollo/lib/stcode.db"
#  else
#    define ERROR_DATABASE "/usr/lib/stcode.db"
#  endif
#endif
#else
/* XXX real place, please */
#define ERROR_DATABASE "/afs/athena.mit.edu/mit/ncs/src/nck/nck/stcode.db"
#endif

#ifdef vms
#  define ERROR_DATABASE "ncs$exe:stcode.db"
#endif

#ifdef MSDOS
#  define ERROR_DATABASE "\\ncs\\stcode.db"
#endif

/*
 * MAX_SUBSYS and MAX_MODULE are the highest values that a subsys or module code
 * can take on.
 */

#define MAX_SUBSYS 0x7f
#define MAX_MODULE 0xff


/*
 * MAX_SUBMODS is the maximum number of valid [subsystem, module] pairs we can
 * hold.
 */

#define MAX_SUBMODS 1024


/*
 * The header.  Consists of the "header header" (version and count) and "count"
 * number of "header entries".
 */

struct hdr_hdr_t {
    long version;
    long count;
};

struct hdr_elt_t {
    short submod;
    short pad;
    long offset;
};

struct hdr_t {
    struct hdr_hdr_t h;
    struct hdr_elt_t ents[1];
};

/*
 * The header of a module table.
 */

struct modhdr_t {
    short min_code;
    short max_code;
    char ss_name[64];
    char mod_name[64];
};


#if defined(vax) || defined(i8086)

#define swab_16(p) { \
    register char t; \
    t = ((char *) (p))[0]; \
    ((char *) (p))[0] = ((char *) (p))[1]; \
    ((char *) (p))[1] = t; \
}

#define swab_32(p) { \
    register char t; \
    t = ((char *) (p))[0]; \
    ((char *) (p))[0] = ((char *) (p))[3]; \
    ((char *) (p))[3] = t; \
    t = ((char *) (p))[1]; \
    ((char *) (p))[1] = ((char *) (p))[2]; \
    ((char *) (p))[2] = t; \
}

#else

#define swab_16(p)
#define swab_32(p)

#endif 
