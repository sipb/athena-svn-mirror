/* cothreads.c: the cothreads regression test */

/* #undef USE_GTHREADS */

#include <cothreads/cothreads.h>
#include <cothreads/config-private.h>
#ifdef USE_GTHREADS
# include <pthread.h>
#endif
#include <errno.h>

#define NGTHREADS 5
#define MAIN_STACK_SIZE 0x0800000 	/* Apparently 6 MB is the largest stack
                                         * size available in linux while running
                                         * threads, so go with aligned 4 MB? */
#define MAIN_STACK_NCOTHREADS 128	/* 64K stack size */
#define THREAD_STACK_SIZE 0x0200000	/* 2 MB. Take it easy on non-i686 libc */
#define THREAD_STACK_NCOTHREADS 32	/* 64K stack size */

void co_thread (int argc, void **argv)
{
  int pthreadnum =  *(int*)argv[0];
  int cothreadnum = *(int*)argv[1];
  cothread *self = argv[2];
  
  printf ("%d.%d: sleeping 1s...\n", pthreadnum, cothreadnum);
/*  sleep (1); */
  printf ("%d.%d: returning to cothread 0\n", pthreadnum, cothreadnum);

  /* the switch back to the main thread happens automagically */
}

void *pthread (void* _pthreadnum) 
{
  int pthreadnum = *(int*) _pthreadnum;
  int cothreadnum = 1; /* 1 because we create 0 with cothread_create (NULL, ...) */
  int ncothreads;
  cothread *main, *new;
  void *argv[3];
  
  if (pthreadnum > 0) {
    cothreads_init_thread (THREAD_STACK_SIZE, THREAD_STACK_NCOTHREADS);
    ncothreads = THREAD_STACK_NCOTHREADS;
  } else {
    ncothreads = MAIN_STACK_NCOTHREADS;
  }

  main = cothread_create (NULL, 0, NULL, NULL);
  
  while (cothreadnum++ < ncothreads) {
    printf ("%d: spawning a new cothread\n", pthreadnum);
    
    argv[0] = &pthreadnum;
    argv[1] = &cothreadnum;
    argv[2] = cothread_create (co_thread, 3, argv, main);
    new = (cothread*)argv[2];
    
    printf ("%d: switching to cothread %d...\n", pthreadnum, cothreadnum);
    cothread_switch (main, new);
  }
  return NULL;
}

int main (int argc, char *argv[])
{
#ifdef USE_GTHREADS
  pthread_attr_t attr[NGTHREADS];
  pthread_t thread[NGTHREADS];
  void *stack;
#endif
  int pthreadnum[NGTHREADS], i;

#ifdef USE_GTHREADS
  g_thread_init (NULL);
#endif

  cothreads_init (MAIN_STACK_SIZE, MAIN_STACK_NCOTHREADS);

#ifdef USE_GTHREADS
  printf ("0: creating the gthreads\n");
  for (i = 0; i < NGTHREADS; i++) {
    pthreadnum[i] = i + 1;

#ifdef HAVE_POSIX_MEMALIGN
    if (posix_memalign (&stack, MAIN_STACK_SIZE, MAIN_STACK_SIZE))
      perror ("allocating pthread stack of size %d", MAIN_STACK_SIZE);
#else
    if (stack = (void *) valloc (MAIN_STACK_SIZE))
      perror ("allocating pthread stack of size %d", MAIN_STACK_SIZE);
#endif

    pthread_attr_init (&attr[i]);

#ifdef HAVE_PTHREAD_ATTR_SETSTACK
    if (!pthread_attr_setstack (&attr[i], stack, MAIN_STACK_SIZE))
      perror ("setting stack size and address");
#else
    if (!pthread_attr_setstackaddr (&attr[i], stack))
      perror ("setting stack address");
    if (!pthread_attr_setstacksize (&attr[i], MAIN_STACK_SIZE))
      perror ("setting stack size");
#endif
    
    pthread_create (&thread[i], &attr[i], pthread, &pthreadnum[i]);
  }
  
  for (i=0; i<NGTHREADS; i++) {
    printf ("0: joining pthread %d\n", i);
    pthread_join (thread[i], NULL);
  }
#endif

  printf ("0: calling the pthread function directly\n");
  pthreadnum[0] = 0;
  pthread (&pthreadnum[0]);
  
  printf ("exiting\n");
  
  exit (0);
}

