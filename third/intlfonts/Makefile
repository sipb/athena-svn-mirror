INSTALLDIR = /usr/local/share/emacs/fonts

BDFTOPCF = bdftopcf
MKFONTDIR = mkfontdir
COMPRESS =

SUBDIRS =	European Asian Chinese Japanese Ethiopic Misc
SUBDIRS_X =	Chinese-X Japanese-X Korean-X
SUBDIRS_BIG =	European-BIG  Chinese-BIG Japanese-BIG

help:
	@echo "Usage: make [ COMPRESS=compress ] [ all | SUBDIRECTORIES ]"
	@echo
	@echo "This Makefile installs all the fonts in the current directory"
	@echo "to ${INSTALLDIR}."
	@echo
	@echo "If you want to install only some portion of fonts,"
	@echo "give some of the following subdirectories as arguments:"
	@echo "	${SUBDIRS}"
	@echo "	${SUBDIRS_X} ${SUBDIRS_BIG}"
	@echo
	@echo "If you are using a X window system which can handle compressed"
	@echo "fonts (*.pcf.Z), give \"COMPRESS=compress\" as an argument."
	@echo
	@echo "If you want to install fonts to the different directory,"
	@echo "give \"INSTALLDIR=_YOUR_INSTALL_DIRECTORY_\" as an argument."
	@echo
	@echo "Please read README for how to inform your X server about"
	@echo "the new fonts."

${SUBDIRS} ${SUBDIRS_X} ${SUBDIRS_BIG}: FRC
	@[ -d ${INSTALLDIR} ] || mkdir -p ${INSTALLDIR}
	@if [ ! -d $@ ] ; then \
	  if [ -f $@.tar.gz ] ; then \
	    echo "Extracting the archive file $@.tar.gz ..." ; \
	    (zcat $@.tar.gz | tar xf -) ; \
	    chmod a+w $@/*.bdf ; \
	  elif [ -f $@.tar ] ; then \
	    echo "Extracting the archive file $@.tar ..." ; \
	    tar xf $@.tar ; \
	    chmod a+w $@/*.bdf ; \
	  else \
	    echo "Archive file $@.tar.gz does not exist.  Ignore it." ; \
	  fi ; \
	else true ; fi
	@if [ -d $@ ] ; then \
	  pwd=`pwd` ; \
	  cd $@ ; \
	  if [ -z "${COMPRESS}" ] ; then \
	    for f in *.bdf ; do \
	      echo Making ${INSTALLDIR}/`basename $${f} .bdf`.pcf ; \
	      $(BDFTOPCF) $${f} > ${INSTALLDIR}/`basename $${f} .bdf`.pcf ; \
	    done ; \
	  else \
	    for f in *.bdf ; do \
	      echo Making ${INSTALLDIR}/`basename $${f} .bdf`.pcf.Z ; \
	      $(BDFTOPCF) $${f} > ${INSTALLDIR}/`basename $${f} .bdf`.pcf ; \
	      rm -f ${INSTALLDIR}/`basename $${f} .bdf`.pcf.Z ; \
	      $(COMPRESS) ${INSTALLDIR}/`basename $${f} .bdf`.pcf ; \
	    done ; \
	  fi ; \
	  if [ -f fonts.alias ] ; then \
	    if [ -f ${INSTALLDIR}/fonts.alias ] ; then \
	      cat ${INSTALLDIR}/fonts.alias fonts.alias |sort |uniq > temp ; \
	      cat temp > ${INSTALLDIR}/fonts.alias ; \
	      rm -f temp ; \
	    else \
	      cat fonts.alias > ${INSTALLDIR}/fonts.alias ; \
	    fi ; \
	  else true ; fi ; \
	  cd $${pwd}; \
	else true ; fi
	@if [ -d $@ -a ! -f making.all ] ; then \
	  echo "Building ${INSTALLDIR}/fonts.dir..." ; \
	  cd ${INSTALLDIR}; ${MKFONTDIR} ; \
	  echo "-----------------------------------------------------------------------" ; \
	  echo "Please read README for how to inform your X server about the new fonts." ; \
	  echo "-----------------------------------------------------------------------" ; \
	else true ; fi

all:
	@touch making.all
	make COMPRESS="${COMPRESS}" INSTALLDIR=${INSTALLDIR} \
		${SUBDIRS} ${SUBDIRS_X} ${SUBDIRS_BIG}
	@rm -f making.all
	echo "Building ${INSTALLDIR}/fonts.dir..."
	cd ${INSTALLDIR}; $(MKFONTDIR)
	@echo "-----------------------------------------------------------------------"
	@echo "Please read README for how to inform your X server about the new fonts."
	@echo "-----------------------------------------------------------------------"

clean:
	rm -fr ${SUBDIRS} ${SUBDIRS_X} ${SUBDIRS_BIG}

FRC:

## The following target should be used only by a maintainer of intlfonts.
dist: make-dist
	sh make-dist --dist

arc: make-dist
	sh make-dist --arc

split: make-dist
	sh make-dist --split

diff: make-dist
	sh make-dist --diff

pack: make-dist
	sh make-dist --pack
