/* 
 * Stand alone function to read the disk info. 
 * Sets up global arrays dr_name and dr_unit as declared below.
 * This function requires 'int kmem' to be set to the open descriptor
 * for /dev/kmem.  The array 'namelist' should be initailized, with 
 * the proper X_MBDINIT all set.
*/

#include "mon.h"
/* these files should be included somehow
	#include <stdio.h>
	#include <sys/param.h>
	#include <sys/vm.h>
	#include <sys/dk.h>
	#include <nlist.h>
*/
#include <sys/buf.h>
#ifdef vax
#include <vaxuba/ubavar.h>
#include <vaxmba/mbavar.h>
#endif
#ifdef sun
#include <sundev/mbvar.h>
#endif
#ifdef ibm032
#include <caio/ioccvar.h>
#endif

extern char dr_name[DK_NDRIVE][10];
extern char dr_unit[DK_NDRIVE];
extern int kmem;
extern struct nlist namelist[] ;


/* Where kmem is the open file descriptor describing /dev/kmem */
#define steal(where, var) lseek(kmem, where, 0); read(kmem, &var, sizeof var);

/*
 * Read the drive names out of kmem.
 */
#ifdef vax
read_names()
{
	struct mba_device mdev;
	register struct mba_device *mp;
	struct mba_driver mdrv;
	short two_char;
	char *cp = (char *) &two_char;
	struct uba_device udev, *up;
	struct uba_driver udrv;

	mp = (struct mba_device *) namelist[X_MBDINIT].n_value;
	up = (struct uba_device *) namelist[X_UBDINIT].n_value;
	if (up == 0) {
		fprintf(stderr, "vmstat: Disk init info not in namelist\n");
		exit(1);
	}
	if (mp) for (;;) {
		steal(mp++, mdev);
		if (mdev.mi_driver == 0)
			break;
		if (mdev.mi_dk < 0 || mdev.mi_alive == 0)
			continue;
		steal(mdev.mi_driver, mdrv);
		steal(mdrv.md_dname, two_char);
		sprintf(dr_name[mdev.mi_dk], "%c%c", cp[0], cp[1]);
		dr_unit[mdev.mi_dk] = mdev.mi_unit;
	}
	for (;;) {
		steal(up++, udev);
		if (udev.ui_driver == 0)
			break;
		if (udev.ui_dk < 0 || udev.ui_alive == 0)
			continue;
		steal(udev.ui_driver, udrv);
		steal(udrv.ud_dname, two_char);
		sprintf(dr_name[udev.ui_dk], "%c%c", cp[0], cp[1]);
		dr_unit[udev.ui_dk] = udev.ui_unit;
	}
}
#endif

#ifdef sun
read_names()
{
	struct iocc_device mdev;
	register struct iocc_device *mp;
	struct iocc_driver mdrv;
	short two_char;
	char *cp = (char *) &two_char;

	mp = (struct iocc_device *) namelist[X_MBDINIT].n_value;
	if (mp == 0) {
		fprintf(stderr, "iostat: Disk init info not in namelist\n");
		exit(1);
	}
	for (;;) {
		steal(mp++, mdev);
		if (mdev.md_driver == 0)
			break;
		if (mdev.md_dk < 0 || mdev.md_alive == 0)
			continue;
		steal(mdev.md_driver, mdrv);
		steal(mdrv.mdr_dname, two_char);
		sprintf(dr_name[mdev.md_dk], "%c%c%d", cp[0], cp[1]);
		dr_unit[mdev.md_dk] = mdev.md_unit;
	}
}
#endif

#ifdef ibm032
read_names()
{
	struct iocc_device iod;
	register struct iocc_device *mp;
	struct iocc_driver mdrv;
	union {
	short two_short;
	char two_char[2];
	} two;
	char *cp =  two.two_char;

	mp = (struct iocc_device *) namelist[X_MBDINIT].n_value;
	if (mp == 0) {
		fprintf(stderr, "iostat: Disk init info not in namelist\n");
		exit(1);
	}
	for (;;) {
		steal(mp++, iod);
		if (iod.iod_driver == 0)
			break;
		/* if (debug)
			printf("dk=%d alive=%d ctlr=%d\n",iod.iod_dk,
				iod.iod_alive,iod.iod_ctlr);
		*/
		if (iod.iod_dk < 0 || iod.iod_alive == 0)
			continue;
		if (iod.iod_ctlr < 0)
			continue;
		steal(iod.iod_driver, mdrv);
		steal(mdrv.idr_dname, two.two_short);
		sprintf(dr_name[iod.iod_dk], "%c%c%d", cp[0], cp[1],
			dr_unit[iod.iod_dk] = iod.iod_unit);
	}
}
#endif
