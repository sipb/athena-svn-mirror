#ifndef uuid__included
#define uuid__included
#include "idl_base.h"
#include "nbase.h"
typedef ndr_$char uuid_$string_t[37];
extern  void uuid_$gen
#ifdef __STDC__
 (
  /* [out] */uuid_$t *uuid);
#else
 ( );
#endif
extern  void uuid_$encode
#ifdef __STDC__
 (
  /* [in] */uuid_$t *uuid,
  /* [out] */uuid_$string_t s);
#else
 ( );
#endif
extern  void uuid_$decode
#ifdef __STDC__
 (
  /* [in] */uuid_$string_t s,
  /* [out] */uuid_$t *uuid,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$boolean uuid_$equal
#ifdef __STDC__
 (
  /* [in] */uuid_$t *u1,
  /* [in] */uuid_$t *u2);
#else
 ( );
#endif
extern  ndr_$long_int uuid_$cmp
#ifdef __STDC__
 (
  /* [in] */uuid_$t *u1,
  /* [in] */uuid_$t *u2);
#else
 ( );
#endif
extern  ndr_$ulong_int uuid_$hash
#ifdef __STDC__
 (
  /* [in] */uuid_$t *u,
  /* [in] */ndr_$ulong_int modulus);
#else
 ( );
#endif
#endif
