#!/usr/bin/perl -w

# $Id: link-pkgs.pl,v 1.1 2004-03-31 15:32:44 rbasch Exp $

# This script initializes a Solaris package directory in preparation
# for a new patch release, by making symlinks to the packages in the
# current release.  For each package in the current (old) package
# directory, it creates a symlink to the actual package, by reading
# the link in the old directory, to avoid symlink chains.  If the old
# directory contains the actual package, it simply symlinks to that.

# The new and old version numbers may be specified as command line
# arguments; if not specified, the old version defaults to the
# version found in /srvd/.rvdinfo, and the new version to one
# greater than the old patch number.

# The default top-level package directory is /.srvd/pkg; it may be
# changed via the "-d <directory>" option.

use strict;
use warnings FATAL => 'all';
use Getopt::Std;

sub check_pkg($$$);
sub usage(;$);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: $prog [<options>] [<new_version> [<old_version>]]
  Options:
    -d <directory>   specify top of package directory hierarchy
    -v               verbose output
EOF
    exit 1;
}

# Parse the command line arguments.
my %opts;
getopts('d:v', \%opts) || usage;

my $pkgtopdir = $opts{'d'} || '/.srvd/pkg';
my $verbose = $opts{'v'};
my $rvdinfo = '/srvd/.rvdinfo';
my $newversion = shift @ARGV;
my $oldversion = shift @ARGV;
unless ($oldversion) {
    open RVDINFO, "<$rvdinfo" or die "Cannot open $rvdinfo";
    my ($f1, $f2, $f3, $f4, $srvd_version);
    while (<RVDINFO>) {
	($f1, $f2, $f3, $f4, $srvd_version) = split;
    }
    close RVDINFO;
    $oldversion = $srvd_version;
}
unless ($newversion) {
    my ($major, $minor, $patch) = split /\./, $oldversion;
    ++$patch;
    $newversion = "$major.$minor.$patch";
}
print "New version is $newversion\n" if $verbose;
print "Old version is $oldversion\n" if $verbose;

my $oldpkgdir = $pkgtopdir . '/' . $oldversion;
print "The old package directory is $oldpkgdir\n" if $verbose;

# Read the old packages directory, ignoring entries beginning with '.'.
opendir PKG, $oldpkgdir or die "Cannot open $oldpkgdir";
my @pkgs = grep !/^\./, readdir PKG;
closedir PKG;
die "$oldpkgdir is empty" unless @pkgs;

# Create the new directory, if necessary.
my $newpkgdir = $pkgtopdir . '/' . $newversion;
print "The new package directory is $newpkgdir\n" if $verbose;
unless (-d $newpkgdir) {
    print "Creating $newpkgdir ...\n";
    mkdir $newpkgdir or die "Cannot create directory $newpkgdir";
}

# Process each package.  Determine the target for the symlink we will
# create in the new package directory:  If the old package is itself
# a symlink, we copy the link target; if it is a directory, we simply
# link to it.  We also get the package name and version values from
# the pkginfo file, and check that they match the corresponding path
# components in the link target.
foreach my $pkg (@pkgs) {
    my $oldpkgpath = $oldpkgdir . '/' . $pkg;
    stat $oldpkgpath || die "Cannot stat $oldpkgpath";
    unless (-d _) {
	print STDERR "Warning: $oldpkgpath is not a directory\n";
	next;
    }
    my $linktarget = readlink $oldpkgpath || '../' . $oldversion . '/' . $pkg;
    my ($pkg_comp, $ver_comp) = reverse(split(/\//, $linktarget));

    # Check that the package name component in the link target path is
    # what we expect.
    print STDERR "Warning: $oldpkgpath: Unexpected package name component" .
	" $pkg_comp for $pkg\n" if ($pkg_comp ne $pkg);

    # Warn if the name or version number from the pkginfo file do not
    # match the path components in the link target.
    check_pkg($oldpkgpath, $pkg_comp, $ver_comp);

    # Create the new symlink.  Failure is OK if the new path already
    # exists as a package, i.e. as a directory with a pkginfo file.
    my $newpkgpath = $newpkgdir . '/' . $pkg;
    if (symlink($linktarget, $newpkgpath)) {
	print "$newpkgpath -> $linktarget\n" if $verbose;
    } elsif (-d $newpkgpath && -f ($newpkgpath . '/pkginfo')) {
	# The package already exists in the new directory.  Check that
	# it is what we expect.
	print "$newpkgpath already exists\n";
	check_pkg($newpkgpath, $pkg, $newversion);
    } else {
	die "Cannot create symlink $newpkgpath";
    }
}

# Read the package name and version number from a package's pkginfo
# file to warn if they do not match the values expected from its
# path components.
sub check_pkg($$$) {
    my ($pkgpath, $expect_name, $expect_version) = @_;
    my $infofile = $pkgpath . '/pkginfo';
    my $version = '';
    my $name = '';
    open INFO, "<$infofile" or die "Cannot open $infofile";
    while (<INFO>) {
	chomp;
	if (m|^PKG=(.+)|) {
	    $name = $1;
	} elsif (m|^VERSION=(.+)|) {
	    $version = $1;
	}
    }
    close INFO;
    die "Could not find PKG in $infofile" unless $name;
    die "Could not find VERSION in $infofile" unless $version;
    print STDERR "Warning: $pkgpath: expected pkg name $expect_name," .
	" but found name $name\n" if ($name ne $expect_name);
    print STDERR "Warning: $pkgpath: expected pkg version $expect_version," .
	" but found version $version\n" if ($version ne $expect_version);
}
