Name:     	gnome-spell	
Version: 	1.0.5	
Release:	%{?custom_release}%{!?custom_release:1}
License:	GPL
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
URL:		http://www.gnome.org/
Source:		ftp://ftp.gnome.org/pub/GNOME/unstable/sources/%{name}/%{name}-%{version}.tar.gz
Summary:	The spelling component for bonobo
Group:		System Environment/Libraries
Requires:	aspell >= 0.28
Requires:	pspell >= 0.12
Requires:	bonobo >= 0.28
Requires:	gal >= 0.7.99.5
BuildRequires:  aspell-devel >= 0.28
BuildRequires:  pspell-devel >= 0.12
BuildRequires:  bonobo-devel >= 0.28
BuildRequires:  libglade-devel
BuildRequires:	gal-devel >= 0.7.99.5

%description
GNOME/Bonobo speeling component

%prep
%setup -q

%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ;
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS ;
FFLAGS="${FFLAGS:-%optflags}" ; export FFLAGS ;
./configure %{_target_platform} \
        --prefix=%{_prefix} \
        --exec-prefix=%{_exec_prefix} \
        --bindir=%{_bindir} \
        --sbindir=%{_sbindir} \
        --sysconfdir=%{_sysconfdir} \
        --datadir=%{_datadir} \
        --libdir=%{_libdir} \
        --libexecdir=%{_libexecdir} \
        --localstatedir=%{_localstatedir} \
        --sharedstatedir=%{_sharedstatedir} \
        --mandir=%{_mandir} \
        --infodir=%{_infodir}
make

%install
if test "$RPM_BUILD_ROOT" != "/"; then
	rm -rf "$RPM_BUILD_ROOT"
fi
make \
        prefix=%{?buildroot:%{buildroot}}%{_prefix} \
        exec_prefix=%{?buildroot:%{buildroot}}%{_exec_prefix} \
        bindir=%{?buildroot:%{buildroot}}%{_bindir} \
        sbindir=%{?buildroot:%{buildroot}}%{_sbindir} \
        sysconfdir=%{?buildroot:%{buildroot}}%{_sysconfdir} \
        datadir=%{?buildroot:%{buildroot}}%{_datadir} \
        includedir=%{?buildroot:%{buildroot}}%{_includedir} \
        libdir=%{?buildroot:%{buildroot}}%{_libdir} \
        libexecdir=%{?buildroot:%{buildroot}}%{_libexecdir} \
        localstatedir=%{?buildroot:%{buildroot}}%{_localstatedir} \
        sharedstatedir=%{?buildroot:%{buildroot}}%{_sharedstatedir} \
        mandir=%{?buildroot:%{buildroot}}%{_mandir} \
        infodir=%{?buildroot:%{buildroot}}%{_infodir} \
install

%{find_lang} %{name}

%clean
if test "$RPM_BUILD_ROOT" != "/"; then
	rm -rf "$RPM_BUILD_ROOT"
fi

%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README
%{_bindir}/*
%{_datadir}/idl/*.idl
%{_datadir}/oaf/*.oafinfo
%{_datadir}/gnome-spell

%changelog
* Thu Nov 01 2001 Peter Bowen <pzb@datastacks.com>
- Include with gnome-spell in CVS

* Thu Oct 04 2001 David Sainty <dsainty@redhat.com>
- Updated to 0.3

* Mon Sep 03 2001 David Sainty <dsainty@redhat.com>
- Updated to CVS snapshot.

* Fri Aug 10 2001 Peter Bowen <pzb@scyld.com> 0.2-0.1
- Initial Revision