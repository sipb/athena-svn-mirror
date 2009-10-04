/* Renew a credentials cache using a graphical interface. Much of the source
 * is modified from that of kinit.
 *
 * $Id: grenew.c,v 1.6 2004-05-27 18:38:46 ghudson Exp $
 */

#include <gtk/gtk.h>
#include <krb5.h>
#ifdef HAVE_KRB4
#include <kerberosIV/krb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <com_err.h>

char *progname = NULL;
const char *name = NULL;

static void quit();
static void do_error_dialog(char *msg);
static void do_fatal_dialog(char *msg);
static void do_renew(GtkWidget *widget, GtkWidget *entry);
static void dialog_response_cb(GtkWidget *w, gint response, GtkWidget *entry);
static void create_window();
int try_krb4(krb5_context kcontext, krb5_principal me, char *password,
	     krb5_deltat lifetime);
int try_convert524(krb5_context kcontext, krb5_ccache ccache);

/* library functions not declared inside the included headers. */
#ifdef HAVE_KRB4
void krb524_init_ets(krb5_context kcontext);
int krb524_convert_creds_kdc(krb5_context kcontext,
			     krb5_creds * k5creds,
			     CREDENTIALS * k4creds);
#endif HAVE_KRB4

static void quit()
{
  exit(0);
}

static void do_error_dialog(char *msg)
{
  static GtkWidget *window = NULL;

  if (window)
    gtk_widget_destroy(window);
  window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_WARNING,
				  GTK_BUTTONS_OK, msg);
  g_signal_connect(G_OBJECT(window), "response",
		   G_CALLBACK(gtk_widget_destroy), NULL);
  g_signal_connect(G_OBJECT(window), "destroy",
		   G_CALLBACK(gtk_widget_destroyed), &window);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_show(window);
}

static void do_fatal_dialog(char *msg)
{
  GtkWidget *window;

  msg = g_strdup_printf("Fatal Error:\n\n%s", msg);
  window = gtk_message_dialog_new(NULL, 
				  GTK_DIALOG_MODAL,
				  GTK_MESSAGE_ERROR,
				  GTK_BUTTONS_OK, msg);
  g_signal_connect(G_OBJECT(window), "response",
		   G_CALLBACK(quit), NULL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_show(window);
}

static void do_renew(GtkWidget *widget, GtkWidget *entry)
{
  char *pass;
  krb5_error_code code;
  krb5_context kcontext;
  krb5_principal me = NULL;
  char *service_name = NULL;
  krb5_ccache ccache = NULL;
  krb5_creds my_creds;
  char *msg = NULL;
  krb5_deltat start_time = 0;
  krb5_deltat lifetime = 10 * 60 * 60;
  krb5_get_init_creds_opt opts;
  int non_fatal = 0;

  pass = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
  gtk_entry_set_text(GTK_ENTRY(entry), "");

  krb5_get_init_creds_opt_init(&opts);

  if (!pass || !strlen(pass))
    {
      non_fatal = 1;
      msg = g_strdup("Incorrect password: please try again.");
    }
  else if ((code = krb5_init_context(&kcontext)))
    msg = g_strdup_printf("%s when initializing kerberos library",
			  error_message(code));
  else if ((code = krb5_cc_default(kcontext, &ccache)))
    msg = g_strdup_printf("%s while getting default cache.",
			  error_message(code));
  else if ((code = krb5_parse_name(kcontext, name, &me)))
    msg = g_strdup_printf("%s while parsing name %s.", error_message(code),
			  name);
  else if ((code = krb5_get_init_creds_password(kcontext,
						&my_creds, me, pass,
						krb5_prompter_posix, NULL,
						start_time, service_name,
						&opts)))
    {
      if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
	{
	  non_fatal = 1;
	  msg = g_strdup("Incorrect password: please try again.");
	}
      else
	{
	  msg = g_strdup_printf("%s while getting initial"
				" credentials", error_message(code));
	}
    }
  else
    {
      int got_krb4 = try_krb4(kcontext, me, pass, lifetime);
      if ((code = krb5_cc_initialize(kcontext, ccache, me)))
	{
	  msg = g_strdup_printf("%s while initializing cache",
				error_message(code));
	}
      else if ((code = krb5_cc_store_cred(kcontext, ccache,
					  &my_creds)))
	{
	  msg = g_strdup_printf("%s while storing credentials",
				error_message(code));
	}
      else
	{
	  if (!got_krb4)
	    try_convert524(kcontext, ccache);
	  if (me)
	    krb5_free_principal(kcontext, me);
	  if (ccache)
	    krb5_cc_close(kcontext, ccache);

	  krb5_free_context(kcontext);
	}
    }

  g_free(pass);

  if (msg)
    {
      /* encountered an error. don't quit */
      if (non_fatal)
	do_error_dialog(msg);
      else
	do_fatal_dialog(msg);
      g_free(msg);
    }
  else
    /* no errors, we're done */
    {
      system("fsid -a > /dev/null");
      system("zctl load /dev/null > /dev/null");
      exit(0);
    }
}

static void dialog_response_cb(GtkWidget *w, gint response, GtkWidget *entry)
{
  gtk_widget_grab_focus(GTK_WIDGET(entry));

  if (response == GTK_RESPONSE_OK)
    do_renew(w, entry);
  else
    quit();
}

static void create_window()
{
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;

  window = gtk_dialog_new_with_buttons("Renewing authentication", 
				       NULL, 0,
				       GTK_STOCK_OK, 
				       GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,
				       GTK_RESPONSE_CANCEL,
				       NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  label = gtk_label_new("Type your password now to renew your authentication "
			"to the system, which expires every 10 hours.");
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), entry, TRUE, TRUE, 0);
  gtk_widget_grab_focus(entry);
  gtk_widget_show(entry);

  g_signal_connect_object(G_OBJECT(entry), "activate",
			  G_CALLBACK(do_renew), entry, 0);
  g_signal_connect_object(G_OBJECT(window), "response", 
			  G_CALLBACK(dialog_response_cb), entry, 0);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit), NULL);

  gtk_widget_show(window);
}

/* This function is from kinit. */
int try_krb4(kcontext, me, password, lifetime)
	krb5_context kcontext;
	krb5_principal me;
	char *password;
	krb5_deltat lifetime;
{
#ifdef HAVE_KRB4
	krb5_error_code code;
	int krbval;
	char v4name[ANAME_SZ], v4inst[INST_SZ], v4realm[REALM_SZ];
	int v4life;

	/* Translate to a Kerberos 4 principal. */
	code = krb5_524_conv_principal(kcontext, me, v4name, v4inst, v4realm);
	if (code)
		return(code);

	v4life = lifetime / (5 * 60);
	if (v4life < 1)
		v4life = 1;
	if (v4life > 255)
		v4life = 255;

	krbval = krb_get_pw_in_tkt(v4name, v4inst, v4realm, "krbtgt", v4realm,
					   v4life, password);

	if (krbval != INTK_OK) {
		fprintf(stderr, "Kerberos 4 error: %s\n",
			krb_get_err_text(krbval));
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}

/* Convert krb5 tickets to krb4. This function was copied from kinit */
int try_convert524(kcontext, ccache)
	 krb5_context kcontext;
	 krb5_ccache ccache;
{
#ifdef HAVE_KRB4
	krb5_principal me, kpcserver;
	krb5_error_code kpccode;
	int kpcval;
	krb5_creds increds, *v5creds;
	CREDENTIALS v4creds;

	/* or do this directly with krb524_convert_creds_kdc */
	krb524_init_ets(kcontext);

	if ((kpccode = krb5_cc_get_principal(kcontext, ccache, &me))) {
		com_err(progname, kpccode, "while getting principal name");
		return 0;
	}

	/* cc->ccache, already set up */
	/* client->me, already set up */
	if ((kpccode = krb5_build_principal(kcontext,
						    &kpcserver, 
						    krb5_princ_realm(kcontext, me)->length,
						    krb5_princ_realm(kcontext, me)->data,
						    "krbtgt",
						    krb5_princ_realm(kcontext, me)->data,
						NULL))) {
	  com_err(progname, kpccode,
			  "while creating service principal name");
	  return 0;
	}

	memset((char *) &increds, 0, sizeof(increds));
	increds.client = me;
	increds.server = kpcserver;
	increds.times.endtime = 0;
	increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
	if ((kpccode = krb5_get_credentials(kcontext, 0, 
						ccache,
						&increds, 
						&v5creds))) {
		com_err(progname, kpccode,
			"getting V5 credentials");
		return 0;
	}
	if ((kpccode = krb524_convert_creds_kdc(kcontext, 
							v5creds,
							&v4creds))) {
		com_err(progname, kpccode, 
			"converting to V4 credentials");
		return 0;
	}
	/* this is stolen from the v4 kinit */
	/* initialize ticket cache */
	if ((kpcval = in_tkt(v4creds.pname,v4creds.pinst)
		 != KSUCCESS)) {
		com_err(progname, kpcval,
			"trying to create the V4 ticket file");
		return 0;
	}
	/* stash ticket, session key, etc. for future use */
	if ((kpcval = krb_save_credentials(v4creds.service,
						   v4creds.instance,
						   v4creds.realm, 
						   v4creds.session,
						   v4creds.lifetime,
						   v4creds.kvno,
						   &(v4creds.ticket_st), 
						   v4creds.issue_date))) {
		com_err(progname, kpcval,
			"trying to save the V4 ticket");
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  progname = g_get_prgname();
  if (argc > 1)
    name = argv[1];
  else
    name = g_get_user_name();
  create_window();
  gtk_main();
  return 0;
}
