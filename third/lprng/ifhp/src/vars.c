/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: vars.c,v 1.1.1.1 1999-02-17 15:31:03 ghudson Exp $";

#define EXTERN
#include "ifhp.h"

/**** ENDINCLUDE ****/

struct keyvalue Valuelist[] = {

    {  "Accounting_script", "accounting", &Accounting_script, STRV  },
    {  "Autodetect", "autodetect", (char **)&Autodetect, FLGV },
    {  "Banner_file", "banner_file", &Banner_file, STRV  },
    {  "Banner_parse_inputline", "banner_parse_inputline", (char **)&Banner_parse_inputline, FLGV },
    {  "Banner_suppressed", "banner_suppressed", (char **)&Banner_suppressed, FLGV },
    {  "Banner_user", "banner_user", (char **)&Banner_user, FLGV },
    {  "Crlf", "crlf", (char **)&Crlf, FLGV },
    {  "CTRL_D_at_start", "ctrl_d_at_start", (char **)&CTRL_D_at_start, FLGV },
    {  "Dev_retries", "dev_retries", (char **)&Dev_retries, INTV },
    {  "Dev_sleep", "dev_sleep", (char **)&Dev_sleep, INTV },
    {  "Device", "dev", &Device, STRV },
    {  "Force_status", "forcestatus", (char **)&Force_status, FLGV },
    {  "Full_time", "fulltime", (char **)&Full_time, FLGV },
    {  "Initial_timeout", "initial_timeout", (char **)&Initial_timeout, INTV },
    {  "Job_timeout", "job_timeout", (char **)&Job_timeout, INTV },
    {  "Logall", "logall", (char **)&Logall, FLGV },
    {  "Max_status_size", "statusfile_max", (char **)&Max_status_size, INTV },
    {  "Min_status_size", "statusfile_min", (char **)&Min_status_size, INTV },
    {  "Null_pad_count", "nullpad", (char **)&Null_pad_count, INTV },
    {  "No_PCL_EOJ", "no_pcl_eoj", (char **)&No_PCL_EOJ, FLGV },
    {  "No_PS_EOJ", "no_ps_eoj", (char **)&No_PS_EOJ, FLGV },
    {  "Pagecount_interval", "pagecount_interval", (char **)&Pagecount_interval, INTV },
    {  "Pagecount_poll", "pagecount_poll", (char **)&Pagecount_poll, INTV },
    {  "Pagecount_timeout", "pagecount_timeout", (char **)&Pagecount_timeout, INTV },
    {  "Pcl", "pcl", (char **)&Pcl, FLGV },
    {  "Pjl", "pjl", (char **)&Pjl, FLGV },
    {  "Pjl_enter", "pjl_enter", (char **)&Pjl_enter, FLGV },
    {  "Ps", "ps", (char **)&Ps, FLGV },
    {  "Pagecount_ps_code", "pagecount_ps_code", &Pagecount_ps_code, STRV },
    {  "Psonly", "psonly", (char **)&Psonly, FLGV },
    {  "Remove_ctrl", "remove_ctrl", &Remove_ctrl, STRV  },
    {  "Status", "status", (char **)&Status, FLGV },
    {  "Stty_args", "stty", &Stty_args, STRV },
    {  "Summaryfile", "summaryfile", &Summaryfile, STRV },
    {  "Sync_interval", "sync_interval", (char **)&Sync_interval, INTV },
    {  "Sync_timeout", "sync_timeout", (char **)&Sync_timeout, INTV },
    {  "Tbcp", "tbcp", (char **)&Tbcp, FLGV },
    {  "Text", "text", (char **)&Text, FLGV },
    {  "Trace_on_stderr", "trace_on_stderr", (char **)&Trace_on_stderr, FLGV },
    {  "Waitend_interval", "waitend_interval", (char **)&Waitend_interval, INTV },

    { 0, 0, 0 }
};
