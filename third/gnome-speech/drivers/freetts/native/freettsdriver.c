#include <stdio.h>
#include <stdlib.h>
#include <libbonobo.h>
#include "freettsdriver.h"



JNIEXPORT void JNICALL Java_org_GNOME_Speech_FreeTTSSynthesisDriver__1bonoboActivate
(JNIEnv *env,
 jobject obj,
 jstring ior,
 jstring iid,
 jobjectArray args)
{
	int argc;
	char **argv;
	CORBA_ORB orb;
	CORBA_Object objref;
	CORBA_Environment ev;
	const char *native_ior, *native_iid;
	int i;
	jobject o;
	
        /* Ug, get the arguments */

        argc = (*env)->GetArrayLength (env, args)+1;

        /* Allocate space for them */

        argv = calloc (argc, sizeof(char *));

	argv[0] = "freetts-synthesis-driver";

	for (i = 0; i < argc-1; i++) {
		o = (*env)->GetObjectArrayElement (env, args, i);
		argv[i+1] = (char *) (*env)->GetStringUTFChars (env, o, 0);
	}
	
        /* UTF8 strings from the jstrings */

	native_ior = (*env)->GetStringUTFChars (env, ior, 0);
	native_iid = (*env)->GetStringUTFChars (env, iid, 0);

	/* Initialize bonobo */

	bonobo_init (&argc, argv);
  
	/* Get the CORBA_Object for the IOR we got */

	CORBA_exception_init (&ev);
	orb = bonobo_orb ();
	objref = CORBA_ORB_string_to_object (orb, native_ior, &ev);
	if (objref == CORBA_OBJECT_NIL)
	{
		exit (0);
	}
	bonobo_activation_active_server_register (native_iid, objref);
	(*env)->ReleaseStringUTFChars (env, ior, native_ior);
	(*env)->ReleaseStringUTFChars (env, iid, native_iid);
	for (i = 0; i < argc-1; i++) {
		o = (*env)->GetObjectArrayElement (env, args, i);
		(*env)->ReleaseStringUTFChars (env, o, argv[i+1]);
	}
	free (argv);
}

