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
 * X P O R T
 *
 * Interface to the Microsoft MSNET XPORT driver.  (MS/DOS)
 *
 * From "The Microsoft Network Transport Layer Interface v1.01, Oct 1984".
 */

#include "std.h"
#include "xport.h"

/*
 * void xport_$command(tcb)   - in xporta.asm
 */
 
#ifdef NEVER

int xport_$reset()
{
    xport_$tcb_rep rtcb;
    xport_$tcb tcb = &rtcb;
    u_short ret;
    extern void far xport_$null_handler();

    xport_$zero_tcb(tcb);

    tcb->command = XPORT_$RESET;
    tcb->cid = 0;
    tcb->async = xport_$null_handler;

    ret = xport_$command(tcb);
    return(ret == 0);
}

#endif

#ifdef SOCKET_DEBUG

/*** Command code names */
static struct command_name_entry {
    u_char  code;
    char*   description;
} NEAR _xport_$command_name_table[] = {
    XPORT_$CALL,         "call",
    XPORT_$LISTEN,       "listen",
    XPORT_$HANG_UP,      "hang_up",
    XPORT_$SEND,         "send",
    XPORT_$RECEIVE,      "receive",
    XPORT_$RECEIVE_ANY,  "receive_any",
    XPORT_$SEND_CONT,    "send_continue",
    XPORT_$SEND_DATAGRAM,"send_datagram",
    XPORT_$RECEIVE_DATAGRAM,"receive_datagram",
    XPORT_$SEND_BROADCAST,"send_broadcast",
    XPORT_$RECEIVE_BROADCAST,"receive_broadcast",
    XPORT_$RESET,        "reset_name",
    XPORT_$STATUS,       "status",
    XPORT_$CANCEL,       "cancel",
};
#define COMMAND_NAME_TABLE_SIZE \
    (sizeof(_xport_$command_name_table) / sizeof(struct command_name_entry))


/*** Error code names */
static struct error_name_entry {
    u_char  code;
    char*   description;
} NEAR _xport_$error_name_table[] = {
    XPORT_$ILLEGAL_BUFFER_LENGTH,    "illegal data buffer length",
    XPORT_$ILLEGAL_COMMAND,          "illegal command",
    XPORT_$COMMAND_TIMEOUT,          "command timed out",
    XPORT_$MESSAGE_INCOMPLETE,       "message incomplete",
    XPORT_$VCID_OUT_OF_RANGE,        "VCID out of range",
    XPORT_$NO_RESOURCE_AVAILABLE,    "no resource available",
    XPORT_$VC_ALREADY_CLOSED,        "virtual circuit already closed",
    XPORT_$COMMAND_CANCELLED,        "command cancelled",
    XPORT_$LOCAL_RESOURCE,           "local resource error",
    XPORT_$REMOTE_RESOURCE,          "remote resource error",
    XPORT_$VC_ENDED_ABNORMALLY,      "virtual circuit ended abnormally",
    XPORT_$INTERFACE_BUSY,           "interface busy",
    XPORT_$TOO_MANY_COMMANDS,        "too many commands outstanding",
    XPORT_$COMPLETED_WHILE_CANCEL,   "command completed while cancel occuring",
    XPORT_$NOT_VALID_TO_CANCEL,      "command not valid to cancel",
    XPORT_$INVALID_NET_NUMBER,       "invalid net number",
    XPORT_$ILLEGAL_ADDRESS,          "illegal address",
    XPORT_$COMMAND_NOT_FINISHED,     "command is not yet finished",
};
#define ERROR_NAME_TABLE_SIZE \
    (sizeof(_xport_$error_name_table) / sizeof(struct error_name_entry))

/*** Local functions */
static void _xport_$print_address();
static void _xport_$print_net_number();

char* xport_$command_name(cmd_code)
u_char cmd_code;
{
    short i;

    for (i = 0; i < COMMAND_NAME_TABLE_SIZE; i++) {
        if (cmd_code == _xport_$command_name_table[i].code)
            return(_xport_$command_name_table[i].description);
    }
    return("??unknown??");
}

char* xport_$error_name(error_code)
u_char error_code;
{
    short i;

    if (error_code == 0)
        return("ok");
    for (i = 0; i < ERROR_NAME_TABLE_SIZE; i++) {
        if (error_code == _xport_$error_name_table[i].code)
            return(_xport_$error_name_table[i].description);
    }
    return("??unknown??");
}   

void xport_$print_tcb(tcb, level, fp)
xport_$tcb tcb;
int level;
FILE* fp;
{
    util_$tab(level, fp);
    fprintf(fp, "command: 0x%02x (%s)\n", tcb->command,
            xport_$command_name(tcb->command));
    util_$tab(level, fp);
    fprintf(fp, "command id number: %u\n", (u_short) tcb->cid);
    util_$tab(level, fp);
    fprintf(fp, "virtual circuit id number: %u\n", (u_short) tcb->vcid);
    util_$tab(level, fp);
    fprintf(fp, "buffer length: %u\n", tcb->length);
    util_$tab(level, fp);
    fprintf(fp, "buffer address: 0x%04lx_%04lx\n", 
        (((u_long) tcb->baddr)>>16) & 0xffffL, ((u_long) tcb->baddr) & 0xffffL);
    util_$tab(level, fp);
    fprintf(fp, "local network address: ");
    _xport_$print_address(tcb->laddr, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "remote network address: ");
    _xport_$print_address(tcb->raddr, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "async notification routine: 0x%04lx_%04lx\n", 
        (((u_long) tcb->async)>>16) & 0xffffL, ((u_long) tcb->async) & 0xffffL);
    util_$tab(level, fp);
    fprintf(fp, "local network number: ");
    _xport_$print_net_number(tcb->lnet, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "remote network number: "); 
    _xport_$print_net_number(tcb->rnet, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "receive timeout (500 msecs): %u\n", (u_short) tcb->rto);
    util_$tab(level, fp);
    fprintf(fp, "send timeout (500 msecs): %u\n", (u_short) tcb->sto);
}

void xport_$print_acb(acb, level, fp)
xport_$acb acb;
int level;
FILE* fp;
{
    util_$tab(level, fp);
    fprintf(fp, "completed command id: %u\n", (u_short) acb->cid);
    util_$tab(level, fp);
    fprintf(fp, "virtual circuit id: %u\n", (u_short) acb->vcid);
    util_$tab(level, fp);
    fprintf(fp, "error code: 0x%02x (%s)\n", acb->err, xport_$error_name(acb->err));
    util_$tab(level, fp);
    fprintf(fp, "command: 0x%02x (%s)\n", acb->cmd, xport_$command_name(acb->cmd));
    util_$tab(level, fp);
    fprintf(fp, "actual buffer length: %u\n", acb->len);
    util_$tab(level, fp);
    fprintf(fp, "remote network address: ");
    _xport_$print_address(acb->radr, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "remote network number: ");
    _xport_$print_net_number(acb->rnet, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "local network address: ");
    _xport_$print_address(acb->ladr, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "local network number: ");
    _xport_$print_net_number(acb->lnet, fp);
    fprintf(fp, "\n");
}

void xport_$print_status(stat, level, fp)
xport_$status stat;
int level;
FILE* fp;
{
    util_$tab(level, fp);
    fprintf(fp, "host address: ");
    _xport_$print_address(stat->host_address, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "net number: ");
    _xport_$print_net_number(stat->net_number, fp);
    fprintf(fp, "\n");
    util_$tab(level, fp);
    fprintf(fp, "hardware status: 0x%02x\n", stat->hardware_status);
    util_$tab(level, fp);
    fprintf(fp, "software version: %u\n", stat->software_version);
    util_$tab(level, fp);
    fprintf(fp, "packets sent: %lu\n", stat->packets_sent);
    util_$tab(level, fp);
    fprintf(fp, "packets received: %lu\n", stat->packets_received);
    util_$tab(level, fp);
    fprintf(fp, "packets received short: %u\n", stat->packets_received_short);
    util_$tab(level, fp);
    fprintf(fp, "packets aborted: %u\n", stat->packets_aborted);
    util_$tab(level, fp);
    fprintf(fp, "packets misaligned: %u\n", stat->packets_misaligned);
    util_$tab(level, fp);
    fprintf(fp, "packets with crc errors: %u\n", stat->packets_crc_error);
    util_$tab(level, fp);
    fprintf(fp, "address significance: ");
    _xport_$print_address(stat->address_significance, fp);
    fprintf(fp, "\n");
}

static void _xport_$print_address(addr, fp)
u_char addr[16];
FILE* fp;
{
    short i;

    /*** Print format: 0xFFEE_DDCC_BBAA_9988_7766_5544_3322_1100 */
    fprintf(fp, "0x");
    for (i = 0; i < 16; i++) {
        if ((i%2) == 0 && i != 0)
            fprintf(fp, "_");
        fprintf(fp, "%02x", (u_short) addr[15 - i]);
    }
}

static void _xport_$print_net_number(num, fp)
u_char num[4];
FILE* fp;
{
    short i;

    /*** Print format: 0x3210 */
    fprintf(fp, "0x");
    for (i = 0; i < 4; i++) {
        if ((i%2) == 0 && i != 0)
            fprintf(fp, "_");
        fprintf(fp, "%02x", (u_short) num[3 - i]);
    }
}

#endif /*SOCKET_DEBUG*/
