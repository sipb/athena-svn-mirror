Sie finden eine deutsche Version dieses Textes in liesmich.txt!

KOMA-Script-Bundle
==================

This system is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Permission is granted to distributed verbatim copies of all files
together with all other files of the KOMA-script-bundle. You are
allowed to modify one or all of the dtx-files, if and only if you
change the name of the modified file. You are allowed to distribute
the modfied files but only TOGETHER with the unmodified versions. You
have to document all changes and the name of the author of the
changes.

No other permissions to copy or distribute this file in any form are
granted.

You are NOT ALLOWED to take money for the distribution or use of
either this file or a changed version, except for a nominal charge for
copying etc.

To install and use this bundle, you need LaTeX2e.


The files:
----------
  knownbug.txt  - List of bug-reports and -solutions since 1996-11-26.
  komascr.bug   - Sorry this bug-report-file is german only.
  komascr.ins   - Installationdriver for the complete bundle, this 
                  uses scrclass.ins, scrtime.ins and scrpage.ins.
  scrclass.ins  - Installationdriver for scrclass.dtx, scrlettr.dtx 
                  and script20.dtx.
                  It's called by komascr.ins.
  scrclass.dtx   - contains:
    scrartcl.cls  - KOMA-Script-Class (substitute for article.cls)
    scrreprt.cls  - KOMA-Script-Class (substitute for report.cls)
    scrbook.cls   - KOMA-Script-Class (substitute for book.cls)
    typearea.sty  - KOMA-Script-Package for calculation of type area
  scrlettr.dtx   - contains:
    scrlettr.cls  - KOMA-Script-Class (substitute for letter.cls)
    phone.tex     - Interactive file to generate phone-list from 
                    adr-files
    dir.tex       - Interactive file to generate lists from adr-files
    addrconv.bst  - BibTeX-Style to convert BibTeX address databases 
                    into adr-files
    addrconv.tex  - Interactive file to generate needed aux-files
    birthday.bst  - BibTeX-Style to generate birthday adr-files from 
                    BibTeX address databases
    birthday.tex  - Interactive file to generate needed aux-files
    email.bst     - BibTeX-Style to generate email adr-files from 
                    BibTeX address databases
    email.tex     - Interactive file to generate needed aux-files
    example.bib   - BibTeX address database example
  script20.dtx  - contains:
    script.sty    - compatibility-style
    script_s.sty  - compatibility-Style
    script_l.sty  - compatibility-Style
  scrtime.ins   - Installationdriver for scrtime.dtx.
                  It's called by komascr.ins.
  scrtime.dtx   - contains:
    scrtime.sty   - time output
    scrdate.sty   - week-day output
  scrpage.ins   - Installationdriver for scrpage.dtx.
                  It's called by komascr.ins
  scrpage.dtx   - contains:
    scrpage.sty   - Creation of pagehead and pagefoot.
  scrguide.tex  - German Userdocumentation; this needs several parts
                  of the KOMA-Script bundle.
  scrguide.dvi  - Ready to use version of scrguide.
  screnggu.tex  - English version of scrguide.tex; this needs 
                  several parts of the KOMA-Script bundle. This
                  documentation is shorter than scrguide. If there are
                  differences to scrguide try scrguide.
  screnggu.dvi  - Ready to use version of screnggu.
  scrguide.ist  - MakeIndex-Style for scrguide and screnggu.
  scr_new1.tex  - Information of first KOMA-Script-Version (german)
  scr_new2.tex  - More information (german)
  scr_new3.tex  - More information (german)
  scr_new4.tex  - More information (german)
  scr_new5.tex  - More information (german)
  scr_new6.tex  - More information (german)
  scr_new7.tex  - More information (german)
  scr_new8.tex  - More information (german)
  scr_new9.tex  - More information (german)
  scrnew10.tex  - More information (german)
  scrnew11.tex  - More information (german)
  scrnew12.tex  - More information (german)
  scrnew12.tex  - More information (german)
  scrnew13.tex  - More information (german)
  scrnew14.tex  - More information (german)
  scrnew15.tex  - More information (german)
  scrnew16.tex  - Actual information (german)
  liesmich.txt  - If you prefer german.
  readme.txt    - The file you are reading.

  Contact me, if files are missing (and tell me the way you get it):
          MausNet:    Markus Kohm @ HD   (mail-limit!)
          snail-mail: Markus Kohm
                      Fichtenstrasse 63
                      68535 Edingen-Neckarhausen
                      Germany

* ATTENTION: Don't send more than 48kbyte to a MausNet-user at one 
*            day!

  If you cannot reach me at one of the adresses above, ask at one of 
  the following groups:
    MausNet, Fido, Usenet: de.comp.text.tex


Installation:
-------------

* ATTENTION: You need a complete LaTeX2e!

  Copy all files to a directory, where TeX/LaTeX can find them.
  Run komascr.ins through LaTeX, e.g. at UNIX type:
    tex "&latex" komascr.ins
  or
    latex komascr.ins
  You'll get a couple of new cls-, sty- and tex-files. Copy them to
  the coresponding LaTeX-directory.

* ATTENTION: To create the complete documentation (it's german!) you
*            need german.sty! You don't need to generate the
*            documentation, because there are ready dvi files. But the
*            documentation not only documentation, it is an example of
*            using KOMA-Script, too.

  To get documentation run scrguide.tex through LaTeX, e.g. at UNIX
  type:
    tex "&latex" scrguide.tex
  or:
    latex scrguide.tex
  You need three runs, to get table of contents and references right.
  You'll get the user-documentation. If you want an index, you have to
  use precompiled scrguide.ind or you have to create a new index
  running scrguide.idx through makeindex resp. makeindx bevor last
  (third) LaTeX run.
  At UNIX e.g. type:
    makeindex -g -r -s scrguide.ist scrguide.idx
  resp.
    makeindx -g -r -s scrguide.ist scrguide.idx

  If you want the implementation-doxumentation, you have to run the
  dtx-files through LaTeX, e.g. at UNIX type:
    tex "&latex" scrclass.dtx
  oder
    latex scrclass.dtx
  But I think, main-documentation is enough.

+ IMPORTANT: You should run scrguide.tex and each dtx-file through
+            LaTeX three times, because of referenzes and table of
+            contents.

  Last but not least, you should run all scr_news-files through LaTeX
  and read all the documentation.


Bug-reports:
------------
  You may use latexbugs.tex, but don't send the report to the
  LaTeX-team, send it to one of the adresses in this file
  above. Notice that there is a mail-limit at MausNet!
  If you understand german, please read "Fehlermeldungen" at
  liesmich.txt.


Changes since first Version at 1994/07/07:
------------------------------------------
  You'll find all changes at the dtx-files and the glossaries.


Filedate:
---------
  1998/10/27
