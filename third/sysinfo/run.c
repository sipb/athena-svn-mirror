/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: run.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Things related to running system commands.
 */

#include "defs.h"

/*
 * Need default environment for some OS's like HP-UX.
 */
static char		       *DefEnviron[] = {
    "HOME=/dev/null",
    NULL };
extern char		      **Environ;
uid_t				SavedUserID;

/*
 * Set our User ID.
 */
int SetUserID(RealUID, EffectUID)
    uid_t			RealUID;
    uid_t			EffectUID;
{
    if (Debug) printf("SetUserID(%d, %d) current: ruid=%d euid=%d\n", 
		      RealUID, EffectUID, getuid(), geteuid());

    if (setreuid(RealUID, EffectUID) == -1) {
	if (Debug) Error("setreuid to %d, %d failed: %s", 
			 RealUID, EffectUID, SYSERR);
	return(-1);
    }

    if (Debug) printf("SetUserID(%d, %d) new: ruid=%d euid=%d\n",
		      RealUID, EffectUID, getuid(), geteuid());

    return(0);
}

/*
 * Set environment variable "Key" to be "Value".
 */
int SetEnv(Key, Value)
    char		       *Key;
    char		       *Value;
{
    static char			Buff[BUFSIZ];

    (void) sprintf(Buff, "%s=%s", Key, (Value) ? Value : "");
    if (putenv(strdup(Buff)) != 0) {
	if (Debug) Error("putenv(%s) failed.", Buff);
	return(-1);
    }

    return(0);
}

/*
 * Initialize environment before executing external command.
 */
int ExecInit(WithPrivs)
    int				WithPrivs;
{
    static int			First = TRUE;
    register char	      **PtrPtr;

    if (First) {
	First = FALSE;
	SavedUserID = (uid_t) -1;
	/*
	 * Remove environment variables considered to be a security risk.
	 */
	for (PtrPtr = Environ; PtrPtr && *PtrPtr; ++PtrPtr) {
	    if (EQN(*PtrPtr, "IFS=", 4)) {
		if (SetEnv("IFS", NULL) < 0)
		    return(-1);
	    } else if (EQN(*PtrPtr, "LD_", 3)) {
		if (SetEnv(*PtrPtr, NULL) < 0)
		    return(-1);
	    }
	}
    }

    /*
     * Only change user ID if we're setuid root (uid==0).
     */
    if (!WithPrivs && (geteuid() == 0) && ((SavedUserID = getuid()) != 0))
	if (SetUserID(0, SavedUserID) == -1)
	    return(-1);

    return(0);
}

/*
 * Reset things after executing external command.
 */
int ExecEnd(WithPrivs)
    int				WithPrivs;
{
    if (SavedUserID != (uid_t)-1 && SetUserID(SavedUserID, 0) == -1)
	return(-1);
    return(0);
}

/*
 * Run a list of commands (found in cmds) and return command output.
 */
extern char *RunCmds(Cmds, WithPrivs)
    char 		      **Cmds;
    int				WithPrivs;
{
    static char			Buf[BUFSIZ];
    int 			l;
    int				Done = 0;
    FILE 		       *pf;
    register char 	       *p;
    char 		      **Cmd;

    if (ExecInit(WithPrivs) != 0)
	    return((char *)NULL);

    Buf[0] = C_NULL;
    for (Cmd = Cmds; Cmd != NULL && *Cmd != NULL && !Done; ++Cmd) {
	/*
	 * If this command has any args, nuke them for the access() test.
	 */
	strcpy(Buf, *Cmd);
	p = strchr(Buf, ' ');
	if (p != NULL)
	    *p = C_NULL;

	if (access(Buf, X_OK) != 0)
	    continue;

	if (Debug) printf("RunCmd '%s' %s Privs\n", 
			  *Cmd, (WithPrivs) ? "With" : "Without");

	if ((pf = popen(*Cmd, "r")) == NULL)
	    continue;
	if (fgets(Buf, sizeof(Buf), pf) == NULL) {
	    pclose(pf);
	    continue;
	}
	pclose(pf);

	l = strlen(Buf);
	if (Buf[l-1] == '\n') 
	    Buf[l-1] = C_NULL;

	Done = TRUE;
    }
 
    if (ExecEnd(WithPrivs) != 0)
	    return((char *)NULL);

    return((Buf[0]) ? Buf : (char *)NULL);
}

/*
 * Wait for a given process to exit and return
 * that processes exit status.
 */
#if	WAIT_TYPE == WAIT_WAITPID
int WaitForProc(ProcID)
    pid_t			ProcID;
{
    pid_t			RetProcID;
    waitarg_t			ProcStatus;

    RetProcID = waitpid(ProcID, &ProcStatus, 0);

    if (RetProcID == ProcID)
	if (WIFEXITED(ProcStatus))
	    return(WAITEXITSTATUS(ProcStatus));
	else {
	    Error("waitpid(%d, , 0) failed and returned %d: %s.", 
		  ProcID, RetProcID, SYSERR);
	    return(-1);
	}
    else
	return(-1);
}
#endif	/* WAIT_WAITPID */
#if	WAIT_TYPE == WAIT_WAIT4
int WaitForProc(ProcID)
    pid_t			ProcID;
{
    pid_t			RetProcID;
    waitarg_t			ProcStatus;

    RetProcID = wait4(ProcID, &ProcStatus, 0, NULL);

    if (RetProcID == ProcID)
	if (WIFEXITED(ProcStatus))
	    return(WAITEXITSTATUS(ProcStatus));
	else {
	    Error("wait4(%d) failed and returned %d: %s.", 
		  ProcID, RetProcID, SYSERR);
	    return(-1);
	}
    else
	return(-1);
}
#endif	/* WAIT_WAIT4 */

/*
 * Execute a command with given arguments.
 */
int Execute(Cmd, Argv, Env, WithPrivs, StdOut, StdErr)
    char		       *Cmd;
    char		      **Argv;
    char		      **Env;
    int				WithPrivs;
    int			        StdOut;
    int			        StdErr;
{
    pid_t			ProcID = 0;
    int				Status;
    register char	      **PtrPtr;

    if (access(Cmd, X_OK) != 0)
	return(-1);

    if (!Env)
	Env = DefEnviron;

    if (Debug) {
	printf("Execute '%s'", Cmd);
	for (PtrPtr = Argv; PtrPtr && *PtrPtr; ++PtrPtr)
	    printf(" '%s'", *PtrPtr);
	printf("\t%s Privs\n", (WithPrivs) ? "With" : "Without");
    }

    ProcID = fork();
    if (ProcID < 0) {
	Error("Fork failed: %s", SYSERR);
	return(-1);
    } else if (ProcID == 0) {
	/*
	 * Child
	 */
	if (StdOut >= 0)
	    if (dup2(StdOut, fileno(stdout)) < 0)
		Error("dup2(%d, stdout) failed: %s.", StdOut);
	if (StdErr >= 0)
	    if (dup2(StdErr, fileno(stderr)) < 0)
		Error("dup2(%d, stderr) failed: %s.", StdErr);
	ExecInit(WithPrivs);
	execve(Cmd, Argv, Env);
	Error("Execve \"%s\" failed: %s", Cmd, SYSERR);
	exit(127);
    } else {
	/*
	 * Parent
	 */
	Status = WaitForProc(ProcID);
	if (Debug) printf("\tCommand '%s' exited %d.\n", Cmd, Status);
	return(Status);
    }
    return(-1);
}

#if	defined(RUN_TEST_CMD)
static char		       *RunTestCmd[] = RUN_TEST_CMD;
#endif	/* RUN_TEST_CMD */

/*
 * Get the Argument Vector for the command to run.
 */
static char **GetRunArgv(Command)
    char		       *Command;
{
    static char		      **Argv = NULL;
    char		       *Base;
#if	defined(RUN_TEST_CMD)
    register char	      **ArgvPtr;
    register char	      **PtrPtr;
    register int		Count;

    for (Count = 0, PtrPtr = RunTestCmd; PtrPtr && *PtrPtr; ++PtrPtr, ++Count);

    if (Argv)
	(void) free(Argv);
    ArgvPtr = Argv = (char **) xmalloc((Count+2) * sizeof(char *));

    for (PtrPtr = RunTestCmd; PtrPtr && *PtrPtr; ++PtrPtr, ++ArgvPtr)
	*ArgvPtr = *PtrPtr;
    *ArgvPtr = Command;
    *++ArgvPtr = NULL;
#else	/* !RUN_TEST_CMD */
    Base = strrchr(Command, '/');
    if (Base)
	++Base;
    else
	Base = Command;
    if (Argv)
	(void) free(Argv);
    Argv = (char **) xmalloc(4 * sizeof(char *));
    Argv[0] = Command;
    Argv[1] = Base;
    Argv[2] = NULL;
#endif	/* RUN_TEST_CMD */

    return(Argv);
}
    

/*
 * Run a list of test files.  Each test file is run and if the
 * exit status is 0, we return the basename of the command.
 * e.g. If "/bin/vax" exists and returns status 0, return string "vax".
 */
extern char *RunTestFiles(Cmds)
    char 		      **Cmds;
{
    char 		      **Cmd;
    char		      **RunEnv;
    char		      **Argv;
    char		       *Name = NULL;
    register char	       *p;
    static char			Buf[BUFSIZ];
    int				StdOut = -1;
    int				StdErr = -1;

    /*
     * Setup stdout/stderr to go to /dev/null since we
     * only care about the exit status of commands.
     */
    if (!Debug) {
	StdOut = open(_PATH_NULL, O_WRONLY);
	StdErr = open(_PATH_NULL, O_WRONLY);
    }

    for (Cmd = Cmds; Name == NULL && Cmd != NULL && *Cmd != NULL; ++Cmd) {
	/*
	 * If this command has any args, nuke them for the access() test.
	 */
	strcpy(Buf, *Cmd);
	p = strchr(Buf, ' ');
	if (p != NULL)
	    *p = C_NULL;

	if (access(Buf, X_OK) != 0)
	    continue;

	/*
	 * Execute the command with a NULL environment for security
	 * reasons.
	 */
	Argv = GetRunArgv(*Cmd);
	if (Execute(Argv[0], &Argv[1], (char **)NULL, 0, StdOut, StdErr) != 0)
	    continue;

	/*
	 * The name of this architecture is the last part of the Cmd name.
	 */
	strcpy(Buf, *Cmd);
	p = strrchr(Buf, '/');
	if (p != NULL)
	    ++p;
	Name = p;
    }

    if (StdOut >= 0)
	(void) close(StdOut);
    if (StdErr >= 0)
	(void) close(StdErr);

    return(Name);
}
