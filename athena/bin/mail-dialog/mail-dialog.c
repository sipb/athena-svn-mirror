#include <gtk/gtk.h>
#include <gnome.h>

/* mail-dialog: Display a dialog about the change in default mail
 * client.  Exit with status 1 through 5 for the five menu options, or
 * with status 255 if the user presses cancel.
 */

static  const char *text[] = {
  "Athena's new default mail client is Evolution.  This program will make it",
  "easier to send attachments, and will store saved mail on your Post Office",
  "server, from which it can be accessed from Mac and PC clients.  However,",
  "if you prefer, you can continue to use the old xmh mail reader.",
  "",
  "If you decide to use Evolution, you should also learn about the \"pine\"",
  "program to read your mail on the Athena dialups instead of using the old",
  "inc, show, repl, and comp commands."
};

const char *choices[] = {
  "Let me try Evolution, but ask me again next time",
  "Let me use xmh for now, but ask me again next time",
  "I want to run Evolution; don't ask me again",
  "I want to keep using xmh; don't ask me again",
  "Show me information about the pine program"
};

static void choice(GtkWidget *w, gpointer data);

int main(int argc, char **argv)
{
  GtkWidget *dialog, *textbox, *label, *buttons, *button;
  int result, i;

  gnome_init("mail-dialog", "1.0", argc, argv);

  dialog = gnome_dialog_new("Mail Client Selection Dialog",
			    GNOME_STOCK_BUTTON_CANCEL, NULL);

  textbox = gtk_vbox_new(FALSE, 0);
  for (i = 0; i < sizeof(text) / sizeof(*text); i++)
    {
      label = gtk_label_new(text[i]);
      gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
      gtk_box_pack_start(GTK_BOX(textbox), label, TRUE, TRUE, 0);
    }

  buttons = gtk_vbox_new(FALSE, 0);
  for (i = 0; i < sizeof(choices) / sizeof(*choices); i++)
    {
      button = gtk_button_new_with_label(choices[i]);
      gtk_box_pack_start(GTK_BOX(buttons), button, TRUE, TRUE, 0);
      gtk_signal_connect(GTK_OBJECT(button), "clicked",
			 GTK_SIGNAL_FUNC(choice), &choices[i]);
    }

  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), textbox, TRUE, TRUE,
		     GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), buttons, TRUE, TRUE,
		     GNOME_PAD);

  gtk_widget_show_all(dialog);
  gnome_dialog_run(GNOME_DIALOG(dialog));
  exit(255);
}

static void choice(GtkWidget *w, gpointer data)
{
  exit((const char **) data - choices);
}
