/*
 * This file was generated by orbit-idl - DO NOT EDIT!
 */

#include <string.h>
#include "Terminal.h"

void
_ORBIT_skel_GNOME_Terminal_TerminalFactory_create_terminal
   (POA_GNOME_Terminal_TerminalFactory * _ORBIT_servant,
    GIOPRecvBuffer * _ORBIT_recv_buffer, CORBA_Environment * ev,
    GNOME_Terminal_Terminal(*_impl_create_terminal) (PortableServer_Servant
						     _servant,
						     const CORBA_char *
						     geometry,
						     CORBA_Environment * ev))
{
   GNOME_Terminal_Terminal _ORBIT_retval;
   CORBA_char *geometry;

   {				/* demarshalling */
      guchar *_ORBIT_curptr;
      register CORBA_unsigned_long _ORBIT_tmpvar_2;
      CORBA_unsigned_long _ORBIT_tmpvar_3;

      _ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
      if (giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer))) {
	 _ORBIT_curptr = ALIGN_ADDRESS(_ORBIT_curptr, 4);
	 
	    (*((guint32 *) & (_ORBIT_tmpvar_3))) =
	    GUINT32_SWAP_LE_BE(*((guint32 *) _ORBIT_curptr));
	 _ORBIT_curptr += 4;
	 geometry = (void *) _ORBIT_curptr;
	 _ORBIT_curptr += sizeof(geometry[_ORBIT_tmpvar_2]) * _ORBIT_tmpvar_3;
      } else {
	 _ORBIT_curptr = ALIGN_ADDRESS(_ORBIT_curptr, 4);
	 _ORBIT_tmpvar_3 = *((CORBA_unsigned_long *) _ORBIT_curptr);
	 _ORBIT_curptr += 4;
	 geometry = (void *) _ORBIT_curptr;
	 _ORBIT_curptr += sizeof(geometry[_ORBIT_tmpvar_2]) * _ORBIT_tmpvar_3;
      }
   }
   _ORBIT_retval = _impl_create_terminal(_ORBIT_servant, geometry, ev);
   {				/* marshalling */
      register GIOPSendBuffer *_ORBIT_send_buffer;

      _ORBIT_send_buffer =
	 giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->
				    connection, NULL,
				    _ORBIT_recv_buffer->message.u.request.
				    request_id, ev->_major);
      if (_ORBIT_send_buffer) {
	 if (ev->_major == CORBA_NO_EXCEPTION) {
	    ORBit_marshal_object(_ORBIT_send_buffer, _ORBIT_retval);
	 } else
	    ORBit_send_system_exception(_ORBIT_send_buffer, ev);
	 giop_send_buffer_write(_ORBIT_send_buffer);
	 giop_send_buffer_unuse(_ORBIT_send_buffer);
      }
      if (ev->_major == CORBA_NO_EXCEPTION)
	 CORBA_Object_release(_ORBIT_retval, ev);
   }
}
static ORBitSkeleton
get_skel_GNOME_Terminal_Terminal(POA_GNOME_Terminal_Terminal * servant,
				 GIOPRecvBuffer * _ORBIT_recv_buffer,
				 gpointer * impl)
{
   gchar *opname = _ORBIT_recv_buffer->message.u.request.operation;

   switch (opname[0]) {
     default:
	break;
   }
   return NULL;
}

static void
init_local_objref_GNOME_Terminal_Terminal(CORBA_Object obj,
					  POA_GNOME_Terminal_Terminal *
					  servant)
{
   obj->vepv[GNOME_Terminal_Terminal__classid] =
      servant->vepv->GNOME_Terminal_Terminal_epv;
}

void
POA_GNOME_Terminal_Terminal__init(PortableServer_Servant servant,
				  CORBA_Environment * env)
{
   static const PortableServer_ClassInfo class_info =
      { (ORBit_impl_finder) & get_skel_GNOME_Terminal_Terminal,
	 "IDL:GNOME/Terminal/Terminal:1.0",
	 (ORBit_local_objref_init) & init_local_objref_GNOME_Terminal_Terminal

      };
   PortableServer_ServantBase__init(((PortableServer_ServantBase *) servant),
				    env);
   ORBIT_OBJECT_KEY(((PortableServer_ServantBase *) servant)->_private)->
      class_info = (PortableServer_ClassInfo *) & class_info;
   if (!GNOME_Terminal_Terminal__classid)
      GNOME_Terminal_Terminal__classid = ORBit_register_class(&class_info);
}

void
POA_GNOME_Terminal_Terminal__fini(PortableServer_Servant servant,
				  CORBA_Environment * env)
{
   PortableServer_ServantBase__fini(servant, env);
}

static ORBitSkeleton
get_skel_GNOME_Terminal_TerminalFactory(POA_GNOME_Terminal_TerminalFactory *
					servant,
					GIOPRecvBuffer * _ORBIT_recv_buffer,
					gpointer * impl)
{
   gchar *opname = _ORBIT_recv_buffer->message.u.request.operation;

   switch (opname[0]) {
     case 'c':
	switch (opname[1]) {
	  case 'r':
	     switch (opname[2]) {
	       case 'e':
		  switch (opname[3]) {
		    case 'a':
		       switch (opname[4]) {
			 case 't':
			    switch (opname[5]) {
			      case 'e':
				 switch (opname[6]) {
				   case '_':
				      switch (opname[7]) {
					case 'o':
					   if (strcmp((opname + 8), "bject"))
					      break;
					   *impl =
					      (gpointer) servant->vepv->
					      GNOME_GenericFactory_epv->
					      create_object;
					   return (ORBitSkeleton)
					      _ORBIT_skel_GNOME_GenericFactory_create_object;
					   break;
					case 't':
					   if (strcmp
					       ((opname + 8),
						"erminal")) break;
					   *impl =
					      (gpointer) servant->vepv->
					      GNOME_Terminal_TerminalFactory_epv->
					      create_terminal;
					   return (ORBitSkeleton)
					      _ORBIT_skel_GNOME_Terminal_TerminalFactory_create_terminal;
					   break;
					default:
					   break;
				      }
				      break;
				   default:
				      break;
				 }
				 break;
			      default:
				 break;
			    }
			    break;
			 default:
			    break;
		       }
		       break;
		    default:
		       break;
		  }
		  break;
	       default:
		  break;
	     }
	     break;
	  default:
	     break;
	}
	break;
     case 's':
	if (strcmp((opname + 1), "upports"))
	   break;
	*impl = (gpointer) servant->vepv->GNOME_GenericFactory_epv->supports;
	return (ORBitSkeleton) _ORBIT_skel_GNOME_GenericFactory_supports;
	break;
     default:
	break;
   }
   return NULL;
}

static void
init_local_objref_GNOME_Terminal_TerminalFactory(CORBA_Object obj,
						 POA_GNOME_Terminal_TerminalFactory
						 * servant)
{
   obj->vepv[GNOME_GenericFactory__classid] =
      servant->vepv->GNOME_GenericFactory_epv;
   obj->vepv[GNOME_Terminal_TerminalFactory__classid] =
      servant->vepv->GNOME_Terminal_TerminalFactory_epv;
}

void
POA_GNOME_Terminal_TerminalFactory__init(PortableServer_Servant servant,
					 CORBA_Environment * env)
{
   static const PortableServer_ClassInfo class_info =
      { (ORBit_impl_finder) & get_skel_GNOME_Terminal_TerminalFactory,
	 "IDL:GNOME/Terminal/TerminalFactory:1.0",
	 (ORBit_local_objref_init) &

	 init_local_objref_GNOME_Terminal_TerminalFactory };
   PortableServer_ServantBase__init(((PortableServer_ServantBase *) servant),
				    env);
   POA_GNOME_GenericFactory__init(servant, env);
   ORBIT_OBJECT_KEY(((PortableServer_ServantBase *) servant)->_private)->
      class_info = (PortableServer_ClassInfo *) & class_info;
   if (!GNOME_Terminal_TerminalFactory__classid)
      GNOME_Terminal_TerminalFactory__classid =
	 ORBit_register_class(&class_info);
}

void
POA_GNOME_Terminal_TerminalFactory__fini(PortableServer_Servant servant,
					 CORBA_Environment * env)
{
   POA_GNOME_GenericFactory__fini(servant, env);
   PortableServer_ServantBase__fini(servant, env);
}
