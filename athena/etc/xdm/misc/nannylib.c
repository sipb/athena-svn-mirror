#include <stdlib.h>
#include "nanny.h"
#include "pc.h"
#include "var.h"
#include "cvt.h"

int nanny_exchangeVars(varlist *vin, varlist **vout)
{
  pc_state *ps;
  pc_port *outport;
  pc_message *message;
  buffer *buf;
  int ret;

  if (vin == NULL || vout == NULL)
    return 1;

  message = malloc(sizeof(pc_message));
  if (message == NULL)
    return 1;

  if (cvt_vars2buf(&buf, vin))
    {
      free(message);
      return 1;
    }

  message->data = buf->buf;
  message->length = buf->len;

  if (pc_openport(&outport, NANNYPORT))
    {
      free(buf->buf);
      free(buf);
      free(message);
      return 1;
    }

  message->source = outport;
  ret = pc_send(message);

  free(buf->buf);
  free(message);

  if (ret)
    {
      pc_close(outport);
      free(buf);
      return 1;
    }

  if (pc_init(&ps))
    {
      pc_close(outport);
      free(buf);
      return 1;
    }

  pc_addport(ps, outport);
  ret = pc_wait(&message, ps);

  pc_removeport(ps, outport);
  pc_destroy(ps);
  pc_close(outport);

  if (ret)
    {
      free(buf);
      return 1;
    }

  buf->buf = message->data;
  buf->len = message->length;

  if (var_init(vout))
    {
      pc_freemessage(message);
      free(buf);
      return 1;
    }

  if (cvt_buf2vars(*vout, buf))
    {
      pc_freemessage(message);
      free(buf);
      var_destroy(*vout);
      *vout = NULL;
      return 1;
    }

  pc_freemessage(message);
  free(buf);

  return 0;
}

int nanny_setupUser(char *name, int add, char **env, char **args)
{
  varlist *vsend = NULL, *vget = NULL;
  buffer *buf = NULL;
  char *value;

#define C(stuff) if (stuff) goto cleanup

  if (name == NULL || env == NULL || args == NULL)
    return 1;

  if (var_init(&vsend))
    return 1;

  C(var_setString(vsend, N_USER, name));

  C(var_setString(vsend, N_RMUSER, add ? "1" : "0"));

  C(cvt_strings2buf(&buf, env));
  C(var_setValue(vsend, N_ENV, buf->buf, buf->len));
  free(buf->buf);  free(buf);  buf = NULL;

  C(cvt_strings2buf(&buf, args));
  C(var_setValue(vsend, N_XSESSARGS, buf->buf, buf->len));
  free(buf->buf);  free(buf);  buf = NULL;

  C(nanny_exchangeVars(vsend, &vget));
  var_destroy(vsend);  vsend = NULL;

  C(var_getString(vget, N_USER, &value));
  if (strcmp(value, "OK"))
    {
      var_destroy(vget);
      return 1;
    }

  var_destroy(vget);
  return 0;

 cleanup:
  if (vsend)
    var_destroy(vsend);
  if (buf)
    {
      if (buf->buf)
	free(buf->buf);
      free(buf);
    }
  if (vget)
    var_destroy(vget);

  return 1;
}

int nanny_loginUser(char ***env, char ***args)
{
  varlist *vsend = NULL, *vget = NULL;
  buffer buf;
  char *value;
  int len;

  if (env == NULL || args == NULL)
    return 1;

  *env = NULL;
  *args = NULL;

  if (var_init(&vsend))
    return 1;

  C(var_setString(vsend, N_LOGGED_IN, "TRUE"));
  C(var_setString(vsend, N_ENV, cvt_query));
  C(var_setString(vsend, N_XSESSARGS, cvt_query));

  C(nanny_exchangeVars(vsend, &vget));
  var_destroy(vsend);  vsend = NULL;

  C(cvt_var2strings(vget, N_ENV, env));
  C(cvt_var2strings(vget, N_XSESSARGS, args));

  /* XXX Check "LOGGED_IN=OK" */

  var_destroy(vget);
  return 0;

 cleanup:
  if (vsend)
    var_destroy(vsend);
  if (vget)
    var_destroy(vget);
  if (*env)
    cvt_freeStrings(*env);

  return 1;
}

int nanny_getTty(char *tty, int ttylen)
{
  varlist *vsend = NULL, *vget = NULL;
  char *value;

  if (tty == NULL)
    return 1;

  if (var_init(&vsend))
    return 1;

  C(var_setString(vsend, N_TTY, cvt_query));

  C(nanny_exchangeVars(vsend, &vget));
  var_destroy(vsend);  vsend = NULL;

  C(var_getString(vget, N_TTY, &value));

  strncpy(tty, value, ttylen);
  var_destroy(vget);

  return 0;

 cleanup:
  if (vsend)
    var_destroy(vsend);
  if (vget)
    var_destroy(vget);

  return 1;
}
