#!/bin/sh

# $Id: verify.sh,v 1.1 2004-04-27 22:34:05 rbasch Exp $

# This script performs a complete integrity verification of the system,
# checking both the operating system and Athena-supplied software.

# Check the operating system.
echo "Checking Solaris operating system software..."
/srvd/usr/athena/etc/oscheck

# Check Athena packages.
echo "Checking MIT-provided software..."
/srvd/usr/athena/etc/verify-pkgs
