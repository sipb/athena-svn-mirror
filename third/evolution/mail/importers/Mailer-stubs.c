/*
 * This file was generated by orbit-idl-2 - DO NOT EDIT!
 */

#include <string.h>
#define ORBIT2_STUBS_API
#include "Mailer.h"

void
GNOME_Evolution_FolderInfo_getInfo(GNOME_Evolution_FolderInfo _obj,
				   const CORBA_char * foldername,
				   const Bonobo_Listener listener,
				   CORBA_Environment * ev)
{
   POA_GNOME_Evolution_FolderInfo__epv *_ORBIT_epv;

   if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS &&
       ORBIT_STUB_IsBypass(_obj, GNOME_Evolution_FolderInfo__classid) &&
       (_ORBIT_epv =
	(POA_GNOME_Evolution_FolderInfo__epv *) ORBIT_STUB_GetEpv(_obj,
								  GNOME_Evolution_FolderInfo__classid))->
       getInfo) {
      ORBIT_STUB_PreCall(_obj);
      _ORBIT_epv->getInfo(ORBIT_STUB_GetServant(_obj), foldername, listener,
			  ev);
      ORBIT_STUB_PostCall(_obj);
   } else {			/* remote marshal */
      gpointer _args[2];

      _args[0] = (gpointer) & foldername;
      _args[1] = (gpointer) & listener;
      ORBit_small_invoke_stub_n(_obj,
				&GNOME_Evolution_FolderInfo__iinterface.
				methods, 0, NULL, _args, NULL, ev);

   }
}
void
GNOME_Evolution_MailConfig_addAccount(GNOME_Evolution_MailConfig _obj,
				      const GNOME_Evolution_MailConfig_Account
				      * acc, CORBA_Environment * ev)
{
   POA_GNOME_Evolution_MailConfig__epv *_ORBIT_epv;

   if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS &&
       ORBIT_STUB_IsBypass(_obj, GNOME_Evolution_MailConfig__classid) &&
       (_ORBIT_epv =
	(POA_GNOME_Evolution_MailConfig__epv *) ORBIT_STUB_GetEpv(_obj,
								  GNOME_Evolution_MailConfig__classid))->
       addAccount) {
      ORBIT_STUB_PreCall(_obj);
      _ORBIT_epv->addAccount(ORBIT_STUB_GetServant(_obj), acc, ev);
      ORBIT_STUB_PostCall(_obj);
   } else {			/* remote marshal */
      gpointer _args[1];

      _args[0] = (gpointer) acc;
      ORBit_small_invoke_stub_n(_obj,
				&GNOME_Evolution_MailConfig__iinterface.
				methods, 0, NULL, _args, NULL, ev);

   }
}
void
GNOME_Evolution_MailConfig_removeAccount(GNOME_Evolution_MailConfig _obj,
					 const CORBA_char * name,
					 CORBA_Environment * ev)
{
   POA_GNOME_Evolution_MailConfig__epv *_ORBIT_epv;

   if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS &&
       ORBIT_STUB_IsBypass(_obj, GNOME_Evolution_MailConfig__classid) &&
       (_ORBIT_epv =
	(POA_GNOME_Evolution_MailConfig__epv *) ORBIT_STUB_GetEpv(_obj,
								  GNOME_Evolution_MailConfig__classid))->
       removeAccount) {
      ORBIT_STUB_PreCall(_obj);
      _ORBIT_epv->removeAccount(ORBIT_STUB_GetServant(_obj), name, ev);
      ORBIT_STUB_PostCall(_obj);
   } else {			/* remote marshal */
      gpointer _args[1];

      _args[0] = (gpointer) & name;
      ORBit_small_invoke_stub_n(_obj,
				&GNOME_Evolution_MailConfig__iinterface.
				methods, 1, NULL, _args, NULL, ev);

   }
}
void
GNOME_Evolution_MailFilter_addFilter(GNOME_Evolution_MailFilter _obj,
				     const CORBA_char * rule,
				     CORBA_Environment * ev)
{
   POA_GNOME_Evolution_MailFilter__epv *_ORBIT_epv;

   if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS &&
       ORBIT_STUB_IsBypass(_obj, GNOME_Evolution_MailFilter__classid) &&
       (_ORBIT_epv =
	(POA_GNOME_Evolution_MailFilter__epv *) ORBIT_STUB_GetEpv(_obj,
								  GNOME_Evolution_MailFilter__classid))->
       addFilter) {
      ORBIT_STUB_PreCall(_obj);
      _ORBIT_epv->addFilter(ORBIT_STUB_GetServant(_obj), rule, ev);
      ORBIT_STUB_PostCall(_obj);
   } else {			/* remote marshal */
      gpointer _args[1];

      _args[0] = (gpointer) & rule;
      ORBit_small_invoke_stub_n(_obj,
				&GNOME_Evolution_MailFilter__iinterface.
				methods, 0, NULL, _args, NULL, ev);

   }
}
void
GNOME_Evolution_MailFilter_removeFilter(GNOME_Evolution_MailFilter _obj,
					const CORBA_char * rule,
					CORBA_Environment * ev)
{
   POA_GNOME_Evolution_MailFilter__epv *_ORBIT_epv;

   if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS &&
       ORBIT_STUB_IsBypass(_obj, GNOME_Evolution_MailFilter__classid) &&
       (_ORBIT_epv =
	(POA_GNOME_Evolution_MailFilter__epv *) ORBIT_STUB_GetEpv(_obj,
								  GNOME_Evolution_MailFilter__classid))->
       removeFilter) {
      ORBIT_STUB_PreCall(_obj);
      _ORBIT_epv->removeFilter(ORBIT_STUB_GetServant(_obj), rule, ev);
      ORBIT_STUB_PostCall(_obj);
   } else {			/* remote marshal */
      gpointer _args[1];

      _args[0] = (gpointer) & rule;
      ORBit_small_invoke_stub_n(_obj,
				&GNOME_Evolution_MailFilter__iinterface.
				methods, 1, NULL, _args, NULL, ev);

   }
}