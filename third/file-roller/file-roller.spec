%define release 6
%define prefix  /usr
%define name	file-roller
%define version 2.8.0

Summary:	An archive manager for GNOME.
Name:		%{name}
Version:    	%{version}
Release:	%{release}
Copyright:	GPL
Vendor:		GNOME
URL:		http://fileroller.sourceforge.net
Group:		Applications/Archiving
Source0:	%{name}-%{version}.tar.gz
Packager:       Paolo Bacchilega <paolo.bacch@tin.it>
BuildRoot:	%{_builddir}/%{name}-%{version}-root
Requires:       glib2 >= 2.0.0
Requires:       gtk2 >= 2.1.0
Requires:	libgnome >= 2.1.0
Requires:	libgnomeui >= 2.1.0
Requires:	gnome-vfs2 >= 2.1.3
Requires:	libglade2 >= 2.0.0
Requires:	bonobo-activation >= 1.0.0
Requires:	libbonobo >= 2.0.0
Requires:	libbonoboui >= 2.0.0
BuildRequires:	glib2-devel >= 2.0.0
BuildRequires:	gtk2-devel >= 2.1.0
BuildRequires:	libgnome-devel >= 2.1.0
BuildRequires:	libgnomeui-devel >= 2.1.0
BuildRequires:	gnome-vfs2-devel >= 2.1.3
BuildRequires:	libglade2-devel >= 2.0.0
BuildRequires:	bonobo-activation-devel >= 1.0.0
BuildRequires:	libbonobo-devel >= 2.0.0
BuildRequires:	libbonoboui-devel >= 2.0.0
Docdir:         %{prefix}/share/doc

%description
File Roller is an archive manager for the GNOME environment.  This means that 
you can : create and modify archives; view the content of an archive; view a 
file contained in the archive; extract files from the archive.
File Roller is only a front-end (a graphical interface) to archiving programs 
like tar and zip. The supported file types are :
    * Tar archives uncompressed (.tar) or compressed with
          * gzip (.tar.gz , .tgz)
          * bzip (.tar.bz , .tbz)
          * bzip2 (.tar.bz2 , .tbz2)
          * compress (.tar.Z , .taz)
          * lzop (.tar.lzo , .tzo)
    * Zip archives (.zip)
    * Jar archives (.jar , .ear , .war)
    * Lha archives (.lzh)
    * Rar archives (.rar)
    * Single files compressed with gzip, bzip, bzip2, compress, lzop

%prep
%setup 

%build
%configure --disable-schemas-install
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/file-roller
%{_datadir}/applications/file-roller.desktop
%{_datadir}/file-roller/glade/*.glade
%{_datadir}/locale/*/LC_MESSAGES/file-roller.mo
%{_datadir}/application-registry/file-roller.applications
%{_datadir}/mime-info/*
%{_datadir}/pixmaps/file-roller.png
%{_libdir}/bonobo/*.a
%{_libdir}/bonobo/*.la
%{_libdir}/bonobo/*.so
%{_libdir}/bonobo/servers/*.server
%{_datadir}/omf/file-roller/*.omf
%doc %{_datadir}/gnome/help/file-roller
%doc AUTHORS NEWS README COPYING
%config %{_sysconfdir}/gconf/schemas/*

%post
GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/file-roller.schemas
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi

%postun
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi
