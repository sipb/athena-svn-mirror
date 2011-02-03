SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min           hour  mday  mon  wday  user  command
0,15,30,45     *     *     *    *     root  /usr/sbin/athena-auto-update cron
0     	       2     *     *    *     root  echo "/usr/sbin/athena-auto-upgrade" | at "now + $(desync -n 360) minutes" 2>&1 | egrep -v "(job [0-9]+ at)|(^warning: commands will be executed using /bin/sh)"


