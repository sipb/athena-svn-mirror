/* Pthread-friendly coroutines with pth
 * Copyright (C) 2002 Andy Wingo <wingo@pobox.com>
 *
 * cothreads.c: public API implementation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "pth_p.h" /* this pulls in everything */
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdlib.h>

/* older glibc's have MAP_ANON instead of MAP_ANONYMOUS */
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

typedef enum _cothread_block_state cothread_block_state;
typedef struct _cothread_chunk cothread_chunk;
typedef struct _cothread_private cothread_private;


enum _cothread_block_state
{
  COTHREAD_BLOCK_STATE_UNUSED=0,
  COTHREAD_BLOCK_STATE_MAPPED,
  COTHREAD_BLOCK_STATE_IN_USE
};

struct _cothread_chunk {
  cothread_block_state *block_states;
  char *chunk;
  int size;
  int reserved_bottom;
  int nblocks;
};

struct _cothread_private {
  cothread self;
  int argc;
  void **argv;
  void (*func) (int argc, void **argv);
  cothread_chunk *chunk;
  cothread *main;
};


static void*		cothread_private_set	(cothread_chunk *chunk, char *sp, void *priv, size_t size);
static void*		cothread_private_get	(cothread_chunk *chunk, char *sp, void *priv, size_t size);
static void		cothread_stub		(void);
static cothread_chunk*	cothread_chunk_new	(glong size, gint nblocks);
#ifdef USE_GTHREADS
static void		cothread_chunk_free	(cothread_chunk *chunk);
#endif
static gboolean		cothread_stack_alloc	(cothread_chunk *chunk, char **low, char **high);
static cothread_chunk*	cothreads_get_chunk	(void);


/* defaults */
static glong _cothreads_chunk_size = 0x200000;	/* 2MB */
static gint _cothreads_count = 16;		/* = 128 KB stack size */

static gboolean _cothreads_initialized = FALSE;

#ifdef USE_GTHREADS
static GStaticPrivate chunk_key = G_STATIC_PRIVATE_INIT;
#else
static cothread_chunk *_chunk = NULL;
#endif


/**
 * cothreads_initialized:
 *
 * Query the state of the cothreads system.
 *
 * Returns: TRUE if cothreads_init() has already been called, FALSE otherwise
 */
gboolean
cothreads_initialized (void) 
{
  return _cothreads_initialized;
}

/**
 * cothreads_init:
 * @stack_size: Desired pthread stack size, 0 for default value
 * @ncothreads: Maximum number of cothreads per pthread, 0 for default value
 *
 * Initialize the cothreads system. Cothreads must be initialized from the
 * application's main thread. setrlimit(2) is then called on the main thread to
 * ensure that enough space is available on the current process' stack.
 *
 * Returns: TRUE on success, FALSE otherwise
 */
gboolean
cothreads_init (glong stack_size, gint ncothreads)
{
  struct rlimit limit;
  
  if (cothreads_initialized ()) {
    g_warning ("cothread system has already been initialized");
    return FALSE;
  }

  if (!cothreads_init_thread (stack_size, ncothreads))
    return FALSE;

  if (stack_size)
    _cothreads_chunk_size = stack_size;
  if (ncothreads)
    _cothreads_count = ncothreads;
  
  _cothreads_initialized = TRUE;
  
  getrlimit (RLIMIT_STACK, &limit);
  COTHREADS_DEBUG ("current stack limit: %ld; stack max: %ld", limit.rlim_cur, limit.rlim_max);
  COTHREADS_DEBUG ("setting stack limit to %ld", _cothreads_chunk_size);
  limit.rlim_cur = _cothreads_chunk_size;
  if (setrlimit (RLIMIT_STACK, &limit)) {
    perror ("Could not increase the stack size, cothreads *NOT* initialized");
    _cothreads_initialized = FALSE;
    return FALSE;
  }

  return TRUE;
}

/**
 * cothreads_init_thread:
 * @stack_size: Desired pthread stack size, 0 for default value
 * @ncothreads: Maximum number of cothreads per pthread, 0 for default value
 *
 * Set the stack size for the current pthread. This function must be called
 * before the creation of any cothread within the current pthread, and may only
 * be called once. It is automatically called by cothreads_init() for the main
 * thread.
 *
 * Returns: TRUE on success, FALSE otherwise.
 */
gboolean
cothreads_init_thread (glong stack_size, gint ncothreads)
{
  gint i=0, j=0;
  glong tmp;
  cothread_chunk *chunk = NULL;
  
#ifdef USE_GTHREADS
  if ((chunk = g_static_private_get (&chunk_key)))
#else
  if ((chunk = _chunk))
#endif
  {
    g_warning ("thread has already been initialized");
    return FALSE;
  }

  for (i=0; i<sizeof(glong)*8; i++)
    if (stack_size & (1 << i))
      j++;
  if (j > 1) {
    g_warning ("cothreads_init(): argument stack_size must be a power of 2 (%ld given)", stack_size);
    return FALSE;
  }

  if (!stack_size)
    stack_size = _cothreads_chunk_size;
  if (!ncothreads)
    ncothreads = _cothreads_count;
  
  j=0;
  tmp = stack_size / ncothreads;
  for (i=0; i<sizeof(glong)*8; i++)
    if (tmp & (1 << i))
      j++;
  if (j != 1) {
    g_warning ("cothreads_init(): stack_size / ncothreads must be a power of 2");
    return FALSE;
  }

  chunk = cothread_chunk_new (stack_size, ncothreads);

#ifdef USE_GTHREADS
  g_static_private_set (&chunk_key, chunk, (GDestroyNotify) cothread_chunk_free);
#else
  _chunk = chunk;
#endif

  return TRUE;
}

static cothread_chunk*
cothreads_get_chunk (void) 
{
  cothread_chunk *chunk = NULL;
  
#ifdef USE_GTHREADS
  if (!(chunk = g_static_private_get (&chunk_key))) {
    cothreads_init_thread (0, 0);
    chunk = g_static_private_get (&chunk_key);
  }
#else
  if (!(chunk = _chunk)) {
    cothreads_init_thread (0, 0);
    chunk = _chunk;
  }
#endif

  return chunk;
}
  
/**
 * cothread_create:
 * @func: function to start with this cothread
 * @argc: argument count
 * @argv: argument vector
 * @main: cothread to switch back to if @func exits, or NULL
 *
 * Create a new cothread running a given function. You must explictly switch
 * into this cothread to give it control. If @func is NULL, a cothread is
 * created on the current stack with the current stack pointer.
 *
 * Returns: A pointer to the new cothread
 */
cothread*
cothread_create (cothread_func func, int argc, void **argv, cothread *main)
{
  char *low, *high, *dest;
  cothread_private priv;
  cothread *ret = NULL;
  cothread_chunk *chunk;
  char __csf;
  void *current_stack_frame = &__csf;
  
  chunk = cothreads_get_chunk ();

  low = high = NULL;
  memset (&priv, 0, sizeof (priv));
  priv.chunk = chunk;
  
  if (!func) {
    /* we are being asked to save the current thread into a new cothread. this
     * only happens for the first cothread in a thread. */
    low = current_stack_frame;
#if PTH_STACK_GROWTH > 0
    dest = (char*) ((gulong)low | (chunk->size / chunk->nblocks - 1))
      + 1 - getpagesize() * 2;
#else
    dest = (char*) ((gulong)low &~ (chunk->size / chunk->nblocks - 1))
      + getpagesize();
#endif
    COTHREADS_DEBUG ("about to mmap %p, size %d", dest, getpagesize());
    if (mmap (dest, getpagesize(), PROT_READ|PROT_WRITE,
              MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
      g_critical ("mmap failed, captain");
      return NULL;
    }
    ret = (cothread*) cothread_private_set (chunk, low, &priv, sizeof(priv));
    if (!cothread_stack_alloc (chunk, &low, &high))
      g_error ("couldn't create cothread 0");
    else
      COTHREADS_DEBUG ("created cothread 0 (%p) with low=%p, high=%p", ret, low, high);
    
    pth_mctx_save (ret);
    return ret;
  }
  
  if (!cothread_stack_alloc (chunk, &low, &high))
    g_error ("could not allocate a new cothread stack");
  
  priv.argc = argc;
  priv.argv = argv;
  priv.func = func;
  priv.main = main;
  ret = (cothread*)cothread_private_set (chunk, low, &priv, sizeof(priv));
  
  COTHREADS_DEBUG ("created a cothread %p with low=%p, high=%p", ret, low, high);
  
  pth_mctx_set (ret, cothread_stub, low, high);
  
  return ret;
}

/**
 * cothread_self:
 *
 * Gets the current cothread.
 *
 * Returns: A pointer to the current cothread
 */
cothread*
cothread_self (void)
{
  char __csf;
  void *current_stack_frame = &__csf;

  return (cothread*)cothread_private_get (cothreads_get_chunk(), current_stack_frame, NULL, sizeof (cothread_private));
}

/**
 * cothread_reset:
 * @self: the cothread, created by cothread_create
 * @func: function to start with this cothread
 * @argc: argument count
 * @argv: argument vector
 * @main: cothread to switch back to if @func exits, or NULL
 *
 * Reset a cothread to start running @func, as if it were newly created.
 */
void
cothread_reset (cothread *self, cothread_func func, int argc, void **argv, cothread *main)
{
  char *low, *high;
  cothread_chunk *chunk = ((cothread_private*)self)->chunk;
  
  low = (char*) ((gulong)self &~ (chunk->size / chunk->nblocks - 1));
  high = (char*) ((gulong)self | (chunk->size / chunk->nblocks - 1));

  pth_mctx_set (self, cothread_stub, low, high);

  cothread_setfunc (self, func, argc, argv, main);
}
  
/**
 * cothread_setfunc:
 * @self: the cothread, created by cothread_create
 * @func: function to start with this cothread
 * @argc: argument count
 * @argv: argument vector
 * @main: cothread to switch back to if @func exits, or NULL
 *
 * Like cothread_reset, but doesn't reset the machine context.
 */
void
cothread_setfunc (cothread *self, cothread_func func, int argc, void **argv, cothread *main)
{
  cothread_private *priv;

  priv = (cothread_private*) self;

  priv->argc = argc;
  priv->argv = argv;
  priv->func = func;
  priv->main = main;
}

/**
 * cothread_destroy:
 * @thread: cothread to destroy
 *
 * Deallocate any memory used by the cothread data structures.
 */
void
cothread_destroy (cothread *thread)
{
  gint i;
  cothread_private *private = (cothread_private*)thread;
  cothread_chunk *chunk = private->chunk;
  
#if PTH_STACK_GROWTH > 0
  i = (gint) ((gulong)thread - chunk->chunk) / (chunk->size / chunk->nblocks);
#else
  i = (gint) (chunk->chunk + chunk->size - (gulong)thread) / (chunk->size / chunk->nblocks);
#endif

  COTHREADS_DEBUG ("destroying cothread %d (%p, %p)", i, thread, chunk->chunk);

  if (i == 0) {
    g_warning ("You can't destroy cothread 0.");
    return;
  }
  
  g_return_if_fail (i > 0 && i <= chunk->nblocks);

  chunk->block_states[i] = COTHREAD_BLOCK_STATE_MAPPED;

#if 0
  /* unmap the cothread */
  low = (char*) ((gulong)thread | (chunk->size / chunk->nblocks - 1))
    + 1 - getpagesize() * 2;

  COTHREADS_DEBUG ("munmapping %p, size %d", low, chunk->size / chunk->nblocks);
  if (munmap (low, chunk->size / chunk->nblocks) != 0) {
    perror ("munmap failed");
    return;
  }
#endif
}

/**
 * cothreads_alloc_thread_stack:
 * @stack: Will be set to point to the allocated stack location
 * @stacksize: Will be set to the size of the allocated stack
 *
 * Allocates a stack for use with GThreads. Actually we need to patch GThread to
 * allow supplying the stack, so this is probably only for use with pthreads for now...
 *
 * Returns: #TRUE on success, #FALSE otherwise.
 */
gboolean
cothreads_alloc_thread_stack (void** stack, glong* stacksize)
{
#ifdef HAVE_POSIX_MEMALIGN
  if (posix_memalign (stack, _cothreads_chunk_size, _cothreads_chunk_size))
    return FALSE;
#else
  *stack = malloc (_cothreads_chunk_size * (1.0 + 1.0/_cothreads_count));
  if (!stack)
    return FALSE;
  *stack = (void*)((int)*stack &~ (int)(_cothreads_chunk_size / _cothreads_count - 1));
  *stack += 1;
#endif

  *stacksize = _cothreads_chunk_size;

  return TRUE;
}

/* the whole 'page size' thing is to allow for the last page of a stack or chunk
 * to be mmap'd as a boundary page */

static void*
cothread_private_set (cothread_chunk *chunk, char *sp, void *priv, size_t size)
{
  char *dest;
  
#if PTH_STACK_GROWTH > 0
  dest = (char*) ((gulong)sp | (chunk->size / chunk->nblocks - 1))
    - size + 1 - getpagesize();
#else
  dest = (char*) ((gulong)sp &~ (chunk->size / chunk->nblocks - 1))
    + getpagesize();
#endif
  
  COTHREADS_DEBUG ("setting %p to equal %p, size %d", dest, priv, size);
  memcpy (dest, priv, size);
  return dest;
}

static void*
cothread_private_get (cothread_chunk *chunk, char *sp, void *priv, size_t size)
{
  char *src;
  
#if PTH_STACK_GROWTH > 0
  src = (char*) ((gulong)sp | (chunk->size / chunk->nblocks - 1))
    - size + 1 - getpagesize();
#else
  src = (char*) ((gulong)sp &~ (chunk->size / chunk->nblocks - 1))
    + getpagesize();
#endif
  
  if (priv)
    memcpy (priv, src, size);

  return src;
}

static void
cothread_stub (void)
{
  cothread_private *priv;
  
  priv = (cothread_private*) cothread_self();

  /* this idiom is stupid but it's what the old cothreads does. to quote:
   * "we do this to avoid ever returning, we just switch to 0th thread"
   */
  while (TRUE) {
    priv->func (priv->argc, priv->argv);
    
    if (priv->main) {
      COTHREADS_DEBUG ("returning from cothread %p to %p", &priv->self, priv->main);
      cothread_switch (&priv->self, priv->main);
    }
  }
  
  g_critical ("returning from cothread_stub!");
}

static gboolean
cothread_stack_alloc (cothread_chunk *chunk, char **low, char **high)
{
  int block;
    
  *low = 0;
  *high = 0;
  
  if (chunk->block_states[0] == COTHREAD_BLOCK_STATE_UNUSED) {
    chunk->block_states[0] = COTHREAD_BLOCK_STATE_IN_USE;
#if PTH_STACK_GROWTH > 0
    *low  = chunk->chunk + chunk->reserved_bottom;
    *high = chunk->chunk + chunk->size / chunk->nblocks - 1;
#else
    *low  = chunk->chunk + chunk->size * (chunk->nblocks - 1) / chunk->nblocks;
    *high = chunk->chunk + chunk->size - chunk->reserved_bottom - 1;
#endif
    return TRUE;
  }
    
  for (block = 1; block < chunk->nblocks; block++) {
    if (chunk->block_states[block] == COTHREAD_BLOCK_STATE_UNUSED ||
        chunk->block_states[block] == COTHREAD_BLOCK_STATE_MAPPED) {
#if PTH_STACK_GROWTH > 0
      *low  = chunk->chunk + chunk->size * block / chunk->nblocks;
#else
      *low  = chunk->chunk + chunk->size * (chunk->nblocks - block - 1) / chunk->nblocks;
#endif
      *high = *low + chunk->size / chunk->nblocks - 1;
      
      if (chunk->block_states[block] != COTHREAD_BLOCK_STATE_MAPPED) {
        COTHREADS_DEBUG ("about to mmap cothread %d: %p, size %d", block, low, high - low + 1);
        if (mmap (*low, *high - *low + 1, PROT_EXEC|PROT_READ|PROT_WRITE,
                  MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
          g_critical ("mmap failed, captain");
          return FALSE;
        }
      }
  
      chunk->block_states[block] = COTHREAD_BLOCK_STATE_IN_USE;

      return TRUE;
    }
  }
  
  return FALSE;

}

/* size and size / nblocks must be a power of two. */
static cothread_chunk*
cothread_chunk_new (glong size, gint nblocks)
{
  cothread_chunk *ret;
  char __csf;
  void *current_stack_frame = &__csf;
  
  ret = g_new0 (cothread_chunk, 1);
  ret->nblocks = nblocks;
  ret->block_states = g_new0 (cothread_block_state, ret->nblocks);
  
  /* if we don't allocate the chunk, we must already be in it. */
  /* set ret->chunk equal to the lower end of the chunk */
  ret->chunk = (char*) ((unsigned long) current_stack_frame &~ (size - 1));
#if PTH_STACK_GROWTH > 0
  ret->reserved_bottom = (char *)current_stack_frame - ret->chunk + 1;
#else
  ret->reserved_bottom = ret->chunk + size - (char *)current_stack_frame + 1;
#endif
  
  ret->size = size;
  
  COTHREADS_DEBUG ("created new chunk, %p, size=0x%x, %d reserved", ret->chunk, ret->size,
		   ret->reserved_bottom);
  return ret;
}

#ifdef USE_GTHREADS
static void
cothread_chunk_free (cothread_chunk *chunk)
{
  g_free (chunk);
  /* is that it? */
}
#endif
