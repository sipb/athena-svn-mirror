/* Copyright Milan Technology, 1991 1992 */

/* @(#)udpstat.c	2.2 10/16/92 */

#include "udp.h"
#include <stdio.h>

#include <sys/types.h>

/* #include <sys/socket.h> */
#include <sys/errno.h>
#ifndef HPOLD
#include <netinet/in.h>
#endif
#include <signal.h>
#include "dp.h"

void            print_status_info();
int             get_udpstatus();
void            print_active_connection();

/* UDP Status packet */

udp_status_packet udp_stat;

#ifdef ANSI
int
get_printer_status(char *printer, char *error_string, int port)
#else
int
get_printer_status(printer, error_string, port)
	char           *printer;
	char           *error_string;
	int             port;
#endif
{
	int             fastport;
	struct sockaddr_in fp_sock, my_sock;
	unsigned long   temp;
	struct hostent *hp;
	int status;

	bzero((char *) &fp_sock, sizeof(fp_sock));
	fp_sock.sin_family = AF_INET;

	if (!(hp = gethostbyname(printer))) {
		return(0);
	}
	temp = *(unsigned long *) hp->h_addr;
	fp_sock.sin_addr.s_addr = temp;
	fp_sock.sin_port = htons(35);
	if ((fastport = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("error on socket\n");
		return(0);
	}
	/*
	 * need to be able to receive status so bind the socket
	 */
	bzero((char *) &my_sock, sizeof(my_sock));
	my_sock.sin_family = AF_INET;
	my_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	my_sock.sin_port = htons(0);
	if (bind(fastport, (struct sockaddr *) & my_sock, sizeof(my_sock)) < 0) {
		perror("can't bind:");
		return(0);
	}
	status = get_udpstatus(fastport, (struct sockaddr *) & fp_sock, sizeof(fp_sock),
		      error_string, port, printer);
	close(fastport);
	return(status);

}
#ifdef ANSI
int
get_udpstatus(int fastport, struct sockaddr * server, int server_length,
	      char *error_string, int port, char *fastport_name)
#else
int
get_udpstatus(fastport, server, server_length, error_string, port, fastport_name)
	int             fastport;
	struct sockaddr *server;
	int             server_length;
	char           *error_string;
	int             port;
	char           *fastport_name;
#endif
{
	char            command[10];

	int             nbytes, sel_val;
	fd_set          readfds;
	struct timeval  timeout;
	struct sockaddr_in rec_address;
	int             reclen;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	
	command[0] = MILAN_OP; /* a milan query */
	command[1] = GET_STATUS; /* action required */
	
	if ((nbytes = sendto(fastport, command, sizeof(command), 0, server, server_length)) !=
	    sizeof(command)) {
		perror("Cannot send to server:");
		exit(1);
	}
	while (1) {
		FD_ZERO(&readfds);
		FD_SET(fastport, &readfds);
		if ((sel_val = select(fastport + 1, &readfds, 0, 0, &timeout)) < 0) {
			sprintf(error_string, "no response from server \n");
			return(0);
		}
		if (!sel_val) {
			sprintf(error_string, "no response from server %s for %s port\n", fastport_name, port == 2001 ? "Serial" : "Parallel");
			return(0);;
		}
		if (FD_ISSET(fastport, &readfds))
			break;
	}

	reclen = sizeof(rec_address);
	if ((nbytes = recvfrom(fastport, (char *)&udp_stat,
			       sizeof(udp_stat), 0, 
			       (struct sockaddr *)&rec_address, &reclen)) <
	    0) { 
		sprintf(error_string, "udp recvfrom returned error");
		return(0);
	}
	print_status_info(&udp_stat, error_string, port);
	return(1);
}
#ifdef  ANSII
void
print_status_info(udp_status_packet * stat_buff, char *error_string, int port)
#else
void
print_status_info(stat_buff, error_string, port)
	udp_status_packet *stat_buff;
	char           *error_string;
	int             port;
#endif
{
	char            temp_string[100];
	struct hostent *hp;
	unsigned long   host_addr;
	if (port == PARALLEL) {
		if (stat_buff->parallel_addr) {
#ifndef SCO
			host_addr = ntohl(stat_buff->parallel_addr);
#else
			host_addr = stat_buff->parallel_addr;
#endif
			print_active_connection(&host_addr, "parallel ", error_string);
			sprintf(temp_string, "%s\n", stat_buff->message);
			strcat(error_string, temp_string);
			sprintf(temp_string, "Total parallel bytes sent: %d\n\n", ntohl(stat_buff->parallel_bytes));
			strcat(error_string, temp_string);
		} else {
			strcat(error_string, "No active parallel connection\n");
			sprintf(temp_string, "%s\n", stat_buff->message);
			strcat(error_string, temp_string);
			sprintf(temp_string, "Total parallel bytes sent: %d\n\n", ntohl(stat_buff->parallel_bytes));
			strcat(error_string, temp_string);
		}
	}
	if (port == SERIAL) {
		if (stat_buff->serial_addr) {
#ifndef SCO
			host_addr = ntohl(stat_buff->serial_addr);
#else
			host_addr = stat_buff->serial_addr;
#endif
			print_active_connection(&host_addr, "serial ", error_string);
			sprintf(temp_string, "Total serial bytes sent: %d\n\n", ntohl(stat_buff->serial_bytes));
			strcat(error_string, temp_string);
		} else {
			strcat(error_string, "No active serial connection\n");
			sprintf(temp_string, "Total serial bytes sent: %d\n\n", ntohl(stat_buff->serial_bytes));
			strcat(error_string, temp_string);
		}
	}
}

#ifdef ANSI
void
print_active_connection(unsigned long *host_address, char *kind, char *error_string)
#else
void
print_active_connection(host_address, kind, error_string)
	unsigned long  *host_address;
	char           *kind;
	char           *error_string;
#endif
{
	struct hostent *hp;
	char            temp_string[100];
#ifdef __STDC__	
	if (hp = gethostbyaddr((const char *)host_address, 4, AF_INET)) {
#else
	if (hp = gethostbyaddr((char *)host_address, 4, AF_INET)) {
#endif	    
		unsigned long newaddr;
		newaddr = *(unsigned long *)hp->h_addr;
		sprintf(temp_string, "\nActive  %s data session from %s (%s)\n", kind, hp->h_name,
			inet_ntoa((struct in_addr *) (&newaddr)));
	} else {
			sprintf(error_string, "\nActive %s data session from %s\n", kind,
				inet_ntoa((struct in_addr *) (host_address)));
			return;
	}
	strcat(error_string, temp_string);
}
