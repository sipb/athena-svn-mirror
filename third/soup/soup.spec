# Note that this is NOT a relocatable package
%define ver      0.7.10
%define rel      1
%define prefix   /usr

Summary: Soup, a SOAP implementation
Name:      soup
Version:   %ver
Release:   %rel
Copyright: LGPL
Group:     Libraries/Network
Source0:   soup-%{PACKAGE_VERSION}.tar.gz
URL:       http://www.ximian.com/applications/soup
BuildRoot: /var/tmp/soup-%{PACKAGE_VERSION}-root
Docdir:    %{prefix}/doc
Packager:  Alex Graveley <alex@ximian.com>
Requires:  glib >= 1.2
Requires:  libxml >= 1.8

%description
Soup is a SOAP (Simple Object Access Protocol) implementation in C. It 
provides an queued asynchronous callback-based mechanism for sending 
SOAP requests. A WSDL (Web Service Definition Language) to C compiler 
which generates stubs for easily calling remote SOAP methods, and a 
CORBA IDL to WSDL compiler are also included.

Features:
  * Completely Asynchronous
  * Connection cache
  * HTTP chunked transfer support
  * authenticated HTTP, SOCKS4, and SOCKS5 proxy support
  * SSL Support using either OpenSSL or NSS

Soup requires Glib 1.2 and LibXML 1.8.  You can find Glib at 
http://www.gtk.org, and LibXML at 
http://rufus.w3.org/veillard/XML/xml.html.

Comments, questions, and bug reports should be sent to
alex@ximian.com.

The Soup homepage is at http://www.ximian.com/applications/soup.

%package devel
Summary: Header files for the Soup library
Group: Development/Libraries

%description devel
Soup is a SOAP (Simple Object Access Protocol) implementation in C. It 
provides an queued asynchronous callback-based mechanism for sending 
SOAP requests.
This package allows you to develop applications that use the Soup
library.


%prep
%setup

%build
%ifarch alpha
   CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" ./configure --host=alpha-redhat-linux\
	--prefix=%{prefix} \
	--enable-debug=yes \
	--with-gnu-ld
%else
   CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" ./configure \
	--prefix=%{prefix} \
	--enable-debug=yes \
	--with-gnu-ld 
%endif
make

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc README COPYING ChangeLog NEWS TODO AUTHORS INSTALL HACKING doc/html
%{prefix}/lib/lib*.so*

%files devel
%defattr(-, root, root)
%{prefix}/bin/soup-config
%{prefix}/include/soup
%{prefix}/lib/*a
%{prefix}/lib/soupConf.sh
%{prefix}/share/aclocal/soup.m4

%changelog
* Tue Jan 23 2001 Alex Graveley <alex@ximian.com>
- Inital RPM config.