; $Header: /afs/dev.mit.edu/source/repository/packs/config/named.boot,v 1.4 1997-05-08 00:42:56 ghudson Exp $

directory /etc

; As of release 8.1, the named.local hack will not work on Suns (it
; still works on SGIs).  So here's the "correct" way to make local
; records:
;
;	* Add a line to this file reading:
;
;		primary		foo.bar.baz	named.something
;
;	  where foo.bar.baz is the local record you want to make and
;	  named.something is a filename.  For example:
;
;		primary		zephyr.sloc.ns.athena.mit.edu	named.zephyr
;
;	* In /etc/named.something, put:
;
;		@	IN	SOA	bitsy.mit.edu. yourusername.mit.edu. (
;						1	; Serial
;						3600	; Refresh 1 hour
;						300	; Retry 5 minutes
;						2419200	; Expire 28 days
;						3600000	; Minimum 1000 hours
;						)
;
;	  substituting your username for "yourusername."  After the SOA
;	  record, put the local record(s) themselves, without specifying
;	  the name, e.g.:
;
;				TXT	"erato.mit.edu"
;				TXT	"arilinn.mit.edu"
;
;	* Restart named.

; Type		Domain			Source host/file	Backup file
cache		.			named.root
primary		localhost		named.localhost
primary		1.0.0.127.in-addr.arpa	named.localhost.rev
