#include <stdio.h>

main()
{
    int i,j,n;
    char buf[1024];
    
    fputs("%!PS-Adobe-2.1\n", stdout);
    fputs("%%BeginFeature *Duplex True\n", stdout);
    fputs("<< /Duplex true >> setpagedevice\n", stdout);
    fputs("%%EndFeature\n", stdout);
    fflush(stdout);

    while(i = read(0, buf, 1024)) {
	j = 0;
	while (i-j > 0) {
	    if ((n=write(1, buf+j, i-j)) == 0) exit(1);
	    j += n;
	}
    }
    return 0;
}
