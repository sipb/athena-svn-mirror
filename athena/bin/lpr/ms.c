/* 
 * Implement ms capability.  Uses POSIX termios interface.
 * James Clark (jjc@jclark.com) as part of lprps package.
 */

/* 
 * We don't want to include lp.h because of conflicts between 
 * <termios.h> and <sgtty> in SunOS.
 */

#if defined(i386)
#undef _POSIX_SOURCE            /* 386BSD 0.1 no real POSIX ... */
#endif

#include <termios.h>
#include <syslog.h>

extern char 	*printer;
extern int	pfd;
extern char	*MS;

setms()
{
	struct termios t;
	char *s;


	syslog(LOG_ERR, "%s: %s" ,"MS", MS);
	if (!MS) return;
	if (tcgetattr(pfd, &t) < 0) {
		syslog(LOG_ERR, "%s: tcgetattr: %m", printer);
		exit(1);
	}
	
	s = MS;
	for (;;) {
		char *p;
		char saved;

		for (p = s; *p != '\0' && *p != ','; p++)
			;
		saved = *p;
		*p = '\0';
		if (*s && setmode(&t, s) < 0)
			syslog(LOG_ERR, "%s: unknown mode: %s", printer, s);
		if ((*p = saved) == '\0')
			break;
		s = ++p;
	}
	if (tcsetattr(pfd, TCSADRAIN, &t) < 0) {
		syslog(LOG_ERR, "%s: tcsetattr: %m", printer);
		exit(1);
	}
}
