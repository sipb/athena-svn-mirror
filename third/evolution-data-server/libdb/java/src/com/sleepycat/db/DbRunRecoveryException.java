/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997-2002
 *      Sleepycat Software.  All rights reserved.
 *
 * $Id: DbRunRecoveryException.java,v 1.1.1.1 2004-12-17 17:27:10 ghudson Exp $
 */

package com.sleepycat.db;

/**
 *
 * @author Donald D. Anderson
 */
public class DbRunRecoveryException extends DbException
{
    // methods
    //

    public DbRunRecoveryException(String s)
    {
        super(s);
    }

    public DbRunRecoveryException(String s, int errno)
    {
        super(s, errno);
    }
}

// end of DbRunRecoveryException.java
