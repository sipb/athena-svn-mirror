/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/netif.c,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/netif.c,v 1.1 1984-12-13 12:01:10 dgg Exp $
 */

#ifndef lint
static char *rcsid_netif_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/netif.c,v 1.1 1984-12-13 12:01:10 dgg Exp $";
#endif	lint

/*
 *			N E T I F . C
 *
 *  This section of mon handles the network interfaces.
 *
 *	nifinit()	initializes "static" info on interfaces.
 *	nifupdate()	gets "dynamic" info on net interfaces.
 */
#include "mon.h"
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

char	*index();

/*
 * NIFINIT - initialize the static network interfaces info such
 *   as device names, addresses, etc.  Also calls nifupdate() once
 *   to set up initial values for data with running totals.
 */
nifinit()
{
	off_t	firstifnet;		/* offset to first ifnet struct */
	off_t	ifnetp;			/* points to ifnet struct in kmem */
	struct ifnet ifnet;
	struct nifinfo *nifip;

	/* check for valid kernal offset */
	if (namelist[N_IFNET].n_type == NULL) {
		fprintf(stderr, "mon: ifnet symbol not defined!\n");
		return;
	}

	/* get starting address of ifnet structure chain */
	lseek(kmem, namelist[N_IFNET].n_value, 0);
	read(kmem, &firstifnet, sizeof(firstifnet));

	/* get info for each interface in chain */
	numif = 0;			/* count number of interfaces */
	ifnetp = firstifnet;
	nifip = &nifinfo[0];
	while (ifnetp) {
		char	*cp;

		numif++;
		/* get an ifnet entry */
		lseek(kmem, (long)ifnetp, 0);
		read(kmem, &ifnet, sizeof(ifnet));

		/* build its interface name */
		lseek(kmem, (long)ifnet.if_name, 0);
		read(kmem, nifip->name, 15);
		nifip->name[15] = '\0';
		cp = index(nifip->name, '\0');
		sprintf(cp, "%d", ifnet.if_unit);

		/* other info comes here */

		if (++nifip >= &nifinfo[MAXIF])
			break;		/* no more room for interfaces */

		ifnetp = (off_t)ifnet.if_next;	/* link to next ifnet struct */
	}

	nifupdate();	/* init running totals */
}

/*
 * NIFUPDATE - gets the "dynamic" info about the network interfaces.
 */
nifupdate()
{
	off_t	ifnetp;		/* pointer to ifnet struct in kmem */
	struct	ifnet	ifnet;
	int	i;

	if (namelist[N_IFNET].n_type == NULL)
		return;		/* no net interfaces */

	/* get info from ifnet struct chain */
	lseek(kmem, namelist[N_IFNET].n_value, 0);
	read(kmem, &ifnetp, sizeof(ifnetp));
	i = 0;
	while (ifnetp) {
		lseek(kmem, (long)ifnetp, 0);
		read (kmem, &ifnet, sizeof(ifnet));

		/* extract interval data from ifnet and update totals */
		nifdat[i].ipackets = ifnet.if_ipackets - niftot[i].ipackets;
		niftot[i].ipackets = ifnet.if_ipackets;
		nifdat[i].ierrors = ifnet.if_ierrors - niftot[i].ierrors;
		niftot[i].ierrors = ifnet.if_ierrors;
		nifdat[i].opackets = ifnet.if_opackets - niftot[i].opackets;
		niftot[i].opackets = ifnet.if_opackets;
		nifdat[i].oerrors = ifnet.if_oerrors - niftot[i].oerrors;
		niftot[i].oerrors = ifnet.if_oerrors;
		nifdat[i].collisions = ifnet.if_collisions - niftot[i].collisions;
		niftot[i].collisions = ifnet.if_collisions;
		/* extract nifinfo (other stuff) data from ifnet */
		/* OTHER STUFF OF INTEREST HERE */
		nifinfo[i].outqlen = ifnet.if_snd.ifq_len;

		if (++i >= MAXIF)
			break;		/* no more room for interfaces */

		ifnetp = (off_t)ifnet.if_next;	/* link to next ifnet struct */
	}
}
