# Rewrite default rules from .c.o:  to  .c.o: .c
/^\./s/^\(\.[a-z]*\)\(\.[a-z]*\)\( *: *\)$/\1\2\3 \1/

# Change dependency char.
/::/s/::/ \\Option-f\\Option-f /g
/:/s/:/ \\Option-f /g

# Change syntax of Makefile vars.
/\$/s/\${\([a-zA-Z0-9_]*\)}/{\1}/g
/\$/s/\$(\([a-zA-Z0-9_]*\))/{\1}/g

# Change $@ to {targ}
/\$@/s/\$@/{targ}/g

# Change pathname syntax.
/\./s,\.\./\.\./,:::,g
# If line ends with ..  then assume it sets a variable that will
# be used to prefix something else -- eliminate one colon, assuming
# that a slash after the ${name} will turn into the missing colon.
# Mac pathname conventions are IRREGULAR and UGLY!
/\./s,\.\./\.\.$,::,g
/\./s,\.\.$,:,g
/\./s,\.\./,::,g
/\.\//s,\./,:,g
/\//s,/,:,g

/=/s/ = \.$/ = :/

# Comment out any explicit srcdir setting.
# /srcdir/s/^srcdir/# srcdir/

/version/s/^version=/# version=/

/BASEDIR/s/^BASEDIR =.*$/BASEDIR = "{srcroot}"/
/{BASEDIR}:/s/{BASEDIR}:/{BASEDIR}/g
/{srcdir}:/s/{srcdir}:/"{srcdir}"/g
/"{srcdir}":/s/"{srcdir}":/"{srcdir}"/g

# Comment out settings of anything set by mpw host config.
##/CC/s/^CC *=/#CC =/
##/CFLAGS/s/^CFLAGS *=/#CFLAGS =/
##/LDFLAGS/s/^LDFLAGS *=/#LDFLAGS =/

# Change -I usage.
/-I/s/-I\./-i :/g
/-I/s/-I::bfd/-i ::bfd:/g
/-I/s/-I::include/-i ::include:/g
/-I/s/-I/-i /g

# Change -D usage.
/-D/s/\([ =]\)-D\([^ ]*\)/\1-d \2/g

# Change continuation char.
/\\$/s/\\$/\\Option-d/

# Change wildcard char.
/^[^#]/s/\*/\\Option-x/g

# Change path of various types of source files.
/\.[chly]/s/\([ 	><=]\)\([-a-zA-Z0-9_$:"]*\)\.\([chly]\)/\1"{s}"\2.\3/g
/\.[chly]/s/^\([-a-zA-Z0-9_${}:"]*\)\.\([chly]\)/"{s}"\1.\2/g
# Fix some overenthusiasms.
/{s}/s/"{s}""{srcdir}"/"{srcdir}"/g
/{s}/s/"{s}"{\([a-zA-Z0-9_]*\)dir}/"{\1dir}"/g
/{s}/s/"{s}"{\([a-zA-Z0-9_]*\)DIR}/"{\1DIR}"/g
/{s}/s/"{s}""{\([a-zA-Z0-9_]*\)dir}"/"{\1dir}"/g
/{s}/s/"{s}""{\([a-zA-Z0-9_]*\)DIR}"/"{\1DIR}"/g
/{s}/s/"{s}":/:/g
/{s}/s/^"{s}"//g
/^\./s/"{s}"\././g

# Change extension and path of objects, except in the BSDLIB line.
/^BSDLIB/!s/\([ 	=]\)\([-a-zA-Z0-9_${}:"]*\)\.o/\1"{o}"\2.c.o/g
/\.o/s/^\([-a-zA-Z0-9_${}:"]*\)\.o/"{o}"\1.c.o/g
# Clean up.
/\.o/s/"{o}""{o}"/"{o}"/g
/{o}/s/^"{o}"\([a-zA-Z0-9_]*\)=/\1=/g

# Change extension of libs.
# /\.a/s/lib\([a-z]*\)\.a/lib\1.o/g

# Remove non-echo option.
/^	-/s/^	-/	/

# Change cp to duplicate.
# /cp/s/^\([ 	]*\)cp /\1Duplicate -d -y /
# Change mv to rename.
# /mv/s/^\([ 	]*\)mv /\1Rename -y /
# /Rename/s/^\([ 	]*\)Rename -y -f/\1Rename -y/
# Change rm to delete.
/^RM=/s/rm -f/Delete -i -y/
# /rm/s/^\([ 	]*\)rm /\1Delete -y /
# /Delete/s/^\([ 	]*\)Delete -y -f/\1Delete -y/
# Comment out symlinking.
# /ln/s/^\([ 	]*\)ln /\1# ln /

# Remove -c from explicit compiler calls.
# /-c/s/{CC}\(.*\) -c \(.*\)\([-a-z]*\)\.c/{CC}\1 \2\3.c -o "{o}"\3.c.o/g
# Don't ask... prev subst seems to omit the second filename.
# /-o/s/\([-a-z]*\)\.c -o "{o}".c.o/\1\.c -o "{o}"\1.c.o/

# Change linking cc to link.
/LDFLAGS/    s/{CC} \(.*\){CFLAGS}\(.*\){LDFLAGS}/Link \1 \2 {LDFLAGS}/
/CFLAGS_LINK/s/{CC} \(.*\){CFLAGS_LINK}\(.*\){LDFLAGS}/Link \1 \2 {LDFLAGS}/

# Comment out .PHONY rules.
/\.PHONY/s/^\.PHONY/# \.PHONY/
# Comment out .SUFFIXES rules.
/\.SUFFIXES/s/^\.SUFFIXES/# \.SUFFIXES/
# Comment out .PRECIOUS rules.
/\.PRECIOUS/s/^\.PRECIOUS/# \.PRECIOUS/
## Comment out default rules.
##/^\./s/^\(\.[a-z]*\.[a-z]* \)/# \1/

#
#  End of original hack-mf.sed
#
#  Begin original hack-mf2.sed
#
# Transform expressions.

# Set the install program appropriate.
# /INSTALL/s/^INSTALL *= *`.*`:install.sh -c/INSTALL = Duplicate -y/

# Include from the extra-include dir.
# /^INCLUDES = /s/^INCLUDES = /INCLUDES = -i "{srcroot}"extra-include /

# Yuck - remove unconverted autoconf things.
# /@/s/@[^ 	]*@//g

# Note!  There are 8-bit characters in the three lines below:
#   	0xc4, 0xb6, 0xc5.
/Option/s/\\Option-f/�/g
/Option/s/\\Option-d/�/g
/Option/s/\\Option-x/�/g