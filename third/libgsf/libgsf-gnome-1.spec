%define name libgsf-1
%define version 1.8.2
%define release 1
%define prefix /usr

Summary: GNOME specific extensions to libgsf

Name: %{name}
Version: %{version}
Release: %{release}
Group: System Environment/Libraries
License: LGPL

Source: ftp://ftp.gnome.org/pub/GNOME/unstable/sources/libgsf/libgsf-%{version}.tar.gz
Buildroot: /var/tmp/%{name}-%{version}-%{release}-root
URL: http://www.gnumeric.org

Requires: libgsf-1 >= 1.3.0 gnome-vfs >= 2.0.0

%description
GNOME specific extensions to support bonobo and gnome-vfs

%package devel
Summary: Support files necessary to compile applications with libgsf-gnome.
Group: Development/Libraries
Requires: libgsf-gnome-1

%description devel
Libraries, headers, and support files necessary to compile applications using
GNOME specific extensions to libgsf.

%prep

%setup

%build
%ifarch alpha
  MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

if [ ! -f configure ]; then
CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%{prefix}
else
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix}
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
if [ -d $RPM_BUILD_ROOT ]; then rm -r $RPM_BUILD_ROOT; fi
mkdir -p $RPM_BUILD_ROOT%{prefix}
make prefix=$RPM_BUILD_ROOT%{prefix} install

%files
%defattr(644,root,root,755)
%doc AUTHORS COPYING README
%{prefix}/lib/lib*.so.*

%files devel
%defattr(644,root,root,755)
%{prefix}/lib/lib*.so
%{prefix}/lib/*a
%{prefix}/lib/*.sh
%{prefix}/include/libgsf-1/*
%{prefix}/share/doc/gsf/*
%{prefix}/lib/pkgconfig/libgsf-gnome-1.pc

%clean
rm -r $RPM_BUILD_ROOT

%changelog
* Thu Aug 15 2002 Jody Goldberg <jody@gnome.org>
- Initial version