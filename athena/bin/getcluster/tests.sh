#!/bin/sh
# Test getcluster's ability to do proper version resolution.
# $Id: tests.sh,v 1.4 1999-01-22 23:10:27 ghudson Exp $

temp=/tmp/test.out.$$

export AUTOUPDATE NEW_TESTING_RELEASE NEW_PRODUCTION_RELEASE UPDATE_TIME
export VAR VAR1 VAR2 VAR3 VAR4 VAR5
UPDATE_TIME=0

# Test version resolution for AUTOUPDATE false.
AUTOUPDATE=false
VAR=
NEW_TESTING_RELEASE=
NEW_PRODUCTION_RELEASE=
./getcluster -b -d ignored 7.7P << EOM > $temp
var value3 7.8
var value1 7.6
var value2 7.7
var value4 8.0
EOM
. $temp
if [ "$VAR" != value2 -o "$NEW_PRODUCTION_RELEASE" != 8.0 ]; then
	echo "Test 1 failed."
	rm -f $temp
	exit 1
fi
echo "Test 1 passed."

# Test version resolution for AUTOUPDATE true, no testing versions
AUTOUPDATE=true
VAR=
NEW_TESTING_RELEASE=
NEW_PRODUCTION_RELEASE=
./getcluster -b -d ignored 7.7P << EOM > $temp
var value4 8.0
var value3 7.8
var value1 7.6
var value2 7.7
EOM
. $temp
if [ "$VAR" != value4 ]; then
	echo "Test 2 failed."
	rm -f $temp
	exit 1
fi
echo "Test 2 passed."

# Test version resolution for AUTOUPDATE true, testing versions
AUTOUPDATE=true
VAR=
NEW_TESTING_RELEASE=
NEW_PRODUCTION_RELEASE=
./getcluster -b -d ignored 7.7P << EOM > $temp
var value4 8.0 t
var value3 7.8
var value1 7.6
var value2 7.7
EOM
. $temp
if [ "$VAR" != value3 -o "$NEW_TESTING_RELEASE" != 8.0 ]; then
	echo "Test 3 failed."
	rm -f $temp
	exit 1
fi
echo "Test 3 passed."

# Test multiple variables and defaults.  Variable 1 should get no value,
# variable 2 should get the default value, variables 3 and 4 should get
# a value determined by the version, and variable 5 should get the default
# value.
AUTOUPDATE=true
VAR1=
VAR2=
VAR3=
VAR4=
VAR5=
NEW_TESTING_RELEASE=
NEW_PRODUCTION_RELEASE=
./getcluster -b -d ignored 7.7P << EOM > $temp
var3 var3value2 7.7 t
var1 var1value1 8.0 t
var3 var3value4 8.0 t
var2 var2value1
var3 var3value3 7.6
var4 var4value2 7.7
var2 var2value3 7.8 t
var5 var5value1
var1 var1value2 7.6 t
var3 var3value1
var4 var4value1 7.6
var4 var4value3 8.0
var2 var2value2 8.0 t
EOM
. $temp
if [ "$VAR2" != var2value1 -o "$VAR3" != var3value2 -o "$VAR4" != var4value3 \
     -o "$VAR5" != var5value1 -o "$NEW_TESTING_RELEASE" != 8.0 ]; then
	echo "Test 4 failed."
	rm -f $temp
	exit 1
fi
echo "Test 4 passed."

rm -f $temp
