; $Header: /afs/dev.mit.edu/source/repository/packs/config/named.boot,v 1.2 1996-09-20 05:23:51 ghudson Exp $

directory /etc

; type    domain                source host/file                backup file
cache	  .			named.root
primary	  127.in-addr-arpa	named.localhost.rev
