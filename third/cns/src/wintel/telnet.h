#ifndef TELNET_H_INC
#define TELNET_H_INC

#include <windows.h>
#include <stdarg.h>

#include "dialog.h"
#include "screen.h"
#include "struct.h"
#include "proto.h"
#include "winsock.h"
#include "ini.h"

/* globals */
extern char szAutoHostName[64];
extern char szUserName[64];
extern char szHostName[64];

extern void parse(CONNECTION *con,unsigned char *st,int cnt);
extern void send_naws(CONNECTION *con);

#define DEF_WIDTH 80
#define DEF_HEIGHT 24

#endif /* TELNET_H_INC */

