/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: checkcode.c,v 1.3 2000-03-01 21:49:19 ghudson Exp $";

#include "ifhp.h"
#include <zephyr/zephyr.h>

/**** ENDINCLUDE ****/

static void send_zephyr(char *error);

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Check out the HP PCL 5 reference manual set; it includes the PJL manual
which contains a complete (as far as I know) listing of status/error codes.
In case you don't have the manual handy, here's some code I used to handle
the status codes. The code should be fairly obvious. Free feel to use it.

Eelco van Asperen            /                 Erasmus University Rotterdam
----------------------------/   Informatievoorziening en Automatisering FEW
vanasperen@facb.few.eur.nl / PObox 1738, 3000 DR Rotterdam, The Netherlands

The header file, pjlcode.h:

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/

#define	PJLC_GROUP(c)	((c)/1000)
#define	PJLC_CODE(c)	((c)%1000)

#define	PJLC_TRAY(c)	(PJLC_CODE(c) / 100)
#define	PJLC_MEDIA(c)	(PJLC_CODE(c) % 100)

typedef struct {
	int	code;
	char *msg;
} code_t;

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

And the actual code, pjlcode.c:

  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

code_t	 groupcode[] = {
	{ 10, "Informational" },
	{ 11, "Background Paper Mount" },
	{ 12, "Background Paper Tray" },
	{ 15, "Output Bin Status" },
	{ 20, "PJL Parser Errors" },
	{ 25, "PJL Parser Warning" },
	{ 27, "PJL Semantic Errors" },
	{ 30, "Auto Continuable Conditions" },
	{ 32, "PUL File System Errors" },
	{ 35, "Possible Operator Intervention Conditions" },
	{ 40, "Operator Intervention Requited" },
	{ 41, "Paper Mount Required" /* "Foreground Paper Mount" */ },
	{ 42, "Paper Jam" },
	{ 43, "External Paper Handling Device Errors" },
	{ 44, "LJ4000 Paper Jam" },
	{ 50, "Hardware Errors" },
	{ 0, "" },
};

static	code_t	pjlmsg[] = {
	/* Informational Message */
	{ 10000, "powersave mode" },
	{ 10001, "Ready Online" },
	{ 10002, "Ready Offline" },
	{ 10003, "Warming Up" },
	{ 10004, "Self Test" },
	{ 10005, "Reset" },
	{ 10006, "Toner Low" },
	{ 10007, "Cancelling Job" },
	{ 10010, "Status Buffer Overflow" },
	{ 10011, "auxiliary IO initialisation" },
	{ 10013, "self test" },
	{ 10014, "printing test" },
	{ 10015, "typeface list" },
	{ 10016, "engine test" },
	{ 10017, "demo page" },
	{ 10018, "menu reset" },
	{ 10019, "reset active IO" },
	{ 10020, "reset all IO" },
	{ 10022, "config page" },
	{ 10023, "processing job" },
	{ 10024, "waiting" },
	{ 10027, "remove paper from printer" },
	{ 10029, "formfeeding" },

	/* 11xxx (Background Paper Loading Messages */
	{ 11304, "tray 3 empty (tray 3=LC)" },

	/* Background paper tray status */
	{ 12201, "tray 2 open" },
	{ 12202, "tray 2 lifting" },
	{ 12301, "tray 3 open" },
	{ 12302, "tray 3 lifting" },


	/* PJL Parser Error */
	{ 20001, "generic syntax error" },
	{ 20002, "unsupported command" },
	{ 20004, "unsupported personality/system" },
	{ 20006, "illegal character or line terminated by the UEL command" },
	{ 20007, "whitespace or linefeed missing after closing quotes" },
	{ 20008, "invalid character in an alphanumeric value" },
	{ 20009, "invalid character in a numeric value" },
	{ 20010, "invalid character at the start of a string,"
			"alphanumeric value or numeric value" },
	{ 20011, "string missing closing double-quote character" },
	{ 20012, "numeric value starts with a decimal point" },
	{ 20013, "numeric value does not contain any digits" },
	{ 20014, "no alphanumeric value after command modifier" },
	{ 20015, "option name and equal sign encountered"
			" but value field is missing" },
	{ 20016, "more than one command modifier" },
	{ 20017, "command modifier encountered after an option" },
	{ 20018, "command not an alphanumeric value" },
	{ 20019, "numeric value encountered when"
			" an alphanumeric value expected" },
	{ 20020, "string encountered when an alphanumeric value expected" },
	{ 20021, "unsupported command modifier" },
	{ 20022, "command modifier missing" },
	{ 20023, "option missing" },
	{ 20024, "extra data received after option name" },
	{ 20025, "two decimal points in a numeric value" },

	/* PJL Parser Warning */
	{ 25001, "generic warning" },
	{ 25002, "PJL prefix missing" },
	{ 25003, "alphanumeric value too long" },
	{ 25004, "string too long" },
	{ 25005, "numeric value too long" },
	{ 25006, "unsupported option name" },
	{ 25007, "option name requires a value which is missing" },
	{ 25008, "option name requires value of a different type" },
	{ 25009, "option name received with a value,"
			" but this option does not support values" },
	{ 25010, "same option name received more than once" },
	{ 25011, "ignored option name due to value underflow or overflow" },
	{ 25012, "value truncated or rounded" },
	{ 25013, "value out of range; used closest supported limit" },
	{ 25014, "value out of range; ignored" },
	{ 25016, "option name received with an alphanumeric value,"
			" but this value is not supported" },
	{ 25017, "string empty, option ignored" },

	/* PJL Semantic Errors; */
	{ 27001, "generic sematic error" },
	{ 27002, "EOJ encountered without a previous matching JOB command" },
	{ 27004, "value of a read-only variable can not be changed" },

	/* Auto-Continuable Condition */
	{ 30010, "status buffer overflow" },
	{ 30016, "memory overflow" },
	{ 30017, "print overrun" },
	{ 30018, "communication error" },
	{ 30027, "IO buffer overflow" },
	{ 30034, "paper feed error" },
	{ 30035, "NVRAM error" },
	{ 30036, "NVRAM full" },

	/* Possible Operator Intervention Condition */
	{ 35029, "W1 image adapt" },
	{ 35031, "W2 invalid personality" },
	{ 35037, "W3 job aborted" },
	{ 35039, "W9 job 600/LTR" },
	{ 35040, "W0 job 600/A4" },
	{ 35041, "W8 job 600/OFF" },
	{ 35042, "W7 job 300/LGL" },
	{ 35043, "W5 job 300/LTR" },
	{ 35044, "W6 job 300/A4" },
	{ 35045, "W4 job 300/OFF" },
	/* turn off from control panel using SERVICE MSG = NO */
	{ 35075, "User Maintenance Required (200000 copies done)" },
	{ 35078, "powersave mode" },

	/* Operator Intervention Needed */
	{ 40010, "no EP cartridge" },
	{ 40019, "upper out-bin full" },
	{ 40021, "printer open" },
	{ 40022, "paper jam" },
	{ 40024, "FE cartridge" },
	{ 40026, "PC install" },
	{ 40038, "toner low" },
	{ 40046, "insert cartridge" },
	{ 40047, "remove cartridge" },
	{ 40048, "[PJL OpMsg]" },
	{ 40049, "[PJL StMsg]" },
	{ 40050, "service error 50" },
	{ 40051, "temporary error 51" },
	{ 40052, "temporary error 52" },
	{ 40053, "memory error" },
	{ 40054, "54 error" },
	{ 40055, "temporary error 55" },
	{ 40056, "56 Error" },
	{ 40057, "service error 57" },
	{ 40058, "service error 58" },
	{ 40059, "59 error" },
	{ 40061, "RAM parity error" },
	{ 40062, "error during memory check" },
	{ 40063, "internal RAM error" },
	{ 40064, "internal service error 64" },
	{ 40065, "65 error" },
	{ 40067, "67 error" },
	{ 40069, "70 error" },
	{ 40070, "71 error" },
	{ 40071, "72 error" },
	{ 40079, "offline" },

	/* 41xxx (Background Paper Loading) handled by pjl_message() */

	{ 42014, "13.14 paper jam, clear all pages" },
	{ 42114, "13.14 paper jam, clear 1 page" },
	{ 42202, "13.2 paper jam, remove 2 pages" },
	{ 0, 0 },
};

static	code_t	traytab[] = {
	{ 0, "MP" },
	{ 1, "Manual Feed" },
	{ 2, "PC/Upper/Tray2" },
	{ 3, "LC/Lower/Tray3" },
	{ 4, "EE" },
	{ 5, "HCI" },
	{ 0, 0 },
};

static	code_t	mediatab[] = {
	{ 0, "Unknown paper" },
	{ 1, "Unknown envelope" },
	{ 2, "Letter paper" },
	{ 3, "Legal paper" },
	{ 4, "A4 paper" },
	{ 5, "Exec paper" },
	{ 6, "Ledger paper" },
	{ 7, "A3 paper" },
	{ 8, "COM10 envelope" },
	{ 9, "Monarch envelope" },
	{ 10, "C5 envelope" },
	{ 11, "DL envelope" },
	{ 12, "B4 paper" },
	{ 13, "B5 paper" },
	{ 14, "B5 envelope" },
	{ 15, "Custom media" },
	{ 16, "Hagaki" },
	{ 17, "Oufuku-Hagaki" },
	{ 0, 0 },
};

char * lookup(code_t *tab, int target)
{
	DEBUG4("lookup_code: code %d", target );
	while( tab->msg && tab->code != target ) ++tab;
	return( tab->msg );
}

char * pjl_message(char *code_str)
{
	int code = atoi(code_str);

	if (PJLC_GROUP(code) == 11
	 || PJLC_GROUP(code) == 41) {
		/* Background/Foreground Paper Loading */
		static char buf[SMALLBUFFER];
		char *tray, *media;
		
		tray = lookup(traytab, PJLC_TRAY(code)),
		media = lookup(mediatab, PJLC_MEDIA(code));
				
		plp_snprintf(buf, sizeof(buf), "Load `%s' into the `%s' tray",
				media ? media : "(unknown media)",
				tray ? tray : "(unknown)");
		return buf;
	}
	return lookup(pjlmsg, code);
}

char *Check_code( char *code_str )
{
	static char msg[SMALLBUFFER];
	char *s;
	int code = atoi(code_str);

	/* see if it is a quiet code */
	DEBUG4( "check_code: '%s'", code_str );
	s = Find_str_value(&Pjl_error_codes, code_str, Value_sep);
	if( !s ) s = pjl_message( code_str );
	if( !s ){
		plp_snprintf( msg, sizeof(msg),
		"Unknown status code %s", code_str );
	}

	if (PJLC_GROUP(code) >= 40) {
		if (s != NULL) {
			send_zephyr(s);
		} else {
			char error_code_buf[SMALLBUFFER];

			plp_snprintf(error_code_buf, sizeof(error_code_buf),
				     "PJL error code %d", code);
			send_zephyr(error_code_buf);
		}
	}

	return(s);
}

static void send_zephyr(char *error)
{
	ZNotice_t notice;
	char msg[SMALLBUFFER];
	int plen;

	if (ZInitialize() != ZERR_NONE) 
		return;
	memset((char *)&notice, 0, sizeof(notice));

	notice.z_kind = UNACKED;
	notice.z_port = 0;
	notice.z_class = "MESSAGE";
	notice.z_class_inst = "PERSONAL";
	notice.z_opcode = "";
	notice.z_sender = "daemon";
	notice.z_recipient = Loweropts['n' - 'a'];
	notice.z_default_format = "";

	plp_snprintf(msg, sizeof(msg), "%s", Upperopts['P' - 'A']);
	plen = strlen(msg);
	plp_snprintf(msg + plen + 1, sizeof(msg) - plen - 1,
		     "Printer Error: %s may need attention:\n%s\n",
		     Upperopts['P' - 'A'], error);
	notice.z_message = msg;
	notice.z_message_len = plen + strlen(msg + plen + 1) + 2;

	ZSendNotice(&notice, ZNOAUTH);
}
