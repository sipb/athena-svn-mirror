HELP WITH GETTING ASPELL TO COMPILE WITH VC++
=============================================

The following instructions should help you compile Aspell with VC++.
However, I, the author of Aspell, do not officially support the VC++
Port.  If you have a problem please DO NOT contact me, and instead
contact Tony Lavinio or Francois Boudreau.

######################################################################

ASPELL 0.50.4.1 on WIN32
------------------------

This patch will allow Aspell 0.50.4.1 to compile with the VC++
compiler in VisualStudio 6.

All of the changes are wrapped by #ifdef WIN32PORT/#else/#endif, so
that they are very easy to spot.

There are some extra files that are necessary to build under VC++, and
most notable amongst the changes are that methods in DLLs (the Win32
equivalent of shared objects) must be specially decorated to be seen
outside of the DLL.  The ASPELL_API macro was used for this.

The diffs were based on the earlier diffs to 0.50.2 by Francois
Boudreau <fboudreau@accentus.ca>.

This patch produces both aspell.dll and aspell.lib, as well as
aspell.exe and acompress.exe (renamed from compress.exe to avoid
collision with a similarly-named Win32 utility).

This Win32 port was done by Tony Lavinio <aspell at lavinio.net> of
PSC Labs, with help from Ivan Pedruzzi.  We turn all rights to our
changes back over to Kevin Atkinson.

To use this port:
- Create a directory (we'll call it 'aspell')
- Apply the patch aspell-0.50.4.1-vc++.diff or alternatively
  download and unzip aspell-0.50.4.1-vc++-src.zip 
  from ftp://ftp.gnu.org/gnu/aspell
- You can save time building by using the
  aspell-0.50.4.1-vc++-bin.zip file also found at ftp://ftp.gnu.org/gnu/aspell

The dictionaries can be built from the .cwl files by using the line
  acompress d <xx.cwl | aspell -lang=xx create master xx.rws


######################################################################

ASPELL 0.50.2 on WIN32
----------------------

This patch should allow Aspell to compile with VC++.  The changes were
submitted to by Francois Boudreau <fboudreau@accentus.ca>:

  It's a bit of a mess, we were not very consistent in our editing. We
  went for the quickess fix, so a few things were just disabled, like
  file locking and locales. And we had to remove a few const.

I, the author of Aspell, do not even know if it will work or the other
steps needed to get Aspell to compile under Win32.  If you have
problems with this patch please contact Francois Boudreau
<fboudreau@accentus.ca> and not me since I do not have a copy of VC++
and in general dislike Microsoft Windows.

Kevin Atkinson

######################################################################

