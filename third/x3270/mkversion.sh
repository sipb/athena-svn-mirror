#! /bin/sh
# Create version.o from version.txt
#set -x

set -e

. ./version.txt
builddate=`date`
sccsdate=`date +%y/%m/%d`
user=${LOGNAME-$USER}

trap 'rm -f version.c' 0 1 2 15

cat <<EOF >version.c
char *build = "x3270 v$version $builddate $user";
char *app_defaults_version = "$adversion";
static char sccsid[] = "@(#)x3270 v$version $sccsdate $user";
EOF

${1-cc} -c version.c
