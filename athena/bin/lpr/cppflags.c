main() 
{
#ifdef _AUX_SOURCE
    printf("-D_AUX_SOURCE");
#endif
#if defined(AIX) && defined(_I386)
    printf("-DAIX -D_I386 -Di386");
#endif
#ifdef ultrix
    printf("-Dultrix");
#endif
    exit(0);
}
