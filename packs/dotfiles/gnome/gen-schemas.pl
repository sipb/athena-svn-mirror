#!/usr/athena/bin/perl

# gen-schema: A tool for generating the per-panel config schema from a
# readable file format.  This isn't a very generic tool; it only handles
# strings (with no special characters in them), integers, booleans,
# and lists of strings.

print "<?xml version=\"1.0\"?>\n";
print "<gconfschemafile>\n";
print " <schemalist>\n";
print "\n";

$path[0] = "";
while (<>) {
    chop;
    /^( *)(.*)$/;
    $indent = length($1);
    $line = $2;
    next if ($line =~ /^#/ || $line eq "");
    if ($line =~ /^\[(.*)\]$/) {
	$path[$indent + 1] = $path[$indent] . "/" . $1;
    } elsif ($line =~ /^([\w-]+) *= *(.*)$/) {
	$key = $path[$indent] . "/" . $1;
	$valspec = $2;
	if ($valspec =~ /^"(.*)"$/) {
	    $type = "string";
	    $val = $1;
	} elsif ($valspec =~ /^-?\d+$/) {
	    $type = "int";
	    $val = $valspec;
	} elsif ($valspec =~ /^true|false$/) {
	    $type = "bool";
	    $val = $valspec;
	} elsif ($valspec =~ /^\[.*\]$/) {
	    $type = "list";
	    $val = $valspec;
	} elsif ($valspec =~ /^\[/) {
	    $type = "list";
	    $val = $valspec;
	    while (<>) {
		chop;
		s/^ *//;
		$val .= $_;
		last if /\]$/;
	    }
	} else {
	    die "Unrecognized valspec format '$valspec'.\n";
	}
	print "  <schema>\n";
	print "   <key>$key</key>\n";
	print "   <owner>gnome-panel</owner>\n";
	print "   <type>$type</type>\n";
	if ($type eq "list") {
	    print "   <list_type>string</list_type>\n";
	}
	print "   <default>$val</default>\n";
	print "   <locale name=\"C\" />\n";
	print "  </schema>\n";
	print "\n";
    } else {
	die "Unrecognized line format '$line'.\n";
    }
}

print " </schemalist>\n";
print "</gconfschemafile>\n";

