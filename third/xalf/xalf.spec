# Note that this is NOT a relocatable package
%define ver      0.12
%define rel      1
%define prefix   /usr

Summary: A utility to provide feedback when starting X11 applications.
Name: xalf
Version: %ver
Release: %rel
Copyright: GPL
Group: X11/Utilities
Source: xalf-%{ver}.tgz
BuildRoot: /var/tmp/%{name}-root
URL: http://www.lysator.liu.se/~astrand/projects/xalf

%description
This is a small utility to provide feedback when starting X11
applications.  Feedback can be given via four different indicators:
An invisible window (to be used in conjunction with a task pager like
Gnomes tasklist_applet or KDE Taskbar), an generic splashscreen, an
hourglass attached to the mouse cursor or an animated star. 

%changelog
* Sun Apr 22 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.12

* Sun Apr 8 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.11

* Mon Apr 2 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.11_test

* Tue Mar 13 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.10

* Wed Mar 07 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.9

* Mon Feb 19 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.8 

* Mon Feb 12 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.7 (yes, two releases on the same day!)

* Mon Feb 12 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.6

* Wed Jan 31 2001 Peter 흒trand <astrand@lysator.liu.se>
- version 0.5

* Sun Jun 18 2000 Peter Astrand <altic@lysator.liu.se>
- version 0.4

* Thu Jun 1 2000 Peter Astrand <altic@lysator.liu.se>
- version 0.3

* Sat Apr 15 2000 Peter Astrand <altic@lysator.liu.se>
- version 0.2

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
make

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install-strip

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS FAQ COPYING ChangeLog NEWS README INSTALL TODO BUGS extras 
%{prefix}/lib/libxalflaunch.*
%{prefix}/bin/xalf
%{prefix}/bin/xalfoff
%{prefix}/bin/xalf-capplet
%{prefix}/share/pixmaps/hourglass-big.png
%{prefix}/share/pixmaps/hourglass-small.png
%{prefix}/share/control-center/Desktop/xalf-settings.desktop
%{prefix}/share/gnome/apps/Settings/Desktop/xalf-settings.desktop


