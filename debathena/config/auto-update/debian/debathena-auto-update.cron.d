SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min  hour  mday  mon  wday  user  command
0,30  *     *     *    *     root  desync 1800; athena-auto-update cron
