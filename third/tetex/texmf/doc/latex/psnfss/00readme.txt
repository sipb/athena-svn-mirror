------------------------------------------------------------
            PSNFSS 9 -- installation instructions
------------------------------------------------------------
                                                  2002-07-04
                                              Walter Schmidt
                               

Contents
--------

- Overview
- Removing obsolete files
- Installing the virtual fonts, metrics and fd files
- Installing the PSNFSS macro packages
- Fonts required for PSNFSS
- Font map files
- Extra packages required for PSNFSS
- Making sure that everything works
- Files from PSNFSS v7.x, which are no longer part of the
  distribution.


Overview
--------

PSNFSS, originally developed by Sebastian Rahtz, is a set of
LaTeX2e package files to load common PostScript text and
symbol fonts, together with packages for typesetting math
using virtual math fonts which match Times and Palatino.

The collection is useless without the font description (fd)
files, virtual fonts (vf) and font metric (tfm) files for
the font families used by the packages.  On CTAN, those for
the Base 35 fonts are collected in the file lw35nfss.zip.
This archive also contains a font map file for dvips, and a
file named 8r.enc to reencode the Type1 fonts in a way that
is suitable for use with TeX.  Additionally, metrics, fd's
and font map files for the free typefaces Utopia, Charter
and Pazo are provided in the file freenfss.zip.

This document describes how to _install_ or _update_ PSNFSS.
Detailed instructions how to _use_ PSNFSS with LaTeX can be
found in the PDF document psnfss2e.pdf.

*
* Important changes and additions have been introduced with
* this version; see the file changes.txt.  Beside the PSNFSS
* macros, you _must_ re-install the .fd files, virtual fonts
* and font metrics from the archives lw35nfss.zip and
* freenfss.zip.  Furthermore, the Pazo math fonts are to be
* updated to version 1.002 or or 1.003.  The font map files
* are, however, unchanged, as compared with PSNFSS 8.2
*


Removing obsolete files
-----------------------

1) If your current version of PSNFSS is 7.0 or older, you
should remove manually _all_ macro files, .fd files, font
metrics and virtual fonts, that have to do with the PSNFSS
system, the Base35 fonts, the Utopia, Charter and Pazo
fonts.

2) If you have installed version 1.x of the mathpazo package
independently of the PSNFSS system, the package file and the
.fd files for the Pazo fonts are to be removed now.

*
*  3) Make sure that there is only one single instance of
*  the file 8r.enc (TexBase1 encoding vector) in your TeX
*  system; in a TDS-compliant system it must reside in the
*  directory texmf/dvips/psnfss.  Rationale:  Installing
*  PSNFSS version 9 will update this file, and there must
*  not remain any obsolete instances of the file in other 
*  places.
*

4) If your current version of PSNFSS is 8.1 or 8.2, no
further files need to be removed.  The existing files will
just be updated.


Installing the virtual fonts, metrics and fd files
--------------------------------------------------

Obtain the archive files lw35nfss.zip and freenfss.zip
from CTAN:macros/latex/required/psnfss.

If the file system layout of your TeX system complies with
TDS, unzip them in the texmf root directory (usually named
texmf) of your TeX system; all files will be unpacked into
the right directories then.  Otherwise, you have to sort out
manually the files from the .zip archives and copy them to
the appropriate directories of your TeX system.

Note that the archives do _not_ include the metrics of the
"raw" (= not re-encoded) PostScript text fonts.  Whether or
not these metrics are actually required, depends on your TeX
system; besides, they are not special for PSNFSS.


Installing the PSNFSS macro packages
------------------------------------

Put all files from the CTAN directory
macros/latex/required/psnfss (except for the .zip archives!)
into a directory where you keep documented LaTeX sources.
In a TDS compliant system this should be the directory

  texmf/source/latex/psnfss.

Run LaTeX on the installation script psfonts.ins to create
the package (.sty) files.  Move them to a directory where
LaTeX will find them.  In a TDS compliant system this should
be the directory

  texmf/tex/latex/psnfss.

The latter step is executed automagically by the
installation script, provided that your DocStrip program has
been configured appropriately and the target directory
exists already.

Move the documentation file psnfss2e.pdf to a suitable
directory; in a TDS compliant system this should be

  texmf/doc/latex/psnfss.

You may want to typeset the documentation of the package
code, too:  Run the file psfonts.dtx through LaTeX.


Fonts required for PSNFSS
-------------------------

The "Base 35" fonts
  Whether or not these fonts must be available to the dvi
  driver as "real" Type1 fonts, depends on the particular
  application (dvips, pdfTeX, VTeX etc.)  and whether the 
  fonts are to be embedded into the final PostScript or PDF
  documents.
  
Adobe Utopia
Bitstream Charter
  These Type1 fonts can be obtained for free from various
  sources.  Make sure to install the files with the names
  according to the KB scheme:
  
  Utopia-Regular          putr8a.pfb
  Utopia-Italic           putri8a.pfb
  Utopia-Bold             putb8a.pfb
  Utopia-BoldItalic       putbi8a.pfb
  CharterBT-Roman         bchr8a.pfb
  CharterBT-Italic        bchri8a.pfb
  CharterBT-Bold          bchb8a.pfb
  CharterBT-BoldItalic    bchbi8a.pfb
  
Pazo 1.003
  The Type1 fonts can be obtained from the CTAN directory
  fonts/mathpazo.  Notice that PSNFSS 9 needs a recent
  version 1.002 or 1.003 of the Pazo fonts!
  
  PazoMath-Bold           fplmb.pfb
  PazoMathBlackboardBold  fplmbb.pfb
  PazoMath-BoldItalic     fplmbi.pfb
  PazoMath                fplmr.pfb
  PazoMath-Italic         fplmri.pfb

Computer Modern
RSFS (Ralph Smith's Formal Script)
Euler Math
  These font families are required when typesetting math
  using the packages mathptm, mathptmx, mathpple, or
  mathpazo.
  
  They are available in Type1 as well as METAFONT format; it
  is recommended to provide the Type1 variants, if you want
  to create PostScript or PDF documents.  The particular
  fonts eurm10 (Euler Roman 10pt) and eurb10 (Euler Roman
  Bold 10pt) are special:  They _must_ be provided in Type1
  format so that obliqued versions, named eurmo10 and
  eurbo10, can be generated from them.


Font map files
--------------

lw35nfss.zip and freenfss.zip include the following font map
files in the directory dvips/psnfss:

psnfss.map:     for the Base35 fonts, eurmo10 and eurbo10
charter.map:    for Bitstream Charter
utopia.map:     for Adobe Utopia
pazo.map        for the Pazo math fonts

psnfss.map is primarily destined for use with dvips.  The
entries for the fonts "eurmo10" and "eurbo10" may need to be
customized:  Feel free to change the FontNames (EURM10,
EURB10) to lower case, if you have got the Type1 fonts from
MicroPress, rather than the BlueSky distribution.  This
particular change is not regarded as a violation of the
license conditions!  

psnfss.map does _not_ make dvips embed the Base35 fonts.  As
to pdfTeX, you will have to create a modified copy of the
file to make pdfTeX embed all fonts (or at least those,
which are not part of the Base14 set).  This new map file
may also be useful for dvips, if you want to make it embed
all fonts.

The map files charter.map, utopia.map and pazo.map are,
equally suitable for use with either dvips or pdfTeX.

Consult the documentation of your TeX system how to set up
dvips, pdfTeX etc, to actually use the above-mentioned font
map files!

Other programs, such as VTeX, need a different format of the
font map files.  They may also require entries for the raw
(= not reencoded) fonts.  When creating these map files,
take those for dvips as a model!


Extra packages required for PSNFSS
----------------------------------

The "Graphics" bundle must be installed, since PSNFSS will
make use of the package keyval.sty.


Making sure that everything works
---------------------------------

Run the test following files through LaTeX:

  test0.tex
  test1.tex
  test2.tex
  test3.tex
  mathtest.tex 
  pitest.tex


Files from PSNFSS v7.x, which are no longer part of the
distribution
-------------------------------------------------------

The files for the commercial Lucida Bright and MathTime
fonts are distributed from the CTAN directory
macros/latex/contrib/supported/psnfssx/ now.

-- finis
