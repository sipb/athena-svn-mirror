# $Id: Makefile,v 1.1 1996-10-25 22:53:13 ghudson Exp $

SHELL=/bin/sh
ATHLIBDIR=/usr/athena/lib
WEB2C=../../../web2c/web2c
MFVARS=MFINPUTS=../../mf// MFBASES=${WEB2C}/mf MFPOOL=${WEB2C}/mf
MODE=ljfour

NORMALTFMS=	logo8.tfm logo9.tfm logo10.tfm \
		logobf10.tfm logosl10.tfm manfnt.tfm icmcsc10.tfm icmex10.tfm \
		icmmi8.tfm icmsy8.tfm icmtt8.tfm ilasy8.tfm ilcmss8.tfm \
		ilcmssb8.tfm ilcmssi8.tfm lasy10.tfm lasy5.tfm lasy6.tfm \
		lasy7.tfm lasy8.tfm lasy9.tfm lasyb10.tfm lcircle10.tfm \
		lcirclew10.tfm lcmss8.tfm lcmssb8.tfm lcmssi8.tfm line10.tfm \
		linew10.tfm euex10.tfm euex7.tfm euex8.tfm euex9.tfm \
		eufb10.tfm eufb5.tfm eufb6.tfm eufb7.tfm eufb8.tfm eufb9.tfm \
		eufm10.tfm eufm5.tfm eufm6.tfm eufm7.tfm eufm8.tfm eufm9.tfm \
		eurb10.tfm eurb5.tfm eurb6.tfm eurb7.tfm eurb8.tfm eurb9.tfm \
		eurm10.tfm eurm5.tfm eurm6.tfm eurm7.tfm eurm8.tfm eurm9.tfm \
		eusb10.tfm eusb5.tfm eusb6.tfm eusb7.tfm eusb8.tfm eusb9.tfm \
		eusm10.tfm eusm5.tfm eusm6.tfm eusm7.tfm eusm8.tfm eusm9.tfm \
		msam10.tfm msam5.tfm msam6.tfm msam7.tfm msam8.tfm msam9.tfm \
		msbm10.tfm msbm5.tfm msbm6.tfm msbm7.tfm msbm8.tfm msbm9.tfm \
		wncyb10.tfm wncyb9.tfm wncyi8.tfm wncyr7.tfm wncyss8.tfm \
		wncyb5.tfm wncyi10.tfm wncyi9.tfm wncyr8.tfm wncyss9.tfm \
		wncyb6.tfm wncyi5.tfm wncyr10.tfm wncyr9.tfm wncyb7.tfm \
		wncyi6.tfm wncyr5.tfm wncysc10.tfm wncyb8.tfm wncyi7.tfm \
		wncyr6.tfm wncyss10.tfm ocra10.tfm ocra12.tfm ocra17.tfm \
		ocrb10.tfm ocrb10e.tfm ocrb10f.tfm ocrb10g.tfm ocrb10l.tfm \
		ocrb10s.tfm ocrb10x.tfm ocrb5.tfm ocrb6.tfm ocrb7.tfm \
		ocrb8.tfm ocrb9.tfm tengwar.tfm cirth.tfm cirbf.tfm \
		cirsl.tfm cirss.tfm punk10.tfm punk12.tfm punk20.tfm \
		punksl20.tfm punkbx20.tfm

CMTFMS=		cmb10.tfm cmbsy10.tfm cmbx10.tfm cmbx12.tfm \
		cmbx5.tfm cmbx6.tfm cmbx7.tfm cmbx8.tfm cmbx9.tfm \
		cmbxsl10.tfm cmbxti10.tfm cmcsc10.tfm cmdunh10.tfm cmex10.tfm \
		cmex9.tfm cmff10.tfm cmfi10.tfm cmfib8.tfm cminch.tfm \
		cmitt10.tfm cmmi10.tfm cmmi12.tfm cmmi5.tfm cmmi6.tfm \
		cmmi7.tfm cmmi8.tfm cmmi9.tfm cmmib10.tfm cmr10.tfm cmr12.tfm \
		cmr17.tfm cmr5.tfm cmr6.tfm cmr7.tfm cmr8.tfm cmr9.tfm \
		cmsl10.tfm cmsl12.tfm cmsl8.tfm cmsl9.tfm cmsltt10.tfm \
		cmss10.tfm cmss12.tfm cmss17.tfm cmss8.tfm cmss9.tfm \
		cmssbx10.tfm cmssdc10.tfm cmssi10.tfm cmssi12.tfm cmssi17.tfm \
		cmssi8.tfm cmssi9.tfm cmssq8.tfm cmssqi8.tfm cmsy10.tfm \
		cmsy5.tfm cmsy6.tfm cmsy7.tfm cmsy8.tfm cmsy9.tfm \
		cmtcsc10.tfm cmtex10.tfm cmtex8.tfm cmtex9.tfm cmti10.tfm \
		cmti12.tfm cmti7.tfm cmti8.tfm cmti9.tfm cmtt10.tfm \
		cmtt12.tfm cmtt8.tfm cmtt9.tfm cmu10.tfm cmvtt10.tfm \
		cmbsy5.tfm cmbsy6.tfm cmbsy7.tfm cmbsy8.tfm cmbsy9.tfm \
		cmcsc8.tfm cmcsc9.tfm cmex7.tfm cmex8.tfm cmex9.tfm \
		cmmib5.tfm cmmib6.tfm cmmib7.tfm cmmib8.tfm cmmib9.tfm

NORMAL600PK=${NORMALTFMS:.tfm=.600pk}
NORMAL657PK=${NORMALTFMS:.tfm=.657pk}
NORMAL720PK=${NORMALTFMS:.tfm=.720pk}
NORMAL864PK=${NORMALTFMS:.tfm=.864pk}
NORMAL1037PK=${NORMALTFMS:.tfm=.1037pk}
NORMAL1244PK=${NORMALTFMS:.tfm=.1244pk}
NORMAL1493PK=${NORMALTFMS:.tfm=.1493pk}
NORMAL600GF=${NORMALTFMS:.tfm=.600gf}
NORMAL657GF=${NORMALTFMS:.tfm=.657gf}
NORMAL720GF=${NORMALTFMS:.tfm=.720gf}
NORMAL864GF=${NORMALTFMS:.tfm=.864gf}
NORMAL1037GF=${NORMALTFMS:.tfm=.1037gf}
NORMAL1244GF=${NORMALTFMS:.tfm=.1244gf}
NORMAL1493GF=${NORMALTFMS:.tfm=.1493gf}
CM600PK=${CMTFMS:.tfm=.600pk}
CM657PK=${CMTFMS:.tfm=.657pk}
CM720PK=${CMTFMS:.tfm=.720pk}
CM864PK=${CMTFMS:.tfm=.864pk}
CM1037PK=${CMTFMS:.tfm=.1037pk}
CM1244PK=${CMTFMS:.tfm=.1244pk}
CM1493PK=${CMTFMS:.tfm=.1493pk}
CM600GF=${CMTFMS:.tfm=.600gf}
CM657GF=${CMTFMS:.tfm=.657gf}
CM720GF=${CMTFMS:.tfm=.720gf}
CM864GF=${CMTFMS:.tfm=.864gf}
CM1037GF=${CMTFMS:.tfm=.1037gf}
CM1244GF=${CMTFMS:.tfm=.1244gf}
CM1493GF=${CMTFMS:.tfm=.1493gf}

TFMS=	${NORMALTFMS} ${CMTFMS}
PKS=	${NORMAL600PK} ${NORMAL657PK} ${NORMAL720PK} ${NORMAL864PK} \
	${NORMAL1037PK} ${NORMAL1244PK} ${NORMAL1493PK} ${CM600PK} \
	${CM657PK} ${CM720PK} ${CM864PK} ${CM1037PK} ${CM1244PK} ${CM1493PK}

all: mf cmmf ${PKS}

mf:
	rm -f mf
	ln -s ${WEB2C}/mf/virmf mf

cmmf:
	rm -f cmmf
	ln -s ${WEB2C}/mf/virmf cmmf

${NORMAL600GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(0); input '`basename $@ .600gf`
${NORMAL657GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(0.5); input \
		'`basename $@ .657gf`
${NORMAL720GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(1); input \
		'`basename $@ .720gf`
${NORMAL864GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(2); input \
		'`basename $@ .864gf`
${NORMAL1037GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(3); input \
		'`basename $@ .1037gf`
${NORMAL1244GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(4); input \
		'`basename $@ .1244gf`
${NORMAL1493GF}:
	${MFVARS} ./mf '\mode=${MODE}; mag=magstep(5); input \
		'`basename $@ .1493gf`
${CM600GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(0); input \
		'`basename $@ .600gf`
${CM657GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(0.5); input \
		'`basename $@ .657gf`
${CM720GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(1); input \
		'`basename $@ .720gf`
${CM864GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(2); input \
		'`basename $@ .864gf`
${CM1037GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(3); input \
		'`basename $@ .1037gf`
${CM1244GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(4); input \
		'`basename $@ .1244gf`
${CM1493GF}:
	${MFVARS} ./cmmf '\mode=${MODE}; mag=magstep(5); input \
		'`basename $@ .1493gf`

.SUFFIXES: .600gf .600pk .657gf .657pk .720gf .720pk .864gf .864pk .1037gf \
	.1037pk .1244gf .1244pk .1493gf .1493pk
.600gf.600pk:
	${WEB2C}/fontutil/gftopk ./$<
.657gf.657pk:
	${WEB2C}/fontutil/gftopk ./$<
.720gf.720pk:
	${WEB2C}/fontutil/gftopk ./$<
.864gf.864pk:
	${WEB2C}/fontutil/gftopk ./$<
.1037gf.1037pk:
	${WEB2C}/fontutil/gftopk ./$<
.1244gf.1244pk:
	${WEB2C}/fontutil/gftopk ./$<
.1493gf.1493pk:
	${WEB2C}/fontutil/gftopk ./$<

install:
	-mkdir -p ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	-mkdir -p ${DESTDIR}${ATHLIBDIR}/tex/fonts/tfm
	install -c -m 444 ${PKS} ${DESTDIR}${ATHLIBDIR}/tex/fonts/pk
	install -c -m 444 ${TFMS} ${DESTDIR}${ATHLIBDIR}/tex/fonts/tfm

clean:
	rm -f cm cmmf *gf *pk *.log *.tfm
