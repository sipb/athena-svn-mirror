$ ! VMSWCL.COM
$ ! SCCS_data: @(#) VMSWcl.com 1.4 93/08/30 11:44:27
$ !
$ ! Hacked to work with the Motif developer's kit; no attempt made
$ ! to make it compatible with VMSWcl.com.  Based on talks with DEC,
$ ! though, I see no reason why this shouldn't work with later versions
$ ! of DECWindows/Motif - Peter Scott
$ !
$ !     ----------  THIS MAY NOT WORK AS PROVIDED! ----------
$ !
$ ! This is a crude "makefile" for Wcl on VMS (5.3 plus DECWINDOWS MOTIF 1.0)
$ !
$ ! You should specify the value of the WCL_ERRORDB flag passed to
$ ! Wc/WcCreate.c which under UNIX is usually $(LIBDIR)/WclErrorDB.
$ !
$ ! You'll have to copy (since you can't link) *.H into subdirectories
$ ! under [.X11] - same amount of effort as linking, but you'll have to make
$ ! sure they stay in synch if you modify them.  If you copied the
$ ! distribution to VMS from a system that already had the links in place,
$ ! you'll automatically have the copies already.
$ !
$ ! You should specify the value of the XAPPDIR flag passed to
$ ! Wc/WcLoadRes.c which under UNIX is usually $(XAPPLOADDIR),
$ ! although I think this is only needed for the MRI demos.
$ !
$ ! The cpp flags -DUSE_XtResizeWidget and -DUSE_XtMoveWidget need to
$ ! be defined, but this means the resultant Table will
$ ! certainly crash if any gadget children are somehow provided.  Table
$ ! never intends to support Gadgets, and R4 and later intrinsics will
$ ! never allow gadget children of Tables.  However, R3 intrinsics,
$ ! like those provided with early versions of Motif, do allow gadgets
$ ! as children of XmpTables (it is a bug in those releases of Xt).
$ ! If it does not work, then comment out that line so the Table widget
$ ! is not built into the Xmp library.
$ !
$ ! You must figure out how to pre-process the application defaults
$ ! files provided with Mri.  These require cpp and awk on UNIX systems.
$ ! Look at Mri/MakeByHand, WcMakeC.tmpl, WcClient.tmpl, and AppDef.rules.
$ ! However, the ones I (Peter Scott) have tested (Hello, Periodic) work
$ ! without modification.
$ !
$ ! 10/15/91 Martin Brunecky
$ ! 03/01/92 David Smyth (but not tested!!)
$ ! 07/18/93 Peter Scott
$ !
$ Set NoOn
$ olddef = f$environment("DEFAULT")
$ !
$ ! Find where we are (assuming this file is located in the Wcl top directory)
$ this = f$environment("PROCEDURE") - "][" - "]["
$ root = f$element(0,"]",this)
$ mridir = root + ".MRI]"
$ !
$ if P1 .eqs. "TEST" then goto TEST
$ !
$ ! Define logicals necessary to allow syntax like #include <X11/Atom.h>
$ ! All the following weirdness is to allow more than one / in an include directive...
$ !
$ if f$locate(":",root) .ne. f$length(root)
$ then
$ devspec = f$element(0,":",root)
$ root = f$trnlnm(devspec) + f$element(1,":",root) - "]["
$ endif
$ !
$ define Wc  'root'.Wc]
$ !
$ @SYS$COMMON:[DECW$MOTIF]DXM_LOGICALS
$ DEFINE X11 'root'.X11.],SYS$COMMON:[DECW$MOTIF.LIB.XT.X11],SYS$COMMON:[DECW$INCLUDE]
$ define VAXC$INCLUDE X11:,Xm:,DXm:,Mrm:,Wc,SYS$LIBRARY:,DECW$INCLUDE:
$ define C$INCLUDE X11:,Xm:,DXm:,Mrm:,Wc,SYS$LIBRARY:,DECW$INCLUDE:
$ !
$ ! Compile everything in Wc subdirectory
$ SET DEF 'root'.Wc]
$ if f$search("WC.OLB") .eqs. "" then $ LIBRARY/CREATE WC.OLB
$ CCCMD = "CC/DEFINE=VAX"
$!CCCMD = "CC/DEFINE=(XtNameToWidgetBarfsOnGadgets)
$ !
$!CCCMD XT4GETRESL.C   
$!CCCMD XTMACROS.C
$!CCCMD XTNAME.C
$ CCCMD MAPAG.C
$ CCCMD WCACTCB.C      
$ CCCMD WCCONVERT.C      
$ CCCMD WCCREATE.C       
$ CCCMD WCINVOKE.C
$ CCCMD WCLATEBIND.C
$ CCCMD WCLOADRES.C
$ CCCMD WCNAME.C         
$ CCCMD WCREG.C          
$ CCCMD WCREGXT.C
$ CCCMD WCSETVALUE.C
$ CCCMD WCTEMPLATE.C
$ CCCMD WCWARN.C
$ PURGE *.OBJ        
$ LIBR/REPLACE WC.OLB *.OBJ
$ !
$ ! Compile everything in Xmp subdirectory
$ SET DEF 'root'.Xmp]
$ if f$search("XMP.OLB") .eqs. "" then $ LIBRARY/CREATE XMP.OLB
$ CCCMD := CC/DEFINE=(VAX,"""USE_XtMoveWidget""","""USE_XtResizeWidget""")
$ ! 
$ CCCMD XMP.C
$ CCCMD XMPREGALL.C
$ CCCMD TABLE.C
$ CCCMD TABLELOC.C
$ CCCMD TABLEVEC.C
$ PURGE *.OBJ
$ LIBR/REPLACE XMP.OLB *.OBJ
$ !
$ ! Compile and build Mri
$ !
$ ! You also need to pre-process the application resource files
$ ! like the makefiles and Imakefiles do, to tailor them to Motif 1.0
$ ! or Motif 1.1 and to provide installation locations within them.
$ ! I do not know how to do that on a VAX - des
$ !
$ SET DEF 'root'.Mri]
$ CCCMD = "CC/DEFINE=VAX"
$ CCCMD MRI.C
$ LINK/EXE=MRI MRI.OBJ,[-.WC]WC/LIB,[-.XMP]XMP/LIB,[-.WC]WC/LIB,sys$input/OPT
        psect_attr=XMQMOTIF,noshr,lcl
        SYS$SHARE:DECW$MOTIF$XMSHR/SHAREABLE,-
        SYS$SHARE:DECW$MOTIF$XTSHR/SHAREABLE,-
        SYS$SHARE:DECW$XLIBSHR/SHAREABLE,-
        SYS$SHARE:VAXCRTL/SHAREABLE
$ SET DEF 'olddef'
$ !
$ ! Execute MRI demos/tests.  Check to see if you need a SET DISPLAY command
$TEST:
$ MRI == "$''mridir'MRI.EXE"
$ DEFINE DECW$USER_DEFAULTS 'root'.MRI]
$ MRI -rf HELLO
$ MRI -rf GOODBYE
$ MRI -rf WCALL
$ MRI -rf MENUBAR
$ MRI -rf OPTMENU
$ MRI -rf DIALOGS
$ MRI -rf TRAVERSAL
$ MRI -rf LISTRC
$ MRI -rf LISTTABLE
$ MRI -rf FORM
$ MRI -rf PERIODIC
$ MRI -rf PERTEM
$ MRI -rf TEMPLATE
$ MRI -rf POPUP
$ MRI -rf FSB
$ MRI -rf MODAL
$ MRI -rf TABLEDIALOG
$ MRI -rf APPSHELLS
$ EXIT
