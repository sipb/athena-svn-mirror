$! MAKE_VMS.COM
$! A DCL `Make' procedure for compiling and linking XDVI.
$!
$ ON ERROR THEN GOTO EXIT
$ ON CONTROL_Y THEN GOTO EXIT
$ OLD_VERIFY = f$verify (0)
$!
$ MAY_USE_DECC = 1
$!
$ IF F$GETSYI("HW_MODEL") .GE. 1024
$ THEN
$   ARCH_NAME="Alpha"
$   ARCH_PREF="AXP_"
$   HAVE_DECC_VAX = 0
$   USE_DECC_VAX = 0
$   CC="CC/stand=VAX/ansi/obj=.''ARCH_PREF'obj"
$   WRITE SYS$OUTPUT "Compiling on Alpha AXP using DECC"
$ ELSE
$   ARCH_NAME="VAX"
$   ARCH_PREF="VAX_"
$   IF F$TRNLNM("DECC$LIBRARY_INCLUDE") .NES. ""
$   THEN
$!    DECC for VAX available (and maybe VAXC, too!)
$     HAVE_DECC_VAX = 1
$     IF HAVE_DECC_VAX .AND. MAY_USE_DECC
$     THEN
$!      We use DECC: 
$       USE_DECC_VAX = 1
$       CC="CC/DECC/stand=VAX/obj=.''ARCH_PREF'obj"
$     ELSE
$!      We use VAXC: 
$       USE_DECC_VAX = 0
$       CC="CC/VAXC/obj=.''ARCH_PREF'obj"
$     ENDIF
$   ELSE
$!    only VAXC available
$     HAVE_DECC_VAX = 0
$     USE_DECC_VAX = 0
$     CC="CC/obj=.''ARCH_PREF'obj"
$   ENDIF
$   IF USE_DECC_VAX
$   THEN
$     WRITE SYS$OUTPUT "Compiling on VAX using DECC"
$   ELSE
$     WRITE SYS$OUTPUT "Compiling on VAX using VAXC"
$   ENDIF
$ ENDIF
$ LDFLAGS = ""
$ DEFINE X11 DECW$INCLUDE
$ IF USE_DECC_VAX .AND. (F$TRNLNM("DECC$LIBRARY_INCLUDE") .NES. "")
$  THEN
$    DEFINE SYS DECC$LIBRARY_INCLUDE
$  ELSE
$    DEFINE SYS SYS$LIBRARY
$  ENDIF
$ IF ARCH_NAME .EQS. "Alpha"
$  THEN
$    DEFINE DPS SYS$LIBRARY
$  ELSE
$    DEFINE DPS DECW$INCLUDE
$  ENDIF
$!
$ ps_dps = ""
$ ps_gs = ""
$ IF f$edit(p1,"UPCASE") .eqs. "DPS" THEN ps_dps = "PSDPS"
$ IF f$edit(p1,"UPCASE") .eqs. "GS" THEN ps_gs = "PSGS"
$!
$ ps_options = ""
$ ps_head = ""
$ varobjs = ""
$ IF ps_dps .EQS. "PSDPS"
$  THEN
$    ps_head = "PSHEADER"
$    ps_options = ps_options + ", PS_DPS"
$    varobjs = varobjs + "PSDPS.''ARCH_PREF'OBJ, "
$  ENDIF
$ IF ps_gs .EQS. "PSGS"
$  THEN
$    ps_head = "PSHEADER"
$    ps_options = ps_options + ", PS_GS"
$    varobjs = varobjs + "PSGS.''ARCH_PREF'OBJ, "
$  ENDIF
$!
$ OPEN/WRITE optfile VMS_XDVI.'ARCH_PREF'OPT
$ IF ps_dps .EQS. "PSDPS"
$ THEN
$   WRITE optfile "sys$share:xdps$dpsclientshr/share"
$   WRITE optfile "sys$share:xdps$dpsbindingsshr/share"
$   WRITE optfile "sys$share:xdps$dpslibshr/share"
$ ENDIF
$ WRITE optfile "sys$share:decw$xlibshr/share"
$ IF (ARCH_NAME .EQS. "VAX") .AND. (.NOT. USE_DECC_VAX)
$ THEN
$   WRITE optfile "sys$share:vaxcrtl/share"
$   option_file = "VMS_XDVI.''ARCH_PREF'OPT"	! for linking SQUEEZE
$ ELSE
$   option_file = "nl:"				! for linking SQUEEZE
$ ENDIF
$ CLOSE optfile
$!
$ DEFS = "VMS, NOTOOL, USE_PK, USE_GF, TEXXET, GREY, MAKEPK'ps_options'"
$ CC /DEFINE = ('DEFS') DVI_INIT.C
$ CC /DEFINE = ('DEFS') DVI_DRAW.C
$ CC /DEFINE = ('DEFS') GF.C
$ CC /DEFINE = ('DEFS') PK.C
$ CC /DEFINE = ('DEFS') VF.C
$ CC /DEFINE = ('DEFS') UTIL.C
$ CC /DEFINE = ('DEFS', "DEFAULT_FONT_PATH=""TEX_FONTS""", -
     "DEFAULT_VF_PATH=""TEX_VFDIR""", -
     "DEFAULT_FONT_SIZES=""300/329/360/432/518/622/746""") FONT_OPEN.C
$ CC /DEFINE = ('DEFS') SPECIAL.C
$ CC /DEFINE = ('DEFS') EVENTS.C
$ IF ps_dps .NES. ""
$  THEN
$    CC /DEFINE = ('DEFS') PSDPS.C
$  ENDIF
$ IF ps_gs .NES. ""
$  THEN
$    CC /DEFINE = ('DEFS') PSGS.C
$  ENDIF
$ IF ps_head .NES. ""
$  THEN
$    CC /DEFINE = ('DEFS') SQUEEZE.C
$    LINK /EXE=.'ARCH_PREF'EXE SQUEEZE.'ARCH_PREF'OBJ, 'option_file' /OPTION
$    SQUEEZE := $SYS$DISK:[]SQUEEZE.'ARCH_PREF'EXE
$    SQUEEZE PSHEADER.TXT PSHEADER.C
$    CC /DEFINE = ('DEFS') PSHEADER.C
$    varobjs = varobjs + "PSHEADER.''ARCH_PREF'OBJ,"
$  ENDIF
$ CC /DEFINE = ('DEFS') XDVI.C
$!
$ LINK 'LDFLAGS'/EXE=.'ARCH_PREF'EXE XDVI.'ARCH_PREF'OBJ, -
     DVI_INIT.'ARCH_PREF'OBJ, DVI_DRAW.'ARCH_PREF'OBJ, -
     GF.'ARCH_PREF'OBJ, PK.'ARCH_PREF'OBJ, VF.'ARCH_PREF'OBJ, -
     UTIL.'ARCH_PREF'OBJ, FONT_OPEN.'ARCH_PREF'OBJ, SPECIAL.'ARCH_PREF'OBJ, -
     EVENTS.'ARCH_PREF'OBJ, 'varobjs' VMS_XDVI.'ARCH_PREF'OPT/OPTION
$ DELETE/NOLOG VMS_XDVI.'ARCH_PREF'OPT;*
$!
$EXIT:
$ dummy=f$verify ('old_verify')
$ EXIT
