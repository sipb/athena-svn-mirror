/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Maciej Stachowiak <mjs@eazel.com>
 *
 */


/*  medusa-lock.h -  Utility functions for managing file locking
 */


#include <medusa-lock.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef struct MedusaLock MedusaLock;

struct MedusaLock {
        int fd;
};

struct MedusaReadLock {
        int fd;
};

struct MedusaWriteLock {
        int fd;
};

static MedusaLock *medusa_lock_get     (const char *file_name, 
                                        gboolean    write);

static void        medusa_lock_release (MedusaLock *lock);





MedusaReadLock *
medusa_read_lock_get (const char *file_name)
{
        return (MedusaReadLock *) medusa_lock_get (file_name, FALSE);
}

void
medusa_read_lock_release (MedusaReadLock *lock)
{
        medusa_lock_release ((MedusaLock *) lock);
}

MedusaWriteLock *
medusa_write_lock_get (const char *file_name)
{
        return (MedusaWriteLock *) medusa_lock_get (file_name, FALSE);
}

void
medusa_write_lock_release (MedusaWriteLock *lock)
{
        medusa_lock_release ((MedusaLock *) lock);
}



static MedusaLock * 
medusa_lock_get (const char *file_name, 
                 gboolean write)
{
        struct flock lock_info;
        MedusaLock *lock;

        lock = g_new0 (MedusaLock, 1);

        lock->fd = open (file_name, O_CREAT | (write ? O_RDWR : O_RDONLY), 
                         S_IRUSR | S_IWUSR);
        
        if (lock->fd == -1) {
                g_free (lock);
                return NULL;
        }

        lock_info.l_type = write ? F_WRLCK : F_RDLCK;
        lock_info.l_start = 0;
        lock_info.l_whence = SEEK_SET;
        lock_info.l_len = 0;
        lock_info.l_pid = 0;

        while (fcntl (lock->fd, F_SETLKW, &lock_info) == -1) {
                if (errno != EINTR) {
                        close (lock->fd);
                        g_free (lock);
                        return NULL;
                }
        }
        

        return lock;

}


void
medusa_lock_release (MedusaLock *lock)
{
        struct flock lock_info;

        lock_info.l_type = F_UNLCK;
        lock_info.l_start = 0;
        lock_info.l_whence = SEEK_SET;
        lock_info.l_len = 0;
        lock_info.l_pid = 0;

        fcntl (lock->fd, F_SETLK, &lock_info);

        g_free (lock);
}









