@device(Postscript)
@make(manual)
@style(fontfamily TimesRoman, indent 0, TimeStamp="8 March 1952")
@modify(Hd0,PageBreak Off)
@modify(Hd1,PageBreak Off)
@modify(Hd1A,PageBreak Off, flushleft)
@modify(IndexEnv, columns 2, boxed)
@Define(Logotype, Use HDx, Size 10, flushright, Font BodyFont, FaceCode B,
        spaces kept)
@Define(Fnctr, use center, above 0, size -1)
@Define(Title, Use HDx, Size 16, flushright)
@Define(Author, Use HDx, Size 12, flushright, FaceCode R, Above 0.2inch,
                 below 0.6 inch)
@Disable(FigureContents)
@Disable(Contents)

@pagefooting(left="@b(XLogin Design Proposal)", right="@b(@value(filedate))",
	immediate, line="@fnctr[Copyright @Y(C) 1990 @~
                   by the Massachusetts Institute of Technology]")
@pagefooting(left="@b(XLogin Design Proposal)", right="@b(@value(filedate))")
@pageheading(left="", right="@b(Page @value(page))")

@Logotype[M. I. T.   PROJECT   ATHENA]
@picture(height=.4inches, ScaleableLaser = /usr/athena/lib/owl.PS)
@title(X Login Design Proposal)
@author(Mark Rosenstein

@value(filedate))

@set(page=1)

This document proposes a new X-based login program for use in Athena
release 7.2.  Comments are welcome.  Due to time constraints, we are
starting coding on some pieces of this, but nothing is frozen in stone
yet.

@Section(Goals)

Goals for this development include:
@itemize{
Login from ``Press any key to start'' to usable window within 30 seconds

No current functionality is lost

No impact on users' dot files

Access to some programs while not logged in

}

The new @b(xlogin) will subsume the current functionality of both @b(toehold)
and @b(xlogin).

@Section(User Interface)

Since the @b(toehold) functionality will now be in an X program, the
deactivated screen can be somewhat nicer.  It will be a black screen
with an owl and ``Press any key to start'' floating about the screen.
Upon pressing a key or clicking the mouse, this screen will instantly
be replaced by the login greeting window.  Activation takes place
while the user types his/her username and password.

At first glance, the login greeting window will look much as it does
now.  It will grab all keyboard input even if the mouse is not in the
window.  The ``Set Session Options'' button will have its text changed
since it now has more functionality.  A date/time clock has also been
proposed as an addition to this window.  The ``Other Options'' button
(or whatever we decide to call it) will pop up a menu or series of
menus with:
@itemize{
Session Options
@itemize[
Your regular login session

Ignore your customizations

Specify a special login session

Start Over
]

Useful Programs
@itemize[
Help About Athena (xolh)

MIT Information (TechInfo)

Where is a free workstation? (XCluster)

Login to MITVMA
]

Other Functions
@itemize[
Shutdown window system, login on console

Show machine configuration

Shutdown workstation
]
}

The programs that @b(xlogin) can run will all be configured to timeout
and exit after some period of inactivity so that idle workstations
revert to the deactivated state.  When errors occur, a box will pop-up
with an appropriate icon and error text, similar to the current
@b(xlogin).

The @b(xconsole) will be present as well, although it may look
slightly different since it will be re-written without the toolkit to
use less memory.

@Section(Internal Structure)

There were five choices of where to start with the code:
@enumerate{
Modify current @b(xlogin)

Fix bugs in @b(xlogin) proposed a year ago

Modify X Consortium's @b(xdm)

Write it from scratch in X Toolkit

Write it from scratch in Xlib
}

The first three choices were ruled out as many people making
modifications to other older code produces results which are hard to
understand, have poor modularity, and are difficult to debug.  The
advantages to using @b(xdm) are lost when you look at how many changes
would be necessary to get the parallelism we need.  We should put
@b(xdm) in the release so people can support X terminals, but not use
it for our own logins.

This leaves writing it in the toolkit or directly in Xlib.  Since the
size and speed of the actual login window are not too much of an
issue, we will go for the ease of maintainability of writing this in
the toolkit.  The console window, which stays around for the entire
login session, will be written in Xlib because of size considerations.
As an aid in writing the login part of the program, we will write a
login library which is undergoing separate review.

The new system will consist of two programs: @b(xconsole) and
@b(xlogin).  Operation will be as follows:

At boot time, @b(xconsole) will be started by @b(init).  @b(Xconsole)
will start the X server with a font path pointing to a directory on
the root with just the minimum number of fonts to handle @b(xlogin).
Once the X server is running, @b(Xconsole) will create the console
window (but not map it unless it has something to display) and start
@b(xlogin) as a child.

@b(Xlogin) will initially put up the idle screen, then create its
widgets, but not map them.  When the user presses a key or clicks the
mouse, @b(xlogin) will bring down the idle screen and map the greeting
window.  It will also start the activation at this time, and
activation may proceed as the user types their username and password.
Part of activation will include extending the X font path to the real
font directory on the packs.  When the user is done typing, if the
@b(attach) and @i(/etc/activate.local) processes have not finished, it
will wait for those.  It will then handle login as it does now.

When it is time to start the global @i(xsession) script, @b(xlogin)
will replace itself with the shell running this script, so that the
@b(xlogin) process does not hang around through the entire login
session.  The @i(xsession) script will be changed as well.  I am
experimenting with a program that delays execution of scripts based on
the machine's current load average, and expect to get some performance
improvement there.  The interaction with users' dot files will not
change.

@b(Xconsole) will detect logout when its child (now the @i(xsession)
script that @b(xlogin) exec'd over itself) exits.  At this point it
will remove the user from @i(/etc/passwd) and handle the deactivation
activities, then start a new @b(xlogin).  Part of deactivation will be
telling the X server to reset its font path to only include what is
on the root again.  If @b(xconsole)'s child exits with a status
indicating a console login is required, xlogin will kill the X server
and start up @i(/bin/login), again waiting for it to exit and then
restarting X and performing a deactivate.

Files added to the root include:
@itemize{
X server and microcode as needed

Five fonts: fixed, cursor, and New Century Schoolbook in three sizes

@b(Xlogin) and @b(xconsole) binaries

configuration files for @b(xlogin) and @b(xconsole)
}
@b(Toehold) may be removed from the root.

