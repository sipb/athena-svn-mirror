/* copyright (C) 2001 Sun Microsystems */

/*    
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <locale.h>
#include <libintl.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlmemory.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <scrollkeeper.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

extern int xmlLoadExtDtdDefaultValue;

int apply_stylesheets (char *input_file, char *type, int stylesheet_num,
			char **stylesheets, char **outputs, char outputprefs)
{
 	xmlDocPtr res;
	docbDocPtr doc;
	xsltStylesheetPtr cur;
	int i;
	int returnval = 1;
	FILE *fid;
	struct stat buf;
#ifndef SOLARIS
	char line[1024], *start, *end;
	int num;
	FILE *res_fid;
	char *doctype;
	char command[1024];
	char temp1[PATHLEN], temp2[PATHLEN], errors[PATHLEN];
	int temp1_fd, temp2_fd, errors_fd;
#endif

	if (input_file == NULL ||
	    stylesheets == NULL ||
	    outputs == NULL) {
		return 0;
	}

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;
	xmlIndentTreeOutput = 1;

	if (strcmp(type, "sgml") == 0) {
		
#ifdef SOLARIS
		doc = docbParseFile(input_file, NULL);
#else
		snprintf(temp1, PATHLEN, SCROLLKEEPER_STATEDIR "/tmp/scrollkeeper-extract-1.xml.XXXXXX");
		snprintf(temp2, PATHLEN, SCROLLKEEPER_STATEDIR "/tmp/scrollkeeper-extract-2.xml.XXXXXX");
		snprintf(errors, PATHLEN, SCROLLKEEPER_STATEDIR "/tmp/scrollkeeper-extract-errors.XXXXXX");

		temp1_fd = mkstemp(temp1);
		printf ("%s\n", temp1);
		if (temp1_fd == -1) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot create temporary file: %s : %s\n"),temp1, strerror(errno));
			return 0;
		}
		  
		errors_fd = mkstemp(errors);
		if (errors_fd == -1) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot create temporary file: %s : %s\n"),errors, strerror(errno));
			return 0;
		}
		close(errors_fd);

		snprintf(command, 1024, "sgml2xml -xlower -f%s %s > %s", errors, input_file, temp1);
		system(command);
		
		unlink(errors);
		
		fid = fopen(input_file, "r");
		if (fid == NULL) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot read file: %s : %s\n"),input_file, strerror(errno));
		        close(temp1_fd);
			return 0;
		}

		doctype = NULL;
		while(fgets(line, 1024, fid) != NULL) {
			if ((start = strstr(line, "DOCTYPE")) != NULL) {
				start += 7;
				while (*start == ' ') {
					start++;
				}
				end = start;
				while (*end != ' ') {
					end++;
				}
				doctype = malloc(end-start+1);
				check_ptr(doctype, "");
				strncpy(doctype, start, end-start);
				doctype[end-start] = '\0';
				break;
			}
		}

		fclose (fid);

		if (doctype == NULL) {
		        close(temp1_fd);
			unlink(temp1);
			return 0;		
		}

		temp2_fd = mkstemp(temp2);
		if (temp2_fd == -1) {
		        close(temp1_fd);
			unlink(temp1);
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot create temporary file: %s : %s\n"),temp2, strerror(errno));
			return 0;
		}

		fid = fdopen(temp1_fd, "r");
		res_fid = fdopen(temp2_fd, "w");
		if (fid == NULL || res_fid == NULL) {
		        close(temp1_fd);
			unlink(temp1);
		        close(temp2_fd);
			unlink(temp2);
			if (fid)
				fclose (fid);
			if (res_fid)
				fclose (res_fid);
			return 0;
		}
		
		num = 0;
		while (fgets(line, 1024, fid) != NULL) {
			fputs(line, res_fid);
			if (num == 0) {
				num = 1;
				fprintf(res_fid, "<!DOCTYPE %s PUBLIC " DB_PUBLIC_ID DB_SYSTEM_ID ">\n", doctype);
			}
		}
		fclose(fid);
		fclose(res_fid);
		
		doc = xmlParseFile(temp2);
		unlink(temp1);
		unlink(temp2);
		if (doc == NULL) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Document is not well-formed XML: %s\n"), temp2);
			return 0;
		}
#endif /*SOLARIS */
		
	}
	else if (strcmp(type, "xml") == 0) {
		if (stat(input_file, &buf) == -1) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot stat file: %s : %s\n"), input_file, strerror(errno));
			return 0;
		}

		doc = xmlParseFile(input_file);
		xmlXIncludeProcess(doc);
		if (doc == NULL) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Document is not well-formed XML: %s\n"), input_file);
			return 0;
		}
	} else {
		sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot apply stylesheet to document of type: %s\n"), type);
		return 0;
	}

	for(i = 0; i < stylesheet_num; i++) {
		if (stylesheets[i] == NULL ||
		    outputs[i] == NULL) {
			continue;
		}

		fid = fopen(outputs[i], "w");
		if (fid == NULL) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot open output file: %s : %s \n"), outputs[i], strerror(errno));
			returnval = 0;
			continue;
		}

		if (stat(stylesheets[i], &buf) == -1) {
			sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(apply_stylesheets)", _("Cannot stat stylesheet file: %s : %s\n"), stylesheets[i], strerror(errno));
			returnval = 0;
			fclose(fid);
			continue;
		}

		cur = xsltParseStylesheetFile((const xmlChar *)(stylesheets[i]));
		res = xsltApplyStylesheet(cur, doc, NULL);
		xsltSaveResultToFile(fid, res, cur);
	        xmlFreeDoc(res);
        	xsltFreeStylesheet(cur);
		fclose(fid);
	}
	
	xmlFreeDoc(doc);
	
	xmlCleanupParser();
    	xmlMemoryDump();
	
	return returnval;
}
