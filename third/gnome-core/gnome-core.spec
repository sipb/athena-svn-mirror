# Note that this is NOT a relocatable package
%define ver      	1.2.4
%define RELEASE		1
%define rel      	%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define sysconfdir	/etc

Summary: The core programs for the GNOME GUI desktop environment.
Name: 		gnome-core
Version: 	%ver
Release: 	%rel
Copyright: 	LGPL
Group: 		System Environment/Base
Source: ftp://ftp.gnome.org/pub/sources/gnome-core/gnome-core-%{ver}.tar.gz
BuildRoot: 	/var/tmp/%{name}-%{version}-root
URL: 		http://www.gnome.org
Prereq: 	/sbin/install-info
Docdir: 	%{_prefix}/doc

Requires: 	gnome-libs >= 1.0.50
Requires: 	ORBit >= 0.5.0
Requires:	gdk-pixbuf >= 0.7.0

%description
GNOME (GNU Network Object Model Environment) is a user-friendly
set of applications and desktop tools to be used in conjunction with a
window manager for the X Window System.  GNOME is similar in purpose and
scope to CDE and KDE, but GNOME is based completely on free
software.  The gnome-core package includes the basic programs and
libraries that are needed to install GNOME.

You should install the gnome-core package if you would like to use the
GNOME desktop environment.  You'll also need to install the gnome-libs
package.  If you would like to develop GNOME applications, you'll also
need to install gnome-libs-devel.  If you want to use linuxconf with a
GNOME front end, you'll also need to install the gnome-linuxconf package.

%package devel
Summary: GNOME core libraries, includes and more.
Group: 		Development/Libraries
Requires: 	gnome-core
PreReq: 	/sbin/install-info

%description devel
Panel libraries and header files for creating GNOME panels.

%changelog
* Sat Feb 26 2000 Gregory McLean <gregm@comstar.net>
- Updated to 1.1.4
- Autogenerate the %files section.

* Sat Oct 16 1999 Gregory McLean <gregm@comstar.net>
- Updated to 1.0.50
- Sorted the language specific stuff out.

* Sun Oct 03 1999 Gregory McLean <gregm@comstar.net>
- updated to 1.0.50
- Overhauled the %files section.

* Sat Nov 21 1998 Pablo Saratxaga <srtxg@chanae.alphanet.ch>

- Cleaned %files section
- added spanish and french translations for rpm

* Wed Sep 23 1998 Michael Fulbright <msf@redhat.com>

- Built 0.30 release

* Fri Mar 13 1998 Marc Ewing <marc@redhat.com>

- Integrate into gnome-core CVS source tree

%prep
%setup -q

%build
%configure --quiet --disable-gtkhtml-help --sysconfdir=/etc --localstatedir=/var/lib
CFLAGS="$RPM_OPT_FLAGS" make

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} sysconfdir=$RPM_BUILD_ROOT/etc install >install.log 2>&1
#if [ -d $RPM_BUILD_ROOT%{_prefix}/bin ] ; then
# strip `file $RPM_BUILD_ROOT%{_prefix}/bin/* | grep ELF | cut -d':' -f 1`
#fi
if [ -d $RPM_BUILD_ROOT/usr/man ]; then
  find $RPM_BUILD_ROOT/usr/man -type f -exec gzip -9f {} \;
fi
if [ -f %{name}.files ] ; then
  rm -f %{name}.files
fi
##############################################################################
##

function ProcessLang() {
 # rpm provides a handy scriptlet to do the locale stuff lets use that.
 if [ -f /usr/lib/rpm/find-lang.sh ] ; then
  /usr/lib/rpm/find-lang.sh $RPM_BUILD_ROOT %name
  sed "s:(644, root, root, 755):(444, bin, bin, 555):" %{name}.lang >tmp.lang && mv tmp.lang %{name}.lang
  if [ -f %{name}.files ] ; then
    cat %{name}.files %{name}.lang >tmp.files && mv tmp.files %{name}.files
  fi
 fi
}
#
# Build up the list of files that need to be installed.
# its messy but it catches everything that is installed.
#
function ProcessBin() {
  # Gather up all the executable files. Stripping if requested.
  # This will not recurse.
  if [ -d $RPM_BUILD_ROOT%{_prefix}/bin ] ; then
    echo "%defattr (0555, bin, bin)" >>%{name}.files
    find $RPM_BUILD_ROOT%{_prefix}/bin -type f -print | sed "s:^$RPM_BUILD_ROOT::g" >>%{name}.files
  fi
}
function ProcessLib() {
  # Gather up any libraries.
  # Usage: ProcessLib <dir> <type> <output file>
  # Type is either 'runtime' or 'devel'
  if [ -d $1 ] ; then
    echo "%defattr (0555, bin, bin)" >>$3
    case "$2" in
      runtime)
       # Grab runtime libraries
       find $1 -name "*.so.*" -print | sed "s:^$RPM_BUILD_ROOT::g" >>$3
       ;;
      devel)
       find $1 -name "*.so" -print | sed "s:^$RPM_BUILD_ROOT::g" >>$3
       find $1 -name "*.la" -print | sed "s:^$RPM_BUILD_ROOT::g" >>$3
       find $1 -name "*.a" -print | sed "s:^$RPM_BUILD_ROOT::g" >>$3
       find $1 -name "*.sh" -print | sed "s:^$RPM_BUILD_ROOT::g" >>$3
       ;;
    esac
   fi
}

function ProcessDir() {
  # Build a list of files in the specified dir sticking 
  # a %defattr line as specified in front of the mess. This is intended
  # for normal dirs. Use ProcessLib for library dirs 
  # for include dirs. Appending to <output file>.
  # This will recurse.
  #
  # Usage: ProcessDir <dir> <output file> <attr>
  #
  if [ -d $1 ] ; then
   if [ ! -z "$3" ] ; then
     echo "%defattr ($3)" >>$2
   fi
   echo "*** Processing $1"
   find $1 -type f -print | sed "s:^$RPM_BUILD_ROOT::g" >>$2
  fi
}

function BuildFiles() {
  ProcessBin
  ProcessLang 
  for i in `find $RPM_BUILD_ROOT%{_prefix}/share -maxdepth 1 -type d -print | \
     sed "s:^$RPM_BUILD_ROOT%{_prefix}/share::g"` ; do
    echo $i
    case $i in
     /applets|/control-center|/gnome|/gnome-about|/mc|/panelrc|/pixmaps)

         ProcessDir $RPM_BUILD_ROOT%{_prefix}/share$i %{name}.files "0444, bin, bin, 0555"
         ;;
     *)
         ;;
   esac
  done
  ProcessDir $RPM_BUILD_ROOT/etc %{name}.files "0444, bin, bin, 0555"
  ProcessLib $RPM_BUILD_ROOT%{_prefix}/lib runtime %{name}.files
  ProcessDir $RPM_BUILD_ROOT%{_prefix}/share/idl %{name}-devel.files "0444, bin, bin, 0555"
  ProcessLib $RPM_BUILD_ROOT%{_prefix}/lib devel %{name}-devel.files
  ProcessDir $RPM_BUILD_ROOT%{_prefix}/include %{name}-devel.files "0444, bin, bin, 0555"
}

BuildFiles
 
%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f %{name}.files
%doc AUTHORS COPYING ChangeLog NEWS README

%files devel -f %{name}-devel.files
