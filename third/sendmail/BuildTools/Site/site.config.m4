dnl Use tcpwrappers and Hesiod
APPENDDEF(`confENVDEF', `-DTCPWRAPPERS')
APPENDDEF(`confINCDIRS', `-I$(ATHTOOLROOT)/usr/athena/include')
APPENDDEF(`confLIBDIRS', `-L$(ATHTOOLROOT)/usr/athena/lib')
APPENDDEF(`confLIBS', `-lwrap -lhesiod')

dnl Avoid having NIS be defined
define(`confMAPDEF', `-DNDBM -DMAP_REGEX')

dnl Debugging, not optimizing
define(`confOPTIMIZE', `-g')

dnl Install man source, not cat files
define(`confMANROOT', `/usr/athena/man/man')
define(`confMAN1SRC', 1)
define(`confMAN5SRC', 5)
define(`confMAN8SRC', 8)

dnl Install random binaries in /usr/athena to save local disk space
define(`athEBINDIR', `/usr/athena/libexec')
define(`athSBINDIR', `/usr/athena/etc')

dnl Use Athena install; the BuildTools one can't handle non-writable files.
define(`confINSTALL', `install')
