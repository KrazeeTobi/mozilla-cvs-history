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
 * The Original Code is Rhino code, released
 * May 6, 1999.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1997-2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Norris Boyd
 * Roger Lawrence
 * Hannes Wallnoefer
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


package org.mozilla.javascript.optimizer;

import org.mozilla.javascript.*;

public final class OptRuntime extends ScriptRuntime
{

    public static final Double zeroObj = new Double(0.0);
    public static final Double oneObj = new Double(1.0);
    public static final Double minusOneObj = new Double(-1.0);

    public static Object getElem(Object obj, double dblIndex, Scriptable scope)
    {
        int index = (int) dblIndex;
        Scriptable start = obj instanceof Scriptable
                           ? (Scriptable) obj
                           : toObject(scope, obj);
        Scriptable m = start;
        if (((double) index) != dblIndex) {
            String s = toString(dblIndex);
            while (m != null) {
                Object result = m.get(s, start);
                if (result != Scriptable.NOT_FOUND)
                    return result;
                m = m.getPrototype();
            }
        } else {
            while (m != null) {
                Object result = m.get(index, start);
                if (result != Scriptable.NOT_FOUND)
                    return result;
                m = m.getPrototype();
            }
        }
        return Undefined.instance;
    }

    public static Object setElem(Object obj, double dblIndex, Object value,
                                 Scriptable scope)
    {
        int index = (int) dblIndex;
        Scriptable start = obj instanceof Scriptable
                     ? (Scriptable) obj
                     : toObject(scope, obj);
        Scriptable m = start;
        if (((double) index) != dblIndex) {
            String s = toString(dblIndex);
            do {
                if (m.has(s, start)) {
                    m.put(s, start, value);
                    return value;
                }
                m = m.getPrototype();
            } while (m != null);
            start.put(s, start, value);
       } else {
            do {
                if (m.has(index, start)) {
                    m.put(index, start, value);
                    return value;
                }
                m = m.getPrototype();
            } while (m != null);
            start.put(index, start, value);
        }
        return value;
    }

    public static Object add(Object val1, double val2)
    {
        if (val1 instanceof Scriptable)
            val1 = ((Scriptable) val1).getDefaultValue(null);
        if (!(val1 instanceof String))
            return wrapDouble(toNumber(val1) + val2);
        return ((String)val1).concat(toString(val2));
    }

    public static Object add(double val1, Object val2)
    {
        if (val2 instanceof Scriptable)
            val2 = ((Scriptable) val2).getDefaultValue(null);
        if (!(val2 instanceof String))
            return wrapDouble(toNumber(val2) + val1);
        return toString(val1).concat((String)val2);
    }

    public static Object callSimple(Context cx, String id, Scriptable scope,
                                    Object[] args)
        throws JavaScriptException
    {
        Object prop;
        Scriptable obj = scope;
 search: {
            while (obj != null) {
                prop = ScriptableObject.getProperty(obj, id);
                if (prop != Scriptable.NOT_FOUND) {
                    break search;
                }
                obj = obj.getParentScope();
            }
            throw ScriptRuntime.notFoundError(scope, id);
        }

        Scriptable thisArg = ScriptRuntime.getThis(obj);

        if (!(prop instanceof Function)) {
            Object[] errorArgs = { toString(prop)  };
            throw cx.reportRuntimeError(
                getMessage("msg.isnt.function", errorArgs));
        }

        Function function = (Function)prop;
        return function.call(cx, scope, thisArg, args);
    }

    public static Object getPropScriptable(Scriptable obj, String id)
    {
        if (obj == null || obj == Undefined.instance) {
            throw ScriptRuntime.undefReadError(obj, id);
        }
        Object result = ScriptableObject.getProperty(obj, id);
        if (result != Scriptable.NOT_FOUND)
            return result;
        return Undefined.instance;
    }

    public static Object[] padStart(Object[] currentArgs, int count) {
        Object[] result = new Object[currentArgs.length + count];
        System.arraycopy(currentArgs, 0, result, count, currentArgs.length);
        return result;
    }

    public static void initFunction(NativeFunction fn, int functionType,
                                    Scriptable scope, Context cx)
    {
        ScriptRuntime.initFunction(cx, scope, fn, functionType, false);
    }

    public static Object callSpecial(Context cx, Object fun,
                                     Object thisObj, Object[] args,
                                     Scriptable scope,
                                     Scriptable callerThis, int callType,
                                     String fileName, int lineNumber)
        throws JavaScriptException
    {
        return ScriptRuntime.callSpecial(cx, fun, false, thisObj, args, scope,
                                         callerThis, callType,
                                         fileName, lineNumber);
    }

    public static Object newObjectSpecial(Context cx, Object fun,
                                          Object[] args, Scriptable scope,
                                          Scriptable callerThis, int callType)
        throws JavaScriptException
    {
        return ScriptRuntime.callSpecial(cx, fun, true, null, args, scope,
                                         callerThis, callType,
                                         "", -1);
    }

    public static Double wrapDouble(double num)
    {
        if (num == 0.0) {
            if (1 / num > 0) {
                // +0.0
                return zeroObj;
            }
        } else if (num == 1.0) {
            return oneObj;
        } else if (num == -1.0) {
            return minusOneObj;
        } else if (num != num) {
            return NaNobj;
        }
        return new Double(num);
    }

    static String encodeIntArray(int[] array)
    {
        // XXX: this extremely inefficient for small integers
        if (array == null) { return null; }
        int n = array.length;
        char[] buffer = new char[1 + n * 2];
        buffer[0] = 1;
        for (int i = 0; i != n; ++i) {
            int value = array[i];
            int shift = 1 + i * 2;
            buffer[shift] = (char)(value >>> 16);
            buffer[shift + 1] = (char)value;
        }
        return new String(buffer);
    }

    private static int[] decodeIntArray(String str, int arraySize)
    {
        // XXX: this extremely inefficient for small integers
        if (arraySize == 0) {
            if (str != null) throw new IllegalArgumentException();
            return null;
        }
        if (str.length() != 1 + arraySize * 2 && str.charAt(0) != 1) {
            throw new IllegalArgumentException();
        }
        int[] array = new int[arraySize];
        for (int i = 0; i != arraySize; ++i) {
            int shift = 1 + i * 2;
            array[i] = (str.charAt(shift) << 16) | str.charAt(shift + 1);
        }
        return array;
    }

    public static Scriptable newArrayLiteral(Object[] objects,
                                             String encodedInts,
                                             int skipCount,
                                             Context cx,
                                             Scriptable scope)
        throws JavaScriptException
    {
        int[] skipIndexces = decodeIntArray(encodedInts, skipCount);
        return newArrayLiteral(objects, skipIndexces, cx, scope);
    }

}
