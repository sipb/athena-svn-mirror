/* rpmsignature: spit out the signature portion of a package */

#include "system.h"

#include <rpmlib.h>
#include "rpmlead.h"
#include "signature.h"
#include "debug.h"

int main(int argc, char **argv)
{
    FD_t fdi, fdo;
    struct rpmlead lead;
    Header sig;
    
    setprogname(argv[0]);	/* Retrofit glibc __progname */
    if (argc == 1) {
	fdi = Fopen("-", "r.ufdio");
    } else {
	fdi = Fopen(argv[1], "r.ufdio");
    }
    if (Ferror(fdi)) {
	fprintf(stderr, "%s: %s: %s\n", argv[0],
		(argc == 1 ? "<stdin>" : argv[1]), Fstrerror(fdi));
	exit(1);
    }

    if (readLead(fdi, &lead) != RPMRC_OK)
	exit(1);
    if (rpmReadSignature(fdi, &sig, lead.signature_type, NULL) !=  RPMRC_OK) {
	fdo = Fopen("-", "w.ufdio");
	rpmWriteSignature(fdo, sig);
    }
    
    return 0;
}