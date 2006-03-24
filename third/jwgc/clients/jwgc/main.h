#ifndef _MAIN_H_
#define _MAIN_H_

#include <sysdep.h>
#include <libjabber.h>
#include <libjwgc.h>


/* Global variables */
extern struct _Node *program;
extern char *progname;
extern int jab_reauth;
extern jabconn jab_c;
extern jwgconn jwg_c;
extern time_t jab_connect_time;


/* Global defines */
#ifndef USRDESC
#define USRDESC ".jwgc.desc"
#endif

#ifndef DEFDESC
#define DEFDESC "jwgc.desc"
#endif

#ifndef DEFSERVER
#define DEFSERVER "jabber.org"
#endif

#ifndef DEFPORT
#define DEFPORT "5222"
#endif

#ifndef DEFSSLPORT
#define DEFSSLPORT "5223"
#endif

#ifndef DEFRESOURCE
#define DEFRESOURCE "jwgc"
#endif

#ifndef DEFPRESENCE
#define DEFPRESENCE "available"
#endif

#ifndef DEFPRIORITY
#define DEFPRIORITY "0"
#endif

#ifndef DEFUSESSL
#define DEFUSESSL "false"
#endif

#ifndef DEFUSEGPG
#define DEFUSEGPG "false"
#endif

#ifndef MACHINE_TYPE
#define MACHINE_TYPE "unknown"
#endif


/* jabber_handler.c */
void jab_on_packet_handler(jabconn conn, jabpacket packet);
void jab_on_state_handler(jabconn conn, int state);
void fake_own_presence(jabconn conn);


/* jwgc_handler.c */
void jwg_on_event_handler(jwgconn conn, jwgpacket packet);


/* jwgc_variables.c */
void jwg_set_defaults_handler();


/* main.c */
void usage();
int read_in_description_file();


/* notice.c */
char *decode_notice(struct jabpacket_struct *notice);


/* parser.y */
struct _Node *parse_file(FILE *input_file);
struct _Node *parse_buffer(char *input_buffer);


/* status.c */
void show_status();


/* variables.c */
void var_clear_all_variables();

#endif /* _MAIN_H_ */
