# $Id: Makefile,v 1.6 2001-04-08 14:55:21 ghudson Exp $

# Tools
BDFTOPCF=bdftopcf
MKFONTDIR=mkfontdir

# Where to put the founts
INSTALLDIR = $(DESTDIR)/usr/athena/lib/X11/fonts/intlfonts

# alias files
aliases = Chinese/fonts.alias European/fonts.alias Japanese-X/fonts.alias \
	Japanese/fonts.alias

# fonts to build
fonts = Asian/etl16-viscii.pcf \
	Asian/etl24-viscii.pcf \
	Asian/mule-indian-1col-24.pcf \
	Asian/mule-indian-24.pcf \
	Asian/mule-iscii-24.pcf \
	Asian/mule-lao-14.pcf \
	Asian/mule-lao-16.pcf \
	Asian/mule-lao-24.pcf \
	Asian/mule-tibmdx-16.pcf \
	Asian/mule-tibmdx-1col-16.pcf \
	Asian/mule-tibmdx-1col-24.pcf \
	Asian/mule-tibmdx-24.pcf \
	Asian/thai-14.pcf \
	Asian/thai-16.pcf \
	Asian/thai-24.pcf \
	Chinese-BIG/cc40s.pcf \
	Chinese-BIG/cc48s.pcf \
	Chinese-BIG/cns-1-40.pcf \
	Chinese-BIG/cns-2-40.pcf \
	Chinese-BIG/cns-3-40.pcf \
	Chinese-BIG/cns-4-40.pcf \
	Chinese-BIG/cns-5-40.pcf \
	Chinese-BIG/cns-6-40.pcf \
	Chinese-BIG/cns-7-40.pcf \
	Chinese-X/gb16fs.pcf \
	Chinese-X/gb16st.pcf \
	Chinese-X/gb24st.pcf \
	Chinese/cns-1-16.pcf \
	Chinese/cns-1-24.pcf \
	Chinese/cns-2-16.pcf \
	Chinese/cns-2-24.pcf \
	Chinese/cns-3-24.pcf \
	Chinese/cns-4-24.pcf \
	Chinese/cns-5-24.pcf \
	Chinese/cns-6-24.pcf \
	Chinese/cns-7-24.pcf \
	Chinese/etl14-sisheng.pcf \
	Chinese/etl16-sisheng.pcf \
	Chinese/etl24-sisheng.pcf \
	Chinese/guobiao16.pcf \
	Chinese/taipei16.pcf \
	Chinese/taipei24.pcf \
	Ethiopic/ethiomx16f-uni.pcf \
	Ethiopic/ethiomx24f-uni.pcf \
	European-BIG/etl40-latin1.pcf \
	European/a18.pcf \
	European/etl14-cyrillic.pcf \
	European/etl14-greek.pcf \
	European/etl14-koi8.pcf \
	European/etl14-latin1.pcf \
	European/etl14-latin2.pcf \
	European/etl14-latin3.pcf \
	European/etl14-latin4.pcf \
	European/etl14-latin5.pcf \
	European/etl16-cyrillic.pcf \
	European/etl16-greek.pcf \
	European/etl16-koi8.pcf \
	European/etl16-latin1.pcf \
	European/etl16-latin2.pcf \
	European/etl16-latin3.pcf \
	European/etl16-latin4.pcf \
	European/etl16-latin5.pcf \
	European/etl16b-latin1.pcf \
	European/etl16bi-latin1.pcf \
	European/etl16i-latin1.pcf \
	European/etl24-cyrillic.pcf \
	European/etl24-greek.pcf \
	European/etl24-koi8.pcf \
	European/etl24-latin1.pcf \
	European/etl24-latin2.pcf \
	European/etl24-latin3.pcf \
	European/etl24-latin4.pcf \
	European/etl24-latin5.pcf \
	Japanese-BIG/jisksp40.pcf \
	Japanese-BIG/kanji32.pcf \
	Japanese-BIG/kanji48.pcf \
	Japanese-X/12x24rk.pcf \
	Japanese-X/8x16rk.pcf \
	Japanese-X/jiskan16.pcf \
	Japanese-X/jiskan24.pcf \
	Japanese-X/k14.pcf \
	Japanese/a18rk.pcf \
	Japanese/jiskan16-1978.pcf \
	Japanese/jisksp16.pcf \
	Japanese/k16-1990.pcf \
	Japanese/kanji18.pcf \
	Korean-X/hanglg16.pcf \
	Korean-X/hanglm16.pcf \
	Korean-X/hanglm24.pcf \
	Misc/etl14-hebrew.pcf \
	Misc/etl14-ipa.pcf \
	Misc/etl16-arabic0.pcf \
	Misc/etl16-arabic1.pcf \
	Misc/etl16-arabic2.pcf \
	Misc/etl16-hebrew.pcf \
	Misc/etl16-ipa.pcf \
	Misc/etl24-arabic0.pcf \
	Misc/etl24-arabic1.pcf \
	Misc/etl24-arabic2.pcf \
	Misc/etl24-hebrew.pcf \
	Misc/etl24-ipa.pcf

all: $(fonts) fonts.alias

clean:
	rm -f $(fonts) fonts.alias

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
