main() 
    {
#if (defined(AIX) && defined(_I386)) || defined(_IBMR2)
    printf("-lbsd");
#endif
	exit(0);
    }
