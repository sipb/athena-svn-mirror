section     .text
global      _start

_start:

    mov     edx,len
    mov     ecx,msg
    mov     ebx,2
    mov     eax,4
    int     0x80

    mov     ebx,1
    mov     eax,1
    int     0x80

section     .data

msg     db  'ERROR: The program you are trying to run is too old for this system.',0xa,'       (libc5 not available)',0xa,0x0
len     equ $ - msg
