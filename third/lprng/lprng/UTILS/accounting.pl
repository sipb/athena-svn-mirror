#!/usr/local/bin/perl

# accounting.pl,v 1.5 1998/03/29 20:27:25 papowell Exp
# LPRng based accounting script.
# stdin = R/W Accounting File
# stdout = IO device
# stderr = log file
#  
#  command line format:
#   ac [start|stop|truncate] [-options] [accounting file]
#     -Tdebug will turn on debugging
#     start - at start of job; scan accounting file, fix up,
#         put in START entry
#     end  - at end of job; scan accounting file, fix up,
#         put in END entry
#     truncate - truncate the accounting file
#
# Accounting File has format:
# START A=id ....       - accounting script
# start -p=nnn          - filter
# ...
# end   -p=nnn+pagesused  - filter
# END  -p=pagesused     - accounting script
# 
# The accounting script expects to be invoked at the start of each job and
# will put a START line in.  However,  it will only get invoked at the
# end of correctly completed jobs.  This means that for correct accounting
# to be done,  then we need to make sure that jobs get pages assigned to
# them correctly.
#
# For brevity, we will show only the first word on each line in the
# following analysis.  We can make the following assumptions.  
# 
# At job start:
# .*END
#   Correctly updated job file,  the last job completed correctly
#   and the accounting was done completely.
# [.*END,]START,START*
#   None of these jobs was able to have the IF filter get the initial
#   page count.  They had submitted a job,  and used the facility,
#   but no paper/pages were used.
# [.*END],START,start,START*,START,start
#   The first job was able to get the IF filter started, but then was
#   unable to finish correctly.  The last job was able to establish
#   communications.  We can then calculate page counts for the last
#   job,  and update this to:
#    [END],START,start,END(bad),START*,start
#   This should be done before starting a job in order to correctly
#   count pages used prior to the current job.
# 
#   After checking,  a START line will be added to the file.
# 
# At job end:
# 
# [END],START,start,START*,START,start,end
#   We can now update this to:
#   [END],START,start,END(bad),START*,START,start,end,END
# 
#   The analysis for finding the END(bad) job is the same as for the
#   at job start.  The last END uses only the start and end information
#   for the last job entry.
# 
# Updating the accounting file.
#   The file needs to be scanned backwards for the last END entry.
#   This can be done by reading the file in in blocks, and scanning
#   for an END.  Once the END is located in the file,  then the
#   file from that point on only needs to be updated,  if at all.
# 
# we get the arguments and options.  By default, first option
# will be start or end
#

# use strict;

my($JFAIL,$JABORT,$JREMOVE,$JHOLD);
my($debug, $af_file, %opt,@af,$fix,$new);
my($loc,$start_count,$action,$action_count,$pages);
my($last_end,$time,$size,$bsize,$count,$buffer);
my($first_start,$next_start,$first_non_start,$last_non_start);

my($JFAIL) = 32;
my($JABORT) = 33;
my($JREMOVE) = 34;
my($JHOLD) = 37;

# print STDERR "XX:$0 ACCOUNTING " . join(" ",@ARGV) . "\n" if $debug;
$action = "";
if( @ARGV ){
	$action = shift @ARGV;
}
$action = uc($action);
if( $action ne "START" && $action ne "END" && $action ne "TRUNCATE" ){
	print STDERR "$0: first option must be 'START', 'END' or 'TRUNCATE'\n";
	exit $JABORT;
}

# pull out the options

getopts( 'A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:T:S:U:V:W:X:Y:Z:'
. 'a:b:cd:e:f:g:h:i:j:k:l:m:n:o:p:q:r:t:s:u:v:w:x:y:z:');

if( @ARGV ){
	$af_file = shift @ARGV;
}
$debug = 0;
if( exists( $opt{T} ) && $opt{T} =~ m/debug/ ){
	$debug = 1;
}

$time = time;
print STDERR "XX:$0 $action A='$opt{A}' P='$opt{P}' n='$opt{n}' H='$opt{H}' D='$time'\n" if $debug;

# open af_file for R/W
if( !$af_file ){
	die "$0: no accounting file\n";
}
open( AF,"+<$af_file" ) or die "$0: cannot open $af_file r/w - $!\n";

$size = -s AF;
print STDERR "XX:$0 AF size $size\n" if $debug;
$bsize = 0;
$last_end = -1;
$buffer = "";
do {
	# 1k increments
	$bsize = $bsize + 1024; 
	$bsize = $size if( $bsize > $size );
	print STDERR "XX:$0 bsize=$bsize\n" if $debug;
	if( $size > 0 ){
		seek AF, -$bsize, 2 or die "$0: seek of $bsize failed - $!\n";
		$count = read AF, $buffer, $bsize;
		if( !defined($count)){
			die "$0: read of $bsize failed - $!\n";
		} elsif( $count != $bsize ){
			die "$0: read returned $count instead of $bsize\n";
		}
		print STDERR "XX:$0 read \nXX " . join( "\nXX ", split("\n",$buffer))."\n" if $debug;
	}
	$loc = rindex( $buffer, "\nEND");
	print STDERR "XX:$0 loc=$loc\n" if $debug;
	if( $loc >= 0 ){
		$last_end = index( $buffer, "\n", $loc+1 );
		print STDERR "XX:$0 last_end=$last_end\n" if $debug;
		if( $last_end < 0 ){
			print STDERR "XX:$0 bad END entry in file\n" if $debug;
			seek AF, 0, 2 or die "$0: seek to EOF failed\n";
			print AF "\n";
		} else {
			++$last_end;
			$bsize = $bsize - $last_end;
			$buffer = substr( $buffer, $last_end );
		}
	}
} while ( $bsize < $size and $last_end < 0 );

print STDERR "XX:$0 final bsize=$bsize, XX ". join("\nXX ",split("\n",$buffer))."\n" if $debug;

# truncate and exit with 0
if( $action eq "TRUNCATE" ){
	truncate $af_file, 0 or die "$0: cannot truncate $af_file - $!\n";
	seek AF, 0, 0 or die "$0: cannot seek to start $af_file - $!\n";
	print AF $buffer or die "$0: cannot write to $af_file - $!\n";
	close AF or die "$0: cannot close $af_file - $!\n";
	exit 0;
}

@af = split( /\n/, $buffer );

print STDERR "XX:$0 split \nXX ".join("\nXX ",@af)."\n" if $debug;

# case 0: [null]      - empty file   - go on
# case 1: START+       - job aborted  - go on
# case 2: START+,n,n,n - some printed - go on
# case 4: START+,n,n,n,START+ - job aborted - go on
# case 4: START+,n,n,n,START+,n,n - some printed - fix

# END*,START,n,START*,n     - fix
#        ^   ^ ^      ^last non-start
#        ^   ^ ^ next START
#        ^   ^first non-start             ^
#        first start
#              
$fix = 0;
$first_start=0;
while( $first_start < @af and $af[$first_start] !~ /^START/ ){
	++$first_start;
}
$next_start=$first_start;
while( $first_start < @af ){
	while( $first_start+1 < @af and $af[$first_start+1] =~ /^START/ ){
		++$first_start;
	}
	$new = $af[$first_start];
	$new =~ s/^START//;
	print STDERR "XX:$0 found first_start $first_start: $af[$first_start]\n" if $debug;
	print STDERR "XX:$0 new '$new'\n" if $debug;
	$first_non_start = $first_start+1;
	$last_non_start = $first_non_start+0;
	last if( $first_non_start >= @af );
	print STDERR "XX:$0 LINE ". __LINE__.", first_non_start $first_non_start: $af[$first_non_start]\n" if $debug;
	$next_start=$first_start;
	while( ($next_start == $first_start) and ($last_non_start+1) < @af ){
		if( $af[$last_non_start+1] !~ /^START/ ){
			$last_non_start = $last_non_start+1;
		} else {
			$next_start = $last_non_start+1;
		}
	}
	print STDERR "XX:$0 last_non_start $last_non_start: $af[$last_non_start]\n" if $debug;
	print STDERR "XX:$0 LINE " . __LINE__ . " found next_start $next_start\n" if $debug;
	# now we have either n or or no more lines
	if( $next_start != $first_start ){
		while( ($next_start+1) < @af ){
			if( $af[ $next_start+1 ] =~ /^START/ ){
				$next_start = $next_start+1;
			} else {
				$last_non_start = $next_start+1;
				last;
			}
		}
	}
	# at this point, we have first_start-> START line to be billed
	#                        first_non_start-> starting count value
    #                        last_non_start->  starting count value
    #                         Bill = difference between two
	$fix = 1;
	$pages = 0;
	$start_count = Get_count($af[$first_non_start]);
	$action_count = Get_count($af[$last_non_start]);
	print STDERR "XX:$0 start_count=$start_count, end_count=$action_count\n" if $debug;
	$pages = $action_count - $start_count if $pages == 0;
	print STDERR "XX:$0 LINE " .__LINE__." first_start=$first_start, next_start=$next_start\n" if $debug;
	if( $first_start != $next_start ){
		splice(@af, $next_start, 0, "END p=$pages $new");
	} else {
		push(@af, "END p=$pages $new" );
	}
	print STDERR "XX:$0 new AF \nXX ".join("\nXX ",@af)."\n" if $debug;
	if( $first_start == $next_start ){
		last;
	}
	$first_start = $next_start;
}
if( $fix ){
	print STDERR "XX:$0 fixing at offset $bsize output to be \nXX ".join("\nXX ",@af)."\n" if $debug;
	seek AF, -$bsize, 2 or die "$0: seek to $bsize from EOF failed - $!\n";
	foreach (@af ){
		print AF "$_\n" or die "$0: write failed - $!\n";
	}
} else {
	print STDERR "XX:$0 not fixing output\n" if $debug;
}
if( $size > 0 ){
	seek AF, 0 , 2 or die "$0: seek to EOF failed - $!\n";
}

if( $action eq "START" ){
	# this is where you can put in a test to see that the user
	# has not exceeded his quota.  Return $JREMOVE if he has
	# put in a marker for this job.
	print STDERR "XX:$0 doing start\n" if $debug;
	print AF "START A='$opt{A}' P='$opt{P}' n='$opt{n}' H='$opt{H}' D='$time'\n"
		or die "$0: write failed - $!\n";
}

exit 0;
sub Get_count {
	my ($str) = @_;
	my (@vals, $pages);
	$pages =-1;
	@vals = split(" ", $str);
	print STDERR "XX:$0 vals:\nXX ".join("\nXX ",@vals)."\n" if $debug;
	foreach (@vals) {
		print STDERR "XX:$0 testing '$_'\n" if $debug;
		if( /^'*-p/ ){
			print STDERR "XX:$0 found '$_'\n" if $debug;
			($pages) = ($_ =~ m/(\d+)/);
			print STDERR "XX:$0 pages '$pages'\n" if $debug;
			return $pages;
		}
	}
}

#getopt - Process single-character switches with switch clustering
#getopts - Process single-character switches with switch clustering
#  use Getopt::Std;
#  getopt('oDI');    # -o, -D & -I take arg.  Sets opt_* as a side effect.
#  getopt('oDI', \%opts);    # -o, -D & -I take arg.  Values in %opts
#  getopts('oif:');  # -o & -i are boolean flags, -f takes an argument
#    # Sets opt_* as a side effect.
#  getopts('oif:', \%opts);  # options as above. Values in %opts
#
#The getopt() functions processes single-character switches with switch
#clustering.  Pass one argument which is a string containing all switches
#that take an argument.  For each switch found, sets $opt_x (where x is the
#switch name) to the value of the argument, or 1 if no argument.  Switches
#which take an argument don't care whether there is a space between the
#switch and the argument.
#
#For those of you who don't like additional variables being created, getopt()
#and getopts() will also accept a hash reference as an optional second argument. 
#Hash keys will be x (where x is the switch name) with key values the value of
#the argument or 1 if no argument is specified.
#
#

# Usage:
#   getopts('a:bc');	# -a takes arg. -b & -c not. Sets opt_* as a
#   getopts('a:bc',&%hash);	# -a takes arg. -b & -c not. %hash('char')
#                           #as a side effect.

sub getopts {
    my($argumentative, $hash) = @_;
    my(@args,$first,$rest,$pos);
    my($errs) = 0;

    @args = split( / */, $argumentative );
    while(@ARGV && ($_ = $ARGV[0]) =~ /^-(.)(.*)/) {
	($first,$rest) = ($1,$2);
	$pos = index($argumentative,$first);
	if($pos >= 0) {
	    if(defined($args[$pos+1]) and ($args[$pos+1] eq ':')) {
		shift(@ARGV);
		if($rest eq '') {
		    ++$errs unless @ARGV;
		    $rest = shift(@ARGV);
		}
                $opt{$first} = $rest;
	    }
	    else {
                  $opt{$first} = 1;
		if($rest eq '') {
		    shift(@ARGV);
		}
		else {
		    $ARGV[0] = "-$rest";
		}
	    }
	}
	else {
	    print STDERR "Unknown option: $first\n";
	    ++$errs;
	    if($rest ne '') {
		$ARGV[0] = "-$rest";
	    }
	    else {
		shift(@ARGV);
	    }
	}
    }
    $errs == 0;
}