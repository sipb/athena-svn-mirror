$ !                        COPYRIGHT (C) 1989 BY
$ !                DIGITAL EQUIPMENT CORPORATION, MAYNARD
$ !                  MASSACHUSETTS. ALL RIGHTS RESERVED.
$ !
$ !  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
$ !  ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH THE INCLUSION
$ !  OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY OTHER COPIES
$ !  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER
$ !  PERSON. NO TITLE TO AND OWNERSHIP OF THE SOFTWARE IS  HEREBY TRANSFERRED.
$ !
$ !  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
$ !  SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
$ !
$ !  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR RELIABILITY OF ITS
$ !  SOFTWARE ON EQUIPMENT THAT IS NOT SUPPLIED BY DIGITAL.
$ !
$ !*****************************************************************************
$!
$! Modification history
$! 
$! 000  27-Mar-89 Hsin Lee lhh
$!
$ write sys$output "Making LIBUCX"
$
$ if f$search("ucx.dir") .eqs. "" then create/dir [.ucx]
$
$ cc_defines := ucx, ucxdbg
$ @bld1 socket_inet     libnck  [.ucx]
$ @bld1 vms_ucx         libucx  [.ucx]
$ @bld1 vms_debug       libucx  [.ucx]
