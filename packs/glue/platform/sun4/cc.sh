#! /bin/sh
if [ ! -x /mit/sunsoft/sun4bin/cc ]; then
	attach -h -q sunsoft
fi
/mit/sunsoft/sun4bin/cc "$@"
