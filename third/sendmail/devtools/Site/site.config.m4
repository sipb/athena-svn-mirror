dnl Use tcpwrappers and Hesiod
dnl (src/Makefile.m4 has -lwrap: we don't want smrsh, etc, to link with it)
APPENDDEF(`confENVDEF', `-DTCPWRAPPERS -DSASL -DMILTER -DMAP_REGEX -DSTARTTLS')
APPENDDEF(`confINCDIRS', `-I/usr/athena/include -I/usr/athena/include/sasl')
APPENDDEF(`confLIBDIRS', `-L/usr/athena/lib')
APPENDDEF(`confLIBS', `-lhesiod')
APPENDDEF(`conf_libmilter_ENVDEF', `-D_FFR_MILTER_ROOT_UNSAFE')
APPENDDEF(`conf_sendmail_LIBS', `-lsasl2 -lssl -lcrypto')

dnl We don't want NIS support
APPENDDEF(`confMAPDEF', `-UNIS')

dnl Install the old-fashioned way, i.e. setuid root rather than setgid smmsp.
define(`confSETUSERID_INSTALL', 1)

dnl Debugging, not optimizing
define(`confOPTIMIZE', `-g')

dnl Install man source, not cat files
define(`confMANROOT', `/usr/athena/man/man')
define(`confMAN1SRC', 1)
define(`confMAN5SRC', 5)
define(`confMAN8SRC', 8)

dnl Install random binaries in /usr/athena to save local disk space
define(`confEBINDIR', `/usr/athena/libexec')
define(`confSBINDIR', `/usr/athena/etc')
define(`confINCLUDEDIR', `/usr/athena/include')
define(`confLIBDIR', `/usr/athena/lib')

dnl Use Athena install; the devtools one can't handle non-writable files.
define(`confINSTALL', `install')
