/* 
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Mountain View Compiler
 * Company.  Portions created by Mountain View Compiler Company are
 * Copyright (C) 1998-2000 Mountain View Compiler Company. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Jeff Dyer <jeff@compilercompany.com>
 */

package com.compilercompany.ecmascript;

/**
 * class Block
 */

public class Block {
    Node entry, exit;

    int n;

    Block(int n) {
        this.n = n;
    }
    
    void setEntry( Node entry ) {
        this.entry = entry;
    }

    void setExit( Node exit ) {
        this.exit = exit;
    }

    public String toString() {
        return "B"+n/*+"( " + entry + ", " + exit + " )"*/;
    }
}

/*
 * The end.
 */
