#!/usr/bin/perl -w

# $Id: pkg-order.pl,v 1.1 2004-03-31 00:11:02 rbasch Exp $

# This script generates the install order for the packages present
# in a package directory, and writes it to standard output.  When
# no package arguments are given, it will print the install order
# for all packages in the directory; typically this will be saved
# as the .order file in the directory.  When one or more package
# names are given, the install order for just those packages is
# printed; if the -R option is given, the order for those packages
# and all of their prerequisites is printed.
#
# If the -v option is given, the package's version number will also
# be output, as the second field on the output line.  If -r is given,
# the package order is reversed; this is useful for determining the
# order for removing packages.

use strict;
use warnings FATAL => 'all';
use Getopt::Std;

sub get_package($);
sub order($$);
sub usage(;$);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: $prog [<options>] [<package> ...]
  Options:
    -d <directory>   specify package directory, default is current directory
    -R               recurse; print order of packages and all prerequisites
    -r               print in reverse order
    -v               print the package version
EOF
    exit 1;
}

# Parse the command line arguments.
my %opts;
getopts('d:Rrv', \%opts) || usage;

my $pkgdir = $opts{'d'} || '.';
my $recursive = $opts{'R'};
my $reverse_order = $opts{'r'};
my $print_version = $opts{'v'};

# Hash of all packages of interest, keyed by package name.  The value
# is itself a hash, with keys for relevant package data.
my %packages = ();

# If package name arguments are given, we will process only those.
# Otherwise, we will process all packages found in the directory.
my @pkgs;
if (@ARGV) {
    @pkgs = @ARGV;
} else {
    # Find all the packages in the directory, by looking for subdirectories
    # containing a pkginfo file.
    opendir PKG, $pkgdir or die "Cannot open $pkgdir";
    @pkgs = grep((!/^\./ && -f ($pkgdir . '/' . $_ . '/pkginfo')),
		 readdir PKG);
    closedir PKG;
}

# Initialize each package of interest.
my $pkg;
foreach $pkg (@pkgs) {
    get_package($pkg);
}

# We have read the information for each package.  Determine the
# dependency order.  We sort the package names so that the final
# order tends to be alphabetical.
my @ordered_pkgs = ();
foreach $pkg (sort keys %packages) {
    order($pkg, "<root>");
}

# Print the packages in the desired order.
foreach $pkg ($reverse_order ? reverse @ordered_pkgs : @ordered_pkgs) {
    print "$pkg";
    print " $packages{$pkg}{version}" if $print_version;
    print "\n";
}

# Read the pkginfo file for each package, and get the package version
# string from it.  Then read the depend file for the package, and save
# the dependencies as an array value for the depend key for the
# package.  For now, we only handle "P" (prerequisite) entries.
# If the "-R" (recurse) option was given, this subroutine calls
# itself recursively for each of the prerequisites.
sub get_package($) {
    my $pkg = $_[0];

    # Return now if we have already initialized this package.
    return if $packages{$pkg};

    # Get the package version from the pkginfo file.
    my $infofile = $pkgdir . '/' . $pkg . '/pkginfo';
    open INFO, "<$infofile" or die "Cannot open $infofile";
    while (<INFO>) {
	chomp;
	if (m|^VERSION=(.+)|) {
	    $packages{$pkg}{version} = $1;
	    last;
	}
    }
    close INFO;
    unless ($packages{$pkg}{version}) {
	die "Could not read VERSION parameter setting from $infofile";
    }

    # Get the package dependencies.
    my $depfile = $pkgdir . '/' . $pkg . '/install/depend';
    open DEPEND, "<$depfile" or return;
    while (<DEPEND>) {
	chomp;
	next if (m/^(\s*$|#)/);
	my ($flag, $prereq) = split;
	# Only handle prerequisite entries for now.
	next unless $flag eq "P";
	if ($prereq) {
	    push @{$packages{$pkg}{depend}}, $prereq;
	} else {
	    print STDERR "Warning: invalid syntax in $depfile\n";
	}
    }
    close DEPEND;

    # If recursing, initialize prerequisite packages.
    if ($recursive && exists $packages{$pkg}{depend}) {
	foreach my $prereq (@{$packages{$pkg}{depend}}) {
	    get_package($prereq);
	}
    }
}    

# This is a recursive subroutine which calculates the required install
# order for a package, skipping any package which has already been
# processed.  The package name is appended to the ordered_pkgs array,
# after recursing over each of its prerequisites.  The first argument
# is the package name, and the second its dependent; the latter is
# used to trace a dependency loop.
sub order($$) {
    my ($pkg, $dependent) = @_;

    # Detect a loop.
    if ($packages{$pkg}{locked}) {
	print STDERR "Warning: Dependency loop detected for $pkg\n";
	print STDERR "called from: $packages{$pkg}{locked} ... $dependent\n";
	return;
    }

    # Return now if we have already handled this package.
    return if $packages{$pkg}{seen};

    # Use the locked key to detect a loop.
    $packages{$pkg}{locked} = $dependent;

    # Recurse over any dependencies for this package (except for packages
    # which are not of interest).
    if (exists $packages{$pkg}{depend}) {
	foreach my $prereq (@{$packages{$pkg}{depend}}) {
	    order($prereq, $pkg) if $packages{$prereq};
	}
    }

    # Append this package to our ordered list.
    push @ordered_pkgs, $pkg;

    # We are done with this package.
    $packages{$pkg}{seen} = 1;

    # Clear the loop-detector lock.
    delete $packages{$pkg}{locked};
}
