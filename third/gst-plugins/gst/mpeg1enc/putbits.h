/* putbits.h, bit-level output                                              */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

typedef struct _putbits_t putbits_t;

struct _putbits_t {
  unsigned char *outbfr;
  unsigned char *outbase;
  unsigned char temp;
  int outcnt;
  int bytecnt;
  int len;
  int newlen;
};

void putbits_init(putbits_t *pb);
void putbits_new_empty_buffer(putbits_t *pb, int len);
void putbits_new_buffer(putbits_t *pb, unsigned char *buffer, int len);
void putbits(putbits_t *pb, int val, int n);
void alignbits(putbits_t *pb);
int bitcount(putbits_t *pb);
