/*
 * File descriptor Cache
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

/* When someone comes to clean up this code (and I hope it happens soon),
 * they should figure out if there is any need for this file-descriptor
 * caching stuff on contemporary OSs (ie, anything since Ultrix).  Note
 * that no actual data is cached, just an open file descriptor.
 *
 * Note that when the caching is removed, the rest of the code will still
 * need to have access to get_queue and get_log, but these can be modified
 * to use get_file_uncached.
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Id: fdcache.c,v 1.14 1999-03-06 16:49:13 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "rpd.h"
#include "server_defines.h"

#define inc_hand (clock_hand = (++clock_hand)&(CACHESIZE-1));

extern int errno;

static struct entry cache[CACHESIZE];
static struct entry *buckets[CACHESIZE];
static int clock_hand = 0;

/*
 * init_cache
 *
 * Sets up cache for use
 *
 * Returns: nothing
 */

void
init_cache()
{
  int i;

  for(i=0;i<CACHESIZE;i++) {
    cache[i].fd = -1;
    cache[i].use = 1;
    cache[i].question = NULL;
    cache[i].next = NULL;
    cache[i].prev = NULL;
  }
}

void
flush_cache()
{
  int i;

  for(i=0;i<CACHESIZE;i++) {
    if (cache[i].fd != -1)
      close(cache[i].fd);
    cache[i].fd = -1;
    cache[i].use = 1;
    if (cache[i].question != NULL)
      free(cache[i].question);
    cache[i].question = NULL;
    cache[i].next = NULL;
    cache[i].prev = NULL;
  }
}
  
/* Get the list of questions in the queue.
 * 
 * Returns: Pointer to buffer with the log of the question, NULL if the
 * question doesn't exist or there was an error.  Result will be
 * the length of the buffer, or negative for an error
 */

char *get_queue(int *result)
{
  return get_file_uncached(LIST_FILE_NAME, result);
}

/* Get the replay of the log for specified user and instance.
 * 
 * Returns: Pointer to buffer with the log of the question, NULL if the
 * question doesn't exist or there was an error.  Result will be
 * the length of the buffer, or negative for an error
 */

char *get_log(char *username, int instance, int *result, int censored)
{
  char filename[MAXPATHLEN];

  if (censored)			/* if censored, screw the cache stuff... */
    {
      sprintf(filename,"%s/%s_%d.log.censored",
	      LOG_DIRECTORY,username,instance);
      return get_file_uncached(filename, result);
    }
  else
    {
      sprintf(filename,"%s/%s_%d.log",
	      LOG_DIRECTORY,username,instance);
      return get_file_uncached(filename, result);
    }
}

/* Get the contents of a file, without trying to cache it.
 * 
 * Returns: Pointer to buffer with the specified file, NULL if the file
 * doesn't exist or there was an error.  Result will be the length of the
 * buffer, or negative for an error
 */

char *get_file_uncached(char *filename, int *result)
{
  int hash,found;
  int fd;
  int new;
  struct stat file_stat;
  char *ptr;

  fd = open(filename,O_RDONLY,0);
  if (fd < 0)
    {
      *result = 0;
      return(NULL);
    }
  if (fstat(fd,&file_stat) < 0)
    {
      syslog(LOG_ERR,"fstat: %m on %d",fd);
      close(fd);
      *result = errno;
      return(NULL);
    }
  /* Malloc buffer big enough for the file */
  ptr = malloc(file_stat.st_size);
  if (ptr == NULL)
    {
      syslog(LOG_ERR,"get_file_uncached: malloc: error alloc'ing %d bytes\n",
	     file_stat.st_size);
      close(fd);
      *result = -1;
      return(NULL);
    }
  if (read(fd, ptr, file_stat.st_size) != file_stat.st_size)
    {
      syslog(LOG_ERR,"fdcache: read: %m on %d",fd);
      close(fd);
      free(ptr);
      *result = -1;
      return(NULL);
    }
  close(fd);
  /* Set result to file size, and return pointer to text */
  *result = file_stat.st_size;
  return(ptr);
}


/* Get the contents of a file, possibly from a cached file descriptor.
 * Cache the file descriptor for later use.
 * 
 * Returns: Pointer to buffer with the specified file, NULL if the file
 * doesn't exist or there was an error.  Result will be the length of the
 * buffer, or negative for an error
 *
 * Note: get_file_cached has the same semantics as get_file_uncached,
 *   and can be replaced by it if the caching is deemed unnecessary
 */

char *get_file_cached(char *filename, int *result)
{
  int hash,found;
  int fd;
  int new;
  struct entry *head,*ptr;
  struct stat file_stat;

  /* Mark and Increment clock hand */
  cache[clock_hand].use = 1;
  inc_hand;

  hash = get_bucket_index(filename);
  head = buckets[hash];

  found = 0;
  ptr = head;
  while (ptr != NULL) {
    if ((ptr->use == 0) && (!strcmp(filename,ptr->filename))) {
      found = 1;
      break;
    }
    ptr = ptr->next;
  }

  if (found == 0) {
    /* not found in the cache; check disk */
    fd = open(filename,O_RDONLY,0);
    if (fd < 0) {
      *result = 0;
      return(NULL);
    }

    /* Get a free cache table entry, clearing if necessary */
    new = allocate_entry();

    /* copy infomration over */
    cache[new].fd = fd;
    cache[new].filename = malloc(strlen(filename)+1);
    if (! cache[new].filename) {
      syslog(LOG_ERR,"get_file_cached: malloc: error copying '%s'\n",
	     filename);
    }
    strcpy(cache[new].filename, filename);

    /* Stat the file to get size and last mod time */
    if (fstat(fd,&file_stat) < 0) {
      syslog(LOG_ERR,"fstat: %m on %d",fd);
      close(fd);
      cache[new].use = 1;
      cache[new].fd = -1;
      *result = errno;
      return(NULL);
    }

    /* Copy information into cache */
    cache[new].last_mod = file_stat.st_mtime;
    cache[new].length = file_stat.st_size;
    cache[new].inode = file_stat.st_ino;

    /* Malloc buffer big enough for the file */
    cache[new].question = malloc(file_stat.st_size);
    if (cache[new].question == NULL) {
      syslog(LOG_ERR,"get_file_cached: malloc: error alloc'ing %d bytes\n",
	      file_stat.st_size);
      close(fd);
      cache[new].use = 1;
      cache[new].fd = -1;
      *result = -1;
      return(NULL);
    }
    
    /* Read file into buffer */
    if (read(cache[new].fd,cache[new].question,cache[new].length) !=
	cache[new].length) {
      syslog(LOG_ERR,"fdcache: read: %m on %d",cache[new].fd);
      close(fd);
      cache[new].use = 1;
      cache[new].fd = -1;
      free(cache[new].question);
      *result = -1;
      return(NULL);
    }
    
    /* Add to the head of the linked list */
    /* Need to re-set head, since we may have deleted initial bucket when we */
    /* allocated a new cache entry */
    head = buckets[hash];
    cache[new].next = head;
    if (head != NULL)
      head->prev = &cache[new];
    buckets[hash] = &cache[new];

    /* Set result to file size, and return pointer to text */
    *result = cache[new].length;
    return(cache[new].question);
  }
  else {
    /* found in the cache! */
    ptr->use = 0;
    
    /* Check to see that it's a recent copy */
    if (stat(ptr->filename,&file_stat) < 0) {
      delete_entry(ptr);
      if (errno == ENOENT) /* Question gone; not an error */
	*result = 0;
      else {               /* Some other error */
	syslog(LOG_ERR,"fdcache: stat %m on %s",ptr->filename);
	*result = errno;
      }
	return(NULL);
    }

    /* Check to see it's the same file
       (thanks Dave!) */

    if (file_stat.st_ino != ptr->inode) {
      /* Nope, it's an imposter.  Close the current one, and treat as a new */
      /* question */
      delete_entry(ptr);
      return get_file_cached(filename,result);
    }

    /* Check to see if the cache is current */
    if (file_stat.st_mtime > ptr->last_mod) {
      /* File has been modified, need to re-read */
      free(ptr->question);
      ptr->length = file_stat.st_size;
      ptr->last_mod = file_stat.st_mtime;

      /* Alloc new amount of memory */
      ptr->question = malloc(file_stat.st_size);
      if (ptr->question == NULL) {
	syslog(LOG_ERR,"get_file_cached: malloc: error alloc'ing %d bytes\n",
		file_stat.st_size);
	delete_entry(ptr);
	*result = -1;
	return(NULL);
      }
      
      /* rewind file */
      if (lseek(ptr->fd,0,L_SET) == -1) {
	syslog(LOG_ERR,"get_file_cached: lseek: %m");
	delete_entry(ptr);
	*result = -1;
	return(NULL);
      }

      /* Read file into buffer */
      if (read(ptr->fd,ptr->question,ptr->length) != ptr->length) {
	syslog(LOG_ERR,"get_file_cached: read: %m");
	delete_entry(ptr);
	*result = -1;
	return(NULL);
      }
    }
    
    /* Return size and buffer */
    *result = ptr->length;
    return(ptr->question);
  }
}


/*  a simple hash function, from Knuth's "Art of Programming"
 *     name -- hash key
 *  returns: hash-table index
 */
unsigned get_bucket_index(char *name)
{
  unsigned hgen = 0;

  while (*name) {                /* if we're not at the end, calculate hash */
    hgen += *name;               /* C doesn't have a cyclic shift (roll) =( */
    hgen = (hgen << HASHROLL) | (hgen >> (sizeof(unsigned)*8-HASHROLL));
    name++;
  }

  /* truncate the hash result to table size, keeping most-significant bits */
  return (unsigned)((hgen * HASHMUL) >> (sizeof(unsigned)*8-CACHEWIDTH));
}

 int
allocate_entry()
{
  int new;

  while (cache[clock_hand].use == 0) {
    cache[clock_hand].use = 1;
    inc_hand;
  }

  /* found an entry; mark it as touched, and increment hand past it */

  new = clock_hand;
  inc_hand;

  /* If it's in use, clean it up */
  delete_entry(&cache[new]);

  /* Now mark it as used */
  cache[new].use = 0;
  return(new);
}

void
delete_entry(ent)
  struct entry *ent;
{
  if (ent->fd >= 0) 
    close(ent->fd);
  ent->fd = -1;
  if (ent->question != NULL) {
    free(ent->question);
    ent->question = NULL;
  }
  ent->use = 1;
  if (ent->filename == NULL)
    return;
  if (ent->next != NULL)
    ent->next->prev = ent->prev;
  if (ent->prev == NULL)
    /* If it has a null prev pointer, it's the one that buckets points to */
    /* for that hash index */
    buckets[get_bucket_index(ent->filename)] = ent->next;
  else
    ent->prev->next = ent->next;

  free(ent->filename);

  ent->prev = NULL;
  ent->next = NULL;
}
