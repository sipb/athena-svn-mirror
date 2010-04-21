SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min  hour      mday  mon  wday  user  command
0     2,4,6     *     *    *     root  desync -t /var/lib/desync-athena-update 7200 && athena-auto-update cron || exit 0
0     8,14,20   *     *    *     root  desync -t /var/lib/desync-athena-update 21600 && athena-auto-update cron || exit 0
