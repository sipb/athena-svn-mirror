/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/readnames.c,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/readnames.c,v 1.1 1984-12-13 12:01:24 dgg Exp $
 */

#ifndef lint
static char *rcsid_readnames_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/readnames.c,v 1.1 1984-12-13 12:01:24 dgg Exp $";
#endif	lint

/*
 *      R E A D N A M E S
 * 
 * purpose: Reads the names of the disks from kernel space
 *
 */
#include "mon.h"
#include <sys/buf.h>		/* needed by following two includes */
#include <vaxuba/ubavar.h>	/* unibus adapters */
#include <vaxmba/mbavar.h>	/* massbus adapters */

#define steal(where, var) lseek(kmem, where, 0); read(kmem, &var, sizeof var);

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
        if (mp) for (;;) {
                steal(mp++, mdev);
                if (mdev.mi_driver == 0)
                        break;
                if (mdev.mi_dk < 0 || mdev.mi_alive == 0)
                        continue;
                steal(mdev.mi_driver, mdrv);
                steal(mdrv.md_dname, two_char);
                sprintf(dr_name[mdev.mi_dk], "%c%c%d", cp[0], cp[1], mdev.mi_unit);
                dr_unit[mdev.mi_dk] = mdev.mi_unit;
        }
        if (up) for (;;) {
                steal(up++, udev);
                if (udev.ui_driver == 0)
                        break;
                if (udev.ui_dk < 0 || udev.ui_alive == 0)
                        continue;
                steal(udev.ui_driver, udrv);
                steal(udrv.ud_dname, two_char);
                sprintf(dr_name[udev.ui_dk], "%c%c%d", cp[0], cp[1], udev.ui_unit);
                dr_unit[udev.ui_dk] = udev.ui_unit;
        }
}
