#include <olc/olc.h>
#include <olc/olc_tty.h>
#include <olc/olc_parser.h>
#include "olc.h"

#include <zephyr/zephyr.h>
#include "XmAppl.h"          /*  Motif Toolkit  */

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

#define  ASKBOX		1
#define  REPLAYBOX	2
#define  HELPBOX	3
#define  ERRORBOX	4
#define  MOTDBOX	5
#define  MOTDFRAME	6
#define  MOTD		7
