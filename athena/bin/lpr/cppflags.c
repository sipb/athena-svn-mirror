#ifdef ultrix
#include <dbm.h>
#endif

main() 
{
#ifdef _AUX_SOURCE
    printf("-D_AUX_SOURCE");
#endif
#if defined(AIX) && defined(_I386)
    printf("-DAIX -D_I386 -Di386");
#endif
#ifdef ultrix
#ifdef BYTESIZ
    printf("-Dultrix");
#else
    printf("-Dultrix -DULTRIX40");
#endif
#endif
#if defined(_IBMR2) && defined(_AIX)
    printf("-D_IBMR2 -D_AIX");
#endif
    exit(0);
}
