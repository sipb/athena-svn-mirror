/*
 * This file was generated by orbit-idl-2 - DO NOT EDIT!
 */

#include <string.h>
#define ORBIT2_STUBS_API
#define ORBIT_IDL_C_COMMON
#define GNOME_Media_CDDBSlave2_COMMON
#include "GNOME_Media_CDDBSlave2.h"

static const CORBA_unsigned_long ORBit_zero_int = 0;

#if ( (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBTrackEditor_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBTrackEditor)
#define TC_DEF_TC_GNOME_Media_CDDBTrackEditor 1
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct TC_GNOME_Media_CDDBTrackEditor_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_objref,
   0,
   0,
   4,
   0,
   0,
   NULL,
   CORBA_OBJECT_NIL,
   "CDDBTrackEditor",
   "IDL:GNOME/Media/CDDBTrackEditor:1.0",
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_Result_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2_Result)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2_Result 1
static const char *anon_subnames_array3[] =
   { "OK", "REQUEST_PENDING", "ERROR_CONTACTING_SERVER",
"ERROR_RETRIEVING_DATA", "MALFORMED_DATA", "IO_ERROR" };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct TC_GNOME_Media_CDDBSlave2_Result_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_enum,
   0,
   0,
   4,
   0,
   6,
   NULL,
   CORBA_OBJECT_NIL,
   "Result",
   "IDL:GNOME/Media/CDDBSlave2/Result:1.0",
   (char **) anon_subnames_array3,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_QueryResult_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2_QueryResult)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2_QueryResult 1
static const char *anon_subnames_array6[] = { "discid", "result" };
static const CORBA_TypeCode anon_subtypes_array7[] =
   { (CORBA_TypeCode) & TC_CORBA_string_struct,
(CORBA_TypeCode) & TC_GNOME_Media_CDDBSlave2_Result_struct };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct
   TC_GNOME_Media_CDDBSlave2_QueryResult_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_struct,
   0,
   0,
   4,
   0,
   2,
   (CORBA_TypeCode *) anon_subtypes_array7,
   CORBA_OBJECT_NIL,
   "QueryResult",
   "IDL:GNOME/Media/CDDBSlave2/QueryResult:1.0",
   (char **) anon_subnames_array6,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackInfo_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2_TrackInfo)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2_TrackInfo 1
static const char *anon_subnames_array9[] = { "name", "length", "comment" };
static const CORBA_TypeCode anon_subtypes_array10[] =
   { (CORBA_TypeCode) & TC_CORBA_string_struct,
(CORBA_TypeCode) & TC_CORBA_short_struct, (CORBA_TypeCode) & TC_CORBA_string_struct };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct TC_GNOME_Media_CDDBSlave2_TrackInfo_struct
   = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_struct,
   0,
   0,
   4,
   0,
   3,
   (CORBA_TypeCode *) anon_subtypes_array10,
   CORBA_OBJECT_NIL,
   "TrackInfo",
   "IDL:GNOME/Media/CDDBSlave2/TrackInfo:1.0",
   (char **) anon_subnames_array9,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_0 == 'G') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_1 == 'N') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_2 == 'O') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_3 == 'M') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_4 == 'E') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_5 == '_') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_6 == 'M') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_7 == 'e') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_8 == 'd') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_9 == 'i') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_10 == 'a') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_11 == '_') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_12 == 'C') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_13 == 'D') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_14 == 'D') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_15 == 'B') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_16 == 'S') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_17 == 'l') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_18 == 'a') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_19 == 'v') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_20 == 'e') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_21 == '2') \
) && !defined(TC_DEF_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo)
#define TC_DEF_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo 1
static const CORBA_TypeCode anon_subtypes_array13[] =
   { (CORBA_TypeCode) & TC_GNOME_Media_CDDBSlave2_TrackInfo_struct };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct
   TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_sequence,
   0,
   0,
   4,
   0,
   1,
   (CORBA_TypeCode *) anon_subtypes_array13,
   CORBA_OBJECT_NIL,
   NULL,
   NULL,
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_0 == 'G') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_1 == 'N') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_2 == 'O') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_3 == 'M') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_4 == 'E') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_5 == '_') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_6 == 'M') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_7 == 'e') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_8 == 'd') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_9 == 'i') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_10 == 'a') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_11 == '_') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_12 == 'C') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_13 == 'D') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_14 == 'D') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_15 == 'B') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_16 == 'S') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_17 == 'l') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_18 == 'a') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_19 == 'v') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_20 == 'e') \
&& (TC_IMPL_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_21 == '2') \
) && !defined(TC_DEF_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo)
#define TC_DEF_TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo 1
static const CORBA_TypeCode anon_subtypes_array20[] =
   { (CORBA_TypeCode) & TC_GNOME_Media_CDDBSlave2_TrackInfo_struct };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct
   TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_sequence,
   0,
   0,
   4,
   0,
   1,
   (CORBA_TypeCode *) anon_subtypes_array20,
   CORBA_OBJECT_NIL,
   NULL,
   NULL,
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_TrackList_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2_TrackList)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2_TrackList 1
static const CORBA_TypeCode anon_subtypes_array23[] =
   { (CORBA_TypeCode) &
TC_CORBA_sequence_GNOME_Media_CDDBSlave2_TrackInfo_struct };
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct TC_GNOME_Media_CDDBSlave2_TrackList_struct
   = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_alias,
   0,
   0,
   4,
   0,
   1,
   (CORBA_TypeCode *) anon_subtypes_array23,
   CORBA_OBJECT_NIL,
   "TrackList",
   "IDL:GNOME/Media/CDDBSlave2/TrackList:1.0",
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_UnknownDiscID_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2_UnknownDiscID)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2_UnknownDiscID 1
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_except,
   0,
   0,
   1,
   0,
   0,
   NULL,
   CORBA_OBJECT_NIL,
   "UnknownDiscID",
   "IDL:GNOME/Media/CDDBSlave2/UnknownDiscID:1.0",
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif
#if ( (TC_IMPL_TC_GNOME_Media_CDDBSlave2_0 == 'G') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_1 == 'N') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_2 == 'O') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_3 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_4 == 'E') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_5 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_6 == 'M') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_7 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_8 == 'd') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_9 == 'i') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_10 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_11 == '_') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_12 == 'C') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_13 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_14 == 'D') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_15 == 'B') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_16 == 'S') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_17 == 'l') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_18 == 'a') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_19 == 'v') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_20 == 'e') \
&& (TC_IMPL_TC_GNOME_Media_CDDBSlave2_21 == '2') \
) && !defined(TC_DEF_TC_GNOME_Media_CDDBSlave2)
#define TC_DEF_TC_GNOME_Media_CDDBSlave2 1
#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
const struct CORBA_TypeCode_struct TC_GNOME_Media_CDDBSlave2_struct = {
   {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
   CORBA_tk_objref,
   0,
   0,
   4,
   0,
   0,
   NULL,
   CORBA_OBJECT_NIL,
   "CDDBSlave2",
   "IDL:GNOME/Media/CDDBSlave2:1.0",
   NULL,
   NULL,
   -1,
   0,
   0, 0
};
#endif

#ifndef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
CORBA_unsigned_long GNOME_Media_CDDBTrackEditor__classid = 0;
#endif

#ifndef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
CORBA_unsigned_long GNOME_Media_CDDBSlave2__classid = 0;
#endif

/* Interface type data */

static ORBit_IArg GNOME_Media_CDDBTrackEditor_setDiscID__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
ORBit_IMethod GNOME_Media_CDDBTrackEditor__imethods[] = {
   {
    {1, 1, GNOME_Media_CDDBTrackEditor_setDiscID__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_void, "setDiscID", 9,
    0}
   , {
      {0, 0, NULL, FALSE},
      {0, 0, NULL, FALSE},
      {0, 0, NULL, FALSE},
      TC_void, "showWindow", 10,
      0}
};
static CORBA_string GNOME_Media_CDDBTrackEditor__base_itypes[] = {
   "IDL:Bonobo/Unknown:1.0",
   "IDL:omg.org/CORBA/Object:1.0"
};

#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
ORBit_IInterface GNOME_Media_CDDBTrackEditor__iinterface = {
   TC_GNOME_Media_CDDBTrackEditor, {2, 2,
				    GNOME_Media_CDDBTrackEditor__imethods,
				    FALSE},
   {2, 2, GNOME_Media_CDDBTrackEditor__base_itypes, FALSE}
};

static ORBit_IArg GNOME_Media_CDDBSlave2_query__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_short, ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE, "ntrks"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "offsets"},
   {TC_CORBA_long, ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE, "nsecs"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "name"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "version"}
};
static ORBit_IArg GNOME_Media_CDDBSlave2_save__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_save__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getArtist__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getArtist__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setArtist__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "artist"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setArtist__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getDiscTitle__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getDiscTitle__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setDiscTitle__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "disctitle"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setDiscTitle__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getNTrks__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getNTrks__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getAllTracks__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_GNOME_Media_CDDBSlave2_TrackList, ORBit_I_ARG_OUT, "names"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getAllTracks__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setAllTracks__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_GNOME_Media_CDDBSlave2_TrackList, ORBit_I_ARG_IN, "names"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setAllTracks__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getComment__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getComment__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setComment__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "comment"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setComment__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getYear__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getYear__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setYear__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_short, ORBit_I_ARG_IN | ORBit_I_COMMON_FIXED_SIZE, "year"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setYear__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_getGenre__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_getGenre__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};
static ORBit_IArg GNOME_Media_CDDBSlave2_setGenre__arginfo[] = {
   {TC_CORBA_string, ORBit_I_ARG_IN, "discid"},
   {TC_CORBA_string, ORBit_I_ARG_IN, "genre"}
};

/* Exceptions */
static CORBA_TypeCode GNOME_Media_CDDBSlave2_setGenre__exceptinfo[] = {
   TC_GNOME_Media_CDDBSlave2_UnknownDiscID,
   NULL
};

#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
ORBit_IMethod GNOME_Media_CDDBSlave2__imethods[] = {
   {
    {6, 6, GNOME_Media_CDDBSlave2_query__arginfo, FALSE},
    {0, 0, NULL, FALSE},
    {0, 0, NULL, FALSE},
    TC_void, "query", 5,
    0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_save__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_save__exceptinfo, FALSE},
      TC_void, "save", 4,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getArtist__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getArtist__exceptinfo, FALSE},
      TC_CORBA_string, "getArtist", 9,
      0}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setArtist__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setArtist__exceptinfo, FALSE},
      TC_void, "setArtist", 9,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getDiscTitle__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getDiscTitle__exceptinfo, FALSE},
      TC_CORBA_string, "getDiscTitle", 12,
      0}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setDiscTitle__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setDiscTitle__exceptinfo, FALSE},
      TC_void, "setDiscTitle", 12,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getNTrks__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getNTrks__exceptinfo, FALSE},
      TC_CORBA_short, "getNTrks", 8,
      0 | ORBit_I_COMMON_FIXED_SIZE}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_getAllTracks__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getAllTracks__exceptinfo, FALSE},
      TC_void, "getAllTracks", 12,
      0}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setAllTracks__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setAllTracks__exceptinfo, FALSE},
      TC_void, "setAllTracks", 12,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getComment__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getComment__exceptinfo, FALSE},
      TC_CORBA_string, "getComment", 10,
      0}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setComment__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setComment__exceptinfo, FALSE},
      TC_void, "setComment", 10,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getYear__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getYear__exceptinfo, FALSE},
      TC_CORBA_short, "getYear", 7,
      0 | ORBit_I_COMMON_FIXED_SIZE}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setYear__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setYear__exceptinfo, FALSE},
      TC_void, "setYear", 7,
      0}
   , {
      {1, 1, GNOME_Media_CDDBSlave2_getGenre__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_getGenre__exceptinfo, FALSE},
      TC_CORBA_string, "getGenre", 8,
      0}
   , {
      {2, 2, GNOME_Media_CDDBSlave2_setGenre__arginfo, FALSE},
      {0, 0, NULL, FALSE},
      {1, 1, GNOME_Media_CDDBSlave2_setGenre__exceptinfo, FALSE},
      TC_void, "setGenre", 8,
      0}
};
static CORBA_string GNOME_Media_CDDBSlave2__base_itypes[] = {
   "IDL:Bonobo/Unknown:1.0",
   "IDL:omg.org/CORBA/Object:1.0"
};

#ifdef ORBIT_IDL_C_IMODULE_GNOME_Media_CDDBSlave2
static
#endif
ORBit_IInterface GNOME_Media_CDDBSlave2__iinterface = {
   TC_GNOME_Media_CDDBSlave2, {15, 15, GNOME_Media_CDDBSlave2__imethods,
			       FALSE},
   {2, 2, GNOME_Media_CDDBSlave2__base_itypes, FALSE}
};