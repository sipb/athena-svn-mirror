/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_rpc.c,v 1.2 1989-11-13 20:34:50 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_rpc.c,v $
 * $Author: qjb $
 *
 * This file contains functions that are used for network communication.
 * See the comment at the top of rk_lib.c for a description of the naming
 * conventions used within the rkinit library.
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_rpc.c,v 1.2 1989-11-13 20:34:50 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <rkinit.h>
#include <rkinit_err.h>
#include <rkinit_private.h>

extern int errno;
extern char *sys_errlist[];

static int sock;
struct sockaddr_in saddr;

static char errbuf[BUFSIZ];

char *calloc();

char *rki_mt_to_string();

int rki_send_packet(sock, type, length, data)
  int sock;
  char type;
  u_long length;
  char *data;
{
    int len;
    u_char *packet;
    u_long pkt_len;
    u_long net_pkt_len;

    pkt_len = length + PKT_DATA;

    if ((packet = (u_char *)calloc(pkt_len, sizeof(u_char))) == NULL) {
	sprintf(errbuf, "rki_send_packet: failure allocating %d bytes",
		pkt_len * sizeof(u_char));
	rkinit_errmsg(errbuf);
	return(RKINIT_MEMORY);
    }

    net_pkt_len = htonl(pkt_len);

    packet[PKT_TYPE] = type;
    bcopy((char *)&net_pkt_len, packet + PKT_LEN, sizeof(u_long));
    bcopy(data, packet + PKT_DATA, length);
    
    if ((len = write(sock, packet, pkt_len)) != pkt_len) {
	if (len == -1) 
	    sprintf(errbuf, "write: %s", sys_errlist[errno]);
	else 
	    sprintf(errbuf, "write: %d bytes written; %d bytes actually sent", 
		    pkt_len, len);
	rkinit_errmsg(errbuf);
	return(RKINIT_WRITE);
    }

    free(packet);
    return(RKINIT_SUCCESS);
}

int rki_get_packet(sock, type, length, data)
  int sock;
  char type;
  u_long *length;
  char *data;
{
    int len;
    int len_sofar = 0;
    u_long expected_length = 0;
    int got_full_packet = FALSE;
    u_char *packet;

    u_long max_pkt_len;

    max_pkt_len = *length + PKT_DATA;

    if ((packet = (u_char *)calloc(max_pkt_len, sizeof(u_char))) == NULL) {
	sprintf(errbuf, "rki_get_packet: failure allocating %d bytes",
		max_pkt_len * sizeof(u_char));
	rkinit_errmsg(errbuf);
	return(RKINIT_MEMORY);
    }

    while (! got_full_packet) {
	if ((len = read(sock, packet + len_sofar, 
			max_pkt_len - len_sofar)) < 0) {
	    sprintf(errbuf, "read: %s", sys_errlist[errno]);
	    rkinit_errmsg(errbuf);
	    return(RKINIT_READ);
	}
	len_sofar += len;
	if (len_sofar >= PKT_DATA) {
	    bcopy(packet + PKT_LEN, (char *)&expected_length, sizeof(u_long));
	    expected_length = ntohl(expected_length);
	    if (expected_length == len_sofar)
		got_full_packet = TRUE;
	    else if (expected_length < len_sofar) {
		sprintf(errbuf, 
			"read: expected to receive only %d bytes; received %d",
			expected_length, len_sofar);
		rkinit_errmsg(errbuf);
		return(RKINIT_PACKET);
	    }
	    else if (expected_length > max_pkt_len) {
		sprintf(errbuf, "%s %d %s %d",
			"read: expected to receive", expected_length,
			"bytes, but only had room for", max_pkt_len);
		rkinit_errmsg(errbuf);
		return(RKINIT_PACKET);
	    }
	}
    }

    if (packet[PKT_TYPE] != type) {
	sprintf(errbuf, "Expected packet type of %s; got %s",
		rki_mt_to_string(type), 
		rki_mt_to_string(packet[PKT_TYPE]));
	rkinit_errmsg(errbuf);
	return(RKINIT_PACKET); 
    }

    *length = len_sofar - PKT_DATA;
    bcopy(packet + PKT_DATA, data, *length);

    free(packet);

    return(RKINIT_SUCCESS);
}

rki_setup_rpc(host)
  char *host;
{
    struct hostent *hp;
    struct servent *sp;
    int port;

    SBCLEAR(saddr);
    SBCLEAR(hp);
    SBCLEAR(sp);

    if ((hp = gethostbyname(host)) == NULL) {
	sprintf(errbuf, "%s: unknown host.", host);
	rkinit_errmsg(errbuf);
	return(RKINIT_HOST);
    }

    if (sp = getservbyname(SERVENT, "tcp"))
	port = sp->s_port;
    else 
	/* Fall back on known port number */
	port = htons(PORT);

    saddr.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = port;

    if ((sock = socket(hp->h_addrtype, SOCK_STREAM, IPPROTO_IP)) < 0) {
	sprintf(errbuf, "socket: %s", sys_errlist[errno]);
	rkinit_errmsg(errbuf);
	return(RKINIT_SOCKET);
    }
    
    if (connect(sock, (char *)&saddr, sizeof (saddr)) < 0) {
	sprintf(errbuf, "connect: %s", sys_errlist[errno]);
	rkinit_errmsg(errbuf);
	close(sock);
	return(RKINIT_CONNECT);
    }

    return(RKINIT_SUCCESS);
}    

int rki_rpc_exchange_version_info(c_lversion, c_hversion, 
				  s_lversion, s_hversion)
  int c_lversion;
  int c_hversion;
  int *s_lversion;
  int *s_hversion;
{
    int status = RKINIT_SUCCESS;
    u_char version_info[VERSION_INFO_SIZE];
    u_long length = sizeof(version_info);
    
    version_info[0] = (u_char) c_lversion;
    version_info[1] = (u_char) c_hversion;
    
    if ((status = rki_send_packet(sock, MT_CVERSION, length,
				  (char *)version_info)) != RKINIT_SUCCESS)
	return(status);
    
    if ((status = rki_get_packet(sock, MT_SVERSION, &length, 
				 (char *)version_info)) != RKINIT_SUCCESS) 
	return(status);

    *s_lversion = (int) version_info[0];
    *s_hversion = (int) version_info[1];
    
    return(RKINIT_SUCCESS);
}

int rki_rpc_send_rkinit_info(info)
  rkinit_info *info;
{
    rkinit_info info_copy;
    
    bcopy(info, &info_copy, sizeof(rkinit_info));
    info_copy.lifetime = htonl(info_copy.lifetime);
    return(rki_send_packet(sock, MT_RKINIT_INFO, sizeof(rkinit_info), 
			   (char *)&info_copy));
}

int rki_rpc_get_status()
{
    char msg[BUFSIZ];
    int status = RKINIT_SUCCESS;
    u_long length = sizeof(msg);
   
    if (status = rki_get_packet(sock, MT_STATUS, &length, msg))
	return(status);

    if (length == 0)
	return(RKINIT_SUCCESS);
    else {
	rkinit_errmsg(msg);
	return(RKINIT_DAEMON);
    }
}

int rki_rpc_get_ktext(sock, auth, type)
  int sock;
  KTEXT auth;
  u_char type;
{
    int status = RKINIT_SUCCESS;
    u_long length = MAX_KTXT_LEN;

    if (status = rki_get_packet(sock, type, &length, (char *)auth->dat))
	return(status);
    
    auth->length = length;
    
    return(RKINIT_SUCCESS);
}

int rki_rpc_sendauth(auth)
  KTEXT auth;
{
    return(rki_send_packet(sock, MT_AUTH, auth->length, (char *)auth->dat));
}


int rki_rpc_get_skdc(scip)
  KTEXT scip;
{
    return(rki_rpc_get_ktext(sock, scip, MT_SKDC));
}

int rki_rpc_send_ckdc(scip)
  MSG_DAT *scip;
{
    return(rki_send_packet(sock, MT_CKDC, scip->app_length, 
			   (char *)scip->app_data));
}

int rki_get_csaddr(caddrp, saddrp)
  struct sockaddr_in *caddrp;
  struct sockaddr_in *saddrp;
{
    int addrlen = sizeof(struct sockaddr_in);
    
    bcopy((char *)&saddr, (char *)saddrp, addrlen);

    if (getsockname(sock, caddrp, &addrlen) < 0) {
	sprintf(errbuf, "getsockname: %s", sys_errlist[errno]);
	rkinit_errmsg(errbuf);
	return(RKINIT_GETSOCK);
    }

    return(RKINIT_SUCCESS);
}

void rki_cleanup_rpc()
{
    (void) close(sock);
}
