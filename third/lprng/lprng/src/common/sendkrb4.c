/* Kerberos 4 so-called authentication routines */

#include "lp.h"
#include "errorcodes.h"
#include "linksupport.h"
#include "timeout.h"

int Send_krb4_auth(char *host, char *printer, int *sock,
		   struct control_file *cfp, int transfer_timeout)
{
  KTEXT_ST ticket;
  char buffer[1];
  char line[LINEBUFFER];
  int status, ack, i;

  setstatus(cfp, "sending krb4 auth to %s@%s", printer, host);
  plp_snprintf(line, sizeof(line), "%c%s\n", REQ_K4AUTH, printer);
  status = Link_send(host, sock, transfer_timeout, line, strlen(line), &ack);
  if (status)
    {
      setstatus(cfp, "Printer %s@%s does not support krb4 authentication",
		printer, host);
      if (Interactive)
	{
	  strcat(line, "\n" );
	  if (Write_fd_str( 2, line ) < 0)
	    cleanup(0);
	}
      return status;
    }
  status = krb_sendauth(0, *sock, &ticket, KLPR_SERVICE, host,
			krb_realmofhost(host), 0, NULL, NULL,
			NULL, NULL, NULL, "KLPRV0.1");
  if (status != KSUCCESS)
    {
      setstatus(cfp, "krb4 authentication failed to %s@%s", printer, host);
      if (Interactive)
	{
	  strcat(line, "\n" );
	  if (Write_fd_str( 2, line ) < 0)
	    cleanup(0);
	}
      Link_close(sock);
      return JABORT;
    }

  buffer[0] = 0;
  i = Read_fd_len_timeout(transfer_timeout, *sock, buffer, 1);
  Clear_timeout();

  if (i <= 0 || Alarm_timed_out)
    status = LINK_TRANSFER_FAIL;
  else if (buffer[0])
    status = LINK_ACK_FAIL;

  if (status)
    {
      setstatus(cfp, "krb4 authentication failed to %s@%s", printer, host);
      if (Interactive)
	{
	  strcat(line, "\n" );
	  if (Write_fd_str( 2, line ) < 0)
	    cleanup(0);
	}
      Link_close(sock);
    }

  return status;
}
