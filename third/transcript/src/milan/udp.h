/*
 * Copyright Milan Technology 1991, 1992 
 * @(#)udp.h	2.0 10/9/92 
 */

typedef struct {
		unsigned long parallel_addr;
		unsigned long serial_addr;
		unsigned long parallel_bytes;
		unsigned long serial_bytes;
		unsigned long  p_status;
		char message[250];
} udp_status_packet;

#define MILAN_OP 0x40
#define GET_STATUS 0x1
