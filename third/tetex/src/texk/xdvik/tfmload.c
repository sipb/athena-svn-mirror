/* This code was stolen from dvips for xdvi use.  The file itself did
 * not carry any licensing info.  According to other filed the code
 * either public domain, or, in the worst case, GPL.
 *
 * Loads a tfm file.  It marks the characters as undefined.
 *
 * Modified for use by xdvi-t1 by Nicolai Langfeldt */

#include "xdvi-config.h"

#ifdef T1LIB

#include "kpathsea/tex-file.h"

int fallbacktfm=0;

int tfmload(char *name, long *design, long *widths)
{
    int i;
    int li;
    int nw, hd;
    int bc, ec;
    long wtab[256];
    unsigned short chardat[256];
    char *filename;
    FILE *curfnt;

    fallbacktfm=0;

    filename = kpse_find_tfm(name);

    if (filename == NULL) {
      filename = kpse_find_tfm("cmr10.tfm");
      if (fallbacktfm==0) fallbacktfm=1;
    }

    if (filename == NULL)
	return 0;

    curfnt = fopen(filename, "r");

    if (curfnt == NULL)
	return 0;

#if 0
    fprintf(stderr, "Loading metrics from %s\n", filename);
#endif

    /* Next, we read the font data from the tfm file, and store it in
     * our own array.  */

    /* Lengths section, 16 bit unsigned quanities */
    li = two(curfnt);	/* lf = length of file, in words(4 bytes) */
    hd = two(curfnt);	/* lh = length of header data, in words */
    bc = two(curfnt);	/* bc = lowest charcode */
    ec = two(curfnt);	/* ec = largest charcode */
    nw = two(curfnt);	/* nw = number of width words */
    two(curfnt);	/* nh = number of height words */
    two(curfnt);	/* nd = number of depth words */
    two(curfnt);	/* ni = number of italic words */
    two(curfnt);	/* nl = number of words in lig/kern table */
    two(curfnt);	/* nk = number of words in the kern table */
    two(curfnt);	/* ne = number of words in the extensible char tab */
    two(curfnt);	/* np = number of font parameter words */

    /* Header, 32 bit signed quantities */
    four(curfnt);	/* header[0]: checksum */
    *design = four(curfnt);	/* header[1]: design size */

    /* Skip the rest: header[2..(hd-1)] */
    for (i = 2; i < hd; i++)
	li = four(curfnt);

    /* Initialize to undef value */
    for (i = 0; i < 256; i++) {
	chardat[i] = 256;
	widths[i] = 0;
    }

    /* The charinfo array: */
    for (i = bc; i <= ec; i++) {
	chardat[i] = one(curfnt);	/* 8 bits of widht index */
	one(curfnt);	/* skip */
	two(curfnt);	/* skip, jump */
    }

    /* The (compressed!) width table */
    for (i = 0; i < nw; i++)
	wtab[i] = four(curfnt);

    (void)fclose(curfnt);

    /* Now whe have the table and the indecies.  Expand into a directly
       mapped width array */
    for (i = bc; i <= ec; i++)
	if (chardat[i] != 256)
	    widths[i] = wtab[chardat[i]];

    return 1;
}

#endif /* T1LIB */
