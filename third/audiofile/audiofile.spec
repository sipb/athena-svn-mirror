%define  ver     0.1.9
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define  prefix  /usr

Summary: Library to handle various audio file formats
Name: audiofile
Version: %ver
Release: %rel
Copyright: LGPL
Group: Libraries/Sound
Source: ftp://ftp.gnome.org/pub/GNOME/sources/audiofile/audiofile-%{PACKAGE_VERSION}.tar.gz
BuildRoot:/var/tmp/audiofile-%{PACKAGE_VERSION}-root
Docdir: %{prefix}/doc
Obsoletes: libaudiofile

%description
Library to handle various audio file formats.
Used by the esound daemon.

%package devel
Summary: Libraries, includes, etc to develop audiofile applications
Group: Libraries

%description devel
Libraries, include files, etc you can use to develop audiofile applications.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
make $MAKE_FLAGS

%install

rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

#
# makefile is broken, sets exec_prefix explicitely.
#
make exec_prefix=$RPM_BUILD_ROOT/%{prefix} prefix=$RPM_BUILD_ROOT/%{prefix} install 

%clean
rm -rf $RPM_BUILD_ROOT

%changelog

* Fri Nov 20 1998 Michael Fulbright <drmike@redhat.com>
- First try at a spec file

%files
%defattr(-, root, root)
%doc COPYING TODO README ChangeLog docs
%{prefix}/bin/*
%{prefix}/lib/lib*.so.*

%files devel
%defattr(-, root, root)
%{prefix}/lib/lib*.so
%{prefix}/lib/*.a
%{prefix}/include/*
%{prefix}/share/aclocal/*

