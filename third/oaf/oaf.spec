# Note that this is NOT a relocatable package
%define name		oaf
%define ver		0.6.5
%define RELEASE		1
%define rel		%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix		/usr
%define sysconfdir	/etc

Name:		%name
Summary:	Object activation framework for GNOME
Version: 	%ver
Release: 	%rel
License: 	LGPL and GPL
Group:		System Environment/Libraries
Source: 	%{name}-%{ver}.tar.gz
URL: 		http://www.gnome.org/
BuildRoot:	/var/tmp/%{name}-%{ver}-root
Docdir: 	%{prefix}/doc

%description
OAF is an object activation framework for GNOME. It uses ORBit.

%package devel
Summary:	Libraries and include files for OAF
Group:		Development/Libraries
Requires:	%name = %{PACKAGE_VERSION}
Obsoletes:	%{name}-devel

%description devel

%changelog
* Tue Aug 29 2000 Maciej Stachowiak <mjs@eazel.com>
- corrected Copyright field and renamed it to License
* Sun May 21 2000 Ross Golder <rossigee@bigfoot.com>
- created spec file (based on bonobo.spec.in)

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

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS \
	--enable-more-warnings \
	--prefix=%{prefix} \
	--sysconfdir=%{sysconfdir}

make -k

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

%doc AUTHORS COPYING ChangeLog NEWS README
%config %{sysconfdir}/oaf/*.sample
%config %{sysconfdir}/oaf/*.xml
%{prefix}/bin/oaf-client
%{prefix}/bin/oaf-config
%{prefix}/bin/oaf-run-query
%{prefix}/bin/oaf-slay
%{prefix}/bin/oaf-sysconf
%{prefix}/bin/oafd
%{prefix}/lib/*.0
%{prefix}/lib/*.sh
%{prefix}/lib/*.so

%defattr (0444, bin, bin)
%{prefix}/share/idl/*.idl
%{prefix}/share/locale/da/LC_MESSAGES/*.mo
%{prefix}/share/locale/de/LC_MESSAGES/*.mo
%{prefix}/share/locale/no/LC_MESSAGES/*.mo
%{prefix}/share/locale/ru/LC_MESSAGES/*.mo
%{prefix}/share/locale/tr/LC_MESSAGES/*.mo
%{prefix}/share/oaf/*.oafinfo

%files devel

%defattr(0555, bin, bin)
%dir %{prefix}/include/liboaf
%{prefix}/lib/*.la

%defattr(0444, bin, bin)
%{prefix}/include/liboaf/*.h
%{prefix}/share/aclocal/*.m4
