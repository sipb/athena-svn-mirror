#
# Configuration file for OAF
#
prefix=/usr
exec_prefix=${prefix}

orbit_includes="`/gnome/bin/orbit-config --cflags server`"
orbit_libs_output="`/gnome/bin/orbit-config --use-service=name --libs server`"

orbit_libdirs=
orbit_libs=
for i in $orbit_libs_output; do
	case "$i" in
	-L*|-R*) orbit_libdirs="$orbit_libdirs $i" ;;
	*) orbit_libs="$orbit_libs $i" ;;
	esac
done

OAF_LIBDIR="-L${exec_prefix}/lib $orbit_libdirs"
OAF_LIBS="-loaf $orbit_libs"
OAF_INCLUDEDIR="-I${prefix}/include $orbit_includes"
MODULE_VERSION="oaf-0.6.5"

