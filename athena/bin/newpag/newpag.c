#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(argc, argv)
	int argc;
	char **argv;
{
	char *shell = getenv("SHELL");
	int code = setpag();

	if (code != 0) {
		perror("setpag");
		exit(1);
	}
	if (argc == 1)
		execl(shell != NULL ? shell : "/bin/sh", "sh", 0);
	else 
		execvp(argv[1], argv + 1);
	exit(1);
}
