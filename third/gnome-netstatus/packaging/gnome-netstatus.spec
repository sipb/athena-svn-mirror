%define name gnome-netstatus
%define version 2.8.0
%define  RELEASE 1
%define  release     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Name: %{name}
Summary: Applet that displays the network status in a gnome panel.
Version: %{version}
Release: %{release}
Source: ftp://ftp.gnome.org/pub/GNOME/sources/gnome-netstatus/0.11/%{name}-%{version}.tar.bz2
Group: Applications
URL: ftp://ftp.gnome.org/pub/GNOME/sources/gnome-netstatus
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
License: GPL
Requires: glib2 >= 2.0.0 gtk2 >= 2.0.0
BuildRequires: glib2-devel >= 2.0.0 gtk2-devel >= 2.0.0

%description
Applet that displays the network status in a gnome panel.

%prep
%__rm -rf $RPM_BUILD_ROOT

%setup -q -n %{name}-%{version}

%build
./configure --prefix=%{_prefix} --sysconfdir=/etc

%__make

%install
umask 022

%__make DESTDIR=$RPM_BUILD_ROOT install

%clean
%__rm -rf $RPM_BUILD_ROOT $RPM_BUILD_DIR/file.list.%{name}

#%files -f %{name}.lang
%files
%defattr(755,root,root,755)
%{_libexecdir}/*
%{_datadir}/%{name}/*
%{_datadir}/gnome-2.0/ui/*
%{_datadir}/icons/%{name}/*
%{_datadir}/pixmaps/*
%{_libdir}/bonobo/servers/*
/etc/gconf/schemas/*

%changelog
* Fri Aug 15 2003 Rui M. Seabra <rms@1407.org>
- Create spec
