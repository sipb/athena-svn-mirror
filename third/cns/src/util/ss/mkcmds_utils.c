/*
 * Copyright 1987 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */
#include "mit-sipb-copyright.h"
#include <stdio.h>
#include "ss.h"

extern FILE *output_file;

extern int exit();
extern char *malloc(), *gensym(), *str_concat3(), *quote(), *ds();

write_ct(hdr, rql)
	char *hdr, *rql;
{
	char *sym;
	sym = gensym("ssu");
	fputs("static ss_request_entry ", output_file);
	fputs(sym, output_file);
	fputs("[] = {\n", output_file);
	fputs(rql, output_file);
	fputs("\t{ (char **)0, (", output_file);
	fputs(FUNCTION_TYPE_NAME, output_file);
	fputs("(*)())0, (char *)0, 0 }\n};\n\nss_request_table ", output_file);
	fputs(hdr, output_file);
	fprintf(output_file, " = { %d, ", SS_RQT_TBL_V2);
	fputs(sym, output_file);
	fputs(" };\n", output_file);
}

char *
generate_cmds_string(cmds)
	char *cmds;
{
	char *var_name;

	var_name = gensym("ssu");
	fputs("static char *", output_file);
	fputs(var_name, output_file);
	fputs("[] = { ", output_file);
	fputs(cmds, output_file);
	fputs(", (char *)0 };\n", output_file);
	return(var_name);
}

generate_function_definition(func)
	char *func;
{
	fputs("extern ", output_file);
	fputs(FUNCTION_TYPE_NAME, output_file);
	fputs(" ", output_file);
	fputs(func, output_file);
	fputs("();\n", output_file);
}

static char *C_SP = ", ";	/* comma-space */

char *
generate_rqte(func_name, info_string, cmds, options)
	char *func_name;
	char *info_string;
	char *cmds;
	int  options;
{
	int size;
	char *string, *var_name, numbuf[16];
	var_name = generate_cmds_string(cmds);
	generate_function_definition(func_name);
	size = 3;		/* "\t{ " */
	size += strlen(var_name)+2; /* "quux, " */
	size += strlen(func_name)+2; /* "foo, " */
	size += strlen(info_string)+4; /* "\"Info!\", " */
	sprintf(numbuf, "%d", options);
	size += strlen(numbuf);
	size += 4;		/* " }," + NL */
	string = malloc(size * sizeof(char *));
	strcpy(string, "\t{ ");
	strcat(string, var_name);
	strcat(string, C_SP);
	strcat(string, func_name);
	strcat(string, C_SP);
	strcat(string, info_string);
	strcat(string, C_SP);
	strcat(string, numbuf);
	strcat(string, " },\n");
	return(string);
}
