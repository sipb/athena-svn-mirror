/* -*- Mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
#include <config.h>

/* needed for sigaction and friends under 'gcc -ansi -pedantic' on 
 * GNU/Linux */
#ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE 1
#endif
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>

#include <unistd.h>
#include <gnome.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void run_in_terminal(const gchar *command);

int main(int argc, char *argv[])
{
  GtkWidget *mainwin;
  gchar* msg;
  struct sigaction sa;
  poptContext ctx;
  const char **args;
  const char *app_version = NULL;
  int res;
  int rescount = 0;
  int close_res = -1;
  int bug_buddy_res = -1;
  int debug_res = -1;
  int olc_res = -1;
  int sendbug_res = -1;
  gchar *appname;
  gchar *bug_buddy_path = NULL;
  gchar *debugger = NULL;
  gchar *debugger_path = NULL;
  
  int bb_sm_disable = 0;

  /* We do this twice to make sure we don't start running ourselves... :) */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigaction(SIGSEGV, &sa, NULL);


  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  /* in case gnome-session is segfaulting :-) */
  gnome_client_disable_master_connection();

  putenv("GNOME_DISABLE_CRASH_DIALOG=1"); /* Don't recurse */  
  gnome_init_with_popt_table("gnome_segv", VERSION, argc, argv, NULL, 0, &ctx);

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigaction(SIGSEGV, &sa, NULL);

  args = poptGetArgs(ctx);
  if (args && args[0] && args[1])
    {
      if (strcmp(g_filename_pointer (args[0]), "gnome-session") == 0)
        {
          msg = g_strdup_printf(_("The GNOME Session Manager (process %d) has crashed\n"
                                  "due to a fatal error (%s).\n"
                                  "When you close this dialog, all applications will close "
                                  "and your session will exit.\nPlease save all your files "
                                  "before closing this dialog."),
                                getppid(), g_strsignal(atoi(args[1])));
          bb_sm_disable = 1;
        }
      else
        {
          msg = g_strdup_printf(_("Application \"%s\" (process %d) has crashed"
                                  " due to a\nfatal error.\n(%s)\n\n"
                                  "You may choose to run olc to check with an"
                                  " On-Line Consultant\nabout this bug;"
                                  " otherwise, please consider running sendbug"
                                  " to\nsubmit an Athena bug report."),
                                args[0], getppid(), g_strsignal(atoi(args[1])));
        }
	if(args[2])
		app_version = args[2];
    }
  else
    {
      fprintf(stderr, _("Usage: gnome_segv appname signum\n"));
      return 1;
    }
  appname = g_strdup(args[0]);

  mainwin = gnome_message_box_new(msg,
                                  GNOME_MESSAGE_BOX_ERROR,
                                  GNOME_STOCK_BUTTON_CLOSE,
                                  NULL);
  close_res = rescount++;
  
  bug_buddy_path = gnome_is_program_in_path ("bug-buddy");
  if (0 && bug_buddy_path != NULL)
    {
      gnome_dialog_append_button(GNOME_DIALOG(mainwin),
                                 _("Submit a bug report"));
      bug_buddy_res = rescount++;
    }

  debugger = getenv("GNOME_DEBUGGER");
  if (debugger && strlen(debugger)>0)
  {
    debugger_path = gnome_is_program_in_path (debugger);
    if (debugger_path != NULL)
      {
        gnome_dialog_append_button(GNOME_DIALOG(mainwin),
                                   _("Debug"));
        debug_res = rescount++;
      }
  }
  
  gnome_dialog_append_button(GNOME_DIALOG(mainwin), _("Run olc"));
  olc_res = rescount++;

  gnome_dialog_append_button(GNOME_DIALOG(mainwin), _("Submit a bug report"));
  sendbug_res = rescount++;

  g_free(msg);

  res = gnome_dialog_run(GNOME_DIALOG(mainwin));

  if (res == -1 || res == close_res)
    {
      return 0;
    }
  if (res == bug_buddy_res)
    {
      gchar *exec_str;
      int retval;

      g_assert(bug_buddy_path);
      exec_str = g_strdup_printf("%s --appname=\"%s\" --pid=%d "
                                 "--package-ver=\"%s\" %s", 
                                 bug_buddy_path, appname, getppid(), 
                                 app_version, bb_sm_disable 
                                 ? "--sm-disable" : "");

      retval = system(exec_str);
      g_free(exec_str);
      if (retval == -1 || retval == 127)
        {
          g_warning("Couldn't run bug-buddy: %s", g_strerror(errno));
        }
    }
  else if (res == debug_res)
    {
      gchar *exec_str;
      int retval;

      g_assert (debugger_path);
      exec_str = g_strdup_printf("%s --appname=\"%s\" --pid=%d "
                                 "--package-ver=\"%s\" %s", 
                                 debugger_path, appname, getppid(), 
                                 app_version, bb_sm_disable 
                                 ? "--sm-disable" : "");

      retval = system(exec_str);
      g_free(exec_str);
      if (retval == -1 || retval == 127)
        {
          g_warning("Couldn't run debugger: %s", g_strerror(errno));
        }
    }
  else if (res == olc_res)
    {
      run_in_terminal("olc");
    }
  else if (res == sendbug_res)
    {
      gchar *exec_str;

      exec_str = g_strdup_printf("sendbug %s", appname);
      run_in_terminal(exec_str);
      g_free(exec_str);
    }

  return 0;
}

static void run_in_terminal(const gchar *command)
{
  gchar *exec_str;
  gchar *terminal_command = "/usr/athena/bin/gnome-terminal --command=";
  int retval;

  exec_str = g_strdup_printf("%s'%s'", terminal_command, command);
  retval = system(exec_str);
  g_free(exec_str);
  if (retval == -1)
    {
      g_warning("Couldn't run %s in terminal: %s",
                command, g_strerror(errno));
    }
}
