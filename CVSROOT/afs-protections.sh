#!/bin/sh

genacl="read:source read adm:source la"

case "$1" in
	repository)
		genacl="$genacl write:staff write"
		;;
	wd)
		genacl="$genacl read:staff read write:update write"
		;;
	*)
		echo "Usage: $0 {repository|wd}"
		exit 1
		;;
esac

public="$genacl system:anyuser read"
fascist="$genacl system:anyuser l"
auth="$genacl system:anyuser l system:authuser read"

attach -h -n -q gnu
`athdir /mit/gnu bin`/gfind . -noleaf \
	-path ./athena/bin/lpr -prune -o \
	-path ./athena/bin/quote -prune -o \
	-path ./athena/bin/login -prune -o \
	-path ./athena/bin/write -prune -o \
	-path ./athena/bin/voldump -prune -o \
	-path ./athena/lib/gdss/lib -prune -o \
	-path ./athena/etc/synctree -prune -o \
	-path ./athena/etc/ftpd -prune -o \
	-type d -print | xargs fs sa -acl $public -clear -dir

find athena/bin/lpr -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/bin/quota -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/bin/login -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/bin/write -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/bin/voldump -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/lib/gdss/lib -type d -print | xargs fs sa -acl $fascist -clear -dir
find athena/etc/synctree -type d -print | xargs fs sa -acl $auth -clear -dir
find athena/etc/ftpd -type d -print | xargs fs sa -acl $fascist -clear -dir
