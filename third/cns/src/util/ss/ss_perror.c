/*
 * Copyright 1987 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */
#include "mit-sipb-copyright.h"
#include "ss_internal.h"

static char C_SP[] = ": ";
static char SP_OPEN[] = " (";

char *
ss_name(sci_idx)
	int sci_idx;
{
	register char *ret_val;
	register ss_data *infop;

	infop = ss_info(sci_idx);
	if (infop->current_request == (char *)NULL) {
		ret_val = malloc((unsigned)
				 (strlen(infop->subsystem_name)+1)
				 * sizeof(char));
		if (ret_val == (char *)NULL)
			return((char *)NULL);
		strcpy(ret_val, infop->subsystem_name);
		return(ret_val);
	}
	else {
		ret_val = malloc((unsigned)sizeof(char) * 
				 (strlen(infop->subsystem_name)+
				  strlen(infop->current_request)+
				  4));
		strcpy(ret_val, infop->subsystem_name);
		strcat(ret_val, SP_OPEN);
		strcat(ret_val, infop->current_request);
		strcat(ret_val, ")");
		return(ret_val);
	}
}

void
ss_perror(sci_idx, code, message)
	int sci_idx, code;
	char *message;
{
	register ss_data *info = ss_info(sci_idx);
	/*
	 * We assume things are breaking down -- can't
	 * even trust malloc, since we might be reporting
	 * errors returned from it.
	 */
	fputs(info->subsystem_name, stderr);
	if (info->current_request) {
		fputs(SP_OPEN, stderr);
		fputs(info->current_request, stderr);
		fputc(')', stderr);
	}
	fputs(C_SP, stderr);
	if (code)
		fputs(error_message(code), stderr);
	if (*message) {	/* want non-empty message */
		if (code) /* don't put double */
			fputs(C_SP, stderr);
		fputs(message, stderr);
	}
	fputc('\n', stderr);
}
