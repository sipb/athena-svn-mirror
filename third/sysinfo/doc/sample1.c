#include "mcsysinfo.h"

char *ProgramName = "test1";
int MsgClassFlags = 0;

main()
{
    int s;
    char **pp = NULL;
    static MCSIquery_t Query;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Cmd = MCSI_GETHOSTNAME;
    s = mcSysInfo(&Query);
    if (s == 0)
	printf ("Hostname is <%s>\n", Query.Buf);
    else
	printf ("gethostname failed\n");

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Cmd = MCSI_GETHOSTALIASES;
    s = mcSysInfo(&Query);
    if (s == 0) {
	printf ("Host Aliases =");
	for (pp = (char **) Query.Buf; pp && *pp && **pp; ++pp)
	    printf ("%s ", *pp);
	printf ("\n");
    }

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Cmd = MCSI_GETHOSTADDRS;
    s = mcSysInfo(&Query);
    if (s == 0) {
	printf ("Host Addresses =");
	for (pp = (char **) Query.Buf; pp && *pp && **pp; ++pp)
	    printf ("%s ", *pp);
	printf ("\n");
    }
}
