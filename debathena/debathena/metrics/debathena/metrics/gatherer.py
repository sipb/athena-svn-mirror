#!/usr/bin/python
"""Debathena metrics collection.
Usage: debathena-metrics

This script wraps an X session, collecting statistics on what programs
are run by the user over the course of the session.

This script also collects notifications about software being installed
through apt.

At the end of the login session, the collected information is batched
and sent to the Athena syslog server, along with the duration of the
login session.
"""


import errno
import fcntl
import os
import pwd
import socket
import syslog
import time
import uuid

import dbus
import dbus.mainloop.glib
import dbus.service
import gobject

from debathena.metrics import connector


DBUS_OBJECT = '/'
DBUS_INTERFACE = 'edu.mit.debathena.Metrics'


LOG_LEVEL = syslog.LOG_DAEMON | syslog.LOG_INFO
LOG_SERVER = ('wslogger.mit.edu', 514)


def make_non_blocking(fd):
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)


class Metrics(dbus.service.Object):
    def __init__(self, loop, uid, *args):
        super(Metrics, self).__init__(*args)

        self.loop = loop
        self.uid = uid
        self.installed_packages = set()
        self.executed_programs = set()
        self.start_time = time.time()
        self.session_uuid = str(uuid.uuid4())

        proc_conn = connector.Connector()
        make_non_blocking(proc_conn)
        gobject.io_add_watch(
            proc_conn,
            gobject.IO_IN,
            self.run_program,
            )

        dbus.SystemBus().add_signal_receiver(
            self.install_package,
            signal_name='InstallPackage',
            dbus_interface=DBUS_INTERFACE,
            path=DBUS_OBJECT,
            )

        dbus.SystemBus().add_signal_receiver(
            self.logout,
            signal_name='LogOut',
            dbus_interface=DBUS_INTERFACE,
            path=DBUS_OBJECT,
            )

    def run_program(self, fd, cond):
        while True:
            try:
                ev = fd.recv_event()
            except IOError, e:
                if e.errno == errno.EAGAIN:
                    break
                raise

            if ev.what == connector.PROC_EVENT_EXEC:
                try:
                    prog = os.readlink("/proc/%d/exe" % ev.process_pid)
                    self.executed_programs.add(prog)
                except OSError, e:
                    if e.errno == errno.ENOENT:
                        continue
                    raise

        return True

    def install_package(self, package):
        self.installed_packages.add(str(package))

    def logout(self):
        session_length = time.time() - self.start_time

        lines = []
        lines.append('session_len: %d' % session_length)

        for p in self.installed_packages:
            lines.append('installed_package: %s' % p)

        for p in self.executed_programs:
            lines.append('executed_programs: %s' % p)

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        for l in lines:
            s.sendto('<%d> debathena-metrics %s: %s\n' % (LOG_LEVEL, self.session_uuid, l),
                     LOG_SERVER)

        self.loop.quit()


def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    uid = pwd.getpwnam(os.environ['USER']).pw_uid

    loop = gobject.MainLoop()
    m = Metrics(loop, uid)
    loop.run()


if __name__ == '__main__':
    main()
