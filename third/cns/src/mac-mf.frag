#
# MPW-style lines for the MakeFile.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.
#
# This first part is long enough that NFS/Share doesn't notice the non-ASCII
# characters in the rest of the file, so it claims that the file is type
# TEXT, which is what we want.  The non-ASCII chars are necessary for MPW 
# Make.

s = "{srcdir}"

o = :

# File in object dir can come from either the current dir or srcdir.
 
"{o}" Ä : "{s}"

# Default rule that puts each file into separate segment.

.c.o Ä .c
   {CC}  {DepDir}{Default}.c {CFLAGS} -s {Default} -o {TargDir}{Default}.c.o

#
# End of MPW-style lines for MakeFile.
#
