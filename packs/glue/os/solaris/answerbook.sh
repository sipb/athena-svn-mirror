#!/bin/sh
AB_CARDCATALOG=/opt/SUNWabe/ab_cardcatalog
export AB_CARDCATALOG
xhost `hostname`
/usr/openwin/bin/answerbook
