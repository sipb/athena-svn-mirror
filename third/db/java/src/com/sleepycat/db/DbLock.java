/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 *	$Id: DbLock.java,v 1.1.1.2 2002-02-11 16:28:47 ghudson Exp $
 */

package com.sleepycat.db;

/**
 *
 * @author Donald D. Anderson
 */
public class DbLock
{
    protected native void finalize()
         throws Throwable;

    // methods
    //
    public native void put(DbEnv env)
         throws DbException;

    // get/set methods
    //

    // private data
    //
    private long private_dbobj_ = 0;

    static {
        Db.load_db();
    }
}

// end of DbLock.java
