#include <cothreads/cothreads.h>
#include <errno.h>

#define ITER 200

void co_thread (int argc, void **argv)
{
  int cothreadnum = *(((int**)argv)[0]);
  
  printf ("%d: returning to cothread 0\n", cothreadnum);
}

int main (int argc, char *argv[])
{
  int i;
  void *_argv[1];
  int cothread_num = 1;
  cothread *main, *new;

#ifdef USE_GTHREADS
  g_thread_init(NULL);
#endif

  cothreads_init(0, 0);

  main = cothread_create (NULL, 0, NULL, NULL);
  
  for (i=0; i<ITER; i++) {
    printf ("0: spawning a new cothread (iteration %d)\n", i + 1);
    
    _argv[0] = (void*) &cothread_num;
    new = cothread_create (co_thread, 1, _argv, main);
    
    printf ("0: switching to cothread %d...\n", cothread_num);

    cothread_switch (main, new);

    printf ("0: destroying cothread %d...\n", cothread_num);

    cothread_destroy (new);
  }
  
  printf ("exiting\n");
  
  exit (0);
}

