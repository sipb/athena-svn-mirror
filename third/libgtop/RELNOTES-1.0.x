RELEASE NOTES FOR LIBGTOP 1.0.7
===============================

OVERVIEW
--------

LibGTop is a library that read information about processes and the
running systems. This information include:

General System Information:

cpu		- CPU Usage
mem		- Memory Usage
swap		- Swap Usage (including paging activity)
loadavg		- Load average (including nr_running, nr_tasks, last_pid)
uptime		- Uptime and Idle time, can be calculated from CPU usage

SYS V IPC Limits:

shm_limits	- Shared Memory Limits
msg_limits	- Message Queue Limits
sem_limits	- Semaphore Set Limits

Network:

netload		- Network load
ppp		- PPP statistics

Process List:

proclist	- List of processes

Process information:

proc_state	- cmd, state, uid, gid
proc_uid	- uid,euid,gid,egid,pid,ppid,pgrp
		  session,tty,tpgid,priority,nice
proc_mem	- size,vsize,resident,share,rss,rss_rlim
proc_time	- start_time,rtime,utime,stime,cutime,cstime
		  timeout,it_real_value,frequency
proc_signal	- signal,blocked,sigignore,sigcatch
proc_kernel	- k_flags,min_flt,maj_flt,cmin_flt,cmaj_flt
		  kstk_esp,kstk_eip,nwchan,wchan
proc_segment	- text_rss,shlib_rss,data_rss,stack_rss,dirty_size
		  start_code,end_code,start_stack

Process maps:

proc_args	- Command line arguments
proc_map	- Process map (/proc/<pid>/maps under Linux)

File system usage:

mountlist	- List of currently mounted filesystems
fsusage		- File system usage

PORTABILITY:
-----------

LibGTop is designed to be as portable as possible. None of the
functions and retrieved information should be specific to a specific
operating system. So you only need to port the system dependent part
of the library to a new system and all application programs can then
use libgtop on this new system.

CLIENT/SERVER MODEL:
-------------------

Some systems like DEC OSF/1 or BSD require special privileges for the
calling process to fetch the required information (SUID root/SGID
kmem). To solve this problem, I designed a client/server model which
makes a call to a SUID/SGID server which fetches the required
information whenever it is required. This server is only called for
features that really require privileges, otherwise the sysdeps code
is called directory (every user can get the CPU usage on DEC OSF/1,
but only root can get information about processes other than the
current one).

There is also some kind of daemon which can be used to fetch
information from remote systems (still experimental). This daemon
normally runs as nobody and calls the SUID/SGID itself when needed.

LIBGTOP AND GNOME:
-----------------

Although LibGTop is part of the GNOME desktop environment, its main
interface is totally independent from any particular desktop environment,
so you can also use it as a standalone library in any piece of GPLed
software which makes it also a valuable part of the GNU project.

LibGTop is currently used in various places in the GNOME Project,
for instance in some of the applets in gnome-core and - of cause -
this ultra-cool application called GTop ...

However, you need to give the configure.in script the `--without-gnome'
parameter when you want to use LibGTop without GNOME (this is because,
if you want to use it with GNOME, you need to compile it after the main
GNOME libraries and I wanted to avoid getting unnecessary bug reports
about this).

LIBGTOP AND GNOME - PART II:
---------------------------

LibGTop was tested with FreeBSD 3.0 but it should also work with
FreeBSD 2.2.7, NetBSD and OpenBSD.

Unfortunately, I don't have the power and disk space to install all
possible operating systems out there on my machine and test things myself,
so I depend on people telling me whether it works and sending me bug
reports and patches if not.

However, I consider FreeBSD, NetBSD and OpenBSD as supported systems for
LibGTop and whenever I get bug reports I will do my best to fix them as
quickly as possible.

PLATFORM SPECIFIC NOTES FOR LINUX:
==================================

[I am speaking of the Linux kernel here.]

Under Linux, LibGTop should work without problems and read everything
from /proc.

LibGTop 0.25 also had an experimental kernel interface to read this
information directly from the kernel with a system call - but I have
currently dropped support for this as I am too busy with GNOME
development to keep current with kernel hacking.

PLATFORM SPECIFIC NOTES FOR SOLARIS:
====================================

The development branch of LibGTop (the 1.1.x series) has a first version
of the Solaris port which works at least on Solaris 7.

If you are on a Solaris system and want to give it a try, just fetch the
latest 1.1.x tarball from ftp://ftp.home-of-linux.org/pub/libgtop/1.1/
and try it out.

PLATFORM SPECIFIC NOTES FOR BSD:
=================================

There are a few caveats:

* You need to manually make the `$(prefix)/bin/libgtop_server' SGID to
  kmem after installation and mount the /proc file system of FreeBSD
  (/proc/<pid>/mem is used within kvm_uread ()).

* To get the filenames of the process maps displayed in GTop, you need
  to configure with the `--with-libgtop-inodedb' option (you need GDBM
  for this to work).

  You have then to create an inode database which is used to look up
  filenames. This is done using the `mkinodedb' program which comes
  along with libgtop.

  See the file src/inodedb/README for details:

  The `mkinodedb' program which is build in this directory takes two
  command line arguments: the full pathname of the database to be
  created and the name of a configuration file consisting of directory
  and file names each on a line by itself - see `/etc/ld.so.conf' for
  an example.

  Putting a directory name in this file means all regular files found
  in this directory are included in the database, but it will not
  recursively descend into subdirectories (for instance, we want
  everything in `/usr/lib' but not every single file in `/usr/lib/sgml').
  You can also use filenames to include a single file.

Have fun,

Martin <martin@home-of-linux.org>
