; $Header: /afs/dev.mit.edu/source/repository/packs/config/named.boot,v 1.1 1993-10-12 05:02:03 probe Exp $

directory /etc

; type    domain                source host/file                backup file
cache	  .			named.root
primary	  cache.mit.edu		named.mit
