CC	= gcc
LIBDRS	=	-L../libntp
LIBS	=	-lntp
INCLUDE	= -I.. -I../include -I/usr/include
CFLAGS	= -DHAVE_CONFIG_H -c
LINKFLAGS = -o
TARGET  =  convert 
OBJECTS  = convert.o
#
#  Targets to build
all:	convert

#
#  Objects to build
convert.o:  convert.c
	$(CC) $(INCLUDE) $(CFLAGS)  convert.c


#
#  Target link line
convert:	convert.o 
	$(CC) $(LINKFLAGS) convert convert.o $(LIBDRS) $(LIBS) 

#######
.SUFFIXES:       .c .o

clean:
		rm  -f $(TARGET) $(OBJECTS) 
#
clobber:	clean
		rm -f $(TARGET)
