#!/usr/bin/perl -w

# $Id: gen-stats.pl,v 1.1 2004-04-15 18:00:52 rbasch Exp $

# This script generates the stats file used by the os-checkfiles program
# from the pkgmaps for a set of packages.

use strict;
use warnings FATAL => 'all';
use Getopt::Std;
use File::Basename;

sub user_to_uid($);
sub group_to_gid($);

sub usage(;$);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: $prog [<options>] [<package> ...]
  Options:
    -a <arch>        process only ARCH=<arch> packages
    -d <directory>   specify package directory, default is .
    -v               be verbose (writes to standard error)
EOF
    exit 1;
}

# Parse the command line arguments.
my %opts;
getopts('a:d:v', \%opts) || usage;

my $arch = $opts{'a'};
my $pkgdir = $opts{'d'} || '.';
my $verbose = $opts{'v'};

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

my %uids = ();
my %gids = ();
my %paths = ();

# Process each package.
foreach my $pkg (@pkgs) {
    my ($pkg_arch, $basedir) = ();
    # Get the ARCH and BASEDIR settings from the pkginfo file.
    my $pkginfo = $pkgdir . '/' . $pkg . '/pkginfo';
    open PKGINFO, "<$pkginfo" or die "Cannot open $pkginfo";
    while (<PKGINFO>) {
	chomp;
	if (m|^ARCH=(.+)|) {
	    $pkg_arch = $1;
	} elsif (m|^BASEDIR=(.+)|) {
	    $basedir = $1;
	}
    }
    close PKGINFO;
    die "Invalid pkginfo file $pkginfo" unless ($pkg_arch && $basedir);
    # Skip this package if it has the wrong ARCH.
    next if ($arch && ($arch ne $pkg_arch));
    print STDERR "$pkg ...\n" if $verbose;

    # Strip leading and trailing /'s from the base directory.
    $basedir =~ s|^/*(.*)/*$|$1|o;

    # Read the pkgmap file.
    my $pkgmap = $pkgdir . '/' . $pkg . '/pkgmap';
    open PKGMAP, "<$pkgmap" or die "Cannot open $pkgmap";
    while (<PKGMAP>) {
	chomp;
	my ($part, $type, $class, $path, $mode, $owner, $group, $size,
	    $cksum, $mtime) = split;
	# Only look at regular files, directories, and links.
	next if (index('dfls', $type) == -1);
	next unless $path;
	next if ($type eq 'd' && ($mode eq '?' || $owner eq '?' ||
				  $group eq '?'));
	my ($linkpath, $linktarget) = ();
	if ($type eq 's' || $type eq 'l') {
	    ($linkpath, $linktarget) = split(/=/, $path);
	    next unless $linktarget;
	    $path = $linkpath;
	}
	# Build the full path, delimiting the base directory and the given
	# path as necessary.
	$path = $basedir . ($basedir && ($path !~ m|^/|o) ? '/' : '') . $path;
	if (exists $paths{$path}) {
	    # We have already seen this path; check for a conflict.
	    if ($type ne 'd' ||
		$type ne $paths{$path}{type} ||
		$mode ne $paths{$path}{mode} ||
		user_to_uid($owner) ne $paths{$path}{uid} ||
		group_to_gid($group) ne $paths{$path}{gid}) {
		print STDERR "Skipping $path, conflict between $pkg" .
		    " and $paths{$path}{pkg}\n";
		$paths{$path}{skip} = 1;
	    }
	}
	$paths{$path}{pkg} = $pkg;
	$paths{$path}{type} = $type;
	if ($type eq 'f' || $type eq 'd') {
	    $paths{$path}{mode} = $mode;
	    $paths{$path}{uid} = user_to_uid($owner);
	    $paths{$path}{gid} = group_to_gid($group);
	    if ($type eq 'f') {
		$paths{$path}{size} = $size;
		$paths{$path}{mtime} = $mtime;
	    }
	} elsif ($type eq 's') {
	    $paths{$path}{linktarget} = $linktarget;
	} elsif ($type eq 'l') {
	    # Hard link targets in the pkgmap are relative to the path;
	    # convert them to full paths.
	    my $dir = dirname $path;
	    my @comps;
	    for (@comps = split('/', $linktarget);
		 $comps[0] eq '..';
		 shift @comps) {
		$dir = dirname $dir;
	    }
	    $paths{$path}{linktarget} = ($dir eq '.' ? '' : ($dir . '/')) .
		join('/', @comps);
	}
    }
    close PKGMAP;
}

# Print out the stats line for each path whose "skip" flag is not set.
print STDERR "Writing stats ...\n" if $verbose;
for my $path (sort keys %paths) {
    next if $paths{$path}{skip};
    if ($paths{$path}{type} eq 'f') {
	# The path is a regular file.
	print "f $paths{$path}{mode} $paths{$path}{size} $paths{$path}{uid}" .
	    " $paths{$path}{gid} $paths{$path}{mtime} $path\n";
    } elsif ($paths{$path}{type} eq 'd') {
	# path is a directory.
	print "d $paths{$path}{mode} $paths{$path}{uid} $paths{$path}{gid}" .
	    " $path\n";
    } elsif ($paths{$path}{type} eq 's' || $paths{$path}{type} eq 'l') {
	# path is a symbolic or hard link.
	print ($paths{$path}{type} eq 'l' ? "h" : "l");
	print " $paths{$path}{linktarget} $path\n";
    }
}

# Look up the uid for the given user name.  We cache results in the %uids
# hash for speed.
sub user_to_uid($) {
    my $username = $_[0];
    my $uid;
    if (exists $uids{$username}) {
	$uid = $uids{$username};
    } else {
	$uid = getpwnam $username;
	die "Unknown username $username\n" unless defined $uid;
	$uids{$username} = $uid;
	print STDERR "User $username has uid $uid\n" if $verbose;
    }
    return $uid;
}

# Look up the gid for the given group name.  We cache results in the %gids
# hash for speed.
sub group_to_gid($) {
    my $groupname = $_[0];
    my $gid;
    if (exists $gids{$groupname}) {
	$gid = $gids{$groupname};
    } else {
	$gid = getgrnam $groupname;
	die "Unknown group $groupname\n" unless defined $gid;
	$gids{$groupname} = $gid;
	print STDERR "Group $groupname has gid $gid\n" if $verbose;
    }
    return $gid;
}
