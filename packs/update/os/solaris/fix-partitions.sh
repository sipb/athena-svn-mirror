#!/bin/sh
#
# $Id: fix-partitions.sh,v 1.1 2001-06-20 14:33:48 zacheiss Exp $
#
# Script to fix workstations that have overlapping swap and AFS
# cache partitions.
#
# We assume we're being run from setup-swap-boot and that swapping
# has already been disabled.

# What disk are we dealing with?
# We split off last element of the path because we'll need it as
# an argument to "format" later.
rootdisk=`mount | awk '$1 == "/" { split($3, s, "/"); print s[4]; }'`

# We run prtvtoc to determine the current partitioning.
# The output of prtvtoc has this format:
# 
# partition  tag  flags  first sector  sector count  last sector  mountpoint
#
# lines beginning with "*" are basically comments.
# 
# We want to know the first and last blocks of the swap partition, and the
# first block of the cache partition.
eval `/usr/sbin/prtvtoc /dev/dsk/$rootdisk | awk '{
    if ($1 != "*")
      i++;
    if ($1 == "1" && $2 == "3")
      split($0, s1);
    if ($1 == "3" && $2 == "0")
      split($0, s2);
    } END {
    if (i != 4)
      exit;
    print "firstswap=" s1[4], "lastswap=" s1[6], "firstcache=" s2[4];
    }'`

# Punt if the awk above failed to fill in any of these variables.
if [ -z "$firstswap" -o -z "$lastswap" -o -z "$firstcache" ]; then
    exit 0
fi

# If the swap partition straddles the beginning of the
# cache partition, we suck.
if [ "$firstswap" -lt "$firstcache" -a "$lastswap" -ge "$firstcache" ]; then
    newsize=`expr $firstcache - $firstswap`
else
    exit 0
fi

# Pipe things to format.   Wish for a better tool.
echo "partition\n1\n\n\n\n${newsize}b\nlabel\ny\nquit\nquit\n" | \
    format "$rootdisk" > /dev/null 2>&1

exit 0

