#include <sys/file.h>
#include <sgtty.h>

char *ris = "\033c";		/* Reset printer. */
char *lnm = "\033[20h";		/* Line feed new line mode on. */

main()
{
    register int count;
    char buf[1024];
    
    write(1, ris, 2);
    write(1, lnm, 5);
    while ((count = read(0, buf, 1024)) > 0) write(1, buf, count);
}
