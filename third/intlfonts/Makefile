# $Id: Makefile,v 1.2 1999-04-08 05:05:15 tb Exp $

# Font source directories
vpath = Asian Chinese Chinese-BIG Chinese-X Ethiopic European \
	European-BIG Japanese Japanese-BIG Japanese-X Korean-X Misc

# Tools
BCFTOPDF=bcftopdf
MKFONTDIR=mkfontdir

# Where to put the founts
INSTALLDIR = $(DESTDIR)/usr/athena/lib/X11/fonts/intlfonts

# alias files
aliases = Chinese/fonts.alias European/fonts.alias Japanese-X/fonts.alias \
	Japanese/fonts.alias

# fonts to build
fonts = etl16-viscii.pdf etl24-viscii.pdf mule-indian-1col-24.pdf \
	mule-indian-24.pdf mule-iscii-24.pdf mule-lao-14.pdf \
	mule-lao-16.pdf mule-lao-24.pdf mule-tibmdx-16.pdf \
	mule-tibmdx-1col-16.pdf mule-tibmdx-1col-24.pdf \
	mule-tibmdx-24.pdf thai-14.pdf thai-16.pdf thai-24.pdf \
	cns-1-16.pdf cns-1-24.pdf cns-2-16.pdf cns-2-24.pdf \
	cns-3-24.pdf cns-4-24.pdf cns-5-24.pdf cns-6-24.pdf \
	cns-7-24.pdf etl14-sisheng.pdf etl16-sisheng.pdf \
	etl24-sisheng.pdf fonts.alias guobiao16.pdf taipei16.pdf \
	taipei24.pdf cc40s.pdf cc48s.pdf cns-1-40.pdf cns-2-40.pdf \
	cns-3-40.pdf cns-4-40.pdf cns-5-40.pdf cns-6-40.pdf \
	cns-7-40.pdf gb16fs.pdf gb16st.pdf gb24st.pdf \
	ethiomx16f-uni.pdf ethiomx24f-uni.pdf a18.pdf \
	etl14-cyrillic.pdf etl14-greek.pdf etl14-koi8.pdf \
	etl14-latin1.pdf etl14-latin2.pdf etl14-latin3.pdf \
	etl14-latin4.pdf etl14-latin5.pdf etl16-cyrillic.pdf \
	etl16-greek.pdf etl16-koi8.pdf etl16-latin1.pdf \
	etl16-latin2.pdf etl16-latin3.pdf etl16-latin4.pdf \
	etl16-latin5.pdf etl16b-latin1.pdf etl16bi-latin1.pdf \
	etl16i-latin1.pdf etl24-cyrillic.pdf etl24-greek.pdf \
	etl24-koi8.pdf etl24-latin1.pdf etl24-latin2.pdf \
	etl24-latin3.pdf etl24-latin4.pdf etl24-latin5.pdf \
	etl40-latin1.pdf a18rk.pdf jiskan16-1978.pdf jisksp16.pdf \
	k16-1990.pdf kanji18.pdf jisksp40.pdf kanji32.pdf kanji48.pdf \
	12x24rk.pdf 8x16rk.pdf jiskan16.pdf jiskan24.pdf k14.pdf \
	hanglg16.pdf hanglm16.pdf hanglm24.pdf etl14-hebrew.pdf \
	etl14-ipa.pdf etl16-arabic0.pdf etl16-arabic1.pdf \
	etl16-arabic2.pdf etl16-hebrew.pdf etl16-ipa.pdf \
	etl24-arabic0.pdf etl24-arabic1.pdf etl24-arabic2.pdf \
	etl24-hebrew.pdf etl24-ipa.pdf

all: $(fonts) fonts.alias

clean:
	rm -f *.pdf fonts.alias

install: all
	-mkdir -p $(INSTALLDIR)
	cp $(fonts) $(INSTALLDIR)
	touch $(INSTALLDIR)/fonts.alias
	cat fonts.alias $(INSTALLDIR)/fonts.alias | sort | uniq > temp
	mv temp $(INSTALLDIR)/fonts.alias
	(cd $(INSTALLDIR); $(MKFONTDIR))

fonts.alias: $(aliases)
	cat $(aliases) | sort | uniq > fonts.alias

%.bcf: %.pdf
	$(BCFTOPDF) $< > $@
