/* Bug Dick Porter <dick@cymru.net> with any problem reports. GPL'd. */
#include <signal.h>
#include <stdio.h>
#include "CosEventChannel.h"

/* The following is mostly taken from CosEventChannel-impl.c as
   generated by "orbit-idl -Eskeleton_impl CosEventChannel.idl", only
   the stub implemementations have been removed and some lines have
   been added, those are marked with "added". Many things for the
   {Push,Pull}{Consumer,Supplier} has been changed to their Proxy
   versions */

/***************************************************************************/
/*                         begin of generated part                         */
/***************************************************************************/

/*** App-specific servant structures ***/

typedef struct {
   POA_CosEventChannelAdmin_EventChannel servant;
   PortableServer_POA poa;
  
  CosEventChannelAdmin_SupplierAdmin supplier_admin; /* added */
  CosEventChannelAdmin_ConsumerAdmin consumer_admin; /* added */

  GList *push_supplier; /* added */
  GList *pull_supplier; /* added */
  GList *push_consumer; /* added */
  GList *pull_consumer; /* added */

} impl_POA_CosEventChannelAdmin_EventChannel;

typedef struct {
   POA_CosEventChannelAdmin_ProxyPushConsumer servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */

} impl_POA_CosEventChannelAdmin_ProxyPushConsumer;

typedef struct {
   POA_CosEventChannelAdmin_ProxyPullSupplier servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */
  
} impl_POA_CosEventChannelAdmin_ProxyPullSupplier;

typedef struct {
   POA_CosEventChannelAdmin_ProxyPullConsumer servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */
  
} impl_POA_CosEventChannelAdmin_ProxyPullConsumer;

typedef struct {
   POA_CosEventChannelAdmin_ProxyPushSupplier servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */
  
} impl_POA_CosEventChannelAdmin_ProxyPushSupplier;

typedef struct {
   POA_CosEventChannelAdmin_ConsumerAdmin servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */
  
} impl_POA_CosEventChannelAdmin_ConsumerAdmin;

typedef struct {
   POA_CosEventChannelAdmin_SupplierAdmin servant;
   PortableServer_POA poa;

  impl_POA_CosEventChannelAdmin_EventChannel* channel_servant; /* added */

} impl_POA_CosEventChannelAdmin_SupplierAdmin;

typedef struct {
   POA_CosEventChannelAdmin_EventChannelFactory servant;
   PortableServer_POA poa;

} impl_POA_CosEventChannelAdmin_EventChannelFactory;

/*** Implementation stub prototypes ***/

void
 impl_CosEventChannelAdmin_ProxyPushConsumer_push(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
				     CORBA_any * data,
				     CORBA_Environment * ev);

void
 impl_CosEventChannelAdmin_ProxyPushConsumer_disconnect_push_consumer(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
						    CORBA_Environment * ev);

void
 impl_CosEventChannelAdmin_ProxyPushSupplier_disconnect_push_supplier(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant,
						    CORBA_Environment * ev);

CORBA_any *
 impl_CosEventChannelAdmin_ProxyPullSupplier_pull(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
				     CORBA_Environment * ev);

CORBA_any *
 impl_CosEventChannelAdmin_ProxyPullSupplier_try_pull(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
					 CORBA_boolean * has_event,
					 CORBA_Environment * ev);

void
 impl_CosEventChannelAdmin_ProxyPullSupplier_disconnect_pull_supplier(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
						    CORBA_Environment * ev);

void
 impl_CosEventChannelAdmin_ProxyPullConsumer_disconnect_pull_consumer(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_ProxyPushConsumer__destroy(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
						    CORBA_Environment * ev);
void
 impl_CosEventChannelAdmin_ProxyPushConsumer_connect_push_supplier(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
				    CosEventComm_PushSupplier push_supplier,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_ProxyPullSupplier__destroy(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
						    CORBA_Environment * ev);
void
 impl_CosEventChannelAdmin_ProxyPullSupplier_connect_pull_consumer(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
				    CosEventComm_PullConsumer pull_consumer,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_ProxyPullConsumer__destroy(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant,
						    CORBA_Environment * ev);
void
 impl_CosEventChannelAdmin_ProxyPullConsumer_connect_pull_supplier(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant,
				    CosEventComm_PullSupplier pull_supplier,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_ProxyPushSupplier__destroy(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant,
						    CORBA_Environment * ev);
void
 impl_CosEventChannelAdmin_ProxyPushSupplier_connect_push_consumer(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant,
				    CosEventComm_PushConsumer push_consumer,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_ConsumerAdmin__destroy(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant,
						    CORBA_Environment * ev);

CosEventChannelAdmin_ProxyPushSupplier
impl_CosEventChannelAdmin_ConsumerAdmin_obtain_push_supplier(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant,
						    CORBA_Environment * ev);

CosEventChannelAdmin_ProxyPullSupplier
impl_CosEventChannelAdmin_ConsumerAdmin_obtain_pull_supplier(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_SupplierAdmin__destroy(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant,
						    CORBA_Environment * ev);

CosEventChannelAdmin_ProxyPushConsumer
impl_CosEventChannelAdmin_SupplierAdmin_obtain_push_consumer(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant,
						    CORBA_Environment * ev);

CosEventChannelAdmin_ProxyPullConsumer
impl_CosEventChannelAdmin_SupplierAdmin_obtain_pull_consumer(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant,
						    CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_EventChannel__destroy(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						    CORBA_Environment * ev);

CosEventChannelAdmin_ConsumerAdmin
impl_CosEventChannelAdmin_EventChannel_for_consumers(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						     CORBA_Environment * ev);

CosEventChannelAdmin_SupplierAdmin
impl_CosEventChannelAdmin_EventChannel_for_suppliers(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						     CORBA_Environment * ev);

void
 impl_CosEventChannelAdmin_EventChannel_destroy(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						CORBA_Environment * ev);

static void impl_CosEventChannelAdmin_EventChannelFactory__destroy(impl_POA_CosEventChannelAdmin_EventChannelFactory * servant,
                                                    CORBA_Environment * ev);

CosEventChannelAdmin_EventChannel
impl_CosEventChannelAdmin_EventChannelFactory_new_event_channel(impl_POA_CosEventChannelAdmin_EventChannelFactory * servant,
                                                    CORBA_Environment * ev);

/*** epv structures ***/

static POA_CosEventComm_PushConsumer__epv impl_CosEventComm_PushConsumer_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushConsumer_push,

   (gpointer) & impl_CosEventChannelAdmin_ProxyPushConsumer_disconnect_push_consumer,

};
static POA_CosEventComm_PushSupplier__epv impl_CosEventComm_PushSupplier_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushSupplier_disconnect_push_supplier,

};
static POA_CosEventComm_PullSupplier__epv impl_CosEventComm_PullSupplier_epv =
{
   NULL,                        /* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullSupplier_pull,

   (gpointer) & impl_CosEventChannelAdmin_ProxyPullSupplier_try_pull,

   (gpointer) & impl_CosEventChannelAdmin_ProxyPullSupplier_disconnect_pull_supplier,

};
static POA_CosEventComm_PullConsumer__epv impl_CosEventComm_PullConsumer_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullConsumer_disconnect_pull_consumer,

};

static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_ProxyPushConsumer_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushConsumer__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_ProxyPushConsumer__epv impl_CosEventChannelAdmin_ProxyPushConsumer_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushConsumer_connect_push_supplier,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_ProxyPullSupplier_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullSupplier__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_ProxyPullSupplier__epv impl_CosEventChannelAdmin_ProxyPullSupplier_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullSupplier_connect_pull_consumer,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_ProxyPullConsumer_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullConsumer__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_ProxyPullConsumer__epv impl_CosEventChannelAdmin_ProxyPullConsumer_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPullConsumer_connect_pull_supplier,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_ProxyPushSupplier_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushSupplier__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_ProxyPushSupplier__epv impl_CosEventChannelAdmin_ProxyPushSupplier_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ProxyPushSupplier_connect_push_consumer,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_ConsumerAdmin_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_ConsumerAdmin__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_ConsumerAdmin__epv impl_CosEventChannelAdmin_ConsumerAdmin_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_ConsumerAdmin_obtain_push_supplier,

   (gpointer) & impl_CosEventChannelAdmin_ConsumerAdmin_obtain_pull_supplier,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_SupplierAdmin_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_SupplierAdmin__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_SupplierAdmin__epv impl_CosEventChannelAdmin_SupplierAdmin_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_SupplierAdmin_obtain_push_consumer,

   (gpointer) & impl_CosEventChannelAdmin_SupplierAdmin_obtain_pull_consumer,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_EventChannel_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_CosEventChannelAdmin_EventChannel__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_CosEventChannelAdmin_EventChannel__epv impl_CosEventChannelAdmin_EventChannel_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_CosEventChannelAdmin_EventChannel_for_consumers,

   (gpointer) & impl_CosEventChannelAdmin_EventChannel_for_suppliers,

   (gpointer) & impl_CosEventChannelAdmin_EventChannel_destroy,

};
static PortableServer_ServantBase__epv impl_CosEventChannelAdmin_EventChannelFactory_base_epv =
{
   NULL,                        /* _private data */
   (gpointer) & impl_CosEventChannelAdmin_EventChannelFactory__destroy,         
/* finalize routine */
   NULL,                        /* default_POA routine */
};
static POA_CosEventChannelAdmin_EventChannelFactory__epv impl_CosEventChannelAdmin_EventChannelFactory_epv =
{
   NULL,                        /* _private */
(gpointer) & impl_CosEventChannelAdmin_EventChannelFactory_new_event_channel,

};

/*** vepv structures ***/


static POA_CosEventChannelAdmin_ProxyPushConsumer__vepv impl_CosEventChannelAdmin_ProxyPushConsumer_vepv =
{
   &impl_CosEventChannelAdmin_ProxyPushConsumer_base_epv,
   &impl_CosEventComm_PushConsumer_epv,
   &impl_CosEventChannelAdmin_ProxyPushConsumer_epv,
};
static POA_CosEventChannelAdmin_ProxyPullSupplier__vepv impl_CosEventChannelAdmin_ProxyPullSupplier_vepv =
{
   &impl_CosEventChannelAdmin_ProxyPullSupplier_base_epv,
   &impl_CosEventComm_PullSupplier_epv,
   &impl_CosEventChannelAdmin_ProxyPullSupplier_epv,
};
static POA_CosEventChannelAdmin_ProxyPullConsumer__vepv impl_CosEventChannelAdmin_ProxyPullConsumer_vepv =
{
   &impl_CosEventChannelAdmin_ProxyPullConsumer_base_epv,
   &impl_CosEventComm_PullConsumer_epv,
   &impl_CosEventChannelAdmin_ProxyPullConsumer_epv,
};
static POA_CosEventChannelAdmin_ProxyPushSupplier__vepv impl_CosEventChannelAdmin_ProxyPushSupplier_vepv =
{
   &impl_CosEventChannelAdmin_ProxyPushSupplier_base_epv,
   &impl_CosEventComm_PushSupplier_epv,
   &impl_CosEventChannelAdmin_ProxyPushSupplier_epv,
};
static POA_CosEventChannelAdmin_ConsumerAdmin__vepv impl_CosEventChannelAdmin_ConsumerAdmin_vepv =
{
   &impl_CosEventChannelAdmin_ConsumerAdmin_base_epv,
   &impl_CosEventChannelAdmin_ConsumerAdmin_epv,
};
static POA_CosEventChannelAdmin_SupplierAdmin__vepv impl_CosEventChannelAdmin_SupplierAdmin_vepv =
{
   &impl_CosEventChannelAdmin_SupplierAdmin_base_epv,
   &impl_CosEventChannelAdmin_SupplierAdmin_epv,
};
static POA_CosEventChannelAdmin_EventChannel__vepv impl_CosEventChannelAdmin_EventChannel_vepv =
{
   &impl_CosEventChannelAdmin_EventChannel_base_epv,
   &impl_CosEventChannelAdmin_EventChannel_epv,
};
static POA_CosEventChannelAdmin_EventChannelFactory__vepv impl_CosEventChannelAdmin_EventChannelFactory_vepv =
{
   &impl_CosEventChannelAdmin_EventChannelFactory_base_epv,
   &impl_CosEventChannelAdmin_EventChannelFactory_epv,
};

/*** Stub implementations ***/

/* You shouldn't call this routine directly without first deactivating the servant... */
static CosEventChannelAdmin_ProxyPushConsumer 
impl_CosEventChannelAdmin_ProxyPushConsumer__create(PortableServer_POA poa,
						    impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, /* added */ 
						    CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPushConsumer retval;
   impl_POA_CosEventChannelAdmin_ProxyPushConsumer *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_ProxyPushConsumer, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_ProxyPushConsumer_vepv;
   newservant->poa = poa;
   
   newservant->channel_servant = channel_servant; /* added */
   g_list_append( channel_servant->push_consumer, newservant ); /* added */

   POA_CosEventChannelAdmin_ProxyPushConsumer__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_ProxyPushConsumer__destroy(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_ProxyPushConsumer__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_ProxyPullSupplier 
impl_CosEventChannelAdmin_ProxyPullSupplier__create(PortableServer_POA poa, 
						    impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, 
						    /* added */CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPullSupplier retval;
   impl_POA_CosEventChannelAdmin_ProxyPullSupplier *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_ProxyPullSupplier, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_ProxyPullSupplier_vepv;
   newservant->poa = poa;
   
   newservant->channel_servant = channel_servant; /* added */
   g_list_append( channel_servant->pull_supplier, newservant ); /* added */

   POA_CosEventChannelAdmin_ProxyPullSupplier__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_ProxyPullSupplier__destroy(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_ProxyPullSupplier__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_ProxyPullConsumer 
impl_CosEventChannelAdmin_ProxyPullConsumer__create(PortableServer_POA poa,
						    impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, /* added */ 
						    CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPullConsumer retval;
   impl_POA_CosEventChannelAdmin_ProxyPullConsumer *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_ProxyPullConsumer, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_ProxyPullConsumer_vepv;
   newservant->poa = poa;
   
   newservant->channel_servant = channel_servant; /* added */
   g_list_append( channel_servant->pull_consumer, newservant ); /* added */

   POA_CosEventChannelAdmin_ProxyPullConsumer__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_ProxyPullConsumer__destroy(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_ProxyPullConsumer__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_ProxyPushSupplier 
impl_CosEventChannelAdmin_ProxyPushSupplier__create(PortableServer_POA poa, 
						    impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, /* added */
						    CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPushSupplier retval;
   impl_POA_CosEventChannelAdmin_ProxyPushSupplier *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_ProxyPushSupplier, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_ProxyPushSupplier_vepv;
   newservant->poa = poa;
    
   newservant->channel_servant = channel_servant; /* added */
   g_list_append( channel_servant->push_supplier, newservant ); /* added */

  POA_CosEventChannelAdmin_ProxyPushSupplier__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_ProxyPushSupplier__destroy(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_ProxyPushSupplier__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_ConsumerAdmin 
impl_CosEventChannelAdmin_ConsumerAdmin__create(PortableServer_POA poa, 
						impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, /* added */
						CORBA_Environment * ev)
{
   CosEventChannelAdmin_ConsumerAdmin retval;
   impl_POA_CosEventChannelAdmin_ConsumerAdmin *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_ConsumerAdmin, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_ConsumerAdmin_vepv;
   newservant->poa = poa;

   newservant->channel_servant = channel_servant; /* added */

   POA_CosEventChannelAdmin_ConsumerAdmin__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_ConsumerAdmin__destroy(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_ConsumerAdmin__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_SupplierAdmin 
impl_CosEventChannelAdmin_SupplierAdmin__create(PortableServer_POA poa, 
						impl_POA_CosEventChannelAdmin_EventChannel* channel_servant, /* added */
						CORBA_Environment * ev)
{
   CosEventChannelAdmin_SupplierAdmin retval;
   impl_POA_CosEventChannelAdmin_SupplierAdmin *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_SupplierAdmin, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_SupplierAdmin_vepv;
   newservant->poa = poa;

   newservant->channel_servant = channel_servant; /* added */

   POA_CosEventChannelAdmin_SupplierAdmin__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_SupplierAdmin__destroy(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_SupplierAdmin__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_EventChannel 
impl_CosEventChannelAdmin_EventChannel__create(PortableServer_POA poa, CORBA_Environment * ev)
{
   CosEventChannelAdmin_EventChannel retval;
   impl_POA_CosEventChannelAdmin_EventChannel *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_EventChannel, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_EventChannel_vepv;
   newservant->poa = poa;
   
   newservant->supplier_admin = 
     impl_CosEventChannelAdmin_SupplierAdmin__create( poa, newservant, ev ); 
   /* added */
   newservant->consumer_admin = 
     impl_CosEventChannelAdmin_ConsumerAdmin__create( poa, newservant, ev ); 
   /* added */

   newservant->push_supplier = g_list_alloc(); /* added */
   newservant->pull_supplier = g_list_alloc(); /* added */
   newservant->push_consumer = g_list_alloc(); /* added */ 
   newservant->pull_consumer = g_list_alloc(); /* added */

   POA_CosEventChannelAdmin_EventChannel__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_CosEventChannelAdmin_EventChannel__destroy(impl_POA_CosEventChannelAdmin_EventChannel * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_EventChannel__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static CosEventChannelAdmin_EventChannelFactory 
impl_CosEventChannelAdmin_EventChannelFactory__create(PortableServer_POA poa, CORBA_Environment * ev)
{
   CosEventChannelAdmin_EventChannelFactory retval;
   impl_POA_CosEventChannelAdmin_EventChannelFactory *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_CosEventChannelAdmin_EventChannelFactory, 1);
   newservant->servant.vepv = &impl_CosEventChannelAdmin_EventChannelFactory_vepv;
   newservant->poa = poa;
   POA_CosEventChannelAdmin_EventChannelFactory__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the serva
nt... */
static void
impl_CosEventChannelAdmin_EventChannelFactory__destroy(impl_POA_CosEventChannelAdmin_EventChannelFactory * servant, CORBA_Environment * ev)
{

   POA_CosEventChannelAdmin_EventChannelFactory__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

/***************************************************************************/
/*                          end of generated part                          */
/***************************************************************************/

void
impl_CosEventChannelAdmin_ProxyPushConsumer_push(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
				    CORBA_any * data,
				    CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPushConsumer_disconnect_push_consumer(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
						     CORBA_Environment * ev)
{
}
void
impl_CosEventChannelAdmin_ProxyPushSupplier_disconnect_push_supplier(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant,
						     CORBA_Environment * ev)
{
}

CORBA_any *
impl_CosEventChannelAdmin_ProxyPullSupplier_pull(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
				    CORBA_Environment * ev)
{
   CORBA_any *retval;

   return retval;
}

CORBA_any *
impl_CosEventChannelAdmin_ProxyPullSupplier_try_pull(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
					CORBA_boolean * has_event,
					CORBA_Environment * ev)
{
   CORBA_any *retval;

   return retval;
}

void
impl_CosEventChannelAdmin_ProxyPullSupplier_disconnect_pull_supplier(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
						     CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPullConsumer_disconnect_pull_consumer(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant,
						     CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPushConsumer_connect_push_supplier(impl_POA_CosEventChannelAdmin_ProxyPushConsumer * servant,
				    CosEventComm_PushSupplier push_supplier,
						     CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPullSupplier_connect_pull_consumer(impl_POA_CosEventChannelAdmin_ProxyPullSupplier * servant,
				    CosEventComm_PullConsumer pull_consumer,
						     CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPullConsumer_connect_pull_supplier(impl_POA_CosEventChannelAdmin_ProxyPullConsumer * servant,
				    CosEventComm_PullSupplier pull_supplier,
						     CORBA_Environment * ev)
{
}

void
impl_CosEventChannelAdmin_ProxyPushSupplier_connect_push_consumer(impl_POA_CosEventChannelAdmin_ProxyPushSupplier * servant,
				    CosEventComm_PushConsumer push_consumer,
						     CORBA_Environment * ev)
{
}

CosEventChannelAdmin_ProxyPushSupplier
impl_CosEventChannelAdmin_ConsumerAdmin_obtain_push_supplier(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant,
						     CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPushSupplier retval = 
     impl_CosEventChannelAdmin_ProxyPushSupplier__create( servant->poa, servant->channel_servant, ev );
   
   return retval;
}

CosEventChannelAdmin_ProxyPullSupplier
impl_CosEventChannelAdmin_ConsumerAdmin_obtain_pull_supplier(impl_POA_CosEventChannelAdmin_ConsumerAdmin * servant,
						     CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPullSupplier retval;

   return retval;
}

CosEventChannelAdmin_ProxyPushConsumer
impl_CosEventChannelAdmin_SupplierAdmin_obtain_push_consumer(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant,
						     CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPushConsumer retval;

   return retval;
}

CosEventChannelAdmin_ProxyPullConsumer
impl_CosEventChannelAdmin_SupplierAdmin_obtain_pull_consumer(impl_POA_CosEventChannelAdmin_SupplierAdmin * servant,
						     CORBA_Environment * ev)
{
   CosEventChannelAdmin_ProxyPullConsumer retval;

   return retval;
}

CosEventChannelAdmin_ConsumerAdmin
impl_CosEventChannelAdmin_EventChannel_for_consumers(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						     CORBA_Environment * ev)
{
   return servant->consumer_admin;
}

CosEventChannelAdmin_SupplierAdmin
impl_CosEventChannelAdmin_EventChannel_for_suppliers(impl_POA_CosEventChannelAdmin_EventChannel * servant,
						     CORBA_Environment * ev)
{
   return servant->supplier_admin;
}

void
impl_CosEventChannelAdmin_EventChannel_destroy(impl_POA_CosEventChannelAdmin_EventChannel * servant,
					       CORBA_Environment * ev)
{
}

CosEventChannelAdmin_EventChannel
impl_CosEventChannelAdmin_EventChannelFactory_new_event_channel(impl_POA_CosEventChannelAdmin_EventChannelFactory * servant,
                                                     CORBA_Environment * ev)
{
  return impl_CosEventChannelAdmin_EventChannel__create( servant->poa, ev);
}

void
signal_handler(int signo)
{
  /* syslog(LOG_ERR,"Receveived signal %d\nshutting down.", signo); */
  exit(1);
}

int
main (int argc, char *argv[])
{
  CORBA_ORB orb;
  CORBA_Environment ev;
  CosEventChannelAdmin_EventChannelFactory factory;
  CORBA_char *objref;
  PortableServer_POA root_poa;
  PortableServer_POAManager pm;
  struct sigaction act;
  sigset_t         empty_mask;
  
  sigemptyset(&empty_mask);
  act.sa_handler = signal_handler;
  act.sa_mask    = empty_mask;
  act.sa_flags   = 0;
  
  sigaction(SIGINT,  &act, 0);
  sigaction(SIGHUP,  &act, 0);
  sigaction(SIGSEGV, &act, 0);
  sigaction(SIGABRT, &act, 0);
  
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, 0);
  
  CORBA_exception_init (&ev);
  
  orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);

  root_poa = (PortableServer_POA)
    CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);

  factory = impl_CosEventChannelAdmin_EventChannelFactory__create(root_poa, 
								  &ev);

  objref = CORBA_ORB_object_to_string (orb, factory, &ev);

  g_print ("%s\n", objref);
  fflush (stdout);

  CORBA_free (objref);

#if 0
  /* Don't release it so that the same-address-space info will stay around */
  CORBA_Object_release (factory, &ev);
#endif

  pm = PortableServer_POA__get_the_POAManager (root_poa, &ev);
  PortableServer_POAManager_activate (pm, &ev);

  CORBA_ORB_run (orb, &ev);
  return 0;
}
