/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 *	$Id: DbEnvFeedback.java,v 1.1.1.1 2002-02-11 16:25:06 ghudson Exp $
 */

package com.sleepycat.db;

public interface DbEnvFeedback
{
    // methods
    //
    public abstract void feedback(DbEnv env, int opcode, int pct);
}

// end of DbFeedback.java
