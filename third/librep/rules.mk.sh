# rules.mk.sh -- Build dynamically-loadable objects for librep
# $Id: rules.mk.sh,v 1.2 2000-12-11 22:44:01 ghudson Exp $

repdir=$1
repexecdir=$2
repdocfile=$3

cat <<EOF
# rules.mk

repdir=$repdir
repexecdir=$repexecdir
rpath_repexecdir=$repexecdir
repdocfile=$repdocfile

rep_LIBTOOL=\$\$ATHTOOLROOT\$(repexecdir)/libtool
rep_INSTALL_ALIASES=\$\$ATHTOOLROOT\$(repexecdir)/install-aliases

# use this like:
# foo.la : foo.lo bar.lo
#	\$(rep_DL_LD) link-opts...

rep_DL_LD=\$(rep_LIBTOOL) --mode=link \$(CC) -avoid-version -module \
	  -rpath \$(rpath_repexecdir)

rep_DL_INSTALL=\$(rep_LIBTOOL) --mode=install \$(INSTALL)
rep_DL_UNINSTALL=\$(rep_LIBTOOL) --mode=uninstall rm

# Rule for libtool controlled C objects
%.lo : %.c
	\$(rep_LIBTOOL) --mode=compile \$(CC) -c \$(CPPFLAGS) \$(CFLAGS) \$<

EOF
