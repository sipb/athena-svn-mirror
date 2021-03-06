package Mail::ExpandAliases;

# ----------------------------------------------------------------------
# Makefile.PL - Generate a Makefile
# Copyright (C) 2002 darren chamberlain <darren@cpan.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; version 2.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA
# ----------------------------------------------------------------------

use strict;

use ExtUtils::MakeMaker;
use File::Basename qw(basename);

my $dist = '$(DISTNAME)-$(VERSION).tar.gz';
my @EXE_FILES = ('expand-alias');
my %PREREQ_PM = ('IO::File');
my %clean     = ('FILES' => "$dist $dist.asc distdir");
my %macro     = (
    'GPG' => 'gpg',
    'AUTHOR' => 'darren@cpan.org',
    'SIGN'  => '--detach-sign --armor',
);

WriteMakefile(
    NAME            => __PACKAGE__,
    VERSION_FROM    => 'ExpandAliases.pm',
    EXE_FILES       => \@EXE_FILES,
    PREREQ_PM       => \%PREREQ_PM,
    clean           => \%clean,
    macro           => \%macro,
);

package MY;
sub dist_dir {
	my $self = shift;
	my $stuff = $self->SUPER::dist_dir(@_);

	return "$stuff\ttouch distdir\n\n";
}

sub postamble {
    return <<'P';
distsign :: $(DISTVNAME).tar$(SUFFIX)
	$(GPG) $(SIGN) -u $(AUTHOR) $(DISTVNAME).tar$(SUFFIX)
	@cat $(DISTVNAME).tar$(SUFFIX).asc
P
}

__END__
