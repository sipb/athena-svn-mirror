#include <sysdep.h>
#include "include/libjwgc.h"

/* $Id: JFile.c,v 1.1.1.1 2006-03-10 15:32:57 ghudson Exp $ */

int 
JSetupComm()
{
	mode_t prevmode;
	int sock;
	struct sockaddr_un sockaddr;
	char file[MAXNAMLEN];
	struct linger li;
	char *envptr;

	li.l_onoff = 1;
	li.l_linger = 900;

	prevmode = umask(S_IRWXO|S_IRWXG);
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}
	envptr = getenv("JWGCSCK");
	if (!envptr) {
		(void) sprintf(file, "/tmp/jwgcsck.%d", getuid());
		envptr = file;
	}
	unlink(envptr);

	sockaddr.sun_family = AF_UNIX;
	strcpy(sockaddr.sun_path, envptr);
	if (bind(sock, (struct sockaddr *)&sockaddr,
		strlen(sockaddr.sun_path)+1 + sizeof(sockaddr.sun_family))
			< 0) {
		return -1;
	}
	if (listen(sock, 10) < 0) {
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li))
			== -1) {
		return -1;
	}


	umask(prevmode);

	return sock;
}

int 
JConnect()
{
	int sock;
	struct sockaddr_un sockaddr;
	char file[MAXNAMLEN];
	struct linger li;
	char *envptr;

	li.l_onoff = 1;
	li.l_linger = 900;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}
	envptr = getenv("JWGCSCK");
	if (!envptr) {
		(void) sprintf(file, "/tmp/jwgcsck.%d", getuid());
		envptr = file;
	}
	sockaddr.sun_family = AF_UNIX;

	strcpy(sockaddr.sun_path, envptr);
	if (connect(sock, (struct sockaddr *)&sockaddr,
		strlen(sockaddr.sun_path)+1 + sizeof(sockaddr.sun_family))
			< 0) {
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li))
			== -1) {
		return -1;
	}


	return sock;
}

void 
JCleanupSocket()
{
	char savefile[MAXNAMLEN];
	FILE *savefd;
	char *envptr;

	envptr = getenv("JWGCSCK");
	if (!envptr) {
		(void) sprintf(savefile, "/tmp/jwgcsck.%d", getuid());
		envptr = savefile;
	}
	unlink(envptr);
}
