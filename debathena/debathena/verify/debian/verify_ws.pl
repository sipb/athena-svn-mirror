#!/usr/bin/perl -w
#
# Workstation "verification" script
#

use strict;
use Getopt::Std;
use File::Basename;
use AptPkg::Config '$_config';
use AptPkg::Cache;

my $logfile = "/var/log/verify_ws.log";
my $verify_dir = "/afs/athena.mit.edu/system/athena10/verify";

($> == 0) || die "You must be root to run this.\n";

our ($opt_d,$opt_s) = (0,'');

getopts('ds:') || die "Usage: $0 [-d]\n";

if ($opt_d) {
    open(LOG, ">&STDOUT") || die "Can't dup stdout";
} else {
    open(LOG, ">>$logfile") || die "Cannot append to logfile: $!";
}

my %MIRRORS_OK = ();
my %COMPONENTS_OK = ();
my %SUITES_OK = ();
my %DA_MIRRORS_OK = ();
my %DA_COMPONENTS_OK = ();
my %DA_SUITES_OK = ();
my %APTKEYS_OK = ();
my %DEBSUMS_MISSING_PKG_OK = ();
my %DEBSUMS_MISSING_FILE_OK = ();
my %DEBSUMS_CHANGED_FILE_OK = ();

my $errors = 0;
my $warns = 0;
my %checks = ('sources' => 1,
	      'keys' => 1,
	      'debsums' => 1,
	      'policy', => 1 );


# Initialize the APT configuration
$_config->init;
my $cache = AptPkg::Cache->new;
my $policy = $cache->policy;

# Basic setup
my $codename = `/usr/bin/lsb_release -sc`;
die "Can't determine codename" unless ($? == 0);
chomp($codename);

if ($opt_s) {
    foreach my $skip (split(',', $opt_s)) {
	die "Can't skip unknown check '$skip'" unless exists($checks{$skip});
	warn("Skipping check '$skip'");
	$checks{$skip} = 0;
    }
}
	    
sub debug {
    $opt_d && print LOG "DEBUG: ", @_, "\n";
}

sub error {
    $errors = 1;
    print LOG "ERROR: ", @_, "\n";
}

sub wank {
    $warns = 1;
    print LOG "WARNING: ", @_, "\n";
}

sub loadConfigFile {
    my ($filename, $hashref) = @_;
    open(F, join('/', $verify_dir, $codename, $filename)) || 
	die "Can't open '$filename' file: $!";
    foreach my $line (<F>) {
	next if ($line =~ /^#/);
	next unless ($line =~ /\w/);
	chomp $line;
	$hashref->{$line} = 1;
    }
    close(F);
}
    
sub checkSourcesList {
    my ($filename, $mirrorsok, $suitesok, $componentsok) = @_;
    if (open(SLIST, $filename)) {
	while (<SLIST>) {
	    next if /^#/;
	    next unless /\w/;
	    my ($type, $mirror, $suite, @components) = split(' ', $_);
	    error($filename, ":", $., " Unknown first field ($type)") unless 
		($type =~ /^deb(-src){0,1}$/);
	    $mirror =~ s|/+$||g;
	    error($filename, ":", $., " Unknown mirror ($mirror)") unless
		exists($mirrorsok->{$mirror});
	    error($filename, ":", $., " Unknown suite ($suite)") unless
		exists($suitesok->{$suite});
	    foreach my $c (@components) {
		error($filename, ":", $., " Unknown component ($c)") unless
		    exists($componentsok->{$c});
	    }
	}
	close(SLIST);
    } else {
	error("Couldn't open file ($filename): $!");
    }
}	    

sub checkAptSources {
    my $sourceslist = join('', $_config->get('Dir'),
			   $_config->get('Dir::Etc'),
			   $_config->get('Dir::Etc::sourcelist'));
    
    wank("sources.list ($sourceslist) looks funny") unless 
	((-f $sourceslist) && ($sourceslist eq "/etc/apt/sources.list"));
    
    
    my $sourceslistd = join('', $_config->get('Dir'),
			    $_config->get('Dir::Etc'),
			    $_config->get('Dir::Etc::sourceparts'));
    
    wank("sources.list.d ($sourceslistd) looks funny") unless 
	((-d $sourceslistd ) && ($sourceslistd eq "/etc/apt/sources.list.d"));

    debug("Looking at sources.list ($sourceslist)");
    checkSourcesList($sourceslist, \%MIRRORS_OK, \%SUITES_OK, \%COMPONENTS_OK);
    
    foreach my $file (glob '/etc/apt/sources.list.d/*.list') {
	debug("Looking at $file");
	if (basename($file) eq "debathena.list") {
	    checkSourcesList($file, 
			     \%DA_MIRRORS_OK,
			     { $codename => 1 },
			     \%DA_COMPONENTS_OK);
	} elsif (basename($file) eq "debathena.clusterinfo.list") {
	    checkSourcesList($file,
			     \%DA_MIRRORS_OK,
			     \%DA_SUITES_OK,
			     \%DA_COMPONENTS_OK);
	} else {
	    error("Unknown additional sources.list file ($file)");
	}
    }
}

sub checkAptKeys {
    debug("Checking apt keys...");
    my $apt_keys = qx'/usr/bin/apt-key finger';
    die "Can't run apt-key" unless ($? == 0);
    $apt_keys =~ s/^.*?\n(?=pub)//s;
    foreach my $k (split(/\n\n/, $apt_keys)) {
	if ($k =~ /^\s+Key fingerprint = (.*)\nuid\s+(\S.*)$/m) {
	    error("Unknown fingerprint ($1) for key ($2)") unless exists($APTKEYS_OK{$1});
	}
    }
}
	
sub debsums {
    debug("Running debsums");
    # Bad-ideas: Since debsums is itself written in Perl...
    open(DEBSUMS, "/usr/bin/debsums -as 2>&1 |") || die "Can't run debsums";
    foreach my $sum (<DEBSUMS>) {
	chomp $sum;
	if ($sum =~ /^debsums: no md5sums for (\S+)/) {
	    error("$sum") unless exists($DEBSUMS_MISSING_PKG_OK{$1});
	} elsif ($sum =~ /^debsums: changed file (\S+)/) {
	    error("$sum") unless exists($DEBSUMS_CHANGED_FILE_OK{$1});
	} elsif ($sum =~ /^debsums: missing file (\S+)/) {
	    error("$sum") unless exists($DEBSUMS_MISSING_FILE_OK{$1});
	} else {
	    error("Unexpected debsums output: $sum");
	}
    }
    close(DEBSUMS);
}

sub checkPackage {
    my $pkgname = shift;
    debug("Checking package $pkgname");
    my $pkg = $cache->{$pkgname};
    unless ($pkg) {
	error("Can't find $pkgname in cache");
	return 0;
    }
#    use Data::Dumper;
#    $Data::Dumper::Maxdepth = 2;
#    print Dumper($pkg);
#    exit;
    if ($pkg->{CurrentState} ne 'Installed') {
	if ($pkg->{CurrentState} eq 'ConfigFiles') {
	    wank("Package $pkgname still has config files");
	} elsif ($pkg->{CurrentState} eq 'NotInstalled') {
	    wank("Package $pkgname should have been autoremoved.");
	} else {
	    error("Package $pkgname in weird state " . $pkg->{CurrentState});
	}
	return 0;
    }
    my $currver = '';
    if ($pkg->{CurrentVer}) {
	$currver = $pkg->{CurrentVer}{VerStr};
    }
    my $fromrepo = 0;
    foreach my $file (@{$pkg->{CurrentVer}{FileList}}) {
	next if ($file->{File}->{IndexType} ne 'Debian Package Index');
	if ($file->{File}->{Origin} =~ /^(Ubuntu|Debathena)$/) {
	    $fromrepo = 1;
	}
    }
    if ($pkgname =~ /^linux-(headers|image)-/) {
	wank("Old kernel package ($pkgname) needs cleanup!");
    } else {
	error("$pkgname ($currver) cannot be installed from a repository!") unless ($fromrepo);
    }
}
	
sub checkInstallability {
    debug("Checking installability of installed packages");
    # Todo: multiarch
    my %seen = ();
    # Seriously, why can't I do this natively?
    open(PKGLIST, '/usr/bin/dpkg-query -W -f \'${Package}\n\' |') || die "Can't run dpkg-query";
    while (<PKGLIST>) {
	chomp;
	next if exists($seen{$_});
	$seen{$_} = 1;
	checkPackage($_);
    }
    close(PKGLIST);
}

# __main__

defined($ENV{'APT_CONFIG'}) && wank("APT_CONFIG is defined and shouldn't be");

print LOG "Workstation verification beginning at ", 
    scalar(localtime()), "\n";

# Load configuration from AFS
foreach ('', '-updates', '-security') {
    $SUITES_OK{join('', $codename, $_)} = 1;
}
$DA_SUITES_OK{$codename} = 1;
if (-s "/var/run/athena-clusterinfo.sh") {
    my $apt_release = qx'. /var/run/athena-clusterinfo.sh && echo -n $APT_RELEASE';
    if ($apt_release !~ /^(production|proposed|development)$/) {
	error("Unknown APT_RELEASE value ($apt_release)");
    } elsif ($apt_release ne "production") {
	$DA_SUITES_OK{join('', $codename, '-', $apt_release)} = 1;
	if ($apt_release eq "development") {
	    $DA_SUITES_OK{join('', $codename, '-', 'proposed')} = 1;
	}
    }
} else {
    warn("No clusterinfo!");
}
loadConfigFile('mirrors', \%MIRRORS_OK);
loadConfigFile('components', \%COMPONENTS_OK);
loadConfigFile('debathena-mirrors', \%DA_MIRRORS_OK);
loadConfigFile('debathena-components', \%DA_COMPONENTS_OK);
loadConfigFile('aptkeys', \%APTKEYS_OK);
loadConfigFile('debsums-missing-packages', \%DEBSUMS_MISSING_PKG_OK);
loadConfigFile('debsums-missing-files', \%DEBSUMS_MISSING_FILE_OK);
loadConfigFile('debsums-changed-files', \%DEBSUMS_CHANGED_FILE_OK);

$checks{'keys'} && checkAptKeys();
$checks{'sources'} && checkAptSources();
$checks{'debums'} && debsums();
$checks{'policy'} && checkInstallability();
close(LOG);
if ($errors) {
    exit 1;
}
if ($warns) {
    exit 2;
}
exit 0;
