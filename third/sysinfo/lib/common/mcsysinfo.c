/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * This file contains the primary documented API for SysInfo
 */

#include "defs.h"
#include <sys/errno.h>

typedef struct {
    int			Cmd;		/* MCSI_* value */
    int		      (*Func)();	/* Function to call */
} CmdSwitch_t;

CmdSwitch_t CmdSwitch[] = {
    { MCSI_HOSTNAME,		GetHostName },
    { MCSI_HOSTADDRS,		GetHostAddrs },
    { MCSI_HOSTALIASES,		GetHostAliases },
    { MCSI_HOSTID,		GetHostID },
    { MCSI_APPARCH,		GetAppArch },
    { MCSI_KERNARCH,		GetKernArch },
    { MCSI_CPUTYPE,		GetCpuType },
    { MCSI_NUMCPU,		GetNumCpu },
    { MCSI_KERNVER,		GetKernVer },
    { MCSI_MAN,			GetMan },
    { MCSI_MANSHORT,		GetManShort },
    { MCSI_MANLONG,		GetManLong },
    { MCSI_OSDIST,		GetOSDist },
    { MCSI_OSNAME,		GetOSName },
    { MCSI_OSVER,		GetOSVer },
    { MCSI_SERIAL,		GetSerial },
    { MCSI_ROMVER,		GetRomVer },
    { MCSI_MODEL,		GetModel },
    { MCSI_BOOTTIME,		GetBootTime },
    { MCSI_CURRENTTIME,		GetCurrentTime },
    { MCSI_PHYSMEM,		GetMemory },
    { MCSI_VIRTMEM,		GetVirtMem },
    { MCSI_DEVTREE,		DeviceInfoMCSI },
    { MCSI_KERNELVAR,		KernelVarsMCSI },
    { MCSI_SYSCONF,		GetSysConf },
    { MCSI_SOFTINFO,		SoftInfoMCSI },
    { MCSI_PARTITION,		PartInfoMCSI },
    { 0 }
};

/*
 * mcSysInfo() is the exclusive exported API call.
 */
int mcSysInfo(Query)
     MCSIquery_t	       *Query;
{
    CmdSwitch_t       	       *Cmds;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    switch (Query->Op) {
    case MCSIOP_PROGRAM:
	if (Query->InSize && Query->In) {
	    ProgramName = (char *) Query->In;
	    return 0;
	}
	return -1;
	break;
    default:
	for (Cmds = CmdSwitch; Cmds && Cmds->Cmd; ++Cmds) {
	    if (Cmds->Cmd == Query->Cmd)
		return Cmds->Func(Query);
	}
    }

    SImsg(SIM_GERR, "No such mcSysInfo() command as %d", Query->Cmd);
    errno = ENOENT;

    return -1;
}
