SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min           hour  mday  mon  wday  user  command
0,15,30,45     *     *     *    *     root  /usr/sbin/athena-auto-update cron
0              2     *     *    *     root  /usr/sbin/athena-auto-upgrade

