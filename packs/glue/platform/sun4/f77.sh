#!/bin/sh
if [ ! -f /mit/sunsoft/sun4bin/f77 ]; then
    echo "In order to use f77 on the Sun workstations, you will"
    echo "first need to add the sunsoft locker. To do this type:"
    echo " "
    echo "      add sunsoft"
    echo " "
    exit 1
fi
/mit/sunsoft/sun4bin/f77 $*

