SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min  hour  mday  mon  wday  user  command
0     */2   *     *    *     root  desync 7200; athena-auto-update cron
