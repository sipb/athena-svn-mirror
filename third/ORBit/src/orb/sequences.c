#include "orbit.h"
#include "sequences.h"

gpointer CORBA_sequence_octet_free(gpointer mem,
				   gpointer func_data)
{
  CORBA_sequence_octet *seqo = mem;

  if(seqo->_release)
    CORBA_free(seqo->_buffer);

  return (gpointer)((guchar *)mem + sizeof(CORBA_sequence_octet));
}

CORBA_octet *
CORBA_octet_allocbuf(CORBA_unsigned_long len)
{
  return (CORBA_octet *)ORBit_alloc(len, NULL, NULL);
}

CORBA_sequence_octet *CORBA_sequence_octet__alloc(void)
{
  CORBA_sequence_octet *seqo;

  seqo = ORBit_alloc(sizeof(CORBA_sequence_octet),
		     (ORBit_free_childvals)CORBA_sequence_octet_free,
		     GUINT_TO_POINTER(1));

  seqo->_length = seqo->_maximum = 0;
  seqo->_buffer = NULL;
  seqo->_release = CORBA_TRUE;

  return seqo;
}

