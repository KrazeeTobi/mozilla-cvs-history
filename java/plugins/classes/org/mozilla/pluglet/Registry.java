/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are Copyright (C) 1999 Sun Microsystems,
 * Inc. All Rights Reserved. 
 */
package org.mozilla.pluglet;

import java.util.*;

public class Registry {
    static Hashtable table = null;
    public static void setPeer(Object key,long peer) {
        if (table == null) {
            table = new Hashtable(10);
        }
        table.put(key, new Long(peer));
    }
    public static long getPeer(Object key) {
        if (table == null) {
            return 0;
        }
        Object obj = table.get(key);
        if (obj == null) {
            return 0;
        } else {
            return ((Long)obj).longValue(); 
        }
    }
    public static void remove(Object key) {
        if (table != null) {
            table.remove(key);
        }
    }
  
}
