# Note that this is NOT a relocatable package
%define ver      0.25
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix   /usr

Summary: Gnome Print - Printing libraries for GNOME.
Name: 		gnome-print
Version: 	%ver
Release: 	%rel
Copyright: 	LGPL
Group: 		System Environment/Base
Source: ftp://ftp.gnome.org/pub/GNOME/sources/gnome-print/gnome-print-%{ver}.tar.gz
BuildRoot: 	/var/tmp/gnome-print-%{ver}-root
Requires: 	gnome-libs >= 1.0
Requires:       urw-fonts
Requires:       ghostscript-fonts >= 4.03
DocDir:		%{prefix}/doc
Prereq:	/sbin/ldconfig libxml

%description
GNOME (GNU Network Object Model Environment) is a user-friendly set of
applications and desktop tools to be used in conjunction with a window
manager for the X Window System.  GNOME is similar in purpose and scope
to CDE and KDE, but GNOME is based completely on free software.
The gnome-print package contains libraries and fonts that are needed by
GNOME applications wanting to print.

You should install the gnome-print package if you intend on using any of
the GNOME applications that can print. If you would like to develop GNOME
applications that can print you will also need to install the gnome-print
devel package.

%package devel
Summary: Libraries and include files for developing GNOME applications.
Group: 		Development/Libraries

%description devel
GNOME (GNU Network Object Model Environment) is a user-friendly set of
applications and desktop tools to be used in conjunction with a window
manager for the X Window System.  GNOME is similar in purpose and scope
to CDE and KDE, but GNOME is based completely on free software.
The gnome-print-devel package includes the libraries and include files that
you will need when developing applications that use the GNOME printing 
facilities.

You should install the gnome-print-devel package if you would like to 
develop GNOME applications that will use the GNOME printing facilities.
You don't need to install the gnome-print-devel package if you just want 
to use the GNOME desktop enviornment.

%changelog
* Sun Aug 01 1999 Gregory McLean <gregm@comstar.net>
- Undo my draconian uninstall stuff.

* Tue Jul 20 1999 Gregory McLean <gregm@comstar.net>
- Stab at cleaning up properly when we uninstall.

* Fri Jul 16 1999 Herbert Valerio Riedel <hvr@hvrlab.dhs.org>
- fixed typo in spec

* Wed Jul 14 1999 Gregory McLean <gregm@comstar.net>
- Added fonts to the spec.

* Mon Jul 05 1999 Gregory McLean <gregm@comstar.net>
- Fleshed out the descriptions.

%prep
%setup -q

%build
# Needed for snapshot releases.
%ifarch alpha
  MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $MYARCH_FLAGS --prefix=%prefix --localstatedir=/var/lib
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%prefix --localstatedir=/var/lib
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install
# This is ugly
#
cd fonts
install -c *.font $RPM_BUILD_ROOT%{prefix}/share/fonts

%clean
rm -rf $RPM_BUILD_ROOT

%post
if ! grep %{prefix}/lib /etc/ld.so.conf > /dev/null ; then
  echo "%{prefix}/lib" >> /etc/ld.so.conf
fi
/sbin/ldconfig
perl $RPM_BUILD_ROOT/run-gnome-font-install %{prefix}/bin/gnome-font-install \
	%{prefix}/share/ $RPM_BUILD_ROOT

%postun 
/sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README
%{prefix}/lib/lib*.so.*
%{prefix}/bin/*
%{prefix}/share/fonts/afms/adobe/*
%{prefix}/share/fonts/*.font
%config %{prefix}/share/gnome-print/profiles/Postscript.profile

%files devel
%defattr(-, root, root)

%{prefix}/lib/lib*.so
%{prefix}/lib/*.a
%{prefix}/lib/*.la
%{prefix}/lib/*.sh
%{prefix}/include/*/*
