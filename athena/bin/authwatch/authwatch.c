/* Copyright 2002 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/* authwatch: Watch the expiration time on the user's kerberos
 * tickets; pop up a warning dialog when authentication is about to
 * expire.
 */

static const char rcsid[] = "$Id: authwatch.c,v 1.2 2002-06-04 17:37:15 mwhitson Exp $";

#include <stdio.h>
#include <sys/types.h>

#include <krb5.h>
#include <com_err.h>

#include <gtk/gtk.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-messagebox.h>

#define MAX_TIMEOUT 60*5   /* Maximum interval between checks, in seconds */

krb5_context k5_context;


/* Find the expiration time (unix time) of the TGT currently in the
 * default krb5 credentials cache.  Returns 0 if there are no
 * credentials currently accessible, or on krb5 library errors.
 */

time_t get_krb5_expiration()
{
  time_t expiration = 0;
  krb5_ccache cc;
  krb5_creds creds, mcreds;
  krb5_data *realm;

  memset(&creds, 0, sizeof(creds));
  memset(&mcreds, 0, sizeof(mcreds));

  /* Access the default credentials cache. */
  if (krb5_cc_default(k5_context, &cc) != 0)
    goto cleanup;

  /* Fill in mcreds.client with the primary principal of the ccache */
  if (krb5_cc_get_principal(k5_context, cc, &mcreds.client) != 0)
    goto cleanup;

  /* Fill in mcreds.server with "krbtgt/realm@realm" */
  realm = krb5_princ_realm(k5_context, mcreds.client);
  if (krb5_build_principal_ext(k5_context, &mcreds.server, 
			       realm->length, realm->data,
			       KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME, 
			       realm->length, realm->data,
			       NULL)
      != 0)
    goto cleanup;

  /* Get the TGT from the ccache */
  if (krb5_cc_retrieve_cred(k5_context, cc, (krb5_flags) 0,
			    &mcreds, &creds) != 0)
    goto cleanup;

  expiration = creds.times.endtime;

 cleanup:
  krb5_free_cred_contents(k5_context, &creds);
  krb5_free_cred_contents(k5_context, &mcreds);
  krb5_cc_close(k5_context, cc);

  return expiration;
}


/* Run periodically by the gtk main loop. */

gint timeout_cb(gpointer unused)
{
  time_t now, expiration;
  time_t duration = 0;
  int nstate;
  unsigned int timeout;

  static GtkWidget *dialog = NULL;
  static int state = -1;

  static const struct {
    const unsigned int tte;     /* Time to expiration, in seconds */
    const char *message;        /* Formatted for a gnome_message_box */
  } warnings[] = {
    { 0,
"You have no authentication.  Select ``Renew Authentication'' from the\n"
"Athena ``Utilities'' menu to re-authenticate."
    },

    { 60,
"Your authentication will expire in less than one minute.  Select\n"
"``Renew Authentication'' from the Athena ``Utilities'' menu to re-authenticate."
    },

    { 60*5,
"Your authentication will expire in less than five minutes.  Select\n"
"``Renew Authentication'' from the Athena ``Utilities'' menu to re-authenticate."
    },

    { 60*15,
"Your authentication will expire in less than fifteen minutes.  Select\n"
"``Renew Authentication'' from the Athena ``Utilities'' menu to re-authenticate."
    }
  };
  static const int nwarnings = sizeof(warnings) / sizeof(*warnings);

  expiration = get_krb5_expiration();

  time(&now);
  if (expiration > now)
    duration = expiration - now;

  nstate = nwarnings;
  do
    {
      if (duration > warnings[nstate - 1].tte)
	break;
      nstate--;
    }
  while (nstate > 0);

  if (nstate != state)
    {
      state = nstate;

      if (dialog != NULL)
	gtk_widget_destroy(dialog);

      if (state < nwarnings)
	{
	  dialog = gnome_message_box_new(warnings[state].message,
					 GNOME_MESSAGE_BOX_WARNING,
					 GNOME_STOCK_BUTTON_OK,
					 NULL);

	  if (dialog == NULL)
	    {
	      fprintf(stderr, "authwatch: error creating dialog window\n");
	      exit(1);
	    }
	  gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
			     GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			     &dialog);

	  gtk_widget_show(dialog);
	}
    }

  /* Set our next callback for the predicted time until the next state
   * change, or MAX_TIMEOUT, whichever is smaller. 
   */
  if (state == 0)
    timeout = MAX_TIMEOUT;
  else
    timeout = duration - warnings[state - 1].tte;
  timeout = MIN(timeout, MAX_TIMEOUT);
  gtk_timeout_add(timeout * 1000, timeout_cb, NULL);

  return FALSE;   /* Remove the callback which called us. */
}


main(int argc, char **argv)
{
  krb5_error_code status;

  if (gnome_init("authwatch", "", argc, argv) != 0)
    {
      fprintf(stderr, "authwatch: could not inititalize GNOME library.\n");
      exit(1);
    }

  status = krb5_init_context(&k5_context);
  if (status != 0)
    {
      fprintf(stderr, "authwatch: could not initialize Kerberos v5 library: %s\n",
	      error_message(status));
      exit(1);
    }

  gtk_timeout_add(0, timeout_cb, NULL);
  gtk_main();

  exit(0);
}
