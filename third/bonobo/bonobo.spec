# Note this is NOT a relocatable thing :)
%define name		bonobo
%define ver		0.36
%define RELEASE		pre
%define rel		%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix		/usr
%define sysconfdir	/etc

Name:		%name
Summary:	Library for compound documents in GNOME
Version: 	%ver
Release: 	%rel
Copyright: 	GPL
Group:		System Environment/Libraries
Source: 	%{name}-%{ver}.tar.gz
URL: 		http://www.gnome.org/
BuildRoot:	/var/tmp/%{name}-%{ver}-root
Docdir: 	%{prefix}/doc
Requires:	gnome-libs >= 1.2.5
Requires:	ORBit >= 0.5.4
Requires:	oaf >= 0.5.1
Requires:	libxml >= 1.8.10

%description
Bonobo is a library that provides the necessary framework for GNOME
applications to deal with compound documents, i.e. those with a
spreadsheet and graphic embedded in a word-processing document.

%package devel
Summary:	Libraries and include files for the Bonobo document model
Group:		Development/Libraries
Requires:	%name = %{PACKAGE_VERSION}

%description devel
This package provides the necessary development libraries and include
files to allow you to develop programs using the Bonobo document model.

%changelog
* Wed Oct 18 2000 Eskil Heyn Olsen <eskil@eazel.com>
- Added requirements to the base package
- Added bonobo-ui-extract to the file list of the base pacakge

* Tue Feb 22 2000 Jens Finke <jens.finke@informatik.uni-oldenburg.de>
- Added bonobo.h to the file list of devel package.

* Wed Nov 10 1999 Alexander Skwar <ASkwar@DigitalProjects.com>
- Updated to version 0.5
- fixed spec file
- Un-quiet things
- stripped binaries
- unsetted language environment variables

* Sat Oct 2 1999 Gregory McLean <gregm@comstar.net>
- Updated the spec for version 0.4
- Updated the files section.

* Sun Aug 1 1999 Gregory McLean <gregm@comstar.net>
- Some updates. sysconfdir stuff, quiet down the prep/configure stage.

* Sat May 1 1999 Erik Walthinsen <omega@cse.ogi.edu>
- created spec file

%prep
%setup

%build
%ifarch alpha
  MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

LC_ALL=""
LINGUAS=""
LANG=""
export LC_ALL LINGUAS LANG

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%{prefix} \
	--sysconfdir=%{sysconfdir}

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make -k
fi

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make -k prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install

for FILE in "$RPM_BUILD_ROOT/bin/*"; do
	file "$FILE" | grep -q not\ stripped && strip $FILE
done

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
if ! grep %{prefix}/lib /etc/ld.so.conf > /dev/null ; then
  echo "%{prefix}/lib" >> /etc/ld.so.conf
fi
  
/sbin/ldconfig
  
%postun -p /sbin/ldconfig

%files
%defattr(0555, bin, bin)
%{prefix}/bin/bonobo-application-x-mines
%{prefix}/bin/bonobo-audio-ulaw
%{prefix}/bin/bonobo-echo
%{prefix}/bin/bonobo-sample-canvas-item
%{prefix}/bin/bonobo-sample-controls
%{prefix}/bin/bonobo-sample-hello
%{prefix}/bin/bonobo-sample-paint
%{prefix}/bin/bonobo-text-plain
%{prefix}/bin/echo-client
%{prefix}/bin/efstool
%{prefix}/bin/libefs-config
%{prefix}/bin/sample-container
%{prefix}/bin/sample-control-container
%{prefix}/lib/pkgconfig/libefs.pc
%{prefix}/lib/bonobo/plugin/*
%{prefix}/lib/bonobo/monikers/*
%{prefix}/lib/*.0
%{prefix}/lib/*.1
%{prefix}/lib/*.sh
%{prefix}/lib/*.so


%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README

%defattr (0444, bin, bin)
# %{prefix}/share/bonobo/html/bonobo/*.html
# %{prefix}/share/bonobo/html/bonobo/*.sgml
# %{prefix}/share/bonobo/html/*.html
# %config %{sysconfdir}/CORBA/servers/*.gnorba
%{prefix}/share/bonobo/html/*.hierarchy
%{prefix}/share/bonobo/html/*.sgml
%{prefix}/share/bonobo/html/*.signals
%{prefix}/share/bonobo/html/*.txt
%{prefix}/share/bonobo/html/*.types
%{prefix}/share/idl/*.idl
%{prefix}/share/locale/*/LC_MESSAGES/*.mo
%{prefix}/share/mime-info/*.keys
%{prefix}/share/oaf/*.oaf

%files devel

%defattr(0555, bin, bin)
%dir %{prefix}/include/bonobo
%{prefix}/lib/*.a
%{prefix}/lib/*.la

%defattr(0444, bin, bin)
%{prefix}/include/*.h
%{prefix}/include/bonobo/*.h
