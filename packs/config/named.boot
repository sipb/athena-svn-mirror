; $Header: /afs/dev.mit.edu/source/repository/packs/config/named.boot,v 1.3 1996-09-25 17:49:05 ghudson Exp $

directory /etc

; Type		Domain			Source host/file	Backup file
cache		.			named.root
primary		localhost		named.localhost
primary		1.0.0.127.in-addr.arpa	named.localhost.rev
