#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var.h"
#include "cvt.h"

char *cvt_query = "N_QUERY";

int cvt_buf2vars(varlist *vl, buffer *buf)
{
  char *name, *value, *ptr;

  if (vl == NULL || buf == NULL)
    return 1;

  ptr = buf->buf;
  while (ptr < buf->buf + buf->len)
    {
      name = ptr;
      ptr = strchr(name, '=');
      if (ptr == NULL)
	{
	  var_setString(vl, name, cvt_query);
	  ptr = name + strlen(name) + 1;
	}
      else
	{
	  *ptr = '\0';
	  value = ++ptr;
	  var_setString(vl, name, value);
	  ptr += strlen(ptr) + 1;
	}
    }

  return 0;
}

int cvt_vars2buf(buffer **buf, varlist *vl)
{
  char **vlist, **ptr;
  buffer *b;
  void *data;
  int len, size = 0;
  int offset = 0;

  if (vl == NULL || buf == NULL)
    return 1;

  b = malloc(sizeof(buffer));
  if (b == NULL)
    return 1;

  var_listVars(vl, &vlist);
  ptr = vlist;
  while (*ptr)
    {
      var_getValue(vl, *ptr, &data, &len);
      size += len + strlen(*ptr) + 1;
      ptr++;
    }

  b->buf = malloc(size);
  if (b->buf == NULL)
    {
      free(b);
      var_freeVars(vl, vlist);
      return 1;
    }

  b->len = size;
  ptr = vlist;
  while (*ptr)
    {
      var_getValue(vl, *ptr, &data, &len);
      memcpy(b->buf + offset, *ptr, strlen(*ptr));
      offset += strlen(*ptr);
      b->buf[offset++] = '=';
      memcpy(b->buf + offset, data, len);
      offset += len;
      ptr++;
    }

  var_freeVars(vl, vlist);
  *buf = b;
}
