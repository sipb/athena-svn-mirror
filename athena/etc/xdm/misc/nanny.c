#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "pc.h"

char *name;
#define NANNYPORT "/tmp/nanny"

void process(pc_message *message)
{
  fprintf(stdout, "%s\n", message->data);
  if (message->source)
    pc_send(message);
}

int main(int argc, char **argv)
{
  pc_state *ps;
  pc_port *inport, *outport;
  pc_message *message;
  char *option;

  name = argv[0];

  if (argc == 2)
    option = argv[1];
  else
    option = "ping";

/*  openlog(name, LOG_PID, LOG_USER); */

  ps = pc_init();

  /* Initialize message to send to nanny. We set source to NULL
     in case we wind up sending the message to ourselves. */
  message = malloc(sizeof(pc_message));
  message->data = option;
  message->length = strlen(option) + 1;
  message->source = NULL;

  /* If there's a nanny already running, give it the command. */
  outport = pc_openport(NANNYPORT);
  if (outport)
    {
      message->source = outport;
      pc_send(message);
      free(message);

      pc_addport(ps, outport);
      message = pc_wait(ps);
      if (message != NULL)
	printf("%s\n", message->data);
      exit(0);
    }

  /* There's not a nanny running, so we take over. */

  /* If we're trying to kill the running nanny, we know it doesn't
     exist, so just exit. */
  if (!strcmp(option, "-die"))
    exit(0);

  printf("no nanny contacted; running\n");

  /* Initialize our port for new connections. */
  inport = pc_makeport(NANNYPORT, PC_GRAB|PC_CHMOD, 0, 0, 0700);
  if (inport == NULL)
    {
      printf("couldn't make the port\n");
      exit(1);
    }
  pc_addport(ps, inport);

  /* Process the command line option as though we have received it
     from the port before we start processing from the port. */
  process(message);

  /* fork() here to tell caller the message has been delivered */

  while (1)
    {
      message = pc_wait(ps);

      switch(message->type)
	{
	case PC_NEWCONN:
	  pc_addport(ps, message->data);
	  break;
	case PC_DATA:
	  process(message);
	  pc_removeport(ps, message->source);
	  pc_close(message->source);
	  break;
	case PC_SIGNAL:
	case PC_FD:
	default:
	  fprintf(stdout, "type=%d\n", message->type);
	  break;
	}

      pc_freemessage(message);
    }
}
