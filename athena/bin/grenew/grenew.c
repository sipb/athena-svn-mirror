/* Renew a credentials cache using a graphical interface. Much of the source
 * is modified from that of kinit.
 *
 * $Id: grenew.c,v 1.1.2.1 2001-07-11 18:37:47 ghudson Exp $
 */

#include <gtk/gtk.h>
#include <krb5.h>
#include <kerberosIV/krb.h>
#include <string.h>
#include <stdio.h>
#include <com_err.h>

char *progname = NULL;
char *name = NULL;

static void quit();
static void do_error_dialog(char *msg);
static void do_fatal_dialog(char *msg);
static void do_renew(GtkWidget * widget, GtkWidget * entry);
static void create_window();
int try_krb4(krb5_context kcontext, krb5_principal me, char *password,
	     krb5_deltat lifetime);
int try_convert524(krb5_context kcontext, krb5_ccache ccache);

/* library functions not declared inside the included headers. */
void krb524_init_ets(krb5_context kcontext);
int krb524_convert_creds_kdc(krb5_context kcontext,
			     krb5_creds * k5creds,
			     CREDENTIALS * k4creds);

static void quit()
{
  gtk_exit(0);
}

static void do_error_dialog(char *msg)
{
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *ok;

  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), "Error");
  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		     GTK_SIGNAL_FUNC(gtk_false), NULL);

  msg = g_strdup_printf("Error:\n\n%s", msg);
  label = gtk_label_new(msg);
  gtk_box_pack_end(GTK_BOX(GTK_DIALOG(window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  ok = gtk_button_new_with_label("Ok");
  gtk_signal_connect_object(GTK_OBJECT(ok), "clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    GTK_OBJECT(window));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area), ok, TRUE,
		     TRUE, 0);
  gtk_widget_show(ok);

  gtk_widget_show(window);
}

static void do_fatal_dialog(char *msg)
{
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *ok;

  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), "Error");
  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		     GTK_SIGNAL_FUNC(quit), NULL);

  msg = g_strdup_printf("Fatal Error:\n\n%s", msg);
  label = gtk_label_new(msg);
  gtk_box_pack_end(GTK_BOX(GTK_DIALOG(window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  ok = gtk_button_new_with_label("Ok");
  gtk_signal_connect(GTK_OBJECT(ok), "clicked",
		     GTK_SIGNAL_FUNC(quit), NULL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area), ok, TRUE,
		     TRUE, 0);
  gtk_widget_show(ok);

  gtk_widget_show(window);
}

static void do_renew(GtkWidget * widget, GtkWidget * entry)
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
      gtk_exit(0);
    }
}

static void create_window()
{
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *ok;
  GtkWidget *cancel;

  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), "Renewing Authentication");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		     GTK_SIGNAL_FUNC(quit), NULL);

  label = gtk_label_new("Type your password now to renew your authentication "
			"to the system, which expires every 10 hours.");
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), entry, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(entry), "activate",
		     GTK_SIGNAL_FUNC(do_renew), entry);
  gtk_widget_show(entry);
  gtk_widget_grab_focus(entry);

  ok = gtk_button_new_with_label("Ok");
  gtk_signal_connect(GTK_OBJECT(ok), "clicked",
		     GTK_SIGNAL_FUNC(do_renew), entry);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area), ok, TRUE,
		     TRUE, 0);
  gtk_widget_show(ok);

  cancel = gtk_button_new_with_label("Cancel");
  gtk_signal_connect_object(GTK_OBJECT(cancel), "clicked",
			    GTK_SIGNAL_FUNC(quit), NULL);
  gtk_box_pack_end(GTK_BOX(GTK_DIALOG(window)->action_area), cancel,
		   TRUE, TRUE, 0);
  gtk_widget_show(cancel);

  gtk_widget_show(window);
}

/* This function is from kinit. */
int try_krb4(kcontext, me, password, lifetime)
	krb5_context kcontext;
	krb5_principal me;
	char *password;
	krb5_deltat lifetime;
{
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
}

/* Convert krb5 tickets to krb4. This function was copied from kinit */
int try_convert524(kcontext, ccache)
	 krb5_context kcontext;
	 krb5_ccache ccache;
{
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
