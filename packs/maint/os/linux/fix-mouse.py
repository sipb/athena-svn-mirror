#!/usr/bin/python

# The introduction of udev in RHEL 4 causes the /dev directory to be
# wiped, which can cause the configured X pointer device to become
# invalid.  Red Hat provides /usr/sbin/fix-mouse-psaux, which is
# intended to address the specific problem of /dev/psaux no longer
# existing, and we run that at update time.  But that script is
# incomplete; if the configuration uses /dev/mouse and it does not
# point at /dev/psaux, the pointer will still be misconfigured because
# /dev/mouse no longer exists in RHEL 4.  This script is run at boot
# time from athena-ws, and replaces any missing pointer device with
# /dev/input/mice.

import xf86config
import os
import sys

needsFixed = 0

(xconfig, xconfigpath) = xf86config.readConfigFile()
if not xconfig:
    sys.exit(0)
Xmouse = xf86config.getCorePointer(xconfig)
if not Xmouse:
    sys.exit(0)

for o in Xmouse.options:
    if o.name == "Device" and not os.access(o.val, os.O_RDONLY):
        o.val = "/dev/input/mice"
        needsFixed = 1

if needsFixed:
    xconfig.write(xconfigpath)
sys.exit(0)
