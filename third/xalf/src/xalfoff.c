/*
 * xalfoff - turnoff xalf indicators
 *
 * Peter Åstrand <astrand@lysator.liu.se> 2001. GPLV2. 
 *
 * */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#undef DEBUG 
/* Uncomment below for debugging */
/* #define DEBUG */

#ifdef DEBUG
#   define DPRINTF(args) (fprintf args)
#else
#   define DPRINTF(args) 
#endif

#define PID_ENV_NAME "XALF_LAUNCH_PID"

int main(int   argc,
	 char *argv[])
{
    const char *pid_string;
    long int launch_pid;
    
    pid_string = getenv (PID_ENV_NAME);

    if (!pid_string)
	{
	    DPRINTF ((stderr, "%s: Error: %s not found\n", argv[0], PID_ENV_NAME));
	    exit (1); 
	}

    launch_pid = atol (pid_string);
    
    DPRINTF ((stderr, "xalfoff: Sending signal to process %ld\n", launch_pid));
    
    kill (launch_pid, SIGUSR1);

    return (0);
}
