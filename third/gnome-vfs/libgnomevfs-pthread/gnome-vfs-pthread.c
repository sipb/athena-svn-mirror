#include "gnome-vfs-pthread.h"

gboolean
gnome_vfs_pthread_init(gboolean init_deps)
{
  if(!g_threads_got_initialized)
    g_thread_init(NULL);

  return TRUE;
}
