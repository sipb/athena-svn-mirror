/*


  					W3C Sample Code Library libwww Chunk Class


!
  The Chunk Class
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

The Chunk Class &nbsp;defines a way to automatically handle dynamic strings
and other data types. You create a chunk with an initial size and it will
then automatically grow to accomodate added data to the chunk. It is a general
utility module. It is garanteed that the array is '\0' terminated
at all times (and hence is a valid C type string). The method
HTChunkTerminate can be used to explicitly
add a terminating '\0' and then to include this character in
the chunk size. If left out, the terminating character is not considered
part of the chunk.

Note: The names without a "_" (made as a #define's) are
only provided for backwards compatibility and should not be used.

This module is implemented by HTChunk.c, and it is
a part of the  W3C Sample Code
Library.
*/

#ifndef HTCHUNK_H
#define HTCHUNK_H


/*
.
  The Chunk Class
.

This structure should not be referenced outside this module! We only keep
it here to maintain high performance. Don't use it directly!
*/

typedef struct {
	int	size;		/* In bytes			*/
	int	growby;		/* Allocation unit in bytes	*/
	int	allocated;	/* Current size of *data	*/
	char *	data;		/* Pointer to malloced area or 0 */
} HTChunk;

/*
.
  Create new chunk
.

Create a new chunk and specify the number of bytes to allocate at a time
when the chunk is later extended. Arbitrary but normally a trade-off time
vs. memory
*/

#define HTChunkCreate(growby) HTChunk_new(growby)
extern HTChunk * HTChunk_new (int growby);

/*
.
  Free a chunk
.

Free a chunk created by HTChunkCreatefrom memory
*/

#define HTChunkFree(ch) HTChunk_delete(ch)
extern void HTChunk_delete (HTChunk * ch);

/*
.
  Clear a chunk
.

Keep the chunk in memory but clear all data kept inside. This can be used
if you know that you can reuse the allocated memory instead of allocating
new memory.
*/

#define HTChunkClear(ch) HTChunk_clear(ch)
extern void HTChunk_clear (HTChunk * ch);

/*
.
  Ensure a Chunk has a Certain Amount of Free Space
.

Make sure that a chunk has a certain size. If this is not the case then the
chunk is expanded. Nothing is done if the current size if bigger than the
size requested.
*/

#define HTChunkEnsure(ch, s) HTChunk_ensure(ch, s)
extern void HTChunk_ensure (HTChunk * ch, int s);

/*
.
  Append a character to a chunk
.

Add the character and increment the size of the chunk by one character
*/

#define HTChunkPutc(ch, c) HTChunk_putc(ch, c)
extern void HTChunk_putc (HTChunk * ch, char c);

/*
.
  Append a string to a chunk
.

Add the string and increment the size of the chunk by the length of the string
(without the trailing zero)
*/

#define HTChunkPuts(ch, str) HTChunk_puts(ch, str)
extern void HTChunk_puts (HTChunk * ch, const char *str);

/*
.
  Append a block to a chunk
.

Add the block and increment the size of the chunk by the len
*/

extern void HTChunk_putb (HTChunk * ch, const char *block, int len);


/*
.
  Zero Terminate a chunk
.

As a chunk often is a dynamic string, it needs to be terminated by a zero
in order to be used in C. However, by default any chunk is
always zero terminated, so the only purpose of this function is to
increment the size counter with one corresponding to the zero.
*/

#define HTChunkTerminate(ch)	HTChunk_terminate(ch)
#define HTChunk_terminate(ch)	HTChunk_putc((ch), '\0')

/*
.
  Return Pointer to Data
.

This define converts a chunk to a normal char pointer so that it can be parsed
to any ANSI C string function.
*/

#define HTChunkData(me)         ((me) ? (me)->data : NULL)
#define HTChunk_data(me)         ((me) ? (me)->data : NULL)

/*
.
  CString conversions
.

A Chunk may be build from an allocated string. The chunk assumes control
of the passes string, elminating the need for additional allocations and
string copies.
Once a string is built, the chunk may be destroyed and the string kept around.
*/

extern HTChunk * HTChunk_fromCString	(char * str, int grow);
extern char * HTChunk_toCString		(HTChunk * ch);

/*
.
  Return Current Size
.

Returns the current size of the chunk
*/

#define HTChunkSize(me)         ((me) ? (me)->size : -1)
#define HTChunk_size(me)         ((me) ? (me)->size : -1)

/*
*/

#endif

/*

  

  @(#) $Id: HTChunk.h,v 1.1.1.1 2000-03-10 17:52:56 ghudson Exp $

*/
