#!/usr/bin/python
#
# A rewrite of getcluster(1) in Python
#

FALLBACKFILE = "/etc/cluster.fallback"
LOCALFILE = "/etc/cluster.local"
CLUSTERFILE = "/etc/cluster"
HESTYPE = "cluster"
GENERIC = "public-linux"
UPDATE_INTERVAL = (3600 * 4)

import hesiod
import re
import sys
import errno
import socket
import os
import time
import random
import struct
from optparse import OptionParser

cluster_re = re.compile('^\S+ \S+( \S+){0,2}$')
debugMode = os.getenv('DEBUG_GETCLUSTER', 'no') == 'yes'

def debug(msg, *args):
    if debugMode:
        print >>sys.stderr, "D:", msg % (args)

def perror(msg, *args):
    print >>sys.stderr, ("%s: " + msg) % ((sys.argv[0],) + args)

def merge(list1, list2):
    rv = [] + list1
    for i in list2:
        if i.split(' ', 1)[0] not in [j.split(' ', 1)[0] for j in list1]:
            rv.append(i)
    return rv

def cleanup(l):
    return filter(lambda x: cluster_re.match(x), map(lambda x: x.rstrip(), l))

def vercmp(v1, v2):
    v1l = map(int, map(lambda x: 0 if x is None else x, re.search('^(\d+)(?:\.(\d+)){0,1}',v1).groups()))
    v2l = map(int, map(lambda x: 0 if x is None else x, re.search('^(\d+)(?:\.(\d+)){0,1}',v2).groups()))
    return cmp(v1l, v2l)


def output_var(var, val, bourne=False, plaintext=False):
    val = val.replace("'", "'\\''")
    var = var.upper()
    if bourne:
        print "%s='%s' ; export %s" % (var, val, var)
    elif plaintext:
        print "%s %s" % (var, val)
    else:
        print "setenv %s '%s' ;" % (var, val)

def main():
    parser = OptionParser(usage='%prog [options] version', add_help_option=False)
    parser.add_option("-b", action="store_true", dest="bourne", default=False)
    parser.add_option("-p", action="store_true", dest="plain", default=False)
    parser.add_option("-d", action="store_true", dest="debug", default=False)
    parser.add_option("-f", action="store_true", dest="deprecated", default=False)
    parser.add_option("-l", action="store_true", dest="deprecated", default=False)
    parser.add_option("-h", action="store", type="string", dest="hostname")

    (options, args) = parser.parse_args()
    if len(args) == 2:
        perror("Ignoring deprecated hostname syntax.")
        args.pop(0)
    if len(args) != 1:
        parser.print_usage()
        return 1
    
    if options.deprecated:
        perror("-f and -l are deprecated and will be ignored.")
    if options.bourne and options.plain:
        parser.error("-p and -b are mutually exclusive.")

    ws_version=args[0]
    fallback=[]
    local=[]
    data=[]
    if (options.hostname == None):
        # Get the hostname/cluster name from /etc/cluster if present
        try:
            with open(CLUSTERFILE, 'r') as f:
                options.hostname = f.readline().strip()
        except IOError as e:
            # Fallback to nodename
            options.hostname = os.uname()[1]

        if not options.hostname:
            perror("Cannot determine hostname or %s was empty", CLUSTERFILE)
            return 1

        try:
            with open(FALLBACKFILE, 'r') as f:
                fallback = cleanup(f.readlines())
        except IOError as e:
            pass

        try:
            with open(LOCALFILE, 'r') as f:
                local = cleanup(f.readlines())
        except IOError as e:
            pass

    if options.debug:
        data = cleanup(sys.stdin.readlines())
    else:
        hes = None
        try:
            hes = hesiod.Lookup(options.hostname, HESTYPE)
        except IOError as e:
            if e.errno != errno.ENOENT:
                perror("hesiod_resolve: %s", str(e))
                return 1
        if hes == None:
            try:
                hes = hesiod.Lookup(GENERIC, HESTYPE)
            except IOError as e:
                if e.errno != errno.ENOENT:
                    perror("hesiod_resolve: %s", str(e))
                    return 1
        if hes == None:
            perror("Could not find any Hesiod cluster data.")
            return 1
        data = cleanup(hes.results)

    if len(data) == 0 and len(fallback) == 0 and len(local) == 0:
        if (options.debug):
            perror("No valid cluster data specified on stdin.")
        else:
            perror("No cluster information available for %s", options.hostname)
    # Sort everything into a hash, with varnames as keys and lists of lists as values
    cdata = {}
    for i in map(lambda x: x.split(' '), merge(merge(local, data), fallback)):
        var = i.pop(0)
        if var not in cdata:
            cdata[var] = []
        cdata[var].append(i)

    now = int(time.time())
    update_time = -1
    try:
        update_time = int(os.getenv('UPDATE_TIME', ''))
    except ValueError:
        pass
    autoupdate = os.getenv('AUTOUPDATE', 'false') == 'true'
    # This is secretly a 32-bit integer regardless of platform
    ip = -1 & 0xffffffff
    try:
        ip = socket.inet_aton(os.getenv('ADDR', ''))
    except socket.error:
        pass

    new_prod = '0.0'
    new_testing = '0.0'
    output_time = False
    vars = {}
    for var in cdata:
        maxverforval = '0.0'
        for opt in cdata[var]:
            val = opt.pop(0)
            vers = opt.pop(0) if len(opt) else None
            flags = opt.pop(0) if len(opt) else ''
            debug("Examining: %s %s %s %s", var, val, vers, flags)
            if vers == None:
                # No version data.  Save it as the default
                # if we don't already have one
                if var not in vars:
                    debug("-> Saving %s as default", val)
                    vars[var] = val
            elif vercmp(vers, ws_version) < 0:
                # Too old.  Discard
                debug("-> Discarding %s (%s < %s)", val, vers, ws_version)
            elif ((autoupdate and 't' not in flags) or (vercmp(vers, ws_version) == 0)):
                if vercmp(vers, ws_version) > 0:
                    debug("-> Found newer value (%s > %s)", vers, ws_version)
                    output_time = True
                    if (update_time == -1) or (now < update_time):
                        debug("-> Defering new value.")
                        continue
                if vercmp(vers, maxverforval) >= 0:
                    # We found a better option
                    debug("-> Storing %s (%s > previous max %s)", val, vers, maxverforval)
                    maxverforval = vers
                    vars[var] = val
            else:
                debug("-> Discarding, but checking new versions")
                if 't' in flags and vercmp(vers, new_testing) > 0:
                    debug("-> Storing new testing version %s", vers)
                    new_testing = vers
                if 't' not in flags and vercmp(vers, new_prod) > 0:
                    debug("-> Storing new production version %s", vers)
                    new_prod = vers

    for v in vars:
        output_var(v, vars[v], options.bourne, options.plain)

    if vercmp(new_testing, ws_version) > 0:
        output_var('NEW_TESTING_RELEASE', new_testing, options.bourne, options.plain)

    if vercmp(new_prod, ws_version) > 0:
        output_var('NEW_PRODUCTION_RELEASE', new_prod, options.bourne, options.plain)

    if output_time:
        if update_time == -1:
            random.seed(socket.ntohl(ip))
            # Python doesn't have RAND_MAX?
            update_time = now + random.randint(0,65535) % UPDATE_INTERVAL
        output_var('UPDATE_TIME', "%lu" % (update_time), options.bourne, options.plain)
    
if __name__ == '__main__':
    sys.exit(main())
