#include "mit-copyright.h"
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>

/*			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version).
 * NOTE: this code is taken from 4.3bsd ping, and modified to read a file.
 *
 */
unsigned
in_cksum( filename, statp) char *filename; struct stat *statp;
{
	int fd;
	struct stat statbuf;
	static char buf[ 8192];
	register int nleft;
	register u_short *w;
	register unsigned int sum = 0;
	unsigned int carry = 0;

	fd = open( filename, O_RDONLY);

	/* get the file's optimal blocksize:
	 * we want to call read() as little as possible,
	 * because each call spills the register variables.
	 */
	if ( ! statp) {
		statp = &statbuf;
		fstat( fd, statp);
	}
	/*
	 * Our algorithm is simple, using a 48 bit accumulator (sum),
	 * we add sequential 16 bit words to it, and at the end, fold
	 * back all the carry bits from the top 32 bits into the lower
	 * 16 bits.
	 * this means we can handle files containing 2**32 16-bit words,
	 * that is, 2**33 bytes, which is a little more than 8 gigabytes.
	 */
	while ( 0 < ( nleft = read( fd, buf, (*statp).st_blksize))) {
		w = (u_short *) buf;
		while( nleft > 1 )  {
			sum += *w++;
			nleft -= 2;
		}

		/* mop up an odd byte, if necessary */
		if( nleft == 1 ) {
			((u_char *)w)[ 1] = '\0';
			sum += *w;
		}
		carry += sum >> 16;
		sum &= 0xffff;
	}
	close( fd);

	/*
	 * add back carry's top 16 bits and low 16 bits.
	 * this makes this checksum byte-order independent.
	 */
	sum = sum + (carry >> 16) + (carry & 0xffff);
	sum += (sum >> 16);			/* add resulting carry */
	return( htons( 0xffff & ~sum));		/* discard last carry. */
	/* the ~ (one's complement) isn't really necessary here;
	 * it makes a nonzero checksum for a file of zeroes,
	 * which is useful for network applications of this algorithm.
	 */
}
