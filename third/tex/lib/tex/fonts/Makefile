# $Id: Makefile,v 1.12 1999-05-17 16:40:45 ghudson Exp $

SHELL=/bin/sh
ATHLIBDIR=/usr/athena/lib
WEB2C=../../../web2c/web2c
MF=MFINPUTS=../../mf// MFBASES=${WEB2C} MFPOOL=${WEB2C} ./mf
MODE=ljfour
MFSTART=\batchmode; \mode=${MODE}; \mag=magstep

TFMS=	logo8.tfm logo9.tfm logo10.tfm logobf10.tfm logosl10.tfm manfnt.tfm \
	icmcsc10.tfm icmex10.tfm icmmi8.tfm icmsy8.tfm icmtt8.tfm ilasy8.tfm \
	ilcmss8.tfm ilcmssb8.tfm ilcmssi8.tfm lasy10.tfm lasy5.tfm lasy6.tfm \
	lasy7.tfm lasy8.tfm lasy9.tfm lasyb10.tfm lcircle10.tfm \
	lcirclew10.tfm lcmss8.tfm lcmssb8.tfm lcmssi8.tfm line10.tfm \
	linew10.tfm euex10.tfm euex7.tfm euex8.tfm euex9.tfm eufb10.tfm \
	eufb5.tfm eufb6.tfm eufb7.tfm eufb8.tfm eufb9.tfm eufm10.tfm \
	eufm5.tfm eufm6.tfm eufm7.tfm eufm8.tfm eufm9.tfm eurb10.tfm \
	eurb5.tfm eurb6.tfm eurb7.tfm eurb8.tfm eurb9.tfm eurm10.tfm \
	eurm5.tfm eurm6.tfm eurm7.tfm eurm8.tfm eurm9.tfm eusb10.tfm \
	eusb5.tfm eusb6.tfm eusb7.tfm eusb8.tfm eusb9.tfm eusm10.tfm \
	eusm5.tfm eusm6.tfm eusm7.tfm eusm8.tfm eusm9.tfm msam10.tfm \
	msam5.tfm msam6.tfm msam7.tfm msam8.tfm msam9.tfm msbm10.tfm \
	msbm5.tfm msbm6.tfm msbm7.tfm msbm8.tfm msbm9.tfm wncyb10.tfm \
	wncyb9.tfm wncyi8.tfm wncyr7.tfm wncyss8.tfm wncyb5.tfm wncyi10.tfm \
	wncyi9.tfm wncyr8.tfm wncyss9.tfm wncyb6.tfm wncyi5.tfm wncyr10.tfm \
	wncyr9.tfm wncyb7.tfm wncyi6.tfm wncyr5.tfm wncysc10.tfm wncyb8.tfm \
	wncyi7.tfm wncyr6.tfm wncyss10.tfm ocra10.tfm ocra12.tfm ocra17.tfm \
	ocrb10.tfm ocrb10e.tfm ocrb10f.tfm ocrb10g.tfm ocrb10l.tfm \
	ocrb10s.tfm ocrb10x.tfm ocrb5.tfm ocrb6.tfm ocrb7.tfm ocrb8.tfm \
	ocrb9.tfm tengwar.tfm cirth.tfm cirbf.tfm cirsl.tfm cirss.tfm \
	punk10.tfm punk12.tfm punk20.tfm punksl20.tfm punkbx20.tfm cmb10.tfm \
	cmbsy10.tfm cmbx10.tfm cmbx12.tfm cmbx5.tfm cmbx6.tfm cmbx7.tfm \
	cmbx8.tfm cmbx9.tfm cmbxsl10.tfm cmbxti10.tfm cmcsc10.tfm \
	cmdunh10.tfm cmex10.tfm cmex9.tfm cmff10.tfm cmfi10.tfm cmfib8.tfm \
	cminch.tfm cmitt10.tfm cmmi10.tfm cmmi12.tfm cmmi5.tfm cmmi6.tfm \
	cmmi7.tfm cmmi8.tfm cmmi9.tfm cmmib10.tfm cmr10.tfm cmr12.tfm \
	cmr17.tfm cmr5.tfm cmr6.tfm cmr7.tfm cmr8.tfm cmr9.tfm cmsl10.tfm \
	cmsl12.tfm cmsl8.tfm cmsl9.tfm cmsltt10.tfm cmss10.tfm cmss12.tfm \
	cmss17.tfm cmss8.tfm cmss9.tfm cmssbx10.tfm cmssdc10.tfm cmssi10.tfm \
	cmssi12.tfm cmssi17.tfm cmssi8.tfm cmssi9.tfm cmssq8.tfm cmssqi8.tfm \
	cmsy10.tfm cmsy5.tfm cmsy6.tfm cmsy7.tfm cmsy8.tfm cmsy9.tfm \
	cmtcsc10.tfm cmtex10.tfm cmtex8.tfm cmtex9.tfm cmti10.tfm cmti12.tfm \
	cmti7.tfm cmti8.tfm cmti9.tfm cmtt10.tfm cmtt12.tfm cmtt8.tfm \
	cmtt9.tfm cmu10.tfm cmvtt10.tfm cmbsy5.tfm cmbsy6.tfm cmbsy7.tfm \
	cmbsy8.tfm cmbsy9.tfm cmcsc8.tfm cmcsc9.tfm cmex7.tfm cmex8.tfm \
	cmmib5.tfm cmmib6.tfm cmmib7.tfm cmmib8.tfm cmmib9.tfm

600PK=${TFMS:.tfm=.600pk}
657PK=${TFMS:.tfm=.657pk}
720PK=${TFMS:.tfm=.720pk}
864PK=${TFMS:.tfm=.864pk}
1037PK=${TFMS:.tfm=.1037pk}
1244PK=${TFMS:.tfm=.1244pk}
1493PK=${TFMS:.tfm=.1493pk}
600GF=${TFMS:.tfm=.600gf}
657GF=${TFMS:.tfm=.657gf}
720GF=${TFMS:.tfm=.720gf}
864GF=${TFMS:.tfm=.864gf}
1037GF=${TFMS:.tfm=.1037gf}
1244GF=${TFMS:.tfm=.1244gf}
1493GF=${TFMS:.tfm=.1493gf}

PKS=${600PK} ${657PK} ${720PK} ${864PK} ${1037PK} ${1244PK} ${1493PK}

all: dummy.tfm ${PKS}

mf:
	rm -f mf
	ln -s ${WEB2C}/mf mf

dummy.tfm: mf
	${MF} '\mode=${MODE}; input dummy'

${600GF}: mf
	${MF} '${MFSTART}(0); input '`basename $@ .600gf`
${657GF}: mf
	${MF} '${MFSTART}(0.5); input '`basename $@ .657gf`
${720GF}: mf
	${MF} '${MFSTART}(1); input '`basename $@ .720gf`
${864GF}: mf
	${MF} '${MFSTART}(2); input '`basename $@ .864gf`
${1037GF}: mf
	${MF} '${MFSTART}(3); input '`basename $@ .1037gf`
${1244GF}: mf
	${MF} '${MFSTART}(4); input '`basename $@ .1244gf`
${1493GF}: mf
	${MF} '${MFSTART}(5); input '`basename $@ .1493gf`

.SUFFIXES: .600gf .600pk .657gf .657pk .720gf .720pk .864gf .864pk .1037gf \
	.1037pk .1244gf .1244pk .1493gf .1493pk
.600gf.600pk:
	${WEB2C}/gftopk ./$<
.657gf.657pk:
	${WEB2C}/gftopk ./$<
.720gf.720pk:
	${WEB2C}/gftopk ./$<
.864gf.864pk:
	${WEB2C}/gftopk ./$<
.1037gf.1037pk:
	${WEB2C}/gftopk ./$<
.1244gf.1244pk:
	${WEB2C}/gftopk ./$<
.1493gf.1493pk:
	${WEB2C}/gftopk ./$<

install:
	-mkdir -p ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	-mkdir -p ${DESTDIR}${ATHLIBDIR}/tex/fonts/tfm
	install -c -m 444 ${600PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${657PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${720PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${864PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${1037PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${1244PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${1493PK} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${TFMS} ${DESTDIR}${ATHLIBDIR}/tex/fonts/tfm
	install -c -m 444 dummy.tfm ${DESTDIR}${ATHLIBDIR}/tex/fonts/tfm

clean:
	echo cm *gf *pk *.log *.tfm | xargs rm -f
