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
 * Types for "xport" module.  (MS/DOS)
 */

#ifndef XPORT_INCLUDED
#define XPORT_INCLUDED

/*** The following structures are packed to 1 byte alignment */
#pragma pack(1)

/*** Transport Control Block */
typedef struct {
    u_char  command;    /* command code                         */
    u_char  cid;        /* command id number                    */
    u_char  vcid;       /* virtual circuit id number            */
    u_short length;     /* data buffer length                   */
    char far* baddr;    /* data buffer address                  */
    u_short res1;       /* reserved field                       */
    u_char  laddr[16];  /* local network address                */
    u_char  raddr[16];  /* remote network address               */
    void    (far* async)(); /* asynchronous notification routine    */
    u_char  lnet[4];    /* local network number                 */
    u_char  rnet[4];    /* remote network number                */
    u_char  rto;        /* receive timeout (500 msecs)          */
    u_char  sto;        /* send timeout (500 msecs)             */
    u_char  res2[8];    /* reserved field                       */
} xport_$tcb_rep;
typedef xport_$tcb_rep* xport_$tcb;

/*** Asynchronous Control Block */
typedef struct {
    u_char  cid;        /* cid of completed command             */
    u_char  vcid;       /* vcid of completed command            */
    u_char  err;        /* error code                           */
    u_char  cmd;        /* command of completed command         */
    u_short len;        /* actual used portion of data buffer   */
    u_char  radr[16];   /* remote network address               */
    u_char  rnet[4];    /* remote network number                */
    u_char  ladr[16];   /* local network address                */
    u_char  lnet[4];    /* local network number                 */
} xport_$acb_rep;
typedef xport_$acb_rep* xport_$acb;

/*** Status structure */
typedef struct {
    u_char  host_address[16];
    u_char  net_number[4];
    u_char  hardware_status;
    u_char  software_version;
    u_long  packets_sent;
    u_long  packets_received;
    u_short packets_received_short;
    u_short packets_aborted;
    u_short packets_misaligned;
    u_short packets_crc_error;
    u_char  address_significance[16];
} xport_$status_rep;
typedef xport_$status_rep* xport_$status;

#pragma pack()  /* Packing reverts to command line selection */

/*** Command codes */
#define XPORT_$CALL              0x10
#define XPORT_$LISTEN            0x11
#define XPORT_$HANG_UP           0x12
#define XPORT_$SEND              0x14
#define XPORT_$RECEIVE           0x15
#define XPORT_$RECEIVE_ANY       0x16
#define XPORT_$SEND_CONT         0x17
#define XPORT_$SEND_DATAGRAM     0x20
#define XPORT_$RECEIVE_DATAGRAM  0x21
#define XPORT_$SEND_BROADCAST    0x22
#define XPORT_$RECEIVE_BROADCAST 0x23
#define XPORT_$RESET             0x32
#define XPORT_$STATUS            0x33
#define XPORT_$CANCEL            0x35

/*** Error codes */
#define XPORT_$ILLEGAL_BUFFER_LENGTH     0x01
#define XPORT_$ILLEGAL_COMMAND           0x03
#define XPORT_$COMMAND_TIMEOUT           0x05
#define XPORT_$MESSAGE_INCOMPLETE        0x06
#define XPORT_$VCID_OUT_OF_RANGE         0x08
#define XPORT_$NO_RESOURCE_AVAILABLE     0x09
#define XPORT_$VC_ALREADY_CLOSED         0x0A
#define XPORT_$COMMAND_CANCELLED         0x0B
#define XPORT_$LOCAL_RESOURCE            0x11
#define XPORT_$REMOTE_RESOURCE           0x12
#define XPORT_$VC_ENDED_ABNORMALLY       0x18
#define XPORT_$INTERFACE_BUSY            0x21
#define XPORT_$TOO_MANY_COMMANDS         0x22
#define XPORT_$COMPLETED_WHILE_CANCEL    0x24
#define XPORT_$NOT_VALID_TO_CANCEL       0x26
#define XPORT_$INVALID_NET_NUMBER        0x30
#define XPORT_$ILLEGAL_ADDRESS           0x31
/*#define XPORT_$NETWORK_ERROR           0x4? */
#define XPORT_$COMMAND_NOT_FINISHED      0xFF

extern u_short  xport_$command(xport_$tcb);

#define xport_$copy_tcb(dest, src) \
    memcpy((dest), (src), sizeof(xport_$tcb_rep))
#define xport_$copy_acb(dest, src) \
    memcpy((dest), (src), sizeof(xport_$acb_rep))
#define xport_$copy_status(dest, src) \
    memcpy((dest), (src), sizeof(xport_$status_rep))
#define xport_$zero_tcb(tcb) \
    memset((tcb), '\0', sizeof(xport_$tcb_rep))
#define xport_$zero_acb(acb) \
    memset((acb), '\0', sizeof(xport_$acb_rep))
#define xport_$zero_status(stat) \
    memset((stat), '\0', sizeof(xport_$status_rep))

/***??? Works for Large model programs only */
#define xport_$anr_copy(dest, src, size) \
    xport_$copy_acb((dest), (src))

#ifdef SOCKET_DEBUG
extern short    xport_$command_status(u_char);
extern char*    xport_$error_name(u_char);
extern void     xport_$print_tcb(xport_$tcb, int, FILE*);
extern void     xport_$print_acb(xport_$acb, int, FILE*);
extern void     xport_$print_status(xport_$status, int, FILE*);
#endif

#endif /*!XPORT_$INCLUDED*/
