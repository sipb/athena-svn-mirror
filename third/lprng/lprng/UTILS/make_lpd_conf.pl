#!/usr/local/bin/perl
$date = `date`;
print "# lpd.conf generated from $ARGV[0] on $date\n";
print "#   The values in this file are the default values.\n";
print "#   If you modify the file,  set the value to something other than the default\n";
print "#   For example, '# default force_localhost' means the 'force_localhost' option is on.\n";
print "#   Uncomment this and change it to read 'force_localhost\@'\n\n";

while ( defined($m = <>) &&  !( $m =~ m/XXSTARTXX/) ){
	;
}
while ( defined($m = <>) &&  !( $m =~ m/END/) ){
	chomp($m);
	if ( $m =~ /{/ ){
		$m =~ s/[{}]//g;
		$m =~ s/,$//;
		@m = split( ',',$m, 6 );
		#print "split '" . join("','",@m) . "'\n";
		$name = $m[0]; $type = $m[1];
		$value = "" if not defined($value = $m[5]);
		$name =~ s/[\s\"]//g;
		$value =~ s/[\"]//g;
		$value =~ s/^\s+//;
		$value =~ s/\s+$//;
		$value =~ s/^[=#]//;
		#print "name '$name', type '$type', value '$value'\n";
		if( $type == 0 ){
			if( $value ){
				$m = "$name";
			} else {
				$m = "$name" . "@";
			}
		} else {
				$m = "$name=$value";
		}
		print "#   default $m" . "\n";
	} else {
		$m =~ s:^\s*/\*\s*::;
		$m =~ s:\*/::;
		print "# Purpose: $m" . "\n";
	}
}

while ( defined($m = <>) ) {
	;
}
