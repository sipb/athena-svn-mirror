#include "xolc.h"

int
handle_response(response, req)
     int response;
     REQUEST *req;
{
#ifdef KERBEROS
  char *kmessage = "\nYou will have been properly authenticated when you do not see this\nmessage the next time you run olc. If you were having trouble\nwith a program, try again.\n\nIf you continue to have difficulty, feel free to contact a user\nconsultant by phone. Schedules and phone numbers are posted in\nthe clusters.";
#endif KERBEROS

  switch(response)
    {
    case UNKNOWN_REQUEST:
      fprintf(stderr,"This function cannot be performed by the OLC server.\n");
      fprintf(stderr, "What you want is down the hall to the left.\n");
      return(NO_ACTION);

    case SIGNED_OFF:
      printf("You have signed off of OLC.\n");
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(string_eq(req->target.username,req->requester.username))
        fprintf(stderr, "You are not signed on to OLC.\n");
      else
        fprintf(stderr, "%s (%d) is not signed on to OLC.\n",
                req->target.username,req->target.instance);
      return(NO_ACTION);

    case NO_QUESTION:
      fprintf(stderr,"%s (%d) does not have a question.\n",
	      req->target.username, req->target.instance);
      return(ERROR);

    case HAS_QUESTION:
      fprintf(stderr,"You have a question.\n");
      return(ERROR);

    case NOT_CONNECTED:
      if(string_eq(req->target.username,req->requester.username))
        {
          fprintf(stderr, "You are not connected nor do you seem to have a question.\n");
          fprintf(stderr,"Perhaps you should ask one.\n");
        }
      else
        fprintf(stderr,"%s (%d) is not connected nor is asking a question.\n",
                req->target.username,req->target.instance);
      return(NO_ACTION);

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to do that.\n");
      return(NO_ACTION);

    case TARGET_NOT_FOUND:
      fprintf(stderr,"Target user %s (%d) unknown.\n",  req->target.username,
              req->target.instance);
      return(ERROR);

    case REQUESTER_NOT_FOUND:
      fprintf(stderr,"You [%s (%d)] are unknown.\n",
              req->requester.username,
              req->requester.instance);
      return(ERROR);

    case INSTANCE_NOT_FOUND:
      fprintf(stderr,"Incorrect instance specified.\n");
      return(ERROR);

    case ERROR:
      fprintf(stderr, "Error response from daemon.\n");
      return(ERROR);

    case USER_NOT_FOUND:
      fprintf(stderr,"User, %s, not found.\n",req->target.username);
      return(ERROR);

#ifdef KERBEROS     /* these error codes are < 100 */
    case MK_AP_TGTEXP:
    case RD_AP_EXP:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Your Kerberos ticket has expired. ");
      printf("To renew your Kerberos tickets,\n");
      printf("type:    kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case NO_TKT_FIL:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("You do not have a Kerberos ticket file. To ");
      printf("get one, \ntype:    kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case TKT_FIL_ACC:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Cannot access your Kerberos ticket file.\n");
      printf("Try:              setenv   KRBTKFILE  /tmp/random\n");
      printf("                  kinit\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
    case RD_AP_TIME:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Kerberos authentication failed: workstation clock is");
      printf("incorrect.\nPlease contact Athena operations and move to ");
      printf("another worstation.\n");
      if(OLC)
        printf("%s\n",kmessage);
      exit(ERROR);
#endif KERBEROS

    case SUCCESS:
      return(SUCCESS);

    default:
      fprintf(stderr, "Unknown response %d (fascinating)\n", response);
      return(ERROR);
    }
}
