/* 
 * $Id: rk_rpc.c,v 1.9 1998-05-07 16:58:18 ghudson Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_rpc.c,v $
 * $Author: ghudson $
 *
 * This file contains functions that are used for network communication.
 * See the comment at the top of rk_lib.c for a description of the naming
 * conventions used within the rkinit library.
 */

#if !defined(lint) && !defined(SABER) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid = "$Id: rk_rpc.c,v 1.9 1998-05-07 16:58:18 ghudson Exp $";
#endif /* lint || SABER || LOCORE || RCS_HDRS */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <rkinit.h>
#include <rkinit_err.h>
#include <rkinit_private.h>

static int sock;
struct sockaddr_in saddr;

static char errbuf[BUFSIZ];

char *calloc();

char *rki_mt_to_string();

#ifdef __STDC__
int rki_send_packet(int sock, char type, u_long length, char *data)
#else
int rki_send_packet(sock, type, length, data)
  int sock;
  char type;
  u_long length;
  char *data;
#endif /* __STDC__ */
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
    memcpy(packet + PKT_LEN, &net_pkt_len, sizeof(u_long));
    memcpy(packet + PKT_DATA, data, length);
    if ((len = write(sock, packet, pkt_len)) != pkt_len) {
	if (len == -1) 
	    sprintf(errbuf, "write: %s", strerror(errno));
	else 
	    sprintf(errbuf, "write: %d bytes written; %d bytes actually sent", 
		    pkt_len, len);
	rkinit_errmsg(errbuf);
	return(RKINIT_WRITE);
    }

    free(packet);
    return(RKINIT_SUCCESS);
}

#ifdef __STDC__
int rki_get_packet(int sock, char type, u_long *length, char *data)
#else
int rki_get_packet(sock, type, length, data)
  int sock;
  char type;
  u_long *length;
  char *data;
#endif /* __STDC__ */
{
    u_char header[PKT_DATA];
    int packet_length, data_length, count, len;

    /* Read in the packet header. */
    len = 0;
    while (len < PKT_DATA) {
	count = read(sock, header + len, PKT_DATA - len);
	if (count < 0) {
	    sprintf(errbuf, "read: %s", strerror(errno));
	    rkinit_errmsg(errbuf);
	}
	if (count <= 0)
	    return(RKINIT_READ);
	len += count;
    }

    /* Unmarshal the packet length (which includes the length of the
     * packet header as well as the data length) and make sure we have
     * enough room to hold the packet data. */
    packet_length = header[PKT_LEN] << 24 | header[PKT_LEN + 1] << 16
	| header[PKT_LEN + 2] << 8 | header[PKT_LEN + 3];
    data_length = packet_length - PKT_DATA;
    if (data_length > *length) {
	sprintf(errbuf, "%s %d %s %d",
		"read: expected to receive", data_length,
		"bytes, but only had room for", *length);
	rkinit_errmsg(errbuf);
	return(RKINIT_PACKET);
    }

    /* Read the packet data into the caller's buffer. */
    len = 0;
    while (len < data_length) {
	count = read(sock, data + len, data_length - len);
	if (count < 0) {
	    sprintf(errbuf, "read: %s", strerror(errno));
	    rkinit_errmsg(errbuf);
	}
	if (count <= 0)
	    return(RKINIT_READ);
	len += count;
    }

    /* Check the packet type, possibly dropping the packet. */
    if (header[PKT_TYPE] == MT_DROP) {
	BCLEAR(errbuf);
	rkinit_errmsg(errbuf);
	return(RKINIT_DROPPED);
    }
    if (header[PKT_TYPE] != type) {
	sprintf(errbuf, "Expected packet type of %s; got %s",
		rki_mt_to_string(type), rki_mt_to_string(header[PKT_TYPE]));
	rkinit_errmsg(errbuf);
	return(RKINIT_PACKET);
    }

    /* Everything looks okay. */
    *length = data_length;
    return(RKINIT_SUCCESS);
}

#ifdef __STDC__
int rki_setup_rpc(char *host)
#else
int rki_setup_rpc(host)
  char *host;
#endif /* __STDC__ */
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
    memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
    saddr.sin_port = port;

    if ((sock = socket(hp->h_addrtype, SOCK_STREAM, IPPROTO_IP)) < 0) {
	sprintf(errbuf, "socket: %s", strerror(errno));
	rkinit_errmsg(errbuf);
	return(RKINIT_SOCKET);
    }
    
    if (connect(sock, (struct sockaddr *)&saddr, sizeof (saddr)) < 0) {
	sprintf(errbuf, "connect: %s", strerror(errno));
	rkinit_errmsg(errbuf);
	close(sock);
	return(RKINIT_CONNECT);
    }

    return(RKINIT_SUCCESS);
}    

#ifdef __STDC__
int rki_rpc_exchange_version_info(int c_lversion, int c_hversion, 
				  int *s_lversion, int *s_hversion)
#else
int rki_rpc_exchange_version_info(c_lversion, c_hversion, 
				  s_lversion, s_hversion)
  int c_lversion;
  int c_hversion;
  int *s_lversion;
  int *s_hversion;
#endif /* __STDC__ */
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

#ifdef __STDC__
int rki_rpc_send_rkinit_info(rkinit_info *info)
#else
int rki_rpc_send_rkinit_info(info)
  rkinit_info *info;
#endif /* __STDC__ */
{
    rkinit_info info_copy;
    
    memcpy(&info_copy, info, sizeof(rkinit_info));
    info_copy.lifetime = htonl(info_copy.lifetime);
    return(rki_send_packet(sock, MT_RKINIT_INFO, sizeof(rkinit_info), 
			   (char *)&info_copy));
}

#ifdef __STDC__
int rki_rpc_get_status(void)
#else
int rki_rpc_get_status()
#endif /* __STDC__ */
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

#ifdef __STDC__
int rki_rpc_get_ktext(int sock, KTEXT auth, u_char type)
#else
int rki_rpc_get_ktext(sock, auth, type)
  int sock;
  KTEXT auth;
  u_char type;
#endif /* __STDC__ */
{
    int status = RKINIT_SUCCESS;
    u_long length = MAX_KTXT_LEN;

    if (status = rki_get_packet(sock, type, &length, (char *)auth->dat))
	return(status);
    
    auth->length = length;
    
    return(RKINIT_SUCCESS);
}

#ifdef __STDC__
int rki_rpc_sendauth(KTEXT auth)
#else
int rki_rpc_sendauth(auth)
  KTEXT auth;
#endif /* __STDC__ */
{
    return(rki_send_packet(sock, MT_AUTH, auth->length, (char *)auth->dat));
}


#ifdef __STDC__
int rki_rpc_get_skdc(KTEXT scip)
#else
int rki_rpc_get_skdc(scip)
  KTEXT scip;
#endif /* __STDC__ */
{
    return(rki_rpc_get_ktext(sock, scip, MT_SKDC));
}

#ifdef __STDC__
int rki_rpc_send_ckdc(MSG_DAT *scip)
#else
int rki_rpc_send_ckdc(scip)
  MSG_DAT *scip;
#endif /* __STDC__ */
{
    return(rki_send_packet(sock, MT_CKDC, scip->app_length, 
			   (char *)scip->app_data));
}

#ifdef __STDC__
int rki_get_csaddr(struct sockaddr_in *caddrp, struct sockaddr_in *saddrp)
#else
int rki_get_csaddr(caddrp, saddrp)
  struct sockaddr_in *caddrp;
  struct sockaddr_in *saddrp;
#endif /* __STDC__ */
{
    int addrlen = sizeof(struct sockaddr_in);
    
    memcpy(saddrp, &saddr, addrlen);

    if (getsockname(sock, (struct sockaddr *) caddrp, &addrlen) < 0) {
	sprintf(errbuf, "getsockname: %s", strerror(errno));
	rkinit_errmsg(errbuf);
	return(RKINIT_GETSOCK);
    }

    return(RKINIT_SUCCESS);
}

#ifdef __STDC__
void rki_drop_server(void)
#else
void rki_drop_server()
#endif /* __STDC__ */
{
    (void) rki_send_packet(sock, MT_DROP, 0, "");
}

#ifdef __STDC__
void rki_cleanup_rpc(void)
#else
void rki_cleanup_rpc()
#endif /* __STDC__ */
{
    rki_drop_server();
    (void) close(sock);
}
