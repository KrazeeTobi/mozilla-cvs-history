/* -*- Mode: java; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * The Original Code is Rhino code, released
 * May 6, 1999.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1997-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Norris Boyd
 * Bojan Cekrlic
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

// API class

package org.mozilla.javascript;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Java reflection of JavaScript exceptions.  (Possibly wrapping a Java exception.)
 *
 * @author Mike McCabe
 */
public class JavaScriptException extends Exception {
    /**
     * <p>Pointer to initCause() method of Throwable. 
     * If this method does not exist,
     * (i.e. we're running on something earlier than Java 1.4), the pointer will
     * be null.</p>
     */
    public static Method initCauseMethod = null;

    static {
        // Are we running on a JDK 1.4 or later system?
        try {
            initCauseMethod = Throwable.class.getMethod("initCause", 
                                          new Class[]{Throwable.class});
        } catch (NoSuchMethodException nsme) {
                // We are not running on JDK 1.4
        } catch (SecurityException se) {
                // This should have not happened, but if it does,
                // pretend the method odes not exist.
        }
    }

    /**
     * Create a JavaScript exception wrapping the given JavaScript value.
     *
     * Instances of this class are thrown by the JavaScript 'throw' keyword.
     *
     * @param value the JavaScript value thrown.
     */
    public JavaScriptException(Object value) {
        super(ScriptRuntime.toString(value));
        if ((value instanceof Throwable) && (initCauseMethod != null)) {
            try {
                initCauseMethod.invoke(this, new Object[] {(Throwable) value});
            } catch (IllegalAccessException e) {
                // we cannot help you here
                e.printStackTrace(); // Print the stacktrace to System.err so nice people would know what hit them
            } catch (IllegalArgumentException e) {
                // Should never happen.
                e.printStackTrace(); // Print the stacktrace to System.err so nice people would know what hit them
            } catch (InvocationTargetException e) {
                // This neither, since initCause() does not throw any exceptions (at least in Java 1.4 it doesn't)
                e.printStackTrace(); // Print the stacktrace to System.err so nice people would know what hit them
            }
        }
        this.value = value;
    }

    static JavaScriptException wrapException(Context cx, Scriptable scope,
                                             Throwable exn)
    {
        if (exn instanceof InvocationTargetException)
            exn = ((InvocationTargetException)exn).getTargetException();
        if (exn instanceof JavaScriptException)
            return (JavaScriptException)exn;
        Object wrapper = cx.getWrapFactory().
                            wrap(cx, scope, exn, Throwable.class);
        return new JavaScriptException(wrapper);
    }

    /**
     * Get the exception value originally thrown.  This may be a
     * JavaScript value (null, undefined, Boolean, Number, String,
     * Scriptable or Function) or a Java exception value thrown from a
     * host object or from Java called through LiveConnect.
     *
     * @return the value wrapped by this exception
     */
    public Object getValue() {
        if (value != null && value instanceof Wrapper)
            // this will also catch NativeStrings...
            return ((Wrapper)value).unwrap();
        else
            return value;
    }

    /**
     * The JavaScript exception value.  This value is not
     * intended for general use; if the JavaScriptException wraps a
     * Java exception, getScriptableValue may return a Scriptable
     * wrapping the original Java exception object.
     *
     * We would prefer to go through a getter to encapsulate the value,
     * however that causes the bizarre error "nanosecond timeout value
     * out of range" on the MS JVM.
     * @serial
     */
    Object value;
}
