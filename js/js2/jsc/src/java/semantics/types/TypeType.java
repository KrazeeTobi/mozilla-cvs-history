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
 * class TypeType
 */

public class TypeType extends ObjectType implements Type {

    boolean debug = false;
  
    public static TypeValue type = new TypeValue();

    public String toString() {
        return "type";
    }

    public TypeType() {
	    super("type");
        ObjectType.type.addSub(this);
    }

    public boolean includes(Value value) {

        // A type is always a member of itself.

        if(value.type==TypeType.type) {
           return true;
        }

        // Check the store in the base object.

        return super.includes(value);
    }
    
}

/*
 * The end.
 */
