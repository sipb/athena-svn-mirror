/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmResCvt.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:35 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void AddWmResourceConverters ();
extern void WmCvtStringToCFocus ();
extern void WmCvtStringToCDecor ();
extern void WmCvtStringToCFunc ();
extern void WmCvtStringToIDecor ();
extern void WmCvtStringToIPlace ();
extern void WmCvtStringToKFocus ();
extern void WmCvtStringToSize ();
extern void WmCvtStringToShowFeedback ();
extern void WmCvtStringToUsePPosition ();
extern unsigned char *NextToken ();
extern Boolean StringsAreEqual ();
#else /* _NO_PROTO */
extern void AddWmResourceConverters (void);
extern void WmCvtStringToCFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToCDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToCFunc (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToIDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToIPlace (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToKFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToSize (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToShowFeedback (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern void WmCvtStringToUsePPosition (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
extern unsigned char *NextToken (unsigned char *pchIn, int *pLen, unsigned char **ppchNext);
extern Boolean StringsAreEqual (unsigned char *pch1, unsigned char *pch2, int len);
#endif /* _NO_PROTO */
