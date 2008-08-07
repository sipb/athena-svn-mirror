#!/bin/sh

# busytest.sh: Simple script to query busy information from an Athena
# machine using netcat.  Prints a hex dump of the output; for prettier
# output you can try compiling and running busytest.c.

port=49154
host=$1

printf "\0" | nc -q2 -u $host $port | od -c

