# $Id: Makefile,v 1.5 1999-09-29 20:40:05 danw Exp $

# Font source directories
VPATH = Asian Chinese Chinese-BIG Chinese-X Ethiopic European \
	European-BIG Japanese Japanese-BIG Japanese-X Korean-X Misc

# Tools
BDFTOPCF=bdftopcf
MKFONTDIR=mkfontdir

# Where to put the founts
INSTALLDIR = $(DESTDIR)/usr/athena/lib/X11/fonts/intlfonts

# alias files
aliases = Chinese/fonts.alias European/fonts.alias Japanese-X/fonts.alias \
	Japanese/fonts.alias

# fonts to build
fonts = etl16-viscii.pcf etl24-viscii.pcf mule-indian-1col-24.pcf \
	mule-indian-24.pcf mule-iscii-24.pcf mule-lao-14.pcf \
	mule-lao-16.pcf mule-lao-24.pcf mule-tibmdx-16.pcf \
	mule-tibmdx-1col-16.pcf mule-tibmdx-1col-24.pcf \
	mule-tibmdx-24.pcf thai-14.pcf thai-16.pcf thai-24.pcf \
	cns-1-16.pcf cns-1-24.pcf cns-2-16.pcf cns-2-24.pcf \
	cns-3-24.pcf cns-4-24.pcf cns-5-24.pcf cns-6-24.pcf \
	cns-7-24.pcf etl14-sisheng.pcf etl16-sisheng.pcf \
	etl24-sisheng.pcf fonts.alias guobiao16.pcf taipei16.pcf \
	taipei24.pcf cc40s.pcf cc48s.pcf cns-1-40.pcf cns-2-40.pcf \
	cns-3-40.pcf cns-4-40.pcf cns-5-40.pcf cns-6-40.pcf \
	cns-7-40.pcf gb16fs.pcf gb16st.pcf gb24st.pcf \
	ethiomx16f-uni.pcf ethiomx24f-uni.pcf a18.pcf \
	etl14-cyrillic.pcf etl14-greek.pcf etl14-koi8.pcf \
	etl14-latin1.pcf etl14-latin2.pcf etl14-latin3.pcf \
	etl14-latin4.pcf etl14-latin5.pcf etl16-cyrillic.pcf \
	etl16-greek.pcf etl16-koi8.pcf etl16-latin1.pcf \
	etl16-latin2.pcf etl16-latin3.pcf etl16-latin4.pcf \
	etl16-latin5.pcf etl16b-latin1.pcf etl16bi-latin1.pcf \
	etl16i-latin1.pcf etl24-cyrillic.pcf etl24-greek.pcf \
	etl24-koi8.pcf etl24-latin1.pcf etl24-latin2.pcf \
	etl24-latin3.pcf etl24-latin4.pcf etl24-latin5.pcf \
	etl40-latin1.pcf a18rk.pcf jiskan16-1978.pcf jisksp16.pcf \
	k16-1990.pcf kanji18.pcf jisksp40.pcf kanji32.pcf kanji48.pcf \
	12x24rk.pcf 8x16rk.pcf jiskan16.pcf jiskan24.pcf k14.pcf \
	hanglg16.pcf hanglm16.pcf hanglm24.pcf etl14-hebrew.pcf \
	etl14-ipa.pcf etl16-arabic0.pcf etl16-arabic1.pcf \
	etl16-arabic2.pcf etl16-hebrew.pcf etl16-ipa.pcf \
	etl24-arabic0.pcf etl24-arabic1.pcf etl24-arabic2.pcf \
	etl24-hebrew.pcf etl24-ipa.pcf

all: $(fonts) fonts.alias

clean:
	rm -f *.pcf fonts.alias

check:

install: all
	mkdir -p $(INSTALLDIR)
	cp $(fonts) $(INSTALLDIR)
	touch $(INSTALLDIR)/fonts.alias
	cat fonts.alias $(INSTALLDIR)/fonts.alias | sort | uniq > temp
	mv temp $(INSTALLDIR)/fonts.alias
	(cd $(INSTALLDIR); $(MKFONTDIR))

fonts.alias: $(aliases)
	cat $(aliases) | sort | uniq > fonts.alias

%.pcf: %.bdf
	rm -f $@
	$(BDFTOPCF) $< > $@
