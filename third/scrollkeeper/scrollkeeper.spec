%define xmlcatalog	%{_sysconfdir}/xml/catalog	

Summary:    ScrollKeeper is a cataloging system for documentation on open systems.
Name:       scrollkeeper
Version:    0.3.14
Release:    1
Source0:    http://download.sourceforge.net/scrollkeeper/%{name}-%{version}.tar.gz
License:    LGPL
Group:      System Environment/Base
BuildRoot:  %{_tmppath}/%{name}-buildroot
URL:        http://scrollkeeper.sourceforge.net/
Requires:   libxml2 >= 2.4.19
Requires:   libxslt
BuildRequires:   libxml2-devel
BuildRequires:   libxslt-devel


%description 
ScrollKeeper is a cataloging system for documentation. It manages
documentation metadata, as specified by the Open Source Metadata
Framework (OMF), as well as metadata which it extracts directly
from DocBook documents.  It provides a simple API to allow help 
browsers to find, sort, and search the document catalog. Some 
day it may also be able to communicate with catalog servers on 
the Net to search for documents which are not on the local system.

%prep
%setup

%build
%configure
make %{?_smp_mflags}

%install
if [ ! $RPM_BUILD_ROOT = "/" ]; then rm -rf $RPM_BUILD_ROOT; fi
%makeinstall
if [ ! $RPM_BUILD_ROOT = "/" ]; then rm -rf $RPM_BUILD_ROOT/var; fi
		# We must remove .../var b/c rpm >=4.1 doesn't allow
		# files which are not packaged in the buildroot.

%find_lang %{name}

%clean
if [ ! $RPM_BUILD_ROOT = "/" ]; then rm -rf $RPM_BUILD_ROOT; fi

%pre
rm -rf %{_datadir}/scrollkeeper/Templates || true

%files -f %{name}.lang
%defattr(-,root,root)
%doc COPYING COPYING.DOC AUTHORS README ChangeLog NEWS INSTALL TODO
%doc scrollkeeper-spec.txt
%config %{_sysconfdir}/*
%{_datadir}/omf/scrollkeeper
%{_bindir}/*
%{_libdir}/*
%{_mandir}/*/*
%{_datadir}/xml/scrollkeeper
%{_datadir}/scrollkeeper


%post
if [ $1 = 2 ]; then
  # Upgrading
  echo "`date +"%b %d %X"` Upgrading to ScrollKeeper `scrollkeeper-config --version`..." >> %{_localstatedir}/log/scrollkeeper.log
fi
if [ $1 = 1 ]; then
  # Installing
  echo "`date +"%b %d %X"` Installing ScrollKeeper `scrollkeeper-config --version`..." >> %{_localstatedir}/log/scrollkeeper.log
fi
/usr/bin/xmlcatalog --noout --add "public" \
	"-//OMF//DTD Scrollkeeper OMF Variant V1.0//EN" \
	"%{_datadir}/xml/scrollkeeper/dtds/scrollkeeper-omf.dtd" %xmlcatalog
scrollkeeper-rebuilddb -q -p %{_localstatedir}/lib/scrollkeeper || true
/sbin/ldconfig

%postun
if [ $1 = 0 ]; then
  # SK is being removed, not upgraded.
  # Remove all generated files
  rm -rf %{_localstatedir}/lib/scrollkeeper
  rm -rf %{_localstatedir}/log/scrollkeeper.log
  rm -rf %{_localstatedir}/log/scrollkeeper.log.1
  /usr/bin/xmlcatalog --noout --del \
	"%{_datadir}/xml/scrollkeeper/dtds/scrollkeeper-omf.dtd" %xmlcatalog
fi
/sbin/ldconfig

%changelog
* Sat Dec 6 2003 Malcolm Tredinnick <malcolm@commsecure.com.au>
- Remove some needless Requires.
- Add some more documentation.

* Wed Jan 22 2003 Dan Mueth <muet@alumni.uchicago.edu>
- Fix duplication of *.mo files and added removal of .../var files at end
-  of build to satisfy rpm >= 4.1.

* Sun Apr 7 2002 Dan Mueth <muet@alumni.uchicago.edu>
- Adding DTD to %files and xmlcatalog registration scripts to %post and %postun

* Thu Apr 4 2002 Dan Mueth <muet@alumni.uchicago.edu>
- Integrating some of Red Hat's modifications

* Sun Feb 17 2002 Dan Mueth <muet@alumni.uchicago.edu>
- Making sysconfdir files as %config

* Fri Feb 8 2002 Dan Mueth <d-mueth@uchicago.edu>
- Small updates to keep from blowing away / by accident, thanks to Paul 
-       Heinlein <heinlein@measurecast.com>

* Tue Jan 15 2002 Dan Mueth <d-mueth@uchicago.edu>
- Having variable files only removed by %postun on an rpm removal, not on an upgrade.
- Changing the three manual lines for database rebuildding with scrollkeeper-rebuilddb
- Adding logging lines for upgrading/installing
- Note: From SK 0.2 to 0.3.1, we had a badly written %postun which blows away
-       the rebuilt database after an upgrade :(

* Sun Jan 13 2002 Dan Mueth <d-mueth@uchicago.edu>
- Added BuildRequires for libxml2-devel

* Sat Jan 12 2002 Dan Mueth <d-mueth@uchicago.edu>
- Added %postun to remove log files.

* Mon Mar 5 2001 Dan Mueth <dan@eazel.com>
- Added %postun to remove $datadir/scrollkeeper/templates
  to compensate for breakage in upgrade from 0.1.1 to 0.1.2

* Sun Mar 4 2001 Dan Mueth <dan@eazel.com>
- Added cleaner symbolic link section suggested by Karl 
  Eichwalder <keichwa@users.sourceforge.net>
- Have it blow away the database dir on first install, just
  in case an old tarball version had been installed
- Fixing the Source0 line at the top

* Tue Feb 15 2001 Dan Mueth <dan@eazel.com>
- added line to include the translations .mo file

* Tue Feb 06 2001 Dan Mueth <dan@eazel.com>
- fixed up pre and post installation scripts

* Tue Feb 06 2001 Laszlo Kovacs <laszlo.kovacs@sun.com>
- added all the locale directories and links for the template
  content list files

* Wed Jan 17 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- converted to scrollkeeper.spec.in

* Sat Dec 16 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- help files added

* Fri Dec 8 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- various small fixes added

* Thu Dec 7 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- fixing localstatedir problem
- adding postinstall and postuninstall scripts

* Tue Dec 5 2000 Gregory Leblanc <gleblanc@cu-portland.edu>
- adding COPYING, AUTHORS, etc
- fixed localstatedir for the OMF files

* Fri Nov 10 2000 Gregory Leblanc <gleblanc@cu-portland.edu>
- Initial spec file created.


