#!/bin/sh

genacl="write:source write read:source read adm:source la read:staff read"
genacl="$genacl system:anyuser read"
fascist="system:anyuser l"
lessfascist="system:anyuser l system:authuser read"

find . -type d -print | xargs fs sa -acl $genacl -clear -dir
find athena/bin/lpr -type d -print | xargs fs sa -acl $fascist -dir
find athena/bin/quota -type d -print | xargs fs sa -acl $fascist -dir
find athena/bin/login -type d -print | xargs fs sa -acl $fascist -dir
find athena/bin/write -type d -print | xargs fs sa -acl $fascist -dir
find athena/bin/voldump -type d -print | xargs fs sa -acl $fascist -dir
find athena/lib/gdss/lib -type d -print | xargs fs sa -acl $fascist -dir
find athena/etc/synctree -type d -print | xargs fs sa -acl $lessfascist -dir
find athena/etc/ftpd -type d -print | xargs fs sa -acl $fascist -dir
