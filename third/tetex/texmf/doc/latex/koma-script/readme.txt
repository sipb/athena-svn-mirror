Sie finden eine deutsche Version dieses Textes in liesmich.txt!

KOMA-Script bundle
==================

Redistribution of this bundle is allowed provided that all files that
make up the KOMA-Script bundle are contained.  The list of all files
making up this bundle is given below.

This bundle has been created for the use with LaTeX2e. Although a
great effort has been made to eliminate all bugs, this bundle is
provided `as is' without warranty of any kind, either expressed or
implied, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose.  The entire risk
as to the quality and performance of the program is with you.


The files:
----------

All files marked with an exclamation mark (!) are REQUIRED as a part
of the distribution. You are NOT ALLOWED to distribute them
separately, any distribution MUST contain ALL of the required
files. All unmarked files are OPTIONAL parts of the distribution. You
may leave them out but you MUST NOT redistribute them without
including the REQUIRED files.

   ChangeLog     - List of changes since January 1st, 2001.
 ! komabug.tex   - LaTeX-file for creating a bug report.
		   USE THIS FILE FOR REPORTING BUGS!
 ! komascr.ins   - Install file for the KOMA-Script bundle. Calls
                   (and requires!) scrclass.ins, scrtime.ins and 
		   scrpage.ins.
 ! scrclass.ins  - Install file for scrclass.dtx, scrlettr.dtx and
                   script20.dtx. Required by komascr.ins.
 ! scrclass.dtx  - contains:
    scrartcl.cls  - KOMA-Script class as a replacement for article.cls,
    scrreprt.cls  - KOMA-Script class as a replacement for report.cls,
    scrbook.cls   - KOMA-Script class as a replacement for book.cls,
    scrlttr2.cls  - KOMA-Script class as a replacement for letter.cls,
    typearea.sty  - KOMA-Script package for the calculation of the type
                    area,
    scrlfile.sty  - KOMA-Script package with hooks for loading files, 
                    packages or classes.
 ! scrlettr.dtx  - contains:
    scrlettr.cls  - obsolete KOMA-Script class.
 ! script20.dtx  - contains:
    script.sty    - style provided for compatibility,
    script_s.sty  - style provided for compatibility,
    script_l.sty  - style provided for compatibility.
 ! scrtime.ins   - Install file for scrtime.dtx.
                   Required by komascr.ins.
 ! scrtime.dtx   - contains:
    scrtime.sty   - KOMA-Script package for formatting a time,
    scrdate.sty   - KOMA-Script package for formatting the day of the week.
 ! scrpage.ins   - Install file for scrpage.dtx.
                   Required by komascr.ins.
 ! scrpage.dtx   - contains:
    scrpage.sty   - obsolete KOMA-Script package,
    scrpage2.sty  - KOMA-Script package for designing your own page
                    headers and footers.
   scrguide/     - Directory (folder) with the german manual,
                   containing:
 !  adressen.tex  - chapter of the manual,
 !  adrkonv.tex   - chapter of the manual,
 !  brief.tex     - chapter of the manual,
 !  datladen.tex  - chapter of the manual,
 !  datmzeit.tex  - chapter of the manual,
 !  einleit.tex   - chapter of the manual,
 !  hauptcls.tex  - chapter of the manual,
 !  layout.tex    - chapter of the manual,
 !  satzspgl.tex  - chapter of the manual,
 !  scrguide.bib  - bibliography,
 !  scrguide.bst  - style for BibTeX,
 !  scrguide.dbj  - driver for custombib,
 !  scrguide.ist  - style for MakeIndex,
 !  scrguide.mbs  - modul for custombib,
 !  scrguide2.tex - mainfile of the manual (you need KOMA-Script to
                    run the manual through LaTeX).
 !  english/      - Directory (folder) with the german manual, 
                    containing:
 !    addrconv.tex  - chapter of the english manual,
 !    addressf.tex  - chapter of the english manual,
 !    datetime.tex  - chapter of the english manual,
 !    headfoot.tex  - chapter of the english manual,
 !    introduc.tex  - chapter of the english manual,
 !    letter.tex    - chapter of the english manual,
 !    loadfile.tex  - chapter of the english manual,
 !    main.tex      - language definitions for the english manual,
 !    maincls.tex   - chapter of the english manual,
 !    pglayout.tex  - chapter of the english manual,
 !    scrguide-en.bst - style for BibTeX,
 !    scrguide-en.dbj - driver for custombib,
 ! scrguide.dvi  - A ready-to-use version of german manual (DVI).
   scrguide.ps   - A ready-to-use version of german manual
                   (PostScript). 
   scrguide.pdf  - A ready-to-use version of german manual (Adobe's
                   PDF). 
 ! scrguien.dvi  - A ready-to-use version of english manual (DVI).
   scrguien.ps   - A ready-to-use version of english manual
                   (PostScript). 
   scrguien.pdf  - A ready-to-use version of english manual (Adobe's
                   PDF). 
 ! genindex.pl   - Perl script which generates the spitted index of
                   the manual.
 ! koma-script.tex - Wrapper to scrguide and scrguien.
 ! koma-script.pdf - Ready-to-use version of koma-script.tex.
 ! komascript.pdf - Same like koma-script.pdf.
 ! komascr.pdf   - Same like koma-script.pdf.
 ! scr_new1.tex  - Information concerning the first version of
                   KOMA-Script (German).
 ! scr_new2.tex  - Additional information (German).
 ! scr_new3.tex  - Additional information (German).
 ! scr_new4.tex  - Additional information (German).
 ! scr_new5.tex  - Additional information (German).
 ! scr_new6.tex  - Additional information (German).
 ! scr_new7.tex  - Additional information (German).
 ! scr_new8.tex  - Additional information (German).
 ! scr_new9.tex  - Additional information (German).
 ! scrnew10.tex  - Additional information (German).
 ! scrnew11.tex  - Additional information (German).
 ! scrnew12.tex  - Additional information (German).
 ! scrnew13.tex  - Additional information (German).
 ! scrnew14.tex  - Additional information (German).
 ! scrnew15.tex  - Additional information (German).
 ! scrnew16.tex  - Additional information (German).
 ! scrnew17.tex  - Additional information (German).
 ! scrnew18.tex  - Additional information (German).
 ! scrnew19.tex  - Additional information (German).
 ! scrnew20.tex  - Additional information (German).
 ! scrnew21.tex  - Most recent information (German and English).
   scr_new1.dvi \
   ...           > Ready-to-use versions of the above.
   scrnew21.dvi /  
 ! liesmich.txt  - Read this, if you prefer German.
 ! readme.txt    - You are just reading it.
 ! INSTALL.TXT	 - The installation manual (English)
 ! INSTALLD.TXT	 - The installation manual (German).
 ! LEGAL.TXT	 - The dry part: legal stuff, warranty, license (English)
 ! LEGALDE.TXT	 - The dry part: legal stuff, warranty, license (German)

 If one or more of the above files marked with an exclamation mark (!)
 is missing, please inform the author about the missing files and
 where you have gotten the bundle from.

	  InterNet:  Markus.Kohm@gmx.de
          Post:      Markus Kohm
                     Fichtenstrasse 63
                     68535 Edingen-Neckarhausen
		     Germany

  If you cannot reach me at one of the addresses above, ask at one of 
  the following groups:
    MausNet, Fido, Usenet: de.comp.text.tex
  or try:
    http://koma-script.net.tf

+ NOTE: The CVS of the "KOMA-Script documentation project" is not a
+       distribution. Only this CVS is allowed to dispense with the
+       generated files (this means scrguide.dvi).


Installation:
-------------

The following describes the general procedure for installing this
bundle. If you have a TeX system compatible to TDS (most of the newer
TeX systems are), please read INSTALL.TXT.

* ATTENTION: You need a complete LaTeX2e!

  Copy all files to a directory, where TeX/LaTeX can find them.
  Run komascr.ins through LaTeX, e.g. at UNIX type:
    tex "&latex" komascr.ins
  or
    latex komascr.ins
  You'll get a couple of new cls-, sty- and tex-files. Copy them to
  the corresponding LaTeX-directory. See INSTALL.TXT for further
  information.
  Do not forget to run the initialization procedures specific to your
  TeX distribution, e.g. if you use teTeX, run 'texhash'

* ATTENTION: You needn't and you shouldn't create the complete
*            documentation (it's in German!) or the english short
*            manual by your own, because there are ready-to-use dvi
*            files available.

  If you want the documentation specific to the implementation, you
  have to run the dtx-files through LaTeX. Again, using a UNIX system
  type: 
    tex "&latex" scrclass.dtx
  or
    latex scrclass.dtx

  Most likely you won't need those as, the user manual should be
  sufficient. 

+ IMPORTANT: Do not forget to run scrguide.tex and each dtx-file
+            through LaTeX three times, if you like to have correct
+            references and table of contents.

  Last but not least, you should run all scr_news-files through LaTeX
  and read all the documentation.


Bug-reports:
------------
  To report a bug, run 'komabug.tex' through LaTeX and fill in the
  fields as concise as possible and send the form upon completion to
  the author at one of the addresses above.

* ATTENTION: IT IS HIGHLY RECOMMENDED TO USE KOMABUG.TEX FOR BUG
*            REPORTS.

Changes since first Version at 1994/07/07:
------------------------------------------
  You'll find all changes at the dtx-files and the glossaries.


ISO-Date of file:
-----------------
  2002-06-14