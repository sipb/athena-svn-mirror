/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Run a Command
 */

#include "defs.h"
#include "cmd.h"
#include <errno.h>

extern int			errno;

/*
 * Find the first command in Path that exists 
 */
extern char *CmdFind(Path)
     char		      **Path;
{
    register int		i;

    for (i = 0; Path[i]; ++i)
	if (access(Path[i], X_OK) == 0)
	    return Path[i];

    return (char *) NULL;
}

/*
 * Open (create) a command.
 * Cmd->FD is the filedescriptor to read/write from/to.
 */
extern int CmdOpen(Cmd)
    Cmd_t		       *Cmd;
{
    pid_t			ProcID = 0;
    char		      **PtrPtr;
    int				IOpipe[2];
    int				ChildEnd;
    int				ChildStdEnd;
    int				ParentEnd;
    int				NullFD;
    int				WithPrivs = FALSE;
    char		       *cp;

    if (!Cmd) {
	errno = EINVAL;
	return -1;
    }

    if (FLAGS_ON(Cmd->Flags, CMF_WITHPRIVS))
	WithPrivs = TRUE;

    Cmd->Program = CmdFind(Cmd->CmdPath);
    if (!Cmd->Program) {
	cp = strrchr(Cmd->Argv[0], '/');
	if (cp)
	    ++cp;
	else
	    cp = Cmd->Argv[0];
	SImsg(SIM_INFO, "%s: Cannot locate executable command.", cp);
	return -1;
    }
    Cmd->Argv[0] = Cmd->Program;

    if (Debug) {
	SImsg(SIM_INFO, "CmdOpen  <%s>", Cmd->Program);
	for (PtrPtr = Cmd->Argv+1; PtrPtr && *PtrPtr; ++PtrPtr)
	    SImsg(SIM_INFO, " <%s>", *PtrPtr);
	SImsg(SIM_INFO, "\t%s Privs\n", (WithPrivs) ? "With" : "Without");
    }

    /*
     * Create the I/O pipe
     */
    if (pipe(IOpipe) < 0) {
	SImsg(SIM_GERR, "Create pipe failed: %s", SYSERR);
	return -1;
    }

    /*
     * Setup descriptors
     */
    if (FLAGS_ON(Cmd->Flags, CMF_READ)) {
	ParentEnd = IOpipe[0];
	ChildEnd = IOpipe[1];
    } else if (FLAGS_ON(Cmd->Flags, CMF_WRITE)) {
	ParentEnd = IOpipe[1];
	ChildEnd = IOpipe[0];
    } else {
	SImsg(SIM_GERR, "CmdOpen() - Must specified CMF_READ or CMF_WRITE.");
	return -1;
    }

    ProcID = fork();

    if (ProcID < 0) {
	SImsg(SIM_GERR, "Fork failed, cannot run %s: %s", 
	      Cmd->Program, SYSERR);
	return -1;
    } else if (ProcID == 0) {
	/*
	 * Child
	 */

	(void) close(ParentEnd);
	ChildStdEnd = (FLAGS_ON(Cmd->Flags, CMF_READ)) ? 1 : 0;
	if (ChildEnd != ChildStdEnd)
	    if (dup2(ChildEnd, ChildStdEnd) < 0)
		SImsg(SIM_GERR, "dup2(%d, %d) failed: %s.", 
		      ChildEnd, ChildStdEnd, SYSERR);
	if (!Debug && FLAGS_OFF(Cmd->Flags, CMF_STDERR)) {
	    /*
	     * Redirect stderr to /dev/null
	     */
	    NullFD = open(_PATH_NULL, O_WRONLY);
	    (void) dup2(NullFD, fileno(stderr));
	}

	ExecInit(WithPrivs);
	execve(Cmd->Program, Cmd->Argv, Cmd->Env);

	SImsg(SIM_GERR, "Execve \"%s\" failed: %s", Cmd->Program, SYSERR);
	exit(127);
    } else {
	/*
	 * Parent
	 */
	(void) close(ChildEnd);

	Cmd->ProcID = ProcID;
	Cmd->FD = ParentEnd;

	return 0;
    }

    return -1;
}

/*
 * End a Cmd
 */
extern int CmdClose(Cmd)
     Cmd_t		       *Cmd;
{
    int				Status;

    if (!Cmd)
	return -1;

    (void) close(Cmd->FD);
    Status = WaitForProc(Cmd->ProcID);
    SImsg(SIM_DBG, "CmdClose <%s> exit status=%d", Cmd->Program, Status);

    return(Status);
}
