/*      lb_admin.c, dds/ls, pato, 09/09/86
 *      Location Broker - Admin Tool
 *
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
 */

#include "std.h"
#include <ctype.h>

#include "pbase.h"

#ifdef DSEE
#   include "$(socket.idl).h"
#   include "$(lb.idl).h"
#   include "$(uuid.idl).h"
#   include "$(rrpc.idl).h"
#else
#   include "socket.h"
#   include "lb.h"
#   include "uuid.h"
#   include "rrpc.h"
#endif

#include "pfm.h"

#ifdef apollo
#   include <apollo/pad.h>
#   include <apollo/ios.h>
#   include <apollo/ec2.h>
#endif

#ifdef DOMAIN_DIALOG
#   include <apollo/dialog.h>
#   include "dynum.ins.c"
#   ifdef DSEE
#       include "$(lb_admin.dps).ins.c"
#   else
#       include "lb_admin.ins.c"
#   endif
#endif

/*
** TEMPORARY until nidl starts generating the correct type for bitsets
*/
#define lb_$server_flag_local_mask lb_$server_flag_local

/*
** End of TEMPORARY
*/

#define version "lb_admin/nck version 1.5.1"

#include "lb_args.h"
#include "glb_p.h"


#define STATUS_OK(s) ((s)==NULL || (s)->all == status_$ok)
#define STATUS(s) ((s)->all)
#define TRUE 1
#define FALSE 0

/*
** Internal Exception handling macros
*/

#define EXCEPTION_RANGE \
    { \
    pfm_$cleanup_rec crec; \
    status_$t exception_status; \
    exception_status = pfm_$p_cleanup(&crec);

#define EXCEPTION_END \
        if (exception_status.all != pfm_$cleanup_set) { \
            pfm_$enable(); \
            pfm_$enable_faults(); \
        } else { \
            pfm_$p_rls_cleanup(&crec, &exception_status); \
        } \
    }

#define EXCEPTION_CASE if (exception_status.all != pfm_$cleanup_set) 

#define NORMAL_CASE if (exception_status.all == pfm_$cleanup_set)


#define MAX_OBJECT_LEN 40

typedef int (*function)();

typedef struct {
        int done;
        char command[256];
        function op;
        int num_args;
        char **args;
} command_buf;

typedef struct {
        uuid_$t uuid;
        char uuidstr[MAX_OBJECT_LEN];
} menu_ent;

/*
** Commands:
*/

extern int tty_do_help();
extern int tty_do_quit();
extern int tty_do_nogood();
extern int tty_do_nil();
extern int tty_do_register();
extern int tty_do_set_broker();
extern int tty_do_use_broker();
extern int tty_do_unregister();
extern int tty_do_lookup();
extern int tty_do_set_timeout();
extern int tty_do_garbage_collect();
extern int help_help();
extern int help_all();
extern int help_quit();
extern int help_register();
extern int help_set_broker();
extern int help_use_broker();
extern int help_unregister();
extern int help_lookup();
extern int help_set_timeout();
extern int help_garbage_collect();

static struct {
        char    *name;
        function code;
        function help;
        int     min;
} commands[] = {
        { "add",                tty_do_register,        help_register,          1 },
        { "clean",              tty_do_garbage_collect, help_garbage_collect,   1 }, 
        { "delete",             tty_do_unregister,      help_unregister,        1 },
        { "register",           tty_do_register,        help_register,          1 },
        { "unregister",         tty_do_unregister,      help_unregister,        1 },
        { "set_broker",         tty_do_set_broker,      help_set_broker,        1 },
        { "set_timeout",        tty_do_set_timeout,     help_set_timeout,       4 },
        { "use_broker",         tty_do_use_broker,      help_use_broker,        2 },
        { "lookup",             tty_do_lookup,          help_lookup,            1 },
        { "exit",               tty_do_quit,            help_quit,              1 },
        { "quit",               tty_do_quit,            help_quit,              1 },
        { "help",               tty_do_help,            help_help,              1 },
        { "?",                  tty_do_help,            help_help,              1 },
        { "",                   tty_do_nil,             tty_do_nil,             0 },
        { "garbage_collect",    tty_do_garbage_collect, help_garbage_collect,   1 }, 
        { "gc",                 tty_do_garbage_collect, help_garbage_collect,   2 }, 
        { "*",                  tty_do_nogood,          help_all,               1 },
        { NULL,                 tty_do_nogood,          tty_do_nil,             0 }
};

/*
** Operation constants for process_register_parms().  Basic syntax is
** the same, but each operation is slightly different.
*/
#define OP_LOOKUP 0
#define OP_REGISTER 1
#define OP_UNREGISTER 2

/*
** State variables shared between TTY oriented interface and
** Dialog based interface
*/
#define MAX_RESULTS 100L
#define DIALOG_EC 0
#define IOS_EC 1
#define N_ECS 2

/*
** Broker constants (used to identify target)
*/
#define LOCAL_BROKER 0
#define GLOBAL_BROKER 1

extern uuid_$t          uuid_$nil;
static char             cur_object[MAX_OBJECT_LEN] = { "*" };
static char             cur_obj_type[MAX_OBJECT_LEN] = { "*" };
static char             cur_obj_interface[MAX_OBJECT_LEN] = { "*" };
static uuid_$t          cur_obj;
static uuid_$t          cur_obj_typ;
static uuid_$t          cur_obj_int;
static char             cur_annotation[65] = { "" };
static char             cur_location[256] = { "" };
static char             cur_broker[256] = { "" };
static lb_$server_flag_t cur_service_options = lb_$server_flag_local_mask;
static boolean          must_prompt = TRUE;
static int              use_global_broker = FALSE;
static boolean          verbose = TRUE;
static boolean          verb_flag = TRUE;
static u_long           use_short_timeouts = TRUE;
/*
** Socket addr for the local broker we are talking to.
*/
socket_$addr_t  local_broker_addr;
u_long          local_broker_addr_len;
socket_$addr_t  our_broker_addr;
u_long          our_broker_addr_len;
socket_$addr_t  global_broker_addr;
u_long          global_broker_addr_len;

boolean         local_broker_is_thishost = TRUE;

#ifdef DOMAIN_DIALOG
task_activate ( task_id, status )
    int           task_id;
    status_$t *status;
{
    short   a_count;
    short   i;
                               
    status->all = status_$ok;

    dp_$task_get_activate_count(task_id,&a_count,status);

    for (i = 0; i < a_count; i++)
        dp_$task_deactivate(task_id,status);

    dp_$task_activate(task_id,status);
}               

dp_$string_array_t  new_strings; 

build_msg ( status )
    status_$t *status;
{   
    status_$t st;
    char msg[256];
    socket_$string_t name;
    u_long namelen = sizeof(name);
    u_long port;

    if (use_global_broker)
    {
        socket_$to_name(&global_broker_addr, global_broker_addr_len, 
                    name, &namelen, &port, &st);
        if (!STATUS_OK (&st))
        {
            name[0] = '/0';
            namelen = 0;
        }
        sprintf(msg,"Entries @ global broker: %.*s", namelen, name);
    }
    else
    {
        socket_$to_name(&local_broker_addr, local_broker_addr_len, 
                    name, &namelen, &port, &st);
        if (!STATUS_OK (&st))
        {
            name[0] = '/0';
            namelen = 0;
        }
        sprintf(msg,"Entries @ local broker: %.*s", namelen, name);
    }

    new_strings[0].cur_len = strlen(msg);
    new_strings[0].max_len = 80;
    new_strings[0].chars_p = msg;
    dp_$msg_set_value(loc_msg_t, new_strings,(short)1,status);
}
#endif


main ( argc, argv )
    int argc;
    char *argv[];
{

    lb_$process_args_i(argc, argv, &glb_$uid, &glb_$type_uid, version);

    if (argc > 1)
        if (match_command("-nq", argv[1], 2))
            verb_flag = FALSE;

    init();
    run_test();
}

init ( )
{
    status_$t st;
    socket_$net_addr_t naddr;
    u_long nlen = sizeof naddr;

    pfm_$init((u_long) pfm_$init_signal_handlers);

#if defined(apollo) || defined(MSDOS)
    socket_$inq_my_netaddr((u_long) socket_$dds, &naddr, &nlen, &st);
#else
    socket_$inq_my_netaddr((u_long) socket_$internet, &naddr, &nlen, &st);
#endif
    socket_$set_netaddr(&local_broker_addr, &local_broker_addr_len, &naddr,
                nlen, &st);

    bcopy(&local_broker_addr, &our_broker_addr, (int) local_broker_addr_len);
    our_broker_addr_len = local_broker_addr_len;

    cur_obj_int = uuid_$nil;
    cur_obj_typ = uuid_$nil;
    cur_obj = uuid_$nil;
    lb_$use_short_timeouts(use_short_timeouts);
    if (!isatty(0))
        must_prompt = 0;
}


static do_prompt ( prompt_string )
    char * prompt_string;
{
/* TEMPORARY - IDL is not handling the subrange type definition correctly */
#define ios_$id_t short
/* END OF TEMPORARY */
    int prompt_len;
    status_$t status;
    ios_$id_t output_stream = 1;
    ios_$id_t prompt_stream = 0;


    if (must_prompt) {
        prompt_len = strlen(prompt_string);

#ifdef apollo
        ios_$put(output_stream, ios_$no_put_get_opts, prompt_string,
                                prompt_len, &status);
        pad_$force_prompt(prompt_stream, &status);
#else
        write(output_stream, prompt_string, prompt_len);
#endif
    }
}

static prompt ( )
{
    char *prompt_string = "lb_admin: ";

    if (must_prompt) {
        do_prompt(prompt_string);
    }
}

static make_lower_case ( buf )
    char *buf;
{
    while (*buf != '\0') {
        if (isupper(*buf)) *buf = tolower(*buf);
            buf++;
    }
}

static match_command ( key, str, min_len )
    char *key;
    char *str;
    int min_len;
{
    int i = 0;

    if (*key) while (*key == *str) {
        i++;
        key++;
        str++;
        if (*str == '\0' || *key == '\0')
            break;
    }
    if (*str == '\0' && i >= min_len)
        return TRUE;

    return FALSE;
}

#define CMD_OP 1
#define HELP_OP 2

static function get_op ( s, op )
    char *s;
    int op;
{
    char ignore_case[256];
    int i;

    strncpy(ignore_case, s, sizeof(ignore_case));
    make_lower_case(ignore_case);

    for (i=0; commands[i].name != NULL; i++) {
        if (match_command(commands[i].name, ignore_case, commands[i].min))
            if (op == CMD_OP) {
                return commands[i].code;
            } else if (op == HELP_OP) {
                return commands[i].help;
            } else {
                break;
            }
    }
    return tty_do_nogood;
}


static get_command ( c )
    command_buf  *c;
{
    c->done = gets(c->command) == NULL;
    if (!c->done) {
        args_$get(c->command, &c->num_args, &c->args);
        if (c->num_args > 0)
            c->op = get_op(c->args[0], CMD_OP);
        else
            c->op = tty_do_nil;
    } else {
        c->op = tty_do_quit;
    }
}

run_test ( )
{
    prompt();
    if (must_prompt && dialog_init()) while (1) {
        if (check_activity() == DIALOG_EC) {
            run_dialog();
        } else {
            do_tty_input();
            prompt();
        }
    } else while (1) {
        do_tty_input();
        prompt();
    }
    /* NOTREACHED */
}

do_tty_input ( )
{
    command_buf c;

    get_command(&c);
    (*c.op)(&c);
}



tty_do_nil ( c )
    command_buf *c;
{
}

tty_do_help(c)
   command_buf *c;
{
    int i, j;
    char buf[32];
    char *p;

    if (c->num_args == 1) {
        printf("Known commands are:");
        for (i = 0; *commands[i].name != '\0'; i++) {
            if ((i) % 4 == 0) {
                printf("\n\t");
            }
            p = commands[i].name;
            for (j = 0; (j < sizeof(buf)-1) && *p; j++) {
                buf[j] = *p++;
                if (j == commands[i].min) {
                    buf[j+1] = buf[j];
                    buf[j] = '[';
                    j++;
                }
            }
            if (j != commands[i].min) {
                buf[j++] = ']';
            }
            buf[j] = '\0';
            printf("%-15.15s ", buf);
        }
        printf("\n");
    } else {
        for (i = 1; i < c->num_args; i++) {
            c->args[0] = c->args[i];
            c->op = get_op(c->args[i], HELP_OP);
            (*c->op)(c);
        }
    }
}

help_all(c)
   command_buf *c;
{
    int i;

    for (i = 0; *commands[i].name != '\0'; i++) {
        (*(commands[i].help))(c);
    }
}

help_help ( )
{
    printf("\
help                            -- yields list of known commands\n\
help command1 { command2 ... }  -- yields information on specified commands\n");
}

help_quit ( )
{
    printf("\
exit or quit                    -- terminate session\n");
}

tty_do_quit ( c )
    command_buf *c;
{
    status_$t status;

    printf("bye.\n");
#ifdef DOMAIN_DIALOG
    dp_$terminate(&status);
#endif
    exit(0);
}

tty_do_nogood ( c )
    command_buf *c;
{
    printf("Unknown command: %s\n",c->args[0]);
}

/*
** The real meat of the admin tool begins here:
*/

static status_print ( st )
    status_$t st;
{   
    rpc_$status_print("?(lb_admin) ", st);
}

static void make_uid ( s, u )
    char *s;
    uuid_$t *u;
{
    status_$t st;

    *u = uuid_$nil;
    if (*s != '*') {
        uname_$uuid_from_name(s, u, &st);
    }
}

boolean process_register_parms ( c, op )
    command_buf *c;
    int op;
{
    int i;
    socket_$addr_t temploc;
    u_long temploc_len = sizeof(temploc);
    status_$t st;
    int curindex = 0;

    if (c->num_args > ++curindex) {
        strcpy(cur_object, c->args[curindex]);
    }
    set_object(cur_object); 

    if (c->num_args > ++curindex) {
        strcpy(cur_obj_type, c->args[curindex]);
    }
    set_type(cur_obj_type); 

    if (c->num_args > ++curindex) {
        strcpy(cur_obj_interface, c->args[curindex]);
    }
    set_interface(cur_obj_interface);       

    if (op != OP_LOOKUP) {
        if (c->num_args > ++curindex) {
            socket_$from_name((u_long) socket_$unspec, (ndr_$char *) c->args[curindex],
                    (u_long) strlen(c->args[curindex]),
                    (u_long) socket_$unspec_port, &temploc,
                    &temploc_len, &st);
            if (STATUS_OK(&st)) {
                strcpy(cur_location, c->args[curindex]);
            } else {
                strcpy(cur_location, "");
            }
        } else {
            strcpy(cur_location, "");
        }
        set_location(cur_location);
    

        if (op == OP_REGISTER) {

            if (c->num_args > ++curindex) {
                strcpy(cur_annotation, c->args[curindex]);
            } else {
                cur_annotation[0] = '\0';
            }
            set_annotation(cur_annotation);
        
            cur_service_options = lb_$server_flag_local_mask;

            for (i = ++curindex; i < c->num_args; i++) {
                if (match_command("global",c->args[i],1))
                    cur_service_options &= ~lb_$server_flag_local_mask;
                else if (match_command("local",c->args[i],1))
                    cur_service_options |= lb_$server_flag_local_mask;
                else
                    printf("\tUnknown option '%s'\n",c->args[i]);
            }
            set_service_mode(cur_service_options);
        }
    }
    return TRUE;
}

help_unregister ( )
{
    printf("\
unregister object type interface location\n\
                                -- unregisters the specified entry; '*' is a wildcard\n");
}

tty_do_unregister ( c )
    command_buf *c;
{
    if (c->num_args < 5) {
        printf("Usage: unregister OBJ TYPE INTERFACE LOCATION\n");
    } else {
        if (process_register_parms(c, OP_UNREGISTER))
            do_unregister();
    }
}

help_register ( )
{
        printf("\
register object type interface location annotation { global | local }\n\
                                -- registers the specified entry\n");
}

tty_do_register ( c )
   command_buf *c;
{
    if (c->num_args < 6) {
        printf("Usage: register OBJ TYPE INTERFACE LOCATION ANNOTATION {global | local }\n");
    } else {
        if (process_register_parms(c, OP_REGISTER))
            do_register();
    }
}

static build_register_info ( s, st )
    lb_$entry_t *s;
    status_$t   *st;
{

    s->object = cur_obj; 
    s->obj_type = cur_obj_typ;
    s->obj_interface = cur_obj_int; 
    bcopy(cur_annotation, s->annotation, sizeof(s->annotation));
    s->flags = cur_service_options;

    socket_$from_name((u_long) socket_$unspec, (ndr_$char *) cur_location,
                    (u_long) strlen(cur_location),
                    (u_long) socket_$unspec_port, &s->saddr,
                    &s->saddr_len, st);

    if (!STATUS_OK(st)) {
        s->saddr_len = 0;
    }
    if ( (s->saddr_len > sizeof(s->saddr.family)) &&
           (s->saddr_len <= sizeof(s->saddr)) )
        st->all = status_$ok;
    else
        st->all = lb_$bad_entry;

}

/*
** set_timeout
**  set long or short timeouts.
*/

help_set_timeout ( c )
    command_buf *c;
{
    printf("\
set_timeout { short | long }    -- set rpc timeout mode\n");
}

tty_do_set_timeout ( c )
    command_buf *c;
{
    boolean ok = TRUE;

    if (c->num_args < 2) {
        printf("Using %s RPC timeouts\n", use_short_timeouts ? "short" : "long");
    } else {
        if (match_command("short", c->args[1], 5)) {
            use_short_timeouts = TRUE;
        } else if (match_command("long", c->args[1], 4)) {
            use_short_timeouts = FALSE;
        } else {
            help_set_timeout(c);
            ok = FALSE;
        }

        if (ok) {
            lb_$use_short_timeouts(use_short_timeouts);
        }
    }
}

help_garbage_collect ( c )
    command_buf *c;
{
printf("\
clean obj type interface location -- cleanup registry database\n");
}

tty_do_garbage_collect ( c )
    command_buf *c;
{
    if (process_register_parms(c, OP_LOOKUP))
        do_garbage_collect();
}

help_set_broker ( )
{
    printf("\
set_broker { local | global } location\n\
                                -- select broker location\n");
}

tty_do_set_broker ( c )
    command_buf *c;
{
    int loc_arg = 1;
    int broker = LOCAL_BROKER;

    if (use_global_broker)
        broker = GLOBAL_BROKER;

    if (c->num_args < 2) {
        printf("Usage: set_broker { local | global } LOCATION\n");
    } else {
        if (strcmp(c->args[1], "global") == 0) {
            if (c->num_args < 3) {
                printf("Usage: set_broker { local | global } LOCATION\n");
                return;
            } else {
                loc_arg = 2;
                broker = GLOBAL_BROKER;
            }
        } else if (strcmp(c->args[1], "local") == 0) {
            if (c->num_args < 3) {
                printf("Usage: set_broker { local | global } LOCATION\n");
                return;
            } else {
                loc_arg = 2;
                broker = LOCAL_BROKER;
            }
        }
        if (do_set_broker(broker, c->args[loc_arg])) {
            if (broker == LOCAL_BROKER) {
                set_local_broker(c->args[loc_arg]);
            } else {
                set_global_broker(c->args[loc_arg]);
            }
        } else {
            printf("%s - illegal broker specification\n", c->args[loc_arg]);
        }
    }
}

help_use_broker ( )
{
    printf("\
use_broker local | global       -- select local or global broker for operations\n");
}

tty_do_use_broker ( c )
    command_buf *c;
{
    if (c->num_args < 2) {
        if (use_global_broker) {
            printf("Using global broker @");
            print_socket_info(&global_broker_addr, global_broker_addr_len);
            printf("\n");
        } else {
            printf("Using local broker @ ");
            print_socket_info(&local_broker_addr, local_broker_addr_len);
            printf("\n");
        }
    } else {
        if (strcmp(c->args[1], "global") == 0) {
            do_use_broker(TRUE);
        } else if (strcmp(c->args[1], "local") == 0) {
            do_use_broker(FALSE);
        } else {
            help_use_broker();
            return;
        }
        set_broker(use_global_broker);
    }
}


help_lookup ( )
{
    printf("\
lookup object type interface location  -- search for matching entries\n");
}

tty_do_lookup ( c )
    command_buf *c;
{
    if (process_register_parms(c, OP_LOOKUP))
        do_lookup();
}


print_socket_info ( addr, len )
    socket_$addr_t *addr;
    u_long len;
{
    socket_$string_t name;
    u_long namelen = sizeof(name);
    u_long port;
    status_$t s;

    if (socket_$valid_family((u_long) addr->family, &s)) {
        socket_$to_name(addr, len, name, &namelen, &port, &s);
        if (!STATUS_OK(&s)) {
            printf("<unknown host> ");
        } else {
            printf("%.*s[%lu] ", (int) namelen, name, port);
        }
    } else {
        printf("<invalid address> ");
    }
}

print_annotation ( e )
    lb_$entry_t *e;
{
    char buf[5 * sizeof(e->annotation) + 1];
    char *t = buf;
    unsigned char *s = (unsigned char *) e->annotation;
    int i;

    for (i = 0; i < sizeof(e->annotation); i++) {
        if (*s == '\0') {
            *t = *s;
            break;
        }
        if (isprint(*s)) {
            *t++ = *s++;
        }
        else if (iscntrl(*s)) {
            *t++ = '^';
            if (*s < 'A') {
                *t++ = *s++ + 'A';
            } else {
                *t++ = '?';
                s++;
            }
        } else {
            sprintf(t, "\\0%03.3o", *s++);
            t += 5;
        }
    }
    *t = '\0';
    printf("\"%s\"",buf);
}

void print_broker ( s )
    lb_$entry_t *s;
{
    print_annotation(s);
    printf(" @ ");
    print_socket_info(&s->saddr, s->saddr_len);
    printf("%s\n", s->flags & lb_$server_flag_local_mask ? "" : "global");
}
                        

static void service_print ( num, s, heading, quit, num_processed, p1 )
    u_long      num;
    lb_$entry_t s[];
    int         heading;
    long        *quit;
    u_long      *num_processed;
    u_long      *p1;
{
    int i;
    char uuid_buf[64];
    status_$t status;
    static uuid_$t prev_object, prev_type, prev_interface;
    boolean loc_wild;
    socket_$addr_t  entry_saddr;
    u_long          entry_slen;

    /* filter clean operation on location too */
    loc_wild = FALSE;
    if (!strcmp(cur_location, ""))
        loc_wild = TRUE;    
    else
    {
        socket_$from_name((u_long) socket_$unspec, (ndr_$char *) cur_location, 
                        (u_long) strlen(cur_location), 
                        (u_long) socket_$unspec_port, 
                        &entry_saddr, &entry_slen, &status);
        if (status.all != status_$ok)
            loc_wild = TRUE;
    }
                

    *quit = FALSE;
    for (i=0; i<num; i++) {
        if (!loc_wild)
        {
            if (!socket_$equal(&entry_saddr, entry_slen, &s[i].saddr, 
                   s[i].saddr_len, (u_long) socket_$eq_netaddr, &status))
                continue;
        }

        if ((heading && i == 0) ||
            !(uuid_$equal(&s[i].object, &prev_object) &&
              uuid_$equal(&s[i].obj_type, &prev_type) &&
              uuid_$equal(&s[i].obj_interface, &prev_interface))) {

            printf("------------\n");
            if (uuid_$equal(&s[i].object, &uuid_$nil)) {
                printf("%12s = *\n", "object");
            } else {
                uname_$uuid_to_name(&s[i].object, uuid_buf, (long) sizeof(uuid_buf));
                printf("%12s = %s\n", "object", uuid_buf);
            }

            if (uuid_$equal(&s[i].obj_type, &uuid_$nil)) {
                printf("%12s = *\n", "type");
            } else {
                uname_$uuid_to_name(&s[i].obj_type, uuid_buf, (long) sizeof(uuid_buf));
                printf("%12s = %s\n", "type", uuid_buf);
            }
        
            if (uuid_$equal(&s[i].obj_interface, &uuid_$nil)) {
                printf("%12s = *\n", "interface");
            } else {
                uname_$uuid_to_name(&s[i].obj_interface, uuid_buf, (long) sizeof(uuid_buf));
                printf("%12s = %s\n", "interface", uuid_buf);
            }
            prev_object = s[i].object;
            prev_type = s[i].obj_type;
            prev_interface = s[i].obj_interface;
        }

        print_annotation(&s[i]);
        printf(" @ ");
        print_socket_info(&s[i].saddr, s[i].saddr_len);
        if (! (s[i].flags & lb_$server_flag_local)) {
            printf("global");
        }
        printf("\n");

    }

    if (num_processed)
        *num_processed = num;
}


/*
** Actual operations (register, unregister, lookup, use_broker, set_broker)
*/

/* getres just strips off the leading blanks from the input string */
char *getres ( i )
char *i;
{
    gets(i);
    while(*i == ' ')
        i++;
    return(i);
}

/*
** delete lb entries (prompt user for confirmation unless no_query is set)
*/
do_delete(info, deleted_one)
    lb_$entry_t *info;
    int         *deleted_one;
{ 
    status_$t   status;

    if (use_global_broker) {
        (void) glb_ca_$delete(info, &status);
        if (!STATUS_OK(&status)) {
            status_print(status);
        } else {
            *deleted_one = TRUE;
        }
    } else {
        (void) llb_ca_$delete(&local_broker_addr, 
                                local_broker_addr_len, info, &status);
        if (!STATUS_OK(&status)) {
            status_print(status);
        } else {
            *deleted_one = TRUE;
        }
    }
}

do_check_delete ( info, quit, no_query )
    lb_$entry_t *info;
    long        *quit;
    long        *no_query;
{
    char        del[64];
    char        *delete;
    int         deleted_one = FALSE;

    if (!*no_query) {
        delete = getres(del);
        if (match_command("quit", delete, 1)) {
            *quit = true;
            return deleted_one; 
        }

        if (match_command("go", delete, 1)) {
            *no_query = TRUE;
        }
    }

    if (*no_query || match_command("yes", delete, 1)) { 
        do_delete(info, &deleted_one);
    }

    return deleted_one;
}

typedef enum {
    lbcs_unknown, lbcs_available, lbcs_obsolete, lbcs_registered, lbcs_unavailable, 
    lbcs_site_unavailable, lbcs_bad_address_family, lbcs_interface_unavailable
} lb_contact_status;

contact_server ( info, contact_status )
    lb_$entry_t         *info;
    lb_contact_status   *contact_status;
{
#define MAX_IFS 32
    handle_t                rpc_handle;
    status_$t               status;
    rpc_$if_spec_t          ifs[MAX_IFS];
    long                    l_ifs;
    int                     i;
    lb_$lookup_handle_t     handle;
    lb_$entry_t             result;
    unsigned long           num;
    socket_$addr_t          addr;
    unsigned long           len;

    *contact_status = lbcs_unknown;

    rpc_handle = rpc_$bind(&info->object, &info->saddr,
                                info->saddr_len, &status);
    if (rpc_handle == NULL || status.all != status_$ok)
        return;

    if (use_short_timeouts) {
        rpc_$set_short_timeout(rpc_handle, (u_long) TRUE, &status);
    }

    EXCEPTION_RANGE {
        EXCEPTION_CASE {
            if ((exception_status.all & rpc_$mod) != rpc_$mod) {
                status_print(exception_status);
            } else if (exception_status.all == rpc_$unk_if) {
                *contact_status = lbcs_obsolete;
            } else if (exception_status.all == rpc_$cant_create_sock) {
                *contact_status = lbcs_bad_address_family;
            } else {
                *contact_status = lbcs_unavailable;
            } 
        } 

        NORMAL_CASE {
            (*rrpc_$client_epv.rrpc_$inq_interfaces)(rpc_handle, 
                                    (unsigned long) MAX_IFS, ifs, &l_ifs,
                                    &status);
            if (status.all == status_$ok) {
                if (uuid_$equal(&(info->obj_interface), &uuid_$nil)) {
                    *contact_status = lbcs_available;
                } else {
                    *contact_status = lbcs_interface_unavailable;
                    for (i = 0; i <= l_ifs; i++) {
                        if (uuid_$equal(&(ifs[i].id), &(info->obj_interface))) {
                            *contact_status = lbcs_available;
                            break;
                        }
                    }
                }
            }
        }
    } EXCEPTION_END;

    if (*contact_status == lbcs_unknown || *contact_status == lbcs_unavailable
            || *contact_status == lbcs_interface_unavailable) {
        handle  = lb_$default_lookup_handle;
        addr    = info->saddr;
        len     = info->saddr_len;
        socket_$set_wk_port(&addr, &len, (u_long) socket_$wk_fwd, &status);
        if (status.all != status_$ok)
            return;

        lb_$lookup_range(&info->object, &info->obj_type, &info->obj_interface,
                            &addr, len, &handle, (u_long) 1, &num, 
                            &result, &status);
        if (status.all == status_$ok) {
            *contact_status = lbcs_registered;
        } else if (status.all != lb_$not_registered) {
            *contact_status = lbcs_site_unavailable;
        }
    }
}

garbage_collect ( num, s, heading, quit, num_processed, num_deleted )
    u_long      num;
    lb_$entry_t s[];
    int         heading;
    long        *quit;
    u_long      *num_processed;
    u_long      *num_deleted;
{
    int                     i;
    status_$t               status;
    long                    no_query;
    static long             no_likely_query;
    lb_contact_status       contact_status;
    long                    dummy;
    static u_long           count, ncount; 
    boolean                 loc_wild;
    socket_$addr_t          entry_saddr;
    u_long                  entry_slen;

    if (heading) {
        *quit = FALSE;
        count = 0;
        no_likely_query = FALSE;
    }
                                      
    /* filter clean operation on location too */
    loc_wild = FALSE;
    if (!strcmp(cur_location, ""))
        loc_wild = TRUE;    
    else
    {
        socket_$from_name((u_long) socket_$unspec, (ndr_$char *) cur_location, 
                        (u_long) strlen(cur_location), 
                        (u_long) socket_$unspec_port, 
                        &entry_saddr, &entry_slen, &status);
        if (status.all != status_$ok)
            loc_wild = TRUE;
    }
                
    ncount = 0;
    for (i=0; i<num; i++) {
                                  
        ncount++;
        no_query = FALSE;
        if (*quit) {
            if (num_processed)
                *num_processed = ncount;
            return;
        }

        if (!loc_wild)
        {
            if (!socket_$equal(&entry_saddr, entry_slen, &s[i].saddr, 
                   s[i].saddr_len, (u_long) socket_$eq_netaddr, &status))
            {
                ncount--;
                continue;
            }
        }


        if (!socket_$valid_family((u_long) s[i].saddr.family, &status)) {
            if (count > 0) {
                count = 0;
                do_prompt("\n");
            }

            service_print(1, &s[i], true, &dummy, NULL, NULL);
            if (no_likely_query) {
                printf("\n        Automatic deletion in progress - this entry retained\n");
            } else {
                printf("\n        Invalid Address Family.  Delete? ");

                if (do_check_delete(&s[i], quit, &no_query))
                    (*num_deleted)++;

                if (no_query) {
                    no_likely_query = no_query;
                }
            }

            continue;
        }

        if (count == 0)
            do_prompt("working ");

        do_prompt(". ");
        count++;
        if (count == 10) {
            do_prompt("\n");
            count = 0;
        }

        contact_server(&s[i], &contact_status);

        if (contact_status == lbcs_available || contact_status == lbcs_obsolete) {
            continue;
        }

        if (count > 0) {
            count = 0;
            do_prompt("\n");
        }

        service_print(1, &s[i], true, &dummy, NULL, NULL);
        if (contact_status == lbcs_site_unavailable) {
            printf("\n        Server not responding - unable to contact remote site.");
        } else if (contact_status == lbcs_registered) {
            printf("\n        Server not responding, but registered in remote llbd database.");
        } else if (contact_status == lbcs_bad_address_family) {
            printf("\n        Unable to communicate over selected address family.");
            printf("\n        Therefore unable to verify server status.");
        } else if (contact_status == lbcs_unavailable) {
            if (!no_likely_query) {
                printf("\n        Server not responding, and is NOT registered with remote llbd.  %sDelete? ", 
                            use_short_timeouts ? "\n        [using short timeouts] " : "");
            } else {
                printf("\n        Server not responding, and is NOT registered with remote llbd.");
                printf("\n        Automatic Deletion - entry DELETED.\n");
            }

            if (do_check_delete(&s[i], quit, &no_likely_query))
                (*num_deleted)++;

            continue;
        } else if (contact_status == lbcs_interface_unavailable) {
            if (!no_likely_query) {
                printf("\n        Server does not support this interface.  %sDelete? ", 
                            use_short_timeouts ? "\n        [using short timeouts] " : "");
            } else {
                printf("\n        Server does not support this interface.");
                printf("\n        Automatic Deletion - entry DELETED.\n");
            }

            if (do_check_delete(&s[i], quit, &no_likely_query))
                (*num_deleted)++;

            continue;
        }

        if (no_likely_query) {
            printf("\n        Automatic deletion in progress - this entry retained\n");
        } else {
            printf("  %sDelete? ",
                    use_short_timeouts ? "\n        [using short timeouts] " : "");

            if (do_check_delete(&s[i], quit, &no_query))
                (*num_deleted)++;

            if (no_query) {
                no_likely_query = no_query;
            }
        }
        continue;

    }

    if (num_processed)
        *num_processed = ncount;
}

do_unregister ( )
{
    lb_$entry_t s[MAX_RESULTS];
    status_$t status, st;
    u_long num_results;
    u_long total_results = 0;
    socket_$addr_t  *saddr, entry_saddr;
    u_long saddr_len, entry_slen, i, nents;
    lb_$lookup_handle_t entry_handle;
    boolean triple_wild, loc_wild, entry_wild, found;
    long quit = FALSE;
    long no_query = FALSE;
    int deleted_one;

    verbose = TRUE;
    status.all = 0;
    if (!use_global_broker) {
        saddr = &local_broker_addr;
        saddr_len = local_broker_addr_len;
    } else {
        saddr = NULL;
        saddr_len = 0;
    }
    entry_handle = lb_$default_lookup_handle;

    socket_$from_name((u_long) socket_$unspec, (ndr_$char *) cur_location, 
                        (u_long) strlen(cur_location), 
                        (u_long) socket_$unspec_port, 
                        &entry_saddr, &entry_slen, &st);

    triple_wild = FALSE;
    if ((uuid_$equal(&cur_obj, &uuid_$nil)) 
    && (uuid_$equal(&cur_obj_typ, &uuid_$nil))
    && (uuid_$equal(&cur_obj_int, &uuid_$nil)))
        triple_wild = TRUE;

    entry_wild = FALSE;
    if ((uuid_$equal(&cur_obj, &uuid_$nil)) 
    || (uuid_$equal(&cur_obj_typ, &uuid_$nil))
    || (uuid_$equal(&cur_obj_int, &uuid_$nil)))
        entry_wild = TRUE;

    loc_wild = FALSE;
    if (!strcmp(cur_location, ""))
        loc_wild = TRUE;    

    nents = 0;
    do 
    {
        lb_$lookup_range(&cur_obj, &cur_obj_typ, &cur_obj_int, saddr, 
            saddr_len, &entry_handle, MAX_RESULTS, &num_results, 
            s, &status);
        total_results += num_results;

        if (status.all == lb_$not_registered) 
        {
            status.all = status_$ok;
            if (total_results == 0)
                printf("No entries match.\n");
            return;
        } 
        else if (!STATUS_OK(&status))
        {
            status_print(status);
            break;
        } 

        if (entry_wild || loc_wild)
        {
            for(i = 0; i < num_results; i++)
            {
                if(verb_flag && verbose)
                {
                    if (!(socket_$valid_family((u_long) s[i].saddr.family, &st)))
                        continue;

                    if (triple_wild)
                    {
                        if (!((uuid_$equal(&s[i].object, &uuid_$nil)) 
                        && (uuid_$equal(&s[i].obj_type, &uuid_$nil))
                        && (uuid_$equal(&s[i].obj_interface, &uuid_$nil))))
                            continue;
                    }

                    if (!loc_wild)
                    {
                        if (!socket_$equal(&entry_saddr, entry_slen, &s[i].saddr, 
                                            s[i].saddr_len, (u_long) socket_$eq_netaddr, &st))
                            continue;
                    }

                    nents++;
                    service_print(1, &s[i], true, &st, NULL, NULL);
                    printf("delete ? "); 
                    do_check_delete(&s[i], &quit, &no_query);

                    if (quit)
                        return;
                    
                    if (no_query) 
                        verbose = FALSE;
                }
                else
                    do_delete(&s[i], &deleted_one);
            }
        }
        else
        {
            found = false;
            for(i = 0; i < num_results; i++)
            {
                if (socket_$equal(&entry_saddr, entry_slen, &s[i].saddr, 
                                    s[i].saddr_len, (u_long) socket_$eq_netaddr, &st))
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                nents++;
                build_register_info(&s[i], &status);
                do_delete(&s[i], &deleted_one);
            }
        }
    } while (entry_handle != lb_$default_lookup_handle);
    if (!nents)
        printf("No entries match.\n");
}

do_register ( )
{
    lb_$entry_t s;
    status_$t status;

    status.all = 0;
    build_register_info(&s, &status);
    if (!STATUS_OK(&status))
	{
		status_print(status);
		return;
	}
    if (use_global_broker) {
        glb_ca_$insert(&s, &status);
    } else if (local_broker_is_thishost) {
        llb_ca_$insert(NULL, 0L, &s, &status);
    } else {
        llb_ca_$insert(&local_broker_addr, local_broker_addr_len,
                    &s, &status);
    }
    if (!STATUS_OK(&status))
        status_print(status);
}

/*
** Garbage collection
**
**  Unregister entries that are not active (Always uses a verbose mode
**  to force confirmation of entry deletion).  An entry is deemed to be 
**  garbage if the node is reachable, and the entry doesn't appear in that
**  node's llb database and there is no response on the port.  Entries are
**  marked as "potential garbage" if the node is not reachable.
*/

do_garbage_collect ( )
{
    long    total_processed;
    u_long  num_deleted = 0;

    total_processed = do_process_entries(garbage_collect, &cur_obj, &cur_obj_typ, 
                                            &cur_obj_int, &num_deleted);
    if (total_processed == 0) {
        printf("No entries match.\n");
    } else {
        printf("\n%ld entries deleted of %ld entries processed\n", num_deleted, 
                    total_processed);
    }
}

do_set_broker ( target, location )
    int target;
    char * location;
{
    status_$t st;
    socket_$addr_t saddr;
    u_long slen;

    socket_$from_name((u_long) socket_$unspec, (ndr_$char *) location, (u_long) strlen(location),
                (u_long) socket_$unspec_port, &saddr, &slen, &st);
    if (STATUS_OK(&st)) {
        if (target == LOCAL_BROKER) {
            bcopy(&saddr, &local_broker_addr, (int) slen);
            local_broker_addr_len = slen;
            if (bcmp(&our_broker_addr, &local_broker_addr, (int) slen))
                local_broker_is_thishost = FALSE;
            else
                local_broker_is_thishost = TRUE;
            return TRUE;
        } else {
            glb_ca_$set_server_address ( &saddr, slen, &st); 
            if (STATUS_OK(&st))
            {
                bcopy(&saddr, &global_broker_addr, (int) slen);
                global_broker_addr_len = slen;           
                return TRUE;
            }
            else
                return FALSE;
        }
    }
    return FALSE;
}

do_use_broker ( flag )
    boolean flag;
{
    use_global_broker = flag;
    lb_$use_short_timeouts((u_long)!flag);
    set_broker(use_global_broker);
}


do_process_entries ( op, obj, type, inf, p1 )
    function    op;
    uuid_$t     *obj;
    uuid_$t     *type;
    uuid_$t     *inf;
    u_long      *p1;
{
    static lb_$entry_t  results[MAX_RESULTS];
    u_long              num_results;
    u_long              total_results = 0;
    socket_$addr_t      *saddr;
    u_long              saddr_len;
    status_$t           status;
    status_$t           lb_status;
    lb_$lookup_handle_t entry_handle;
    int                 first_set = TRUE;
    boolean             got_binding = false;
    long                quit = FALSE;
    
    if (!use_global_broker) {
        saddr = &local_broker_addr;
        saddr_len = local_broker_addr_len;
    } else {
        saddr = NULL;
        saddr_len = 0;
    }

    entry_handle = lb_$default_lookup_handle;
    do {
        lb_$lookup_range(obj, type, inf, saddr,
            saddr_len, &entry_handle, MAX_RESULTS, &num_results,
            results, &lb_status);

        /* If we talked to a global broker, determine its location
        ** so that we can inform the user
        */
        if (use_global_broker && !got_binding) {
            global_broker_addr_len = sizeof(global_broker_addr);
            glb_ca_$get_server_address(&global_broker_addr,
                        &global_broker_addr_len, &status);
            if (STATUS_OK(&status)) {
                status_$t st;
                u_long port;
                socket_$string_t name;
                u_long namelen = sizeof(name);
    
                got_binding = true;
                socket_$to_name(&global_broker_addr,
                        global_broker_addr_len, name, &namelen,
                        &port, &st);
                if (STATUS_OK(&st)) {
                    set_global_broker(name);
                    printf("\n    Data from GLB replica: %s\n\n", name);
                }
            }
        }

        if (lb_status.all == lb_$not_registered) {
            lb_status.all = status_$ok;
        } else if (!STATUS_OK(&lb_status)) {
            status_print(lb_status);
            break;
        } else if (num_results > 0) {
            (*op)(num_results, results, first_set, &quit, &num_results, p1);
            total_results += num_results;
            first_set = FALSE;
        }

    } while (entry_handle != lb_$default_lookup_handle && !quit);

    return total_results;
}

do_lookup ( )
{
    long    total_processed;

    total_processed = do_process_entries(service_print, &cur_obj, &cur_obj_typ, 
                                            &cur_obj_int, NULL);
    if (total_processed == 0) {
        printf("No entries match.\n");
    } else if (total_processed > 0) {
        printf("------------\n");
    }
}

/* Dialog routines */

#ifdef DOMAIN_DIALOG
extern char dp_$lb_admin_heap;

static ec2_$ptr_t ecp[N_ECS];
static long ecv[N_ECS];
static int using_dialog = FALSE;

dialog_init ( )
{
    status_$t status;
    ios_$id_t dialog_pad;
    void dialog_do_update();
    dp_$dpd_id id; 
    char *tmp;

#define CHECK_STATUS if (status.all != status_$ok) { printf("STATUS failed %d\n",__LINE__); status_print(status); return FALSE; }

#ifndef pre_sr10
    pad_$isa_dm_pad(ios_$stdout, &status);
    if (status.all != status_$ok)
        return FALSE;
#endif
    pad_$create(0, 0, pad_$transcript, ios_$stdout, pad_$top, 0, 60, &dialog_pad, &status);
    if (status.all != status_$ok)
        return FALSE;
              
    tmp = &dp_$lb_admin_heap;
    dp_$init_dpd_from_memory(dialog_pad, &tmp, dp_$lb_admin_key, &id, &status);
    if (status.all != dp_$key_mismatch)
        CHECK_STATUS;

    dp_$get_ec(dp_$input_ec, &ecp[DIALOG_EC], &status);
    CHECK_STATUS;

    ios_$get_ec(ios_$stdin, ios_$get_ec_key, &ecp[IOS_EC], &status);
    CHECK_STATUS;

    ecv[DIALOG_EC] = ec2_$read(*ecp[DIALOG_EC]) + 1;
    ecv[IOS_EC] = ec2_$read(*ecp[IOS_EC]) + 1;

    dp_$task_activate(dp_$all_task_group, &status);
    CHECK_STATUS;
    dialog_do_update();

    using_dialog = TRUE;
    run_dialog();   /* make sure dialog refreshes its screen */
    init_fields();
    return TRUE;
#undef CHECK_STATUS
}

init_fields ( )
{
    socket_$string_t name;
    u_long namelen = sizeof(name);
    u_long port;
    status_$t st;

    set_object(cur_object);
    set_type(cur_obj_type);
    set_interface(cur_obj_interface);
    set_annotation(cur_annotation);
    socket_$to_name(&local_broker_addr, local_broker_addr_len, 
                    name, &namelen, &port, &st);
    set_local_broker(name);
}

check_activity ( )
{
    status_$t status;
    short which_ec;

    which_ec = ec2_$wait(ecp, ecv, N_ECS, &status) - 1;
    ecv[which_ec] = ec2_$read(*ecp[which_ec]) + 1;
    return which_ec;
}
        
run_dialog ( )
{
    int task;
    int event_id;
    status_$t status;

    dp_$cond_event_wait(&task, &event_id, &status);
    if (event_id == dp_$task_completed) {
        tty_do_quit();
        /* NOTREACHED */
    }
}

static process_string ( target, max_len, task_id, event_id )
    char *target;
    short max_len;
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    status_$t status;
    short len;
    char buf[1024];

    dp_$string_get_value(*task_id, max_len, buf, &len, &status);
    if (status.all == status_$ok) {
        buf[len] = '\0';
        strcpy(target, buf);
    }
}

void get_object ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    process_string(cur_object, sizeof(cur_object) - 1, task_id, event_id);
    set_object(cur_object);
}

set_object ( string )
    char * string;
{
    short len;
    status_$t status;
    uuid_$t t_uuid;

    if (using_dialog) {
        /*
        ** Normalize uuid names
        */
        t_uuid = uuid_$nil;
        uname_$uuid_from_name(string, &t_uuid, &status);
        uname_$uuid_to_name(&t_uuid, string, (long) MAX_OBJECT_LEN);

        len = strlen(string);
        dp_$string_set_value(dp_$_object, string, len, &status);
    }
    make_uid(cur_object, &cur_obj);
}

void get_type ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    process_string(cur_obj_type, sizeof(cur_obj_type) - 1, task_id, event_id);
    set_type(cur_obj_type);
}

set_type ( string )
    char * string;
{
    short len;
    status_$t status;
    uuid_$t t_uuid;

    if (using_dialog) {
        /*
        ** Normalize uuid names
        */
        t_uuid = uuid_$nil;
        uname_$uuid_from_name(string, &t_uuid, &status);
        uname_$uuid_to_name(&t_uuid, string, (long) MAX_OBJECT_LEN);

        len = strlen(string);
        dp_$string_set_value(dp_$_type, string, len, &status);
    }
    make_uid(cur_obj_type, &cur_obj_typ);
}

void get_interface ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    process_string(cur_obj_interface, sizeof(cur_obj_interface) - 1, task_id, event_id);
    set_interface(cur_obj_interface);
}

set_interface ( string )
    char * string;
{
    short len;
    status_$t status;
    uuid_$t t_uuid;

    if (using_dialog) {
        /*
        ** Normalize uuid names
        */
        t_uuid = uuid_$nil;
        uname_$uuid_from_name(string, &t_uuid, &status);
        uname_$uuid_to_name(&t_uuid, string, (long) MAX_OBJECT_LEN);

        len = strlen(string);
        dp_$string_set_value(dp_$_interface, string, len, &status);
    }
    make_uid(cur_obj_interface, &cur_obj_int);
}

void get_annotation ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    process_string(cur_annotation, sizeof(cur_annotation) -1, task_id, event_id);
}

set_annotation ( string )
    char * string;
{
    short len;
    status_$t status;

    if (using_dialog) {
        len = strlen(string);
        dp_$string_set_value(dp_$_annotation, string, len, &status);
    }
}

void get_service_mode ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    status_$t status;
    short max_len = 2;
    short cur_len;
    dp_$set_array_t vals;

#define SET(A,F) ((A) |= (F))
#define UNSET(A,F) ((A) &= ~(F))

    dp_$set_get_value(*task_id, max_len, vals, &cur_len, &status);
    if (cur_len == 1) {
        if (vals[0]) {
            UNSET(cur_service_options, lb_$server_flag_local_mask);
        } else {
            SET(cur_service_options, lb_$server_flag_local_mask);
        }
    }
}

set_service_mode ( mode )
   int mode;
{
    status_$t status;
    short cur_len = 1;
    dp_$set_array_t vals;

    if (using_dialog) {
        if (mode & lb_$server_flag_local_mask) {
            vals[0] = 0;
        } else {
            vals[0] = -1;
        }
        dp_$set_set_value(dp_$_service_mode, vals, cur_len, &status);
    }
}

void get_location ( task_id, event_id )
   dp_$task_id *task_id;
   dp_$event_id *event_id;
{
    process_string(cur_location, sizeof(cur_location) -1, task_id, event_id);
}

set_location ( string )
   char * string;
{
    short len;
    status_$t status;

    if (using_dialog) {
        len = strlen(string);
        dp_$string_set_value(dp_$_location, string, len, &status);
    }
}

void get_broker ( task_id, event_id )
   dp_$task_id *task_id;
   dp_$event_id *event_id;
{
    if (use_global_broker) {
        printf("use_broker local\n");
    } else {
        printf("use_broker global\n");
    }
    do_use_broker(!use_global_broker);
    prompt();
}

set_broker ( val )
   int val;
{
    status_$t status;

    if (using_dialog) {
        dp_$int_set_value(dp_$_broker, val, &status);
    }
}

void get_local_broker ( task_id, event_id )
   dp_$task_id *task_id;
   dp_$event_id *event_id;
{
    char new_broker[sizeof(cur_broker)];

    process_string(new_broker, sizeof(new_broker) -1, task_id, event_id);
    printf("set_broker local %s\n", new_broker);
    if (do_set_broker(LOCAL_BROKER, new_broker)) {
        set_local_broker(new_broker);
    } else {
        set_local_broker(cur_broker);
        printf("%s - illegal broker specification\n", new_broker);
    }
    prompt();
}

set_local_broker ( string )
    char * string;
{
    short len;
    status_$t status;

    if (using_dialog) {
        strncpy(cur_broker, string, sizeof(cur_broker));
        len = strlen(string);
        dp_$string_set_value(dp_$_local_broker, string, len, &status);
    }
}

void get_global_broker ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    char new_broker[sizeof(cur_broker)];

    process_string(new_broker, sizeof(new_broker) -1, task_id, event_id);
    printf("set_broker global %s\n", new_broker);
    if (do_set_broker(GLOBAL_BROKER, new_broker)) {
        set_global_broker(new_broker);
    } else {
        set_global_broker(cur_broker);
        printf("%s - illegal broker specification\n", new_broker);
    }
    prompt();
}

set_global_broker ( string )
   char * string;
{
    short len;
    status_$t status;
    char nstring[1024];

    if (using_dialog) {
        if (string[0] == '\0')
            len = 0;
        else {
            sprintf(nstring, "%s", string);
            len = strlen(nstring);
        }
        dp_$string_set_value(dp_$_global_broker, nstring, len, &status);
    }
}


void get_objects ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    short len, entry_id; 
    status_$t status;
    int task;
    menu_ent *m_ent;

    dynum_$get_entry(object_enum, entry_id, status);
    dynum_$get_user_data(object_enum, entry_id, m_ent, status);
    strcpy(cur_object, m_ent->uuidstr);
    cur_obj = m_ent->uuid;
    len = strlen(cur_object);
    dp_$string_set_value(dp_$_object, cur_object, len, &status);
}

void get_interfaces ( task_id, event_id )
    dp_$task_id *task_id ;
    dp_$event_id *event_id;
{
    short len, entry_id;
    status_$t status;
    int task;
    menu_ent *m_ent;

    dynum_$get_entry(interface_enum, entry_id, status);
    dynum_$get_user_data(interface_enum, entry_id, m_ent, status);
    strcpy(cur_obj_interface, m_ent->uuidstr);
    cur_obj_int = m_ent->uuid; 
    len = strlen(cur_obj_interface);
    dp_$string_set_value(dp_$_interface, cur_obj_interface, len, &status);
}

void get_types ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    short len, entry_id;
    status_$t status;
    int task;
    menu_ent *m_ent;

    dynum_$get_entry(type_enum, entry_id, status);
    dynum_$get_user_data(type_enum, entry_id, m_ent, status);
    strcpy(cur_obj_type, m_ent->uuidstr);
    cur_obj_typ = m_ent->uuid; 
    len = strlen(cur_obj_type);
    dp_$string_set_value(dp_$_type, cur_obj_type, len, &status);
}

int nobjs = 0;
int nintfs = 0;
int ntypes = 0;

void dialog_do_update ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    lb_$entry_t s[MAX_RESULTS];
    u_long num_results;
    socket_$addr_t *saddr, entry_saddr;
    u_long saddr_len, entry_slen;
    status_$t   status, st;
    uuid_$t     object;
    uuid_$t     obj_type;
    uuid_$t     obj_interface;
    lb_$lookup_handle_t entry_handle;
    int i, task;
    short uuid_len, entry_id, j;
    char uuid_buf[40], found; 
    menu_ent *m_ent;
    boolean     got_binding = false;
    boolean     loc_wild;
    
    if (!use_global_broker) {
        saddr = &local_broker_addr;
        saddr_len = local_broker_addr_len;
    } else 
    {
        saddr = NULL;
        saddr_len = 0;
    }
    loc_wild = FALSE;
    if (!strcmp(cur_location, ""))
        loc_wild = TRUE;    
    else
    {
        socket_$from_name((u_long) socket_$unspec, (ndr_$char *) cur_location, 
                        (u_long) strlen(cur_location), 
                        (u_long) socket_$unspec_port, 
                        &entry_saddr, &entry_slen, &st);
        if (st.all != status_$ok)
            loc_wild = TRUE;
    }
                


    task = oneof_blank_g;
    task_activate(task, &status);
    if(nobjs)
        dynum_$delete_all_entries(object_enum, status);
    if(nintfs)
        dynum_$delete_all_entries(interface_enum, status);
    if(ntypes)
        dynum_$delete_all_entries(type_enum, status);

    strcpy(uuid_buf, "*");
    uuid_len = (short)strlen(uuid_buf);
    dynum_$add_entry_sorted(object_enum, uuid_buf,uuid_len, sizeof(menu_ent), 
                            entry_id, status);
    dynum_$get_user_data(object_enum, entry_id, m_ent, status);
    m_ent->uuid = uuid_$nil; 
    strcpy(m_ent->uuidstr, "*");
    nobjs = 1;

    dynum_$add_entry_sorted(interface_enum, uuid_buf, uuid_len, sizeof(menu_ent), 
                            entry_id, status);
    dynum_$get_user_data(interface_enum, entry_id, m_ent, status);
    m_ent->uuid = uuid_$nil; 
    strcpy(m_ent->uuidstr, "*");
    nintfs = 1;

    dynum_$add_entry_sorted(type_enum, uuid_buf, uuid_len, sizeof(menu_ent), 
                            entry_id, status);
    dynum_$get_user_data(type_enum, entry_id, m_ent, status);
    m_ent->uuid = uuid_$nil;
    strcpy(m_ent->uuidstr, "*");
    ntypes = 1;

    entry_handle = lb_$default_lookup_handle;
    do {
        lb_$lookup_range(&cur_obj, &cur_obj_typ, &cur_obj_int, saddr, 
            saddr_len, &entry_handle, MAX_RESULTS, &num_results, 
            s, &status);

        if (status.all == lb_$not_registered)
            status.all = status_$ok;
        else if (!STATUS_OK(&status))
        {
            status_print(status);
            break;
        }
        else if (num_results > 0)
        {
            for (i=0; i<num_results; i++)
            {
                if (!loc_wild)
                {
                    if (!socket_$equal(&entry_saddr, entry_slen, &s[i].saddr, 
                       s[i].saddr_len, (u_long) socket_$eq_netaddr, &st))
                       continue;
                }
                if (!(socket_$valid_family(s[i].saddr.family, &st)))
                    continue;
                if (!uuid_$equal(&s[i].object, &uuid_$nil))
                {
                    found = false;                      
                    for(j = 2; j <= nobjs; j++)
                    {
                        dynum_$get_user_data(object_enum, j, m_ent, status);
                        if(uuid_$equal(&m_ent->uuid, &s[i].object))
                        {
                            found = true;                       
                            break;
                        }
                    }
                    if(!found)
                    {
                        uname_$uuid_to_name(&s[i].object, uuid_buf, sizeof(uuid_buf));
                        uuid_len = (short)strlen(uuid_buf);
                        dynum_$add_entry_sorted(object_enum, uuid_buf, (short)uuid_len,
                                        sizeof(menu_ent), entry_id, status);    
                        dynum_$get_user_data(object_enum, entry_id, m_ent, status);
                        m_ent->uuid = s[i].object; 
                        strcpy(m_ent->uuidstr, uuid_buf);
                        nobjs++;
                    }
                }

                if (!uuid_$equal(&s[i].obj_type, &uuid_$nil))
                {
                    found = false;                      
                    for(j = 2; j <= ntypes; j++)
                    {
                        dynum_$get_user_data(type_enum, j, m_ent, status);
                        if(uuid_$equal(&m_ent->uuid, &s[i].obj_type))
                        {
                            found = true;                       
                            break;
                        }
                    }
                    if(!found)
                    {
                        uname_$uuid_to_name(&s[i].obj_type, uuid_buf, sizeof(uuid_buf));
                        uuid_len = (short)strlen(uuid_buf);
                        dynum_$add_entry_sorted(type_enum, uuid_buf, uuid_len,
                                        sizeof(menu_ent), entry_id, status);    
                        dynum_$get_user_data(type_enum, entry_id, m_ent, status);
                        m_ent->uuid = s[i].obj_type;
                        strcpy(m_ent->uuidstr, uuid_buf);
                        ntypes++;
                    }
                }
            
                if (!uuid_$equal(&s[i].obj_interface, &uuid_$nil)) 
                {
                    found = false;                      
                    for(j = 2; j <= nintfs; j++)
                    {
                        dynum_$get_user_data(interface_enum, j, m_ent, status);
                        if(uuid_$equal(&m_ent->uuid, &s[i].obj_interface))
                        {
                            found = true;                       
                            break;
                        }
                    }
                    if(!found)
                    {
                        uname_$uuid_to_name(&s[i].obj_interface, uuid_buf, sizeof(uuid_buf));
                        uuid_len = (short)strlen(uuid_buf);
                        dynum_$add_entry_sorted(interface_enum, uuid_buf, uuid_len,
                                        sizeof(menu_ent), entry_id, status);    
                        dynum_$get_user_data(interface_enum, entry_id, m_ent, status);
                        m_ent->uuid = s[i].obj_interface; 
                        strcpy(m_ent->uuidstr, uuid_buf);
                        nintfs++;
                    }
                }
            }
        }
        if (use_global_broker && !got_binding)
        {
            global_broker_addr_len = sizeof(global_broker_addr);
            glb_ca_$get_server_address(&global_broker_addr,
                        &global_broker_addr_len, &status);
            if (STATUS_OK(&status)) {
                status_$t st;
                u_long port;
                socket_$string_t name;
                u_long namelen = sizeof(name);

                got_binding = true;
                socket_$to_name(&global_broker_addr, global_broker_addr_len, 
                                name, &namelen, &port, &st);
                if (STATUS_OK(&st))
                    set_global_broker(name);
            }
        }
    } while (entry_handle != lb_$default_lookup_handle);

    build_msg(&status);
    dynum_$set_entry(object_enum, (short)1, status);
    dynum_$set_entry(type_enum, (short)1, status);
    dynum_$set_entry(interface_enum, (short)1, status);
    for(j = 2; j <= nobjs; j++)
    {
        dynum_$get_user_data(object_enum, j, m_ent, status);
        if (uuid_$equal(&m_ent->uuid, &cur_obj))
        {
            dynum_$set_entry(object_enum, j, status);
            break;
        }
    }
    for(j = 2; j <= ntypes; j++)
    {
        dynum_$get_user_data(type_enum, j, m_ent, status);
        if (uuid_$equal(&m_ent->uuid, &cur_obj_typ))
        {
            dynum_$set_entry(type_enum, j, status);
            break;
        }
    }
    for(j = 2; j <= nintfs; j++)
    {
        dynum_$get_user_data(interface_enum, j, m_ent, status);
        if (uuid_$equal(&m_ent->uuid, &cur_obj_int))
        {
            dynum_$set_entry(interface_enum, j, status);
            break;
        }
    }
    dp_$enum_set_value(operand_menu_type, oneof_object_ent, &status);
    task = oneof_object_g;
    task_activate(task, &status);
    set_object(cur_object);
    set_type(cur_obj_type);
    set_interface(cur_obj_interface);
    set_annotation(cur_annotation);
}

void do_operand_switch ( task_id, event_id )
    dp_$task_id *task_id;
    dp_$event_id *event_id;
{
    short len, entry_id;
    status_$t status;
    int task;

    dp_$enum_get_value(operand_menu_type, &entry_id, &status);
    if (entry_id == oneof_object_ent)
        task = oneof_object_g;
    else if (entry_id == oneof_interface_ent)
        task = oneof_interface_g;
    else
        task = oneof_type_g;

    if (status.all == status_$ok)
        task_activate(task, &status);
}
void dialog_do_lookup ( )
{
    printf("    lookup %s\n", cur_object); 
    printf("                     %s\n", cur_obj_type); 
    printf("                     %s\n", cur_obj_interface);
    do_lookup();
    prompt();
}

void dialog_do_clean ( )
{
    printf("     clean %s\n", cur_object); 
    printf("                     %s\n", cur_obj_type); 
    printf("                     %s\n", cur_obj_interface);
    do_garbage_collect();
    prompt();
}

void dialog_do_register ( )
{
    uuid_$t u;
    int st, status, task;
    char *m_ent, buf[40], found;
    short entry_id, j, uuid_len;

    printf("  register %s\n", cur_object); 
    printf("                     %s\n", cur_obj_type); 
    printf("                     %s\n", cur_obj_interface);
    printf("                     \"%s\" @ ", cur_annotation);
    printf("%s", cur_location);
    printf("%s\n", cur_service_options & lb_$server_flag_local_mask ? "" : " global");
    do_register();
    prompt();
}

void dialog_do_unregister ( )
{
    printf("unregister %s\n", cur_object); 
    printf("                     %s\n", cur_obj_type); 
    printf("                     %s\n", cur_obj_interface);
    if (strcmp(cur_location, ""))
    {
        printf("                     %s", cur_location);
        printf("%s\n", cur_service_options & lb_$server_flag_local_mask ? "" : " global");
    }
    do_unregister();
    prompt();
}


#else

dialog_init ( )
{
    return FALSE;
}

check_activity ( )
{
}
    
run_dialog ( )
{
}

set_object ( string )
   char * string;
{
    make_uid(cur_object, &cur_obj);
}

set_type ( string )
   char * string;
{
    make_uid(cur_obj_type, &cur_obj_typ);
}

set_interface ( string )
   char * string;
{
    make_uid(cur_obj_interface, &cur_obj_int);
}

set_annotation ( string )
   char * string;
{
}

set_location ( string )
   char * string;
{
}

set_service_mode ( mode )
   lb_$server_flag_t mode;
{
}

set_broker ( global )
   int global;
{
}

set_local_broker ( string )
   char * string;
{
}

set_global_broker ( string )
   char * string;
{
}
#endif
