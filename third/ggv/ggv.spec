# Note that this is NOT a relocatable package
# defaults for redhat
%define prefix     /usr
%define sysconfdir /etc

%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Summary: GNOME PostScript viewer
Name: 		ggv
Version:	1.99.98
Release: 	%rel
Copyright: 	GPL
Group: 		X11/Utilities
Source:		ftp://ftp.gnome.org/pub/GNOME/sources/%{name}/%{name}-%{version}.tar.gz
BuildRoot: 	/var/tmp/%{name}-%{version}-root
URL: 		http://www.gnome.org/
DocDir: 	%{prefix}/doc

%description
ggv allows you to view PostScript documents, and print ranges
of pages.

%changelog
* Fri Aug 27 1999 Karl Eichwalder <ke@suse.de>
- Added more %doc files
- Fixed the spelling of PostScript and the Source entry

* Sat Aug 21 1999 Herbert Valerio Riedel <hvr@hvrlab.dhs.org>
- Actualized spec file

* Thu Aug 13 1998 Marc Ewing <marc@redhat.com>
- Initial spec file copied from gnome-graphics

%prep
%setup

%build

# libtool workaround for alphalinux
%ifarch alpha
  ARCH_FLAGS="--host=alpha-redhat-linux"
%endif

# Needed for snapshot releases.
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir} --disable-install-schemas
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir} --disable-install-schemas
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS BUGS COPYING ChangeLog MAINTAINERS NEWS README TODO
%{_bindir}/*
%{_datadir}/bin/*
%config %{_sysconfdir}/*

%post
SOURCE=xml::%{sysconfdir}/gconf/gconf.xml.defaults
GCONF_CONFIG_SOURCE=$SOURCE gconftool --makefile-install-rule %{sysconfdir}/gconf/schemas/ggv.schemas 2>/dev/null >/dev/null
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi

%postun
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi
