%define name		bonobo-conf
%define ver		0.14
%define rel		%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Name:		%name
Summary:	Bonobo configuration moniker.
Version: 	%ver
Release: 	1
Copyright: 	GPL
Group:		System Environment/Libraries
Source: 	%{name}-%{ver}.tar.gz
URL: 		http://www.gnome.org/
BuildRoot:	%{_tmppath}/%{name}-%{ver}-root
Requires:       bonobo >= 1.0.0

%description
Bonobo configuration moniker.

%package devel
Summary:	Libraries and include files for the configuration moniker.
Group:		Development/Libraries
Requires:	%name = %{PACKAGE_VERSION}

%description devel
This package provides the necessary development libraries and include
files to allow you to develop programs using the Bonobo configuration moniker.

%prep
%setup

%build
%configure
make

%install
%makeinstall

%clean

%post
if ! grep %{_libdir} /etc/ld.so.conf > /dev/null ; then
  echo "%{_libdir}" >> /etc/ld.so.conf
fi
  
/sbin/ldconfig
  
%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README
%{_bindir}/*
%{_libdir}/bonobo/monikers
%{_libdir}/*.sh
%{_libdir}/lib*.so.*
%{_datadir}/idl/*.idl
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/oaf/*.oaf

%files devel
%defattr(-, root, root)
%{_includedir}/bonobo-conf
%{_libdir}/lib*.so
%{_libdir}/lib*.a
%{_libdir}/lib*.la


%changelog
* Wed Jun 06 2001 Jens Finke <jens@gnome.org>
- created spec file
