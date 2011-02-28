#!/usr/bin/python
#
# HID interface class = 3
# Interface Protocols:
# 0 - None
# 1 - KB
# 2 - Mouse
# 3-255 Reserved

import usb

protocols = { 1: 'keyboard',
	      2: 'mouse'}

found = { 'keyboard' : 'no',
	  'mouse' : 'no' }

for b in usb.busses():
    if 'no' not in found.values():
        break
    for d in b.devices:
        if 'no' not in found.values():
            break
	i = d.configurations[0].interfaces[0][0]
	if i.interfaceClass == 3 and \
	    i.interfaceProtocol in protocols.keys():
		found[protocols[i.interfaceProtocol]] = 'yes'

if 'no' not in found.values():
    print "OK - keyboard and mouse present"
else:
    errs = []
    for k,v in found.items():
        if v == 'no':
            errs.append(k + " missing")
    print "ERROR - " + ', '.join(errs)
    
