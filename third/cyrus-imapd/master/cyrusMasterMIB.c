#include <config.h>

#ifdef HAVE_UCDSNMP

/* This file was generated by mib2c and is intended for use as a mib module
   for the ucd-snmp snmpd agent. */


#ifdef IN_UCD_SNMP_SOURCE
/* If we're compiling this file inside the ucd-snmp source tree */


/* This should always be included first before anything else */
#include <config.h>


/* minimal include directives */
#include "mibincl.h"
#include "util_funcs.h"


#else /* !IN_UCD_SNMP_SOURCE */


#include <ucd-snmp/ucd-snmp-config.h>
#include <ucd-snmp/ucd-snmp-includes.h>
#include <ucd-snmp/ucd-snmp-agent-includes.h>
#include <ucd-snmp/util_funcs.h>


#endif /* !IN_UCD_SNMP_SOURCE */

#include <time.h>
#include <string.h>

#include "cyrusMasterMIB.h"

#include "master.h"
#include "../imap/version.h"

/* 
 * cyrusMasterMIB_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid cyrusMasterMIB_variables_oid[] = { 1,3,6,1,4,1,3,6,1 };


/* 
 * variable4 cyrusMasterMIB_variables:
 *   this variable defines function callbacks and type return information 
 *   for the cyrusMasterMIB mib section 
 */


struct variable4 cyrusMasterMIB_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#define   CYRUSMASTERINFODESCR  1
  { CYRUSMASTERINFODESCR, ASN_OCTET_STR , RONLY , var_cyrusMasterMIB, 2, { 1,1 } },
#define   CYRUSMASTERINFOVERS   2
  { CYRUSMASTERINFOVERS , ASN_OCTET_STR , RONLY , var_cyrusMasterMIB, 2, { 1,2 } },
#define   CYRUSMASTERINFOUPTIME 3
  { CYRUSMASTERINFOUPTIME , ASN_TIMETICKS , RONLY , var_cyrusMasterMIB, 2, { 1,3 } },
#define   SERVICEFORKS          5
  { SERVICEFORKS        , ASN_COUNTER   , RONLY , var_serviceTable, 3, { 2,1,1 } },
#define   SERVICEACTIVE         6
  { SERVICEACTIVE       , ASN_GAUGE     , RONLY , var_serviceTable, 3, { 2,1,2 } },
#define   SERVICENAME           7
  { SERVICENAME         , ASN_OCTET_STR , RONLY , var_serviceTable, 3, { 2,1,3 } },
#define   SERVICEID             8
  { SERVICEID           , ASN_INTEGER   , NOACCESS , var_serviceTable, 3, { 2,1,4 } },
#define   SERVICECONNS          9
  { SERVICECONNS        , ASN_COUNTER   , NOACCESS , var_serviceTable, 3, { 2,1,5 } },
};
/*    (L = length of the oidsuffix) */


static time_t startTime = 0;

/*
 * init_cyrusMasterMIB():
 *   Initialization routine.  This is called when the agent starts up.
 *   At a minimum, registration of your variables should take place here.
 */
void init_cyrusMasterMIB(void) 
{
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("cyrusMasterMIB", cyrusMasterMIB_variables, variable4,
		 cyrusMasterMIB_variables_oid);


    /* place any other initialization junk you need here */
    if (!startTime) {
	startTime = time(NULL);
    }
}


/*
 * var_cyrusMasterMIB():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
unsigned char *
var_cyrusMasterMIB(struct variable *vp, 
                oid     *name, 
                size_t  *length, 
                int     exact, 
                size_t  *var_len, 
                WriteMethod **write_method)
{
    /* variables we may use later */
    static long long_ret;
    static unsigned char string[SPRINT_MAX_LEN];
    /* static oid objid[MAX_OID_LEN]; */
    /* static struct counter64 c64; */

    if (header_generic(vp,name,length,exact,var_len,write_method)
	== MATCH_FAILED )
	return NULL;

    /* 
     * this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {
    case CYRUSMASTERINFODESCR:
	strcpy(string, "Cyrus IMAP server master process");
	*var_len = strlen(string);
	return (unsigned char *) string;
      
    case CYRUSMASTERINFOVERS:
	strcpy(string, CYRUS_VERSION);
	*var_len = strlen(string);
	return (unsigned char *) string;
      
    case CYRUSMASTERINFOUPTIME:
	long_ret = 100 * (time(NULL) - startTime);
	return (unsigned char *) &long_ret;
      
    default:
	ERROR_MSG("");
    }
    return NULL;
}


/*
 * var_serviceTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_cyrusMasterMIB above.
 */
unsigned char *
var_serviceTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    /* variables we may use later */
    static long long_ret;
    static unsigned char string[SPRINT_MAX_LEN];
    /* static oid objid[MAX_OID_LEN]; */
    /* static struct counter64 c64; */
    int index;

    /* 
     * This assumes that the table is a 'simple' table.
     *	See the implementation documentation for the meaning of this.
     *	You will need to provide the correct value for the TABLE_SIZE parameter
     *
     * If this table does not meet the requirements for a simple table,
     *	you will need to provide the replacement code yourself.
     *	Mib2c is not smart enough to write this for you.
     *    Again, see the implementation documentation for what is required.
     */
    if (header_simple_table(vp,name,length,exact,var_len,write_method, nservices)
	== MATCH_FAILED )
	return NULL;


    index = name[*length - 1];

    /* 
     * this is where we do the value assignments for the mib results.
     */
    switch(vp->magic) {
    case SERVICEFORKS:
	long_ret = Services[index - 1].nforks;
	return (unsigned char *) &long_ret;
      
    case SERVICEACTIVE:
	long_ret = Services[index - 1].nactive;
	return (unsigned char *) &long_ret;
      
    case SERVICENAME:
	strcpy(string, Services[index - 1].name);
	*var_len = strlen(string);
	return (unsigned char *) string;
      
    case SERVICEID:
	long_ret = index;
	return (unsigned char *) &long_ret;

    case SERVICECONNS:
	long_ret = Services[index - 1].nconnections;
	return (unsigned char *) &long_ret;

    default:
	ERROR_MSG("");
    }
    return NULL;
}





#endif /* HAVE_UCDSNMP */
