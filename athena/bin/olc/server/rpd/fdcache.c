/*
 * File descriptor Cache
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/fdcache.c,v 1.6 1990-12-02 23:06:55 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

/* Note: cachesize must be a power of two.  Also, the maximum limit on open */
/* file descriptors in a process should be taken into account. */

#define CACHESIZE 16
#define inc_hand (clock_hand = (++clock_hand)&(CACHESIZE-1));

#define LOG_DIRECTORY "/usr/spool/olc"

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

/*
 * get_log
 * 
 * Returns: Pointer to buffer with the log of the question, NULL if the
 * question doesn't exist or there was an error.  Result will be
 * the length of the buffer, or negative for an error
 *
 */

char *
get_log(username,instance,result)
     char *username;
     int instance;
     int *result;
{
  int hash,found;
  int fd;
  int new;
  struct entry *head,*ptr;
  char filename[MAXPATHLEN];
  struct stat file_stat;

  /* Mark and Increment clock hand */
  cache[clock_hand].use = 1;
  inc_hand;

  hash = get_bucket_index(username,instance);
  head = buckets[hash];

  found = 0;
  ptr = head;
  while (ptr != NULL) {
    if (!strcmp(username,ptr->username) && (instance == ptr->instance)) {
      found = 1;
      break;
    }
    ptr = ptr->next;
  }

  if (found == 0) {
    /* not found in the cache; check disk */
    sprintf(filename,"%s/%s_%d.log",LOG_DIRECTORY,username,instance);
    if ((fd = open(filename,O_RDONLY,0)) < 0) {
      *result = 0;
      return(NULL);
    }

    /* Get a free cache table entry, clearing if necessary */
    new = allocate_entry();

    /* copy infomration over */
    cache[new].fd = fd;
    strcpy(cache[new].username,username);
    strcpy(cache[new].filename,filename);
    cache[new].instance = instance;
    

    /* Stat the file to get size and last mod time */
    if (fstat(fd,&file_stat) < 0) {
      syslog(LOG_ERR,"fstat: %m on %d");
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
    if ((cache[new].question = malloc(file_stat.st_size)) == NULL) {
      syslog(LOG_ERR,"get_log: malloc: error alloc'ing %d bytes\n",
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
      return(get_log(username,instance,result));
    }

    /* Check to see if the cache is current */
    if (file_stat.st_mtime > ptr->last_mod) {
      /* File has been modified, need to re-read */
      free(ptr->question);
      ptr->length = file_stat.st_size;
      ptr->last_mod = file_stat.st_mtime;

      /* Alloc new amount of memory */
      if ((ptr->question = malloc(file_stat.st_size)) == NULL) {
	syslog(LOG_ERR,"get_log: malloc: error alloc'ing %d bytes\n",
		file_stat.st_size);
	delete_entry(ptr);
	*result = -1;
	return(NULL);
      }
      
      /* rewind file */
      if (lseek(ptr->fd,0,L_SET) == -1) {
	syslog(LOG_ERR,"get_log: lseek: %m");
	delete_entry(ptr);
	*result = -1;
	return(NULL);
      }

      /* Read file into buffer */
      if (read(ptr->fd,ptr->question,ptr->length) != ptr->length) {
	syslog(LOG_ERR,"get_log: read: %m");
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


/* Returns the index of the hash bucket in the cache array */

 int
get_bucket_index(username,instance)
     char *username;
     int instance;
{
  char *foo;
  
  foo = username;
  while (*foo != '\0') {
    instance += *foo;
    foo++;
  }
  return(instance&(CACHESIZE-1));
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

  if (ent->fd > 0) 
    close(ent->fd);
  ent->fd = -1;
  if (ent->question != NULL) {
    free(ent->question);
    ent->question = NULL;
  }
  ent->use = 1;
  if (ent->next != NULL)
    ent->next->prev = ent->prev;
  if (ent->prev == NULL)
    /* If it has a null prev pointer, it's the one that buckets points to */
    /* for that hash index */
    buckets[get_bucket_index(ent->username,ent->instance)] = ent->next;
  else
    ent->prev->next = ent->next;

  ent->prev = NULL;
  ent->next = NULL;
}
