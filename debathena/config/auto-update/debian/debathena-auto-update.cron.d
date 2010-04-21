SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#min  hour      mday  mon  wday  user  command
0     2,4,6     *     *    *     root  echo "/usr/sbin/athena-auto-update cron" | at "now + $(desync -n 120) minutes"
0     8,14,20   *     *    *     root  echo "/usr/sbin/athena-auto-update cron" | at "now + $(desync -n 360) minutes"
