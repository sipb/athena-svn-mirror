# Makefile fragment for Omega and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of Omega.

Makefile: $(srcdir)/omegadir/omega.mk

# The C sources.
omega_c = omegaini.c omega0.c omega1.c omega2.c omega3.c
omegaware_c = odvicopy.c odvitype.c ofm2opl.c opl2ofm.c \
              ovf2ovp.c ovp2ovf.c otangle.c
omega_o = omegaini.o omega0.o omega1.o omega2.o omega3.o omegaextra.o omega.o

# Generation of the web and ch files.
odvicopy.web: omegaware/odvicopy.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/odvicopy.web $@
odvicopy.ch: omegaware/odvicopy.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/odvicopy.ch $@
odvitype.web: omegaware/odvitype.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/odvitype.web $@
odvitype.ch: omegaware/odvitype.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/odvitype.ch $@
ofm2opl.web: omegaware/ofm2opl.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/ofm2opl.web $@
ofm2opl.ch: omegaware/ofm2opl.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/ofm2opl.ch $@
omega.web: tie tex.web omegadir/om16bit.ch omegadir/omchar.ch
omega.web: omegadir/omfi.ch omegadir/ompar.ch omegadir/omocp.ch
omega.web: omegadir/omfilter.ch omegadir/omtrans.ch omegadir/omdir.ch
	./tie -m omega.web $(srcdir)/tex.web $(srcdir)/omegadir/om16bit.ch \
	 $(srcdir)/omegadir/omchar.ch $(srcdir)/omegadir/omfi.ch \
	 $(srcdir)/omegadir/ompar.ch $(srcdir)/omegadir/omocp.ch \
	 $(srcdir)/omegadir/omfilter.ch $(srcdir)/omegadir/omtrans.ch \
	 $(srcdir)/omegadir/omdir.ch
omega.ch: tie omega.web omegadir/com16bit.ch omegadir/comchar.ch
omega.ch: omegadir/comfi.ch omegadir/compar.ch omegadir/comocp.ch
omega.ch: omegadir/comfilter.ch omegadir/comtrans.ch omegadir/comdir.ch
	./tie -c omega.ch omega.web \
	 $(srcdir)/omegadir/com16bit.ch $(srcdir)/omegadir/comchar.ch \
	 $(srcdir)/omegadir/comfi.ch $(srcdir)/omegadir/compar.ch \
	 $(srcdir)/omegadir/comocp.ch $(srcdir)/omegadir/comfilter.ch \
	 $(srcdir)/omegadir/comtrans.ch $(srcdir)/omegadir/comdir.ch
opl2ofm.web: omegaware/opl2ofm.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/opl2ofm.web $@
opl2ofm.ch: omegaware/opl2ofm.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/opl2ofm.ch $@
otangle.web: omegaware/otangle.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/otangle.web $@
otangle.ch: omegaware/otangle.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/otangle.ch $@
ovf2ovp.web: omegaware/ovf2ovp.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/ovf2ovp.web $@
ovf2ovp.ch: omegaware/ovf2ovp.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/ovf2ovp.ch $@
ovp2ovf.web: omegaware/ovp2ovf.web
	rm -f $@
	$(LN) $(srcdir)/omegaware/ovp2ovf.web $@
ovp2ovf.ch: omegaware/ovp2ovf.ch
	rm -f $@
	$(LN) $(srcdir)/omegaware/ovp2ovf.ch $@

# Bootstrapping otangle requires making it with itself.
otangle: otangle.o
	$(kpathsea_link) otangle.o $(LOADLIBES)
	$(MAKE) $(common_makeargs) otangleboot.p
# otangle.p is a special case, since it is needed to compile itself.  We
# convert and compile the (distributed) otangleboot.p to make a otangle
# which we use to make the other programs.
otangle.p: otangleboot otangle.web otangle.ch
	$(shared_env) ./otangleboot otangle.web otangle.ch

otangleboot: otangleboot.o
	$(kpathsea_link) otangleboot.o $(LOADLIBES)
otangleboot.c otangleboot.h: stamp-otangle $(web2c_programs) $(web2c_aux)
	$(web2c) otangleboot
# omegaware/otangleboot.p is in the distribution.
stamp-otangle: omegaware/otangleboot.p
	rm -f otangleboot.p
	$(LN) $(srcdir)/omegaware/otangleboot.p otangleboot.p
	date >stamp-otangle
# This is not run unless otangle.web or otangle.ch is changed.
otangleboot.p: omegaware/otangle.web omegaware/otangle.ch
	$(shared_env) ./otangle otangle.web otangle.ch
	test -d omegaware || mkdir omegaware
	mv otangle.p omegaware/otangleboot.p
	rm -f otangleboot.p
	$(LN) omegaware/otangleboot.p otangleboot.p
	date >stamp-otangle
	$(MAKE) $(common_makeargs) otangle

# Two additional files
omega.c: omegadir/omega.c 
	rm -f $@
	$(LN) $(srcdir)/omegadir/omega.c $@
omegamem.h: omegadir/omegamem.h
	rm -f $@
	$(LN) $(srcdir)/omegadir/omegamem.h $@

# Some additional programs for Omega: the programs themselves are named
# in the variable otps_programs, defined above.
otps/otp2ocp: otps/otp.h otps/otp.l otps/otp.y otps/routines.c \
              otps/routines.h otps/yystype.h otps/otp2ocp.c
	cd otps && $(MAKE) $(common_makeargs) otp2ocp
otps/outocp: otps/otp.h otps/outocp.c
	cd otps && $(MAKE) $(common_makeargs) outocp

# end of omega.mk
