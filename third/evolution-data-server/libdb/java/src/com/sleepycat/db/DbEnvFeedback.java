/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999-2002
 *      Sleepycat Software.  All rights reserved.
 *
 * $Id: DbEnvFeedback.java,v 1.1.1.1 2004-12-17 17:27:10 ghudson Exp $
 */

package com.sleepycat.db;

public interface DbEnvFeedback
{
    // methods
    //
    public abstract void feedback(DbEnv env, int opcode, int pct);
}

// end of DbFeedback.java
