main() 
    {
#if defined(AIX) && defined(_I386)
    printf("-lbsd");
#endif
	exit(0);
    }
