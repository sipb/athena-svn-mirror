#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var.h"
#include "cvt.h"

char *cvt_query = "N_QUERY";

/* Destructive to the buffer... */
int cvt_buf2vars(varlist *vl, buffer *buf)
{
  char *name, *value, *ptr, *ptr1;
  int len;

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

	  if (ptr1 = strchr(name, '[')) /* not just a string */
	    {
	      ptr1++;
	      len = atoi(ptr1);
	      *ptr1 = '\0'; /* we keep the [ in the name so we know... */
	      var_setValue(vl, name, value, len);
	      ptr += len;
	    }
	  else /* plain ol' string */
	    {
	      var_setString(vl, name, value);
	      ptr += strlen(ptr) + 1;
	    }
	}
    }

  return 0;
}

int cvt_strings2buf(buffer **buf, char **strings)
{
  char **ptr, *copy;
  int len = 0;
  buffer *b;

  if (buf == NULL || strings == NULL)
    return 1;

  b = malloc(sizeof(buffer));
  if (b == NULL)
    return 1;

  for (ptr = strings; *ptr != NULL; ptr++)
    len += strlen(*ptr) + 1;

  b->buf = malloc(len);
  if (b->buf == NULL)
    {
      free(b);
      return 1;
    }
  b->len = len;

  copy = b->buf;
  for (ptr = strings; *ptr != NULL; ptr++)
    {
      strcpy(copy, *ptr);
      copy += strlen(*ptr) + 1;
    }

  *buf = b;
  return 0;
}

int cvt_vars2buf(buffer **buf, varlist *vl)
{
  char **vlist, **ptr;
  buffer *b;
  void *data;
  char lenstr[7];
  int len, size = 0;
  int offset = 0;

  if (vl == NULL || buf == NULL)
    return 1;

  b = malloc(sizeof(buffer));
  if (b == NULL)
    return 1;

  var_listVars(vl, &vlist);
  for (ptr = vlist; *ptr != NULL; ptr++)
    {
      var_getValue(vl, *ptr, &data, &len);
      size += len + strlen(*ptr) + 1; /* 1 for = */
      if (strchr(*ptr, '['))
	{
	  sprintf(lenstr, "%d", len);
	  size += strlen(lenstr) + 1; /* 1 for ] */
	}
    }

  b->buf = malloc(size);
  if (b->buf == NULL)
    {
      free(b);
      var_freeList(vl, vlist);
      return 1;
    }

  b->len = size;
  for (ptr = vlist; *ptr != NULL; ptr++)
    {
      var_getValue(vl, *ptr, &data, &len);
      memcpy(b->buf + offset, *ptr, strlen(*ptr));
      offset += strlen(*ptr);
      if (b->buf[offset - 1] == '[')
	{
	  sprintf(&(b->buf[offset]), "%d", len);
	  offset += strlen(&(b->buf[offset]));
	  b->buf[offset++] = ']'; /* sugar */
	}
      b->buf[offset++] = '=';
      memcpy(b->buf + offset, data, len);
      offset += len;
    }

  var_freeList(vl, vlist);
  *buf = b;
}
