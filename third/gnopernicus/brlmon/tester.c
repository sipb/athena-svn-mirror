/* tester.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */ 
#include <glib.h>
#include <stdlib.h>
#include "util.c"

#define REMOTE_SERVER_PORT 7000
#ifdef INET6
    struct sockaddr_in6	cliAddr, 
			*remoteServAddr;
    #define FL_AF_INET	AF_INET6
    #define IP_TYPE	"IPv6"
    #define sockaddrin	sockaddr_in6
    #define SERV_HOST_ADDR	"::1"	/* host addr for server*/
#else
    struct sockaddr_in	cliAddr, 
			*remoteServAddr;
    #define FL_AF_INET	AF_INET
    #define IP_TYPE	"IPv"
    #define sockaddrin	sockaddr_in
    #define SERV_HOST_ADDR	"127.0.0.1"	/* host addr for server*/
#endif
#define MAX_MSG 100
#define BUFFER_SIZE 256

    
FILE* fp;
int len;
char* buff;

int sd, rc, i;
LincSockLen 		remoteServAddr_len;

void load ()
{
	
	fp = fopen ("brl.xml", "rt");
	
	if (fp)
	{
		/* get file len */
		fseek (fp, 0, SEEK_END);
		len = ftell (fp);
		rewind (fp);
		
		/* read buffer im memory */
		buff = (char* ) malloc (len + 1);
		fread (buff, 1, len, fp);
		
		fclose (fp);
	}
	else
	{
		fprintf (stderr, "Could not open file\n");
	}
}

void parse_cod(char *buff)
{
    int len = strlen(buff);
    int no_of_pack = len/(BUFFER_SIZE-1);
    int rest = len % (BUFFER_SIZE-1);
    gchar *tmp = buff;
    gchar sendtmp[BUFFER_SIZE];
    int i;
    int ended = 0;
    
    if (rest != 0) no_of_pack++;
        
    /*g_message("%d %d %d",len,no_of_pack,rest); */
    
    memset(sendtmp,0x0,sizeof(sendtmp));
    sendtmp[0]=0x02;
    rc = sendto(sd, sendtmp, sizeof(sendtmp), 0, 
		(struct sockaddr *) remoteServAddr, 
		remoteServAddr_len);
		
    if (rc < 1) return;
    for(i=0;i < no_of_pack;i++)
	{
	    memset(sendtmp,0x0,sizeof(sendtmp));
	    
	    strncpy(sendtmp,tmp,BUFFER_SIZE-1);
	
	    fprintf(stderr,"Length:%d Pos:%d\n",len, tmp - buff);
	    tmp = tmp + (BUFFER_SIZE-1);
	    if (i + 1 == no_of_pack)
	    if (rest < BUFFER_SIZE - 1)
		    {
		    sendtmp[rest]=0x03;
		    sendtmp[rest+1]='\0';
		    ended = 1;
		    }
/*		else
		    sendtmp[(BUFFER_SIZE-1)]='\0';*/
	    	    
	    		
	    rc = sendto(sd, sendtmp, sizeof(sendtmp), 0, 
		(struct sockaddr *) remoteServAddr, 
		remoteServAddr_len);
		
	    g_message("%s",sendtmp);
	}
	
    if (ended == 0)
    {
	memset(sendtmp,0x0,sizeof(sendtmp));
	sendtmp[0]=0x03;
	rc = sendto(sd, sendtmp, sizeof(sendtmp), 0, 
		(struct sockaddr *) remoteServAddr, 
		remoteServAddr_len);
    }
}


void 
create_udp_client ()
{
    if ((sd = socket(FL_AF_INET, SOCK_DGRAM, 0)) < 0) {
	printf("Can not open socket \n");
	exit(1);
	}

    /* bind local server port */
#ifdef INET6
    fprintf(stderr,"IPV6:%s",SERV_HOST_ADDR);
    cliAddr.sin6_family 	= FL_AF_INET;
    cliAddr.sin6_addr 		= in6addr_any;
    cliAddr.sin6_port 		= htons(0);
#else
    cliAddr.sin_family 		= FL_AF_INET;
    cliAddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
    cliAddr.sin_port 		= htons(0);
#endif
  
    if (bind (sd, (struct sockaddr *) &cliAddr,sizeof(cliAddr)) < 0) {
	    exit(1);
	}

    g_message("ServerUDP:Waiting for data on port UDP %u\n",REMOTE_SERVER_PORT);    

}

int 
main(int argc, char *argv[]) 
{
  
    load();
    
    create_udp_client();
  
    parse_cod(buff);
    
    sleep(10);  
    
/*    parse_cod("QUIT"); */
    
    g_free(buff);
    
    return 1;
}
