#!/bin/sh
#
# Copyright (C) 2000, 2001  Internet Software Consortium.
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
# DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
# INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
# FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# $Id: tests.sh,v 1.1.1.2 2002-02-03 04:31:08 ghudson Exp $

SYSTEMTESTTOP=..
. $SYSTEMTESTTOP/conf.sh

status=0

echo "I:trying an axfr that should be denied (NOTAUTH)"
$DIG +tcp data.child.example. @10.53.0.3 axfr -p 5300 > dig.out.ns3 || status=1
grep "; Transfer failed." dig.out.ns3 > /dev/null || status=1

echo "I:look for stub zone data without recursion (should not be found)"
$DIG +tcp +norec data.child.example. @10.53.0.3 txt -p 5300 > dig.out.ns3 \
	|| status=1
$PERL ../digcomp.pl knowngood.dig.out.norec dig.out.ns3 || status=1

echo "I:look for stub zone data with recursion (should be found)"
$DIG +tcp data.child.example. @10.53.0.3 txt -p 5300 > dig.out.ns3 || status=1
$PERL ../digcomp.pl knowngood.dig.out.rec dig.out.ns3 || status=1

echo "I:exit status: $status"
exit $status
