/*   
 *   This file is a part of dvipsk distribution.
 *   It is (C) Copyright 1986-94 by Tomas Rokicki.
 *
 *   Modified for use with pdftex by Han The Thanh <thanh@fi.muni.cz>.
 */

/*
 *   This function calculates approximately (whole + num/den) * sf.
 *   No need for real extreme accuracy; one twenty thousandth of an
 *   inch should be sufficient.
 *
 *   No `sf' parameter means to use an old one; inches are assumed
 *   originally.
 *
 *   Assumptions:
 *
 *      0 <= num < den <= 20000
 *      0 <= whole
 */

#include "libpdftex.h"
extern boolean true_dimen;

static long scale(long whole, long num, long den, long sf)
{
   long v ;

   v = whole * sf + num * (sf / den) ;
   if (v / sf != whole || v < 0 || v > 0x40000000L)
      pdftex_warn("arithmetic overflow in parameter") ;
   sf = sf % den ;
   v += (sf * num * 2 + den) / (2 * den) ;
   return(v) ;
}
/*
 *   Convert a sequence of digits into a long; return -1 if no digits.
 *   Advance the passed pointer as well.
 */
integer myatol(char **s)
{
   register char *p ;
   register long result ;

   result = 0 ;
   p = *s ;
   while ('0' <= *p && *p <= '9') {
      if (result > 100000000)
         pdftex_warn("arithmetic overflow in parameter") ;
      result = 10 * result + *p++ - '0' ;
   }
   if (p == *s) {
      pdftex_warn("expected number in %s returning 10", *s) ;
      return 10 ;
   } else {
      *s = p ;
      return(result) ;
   }
}
/*
 *   Get a dimension, allowing all the various extensions, and
 *   defaults.  Returns a value in scaled points.
 */
static long scalevals[] = { 1864680L, 65536L, 786432L, 186468L,
                            1L, 65782L, 70124L, 841489L, 4736286L } ;
static char *scalenames = "cmptpcmmspbpddccin" ;
integer myatodim(char **s)
{
   register long w, num, den, sc ;
   register char *q ;
   char *p ;
   int negative = 0, i ;

   p = *s ;
   if (**s == '-') {
      p++ ;
      negative = 1 ;
   }
   w = myatol(&p) ;
   if (w < 0) {
      pdftex_warn("number too large; 1000 used") ;
      w = 1000 ;
   }
   num = 0 ;
   den = 1 ;
   if (*p == '.') {
      p++ ;
      while ('0' <= *p && *p <= '9') {
         if (den <= 1000) {
            den *= 10 ;
            num = num * 10 + *p - '0' ;
         } else if (den == 10000) {
            den *= 2 ;
            num = num * 2 + (*p - '0') / 5 ;
         }
         p++ ;
      }
   }
   while (*p == ' ')
      p++ ;
   true_dimen = false;
   if (strncmp(p, "true", strlen("true")) == 0) {
     true_dimen = true;
     p += strlen("true");
     while (*p == ' ')
        p++ ;
   }
   for (i=0, q=scalenames; ; i++, q += 2)
      if (*q == 0) {
         pdftex_warn("expected units in %s, assuming inches.", *s);
         sc = scalevals[8] ;
         break ;
      } else if (*p == *q && p[1] == q[1]) {
         sc = scalevals[i] ;
         p += 2 ;
         break ;
      }
   w = scale(w, num, den, sc) ;
   *s = p ;
   return(negative?-w:w) ;
}
