SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min  hour      mday  mon  wday  user  command
0     2,4,6     *     *    *     root  desync 7200;  athena-auto-update cron
0     8,14,20   *     *    *     root  desync 21600; athena-auto-update cron
