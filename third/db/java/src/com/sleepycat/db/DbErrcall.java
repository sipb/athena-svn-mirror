/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 *	$Id: DbErrcall.java,v 1.1.1.2 2002-02-11 16:25:06 ghudson Exp $
 */

package com.sleepycat.db;

/**
 *
 * @author Donald D. Anderson
 */
public interface DbErrcall
{
    // methods
    //
    public abstract void errcall(String prefix, String buffer);
}

// end of DbErrcall.java
