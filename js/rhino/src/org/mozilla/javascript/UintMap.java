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
 * Copyright (C) 1997-2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Igor Bukanov
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

package org.mozilla.javascript;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

/**
 * Map to associate non-negative integers to objects or integers.
 * The map does not synchronize any of its operation, so either use
 * it from a single thread or do own synchronization or perform all mutation
 * operations on one thread before passing the map to others
 *
 * @author Igor Bukanov
 *
 */

class UintMap implements Externalizable {

    static final long serialVersionUID = -4108482308923954951L;

// Map implementation via hashtable,
// follows "The Art of Computer Programming" by Donald E. Knuth

    public UintMap() {
        this(4);
    }

    public UintMap(int initialCapacity) {
        if (initialCapacity < 0) Context.codeBug();
        // Table grow when number of stored keys >= 3/4 of max capacity
        int minimalCapacity = initialCapacity * 4 / 3;
        int i;
        for (i = 2; (1 << i) < minimalCapacity; ++i) { }
        power = i;
        if (check && power < 2) Context.codeBug();
    }

    public boolean isEmpty() {
        return keyCount == 0;
    }

    public int size() {
        return keyCount;
    }

    public boolean has(int key) {
        if (key < 0) Context.codeBug();
        return 0 <= findIndex(key);
    }

    public boolean isObjectType(int key) {
        if (key < 0) Context.codeBug();
        int index = findIndex(key);
        return index >= 0 && isObjectTypeValue(index);
    }

    public boolean isIntType(int key) {
        if (key < 0) Context.codeBug();
        int index = findIndex(key);
        return index >= 0 && !isObjectTypeValue(index);
    }

    /**
     * Get object value assigned with key.
     * @return key object value or null if key is absent or does
     * not have object value
     */
    public Object getObject(int key) {
        if (key < 0) Context.codeBug();
        if (values != null) {
            int index = findIndex(key);
            if (0 <= index) {
                return values[index];
            }
        }
        return null;
    }

    /**
     * Get integer value assigned with key.
     * @return key integer value or defaultValue if key is absent or does
     * not have int value
     */
    public int getInt(int key, int defaultValue) {
        if (key < 0) Context.codeBug();
        if (ivaluesShift != 0) {
            int index = findIndex(key);
            if (0 <= index) {
                if (!isObjectTypeValue(index)) {
                    return keys[ivaluesShift + index];
                }
            }
        }
        return defaultValue;
    }

    /**
     * Get integer value assigned with key.
     * @return key integer value or defaultValue if key does not exist or does
     * not have int value
     * @throws RuntimeException if key does not exist or does
     * not have int value
     */
    public int getExistingInt(int key) {
        if (key < 0) Context.codeBug();
        if (ivaluesShift != 0) {
            int index = findIndex(key);
            if (0 <= index) {
                if (!isObjectTypeValue(index)) {
                    return keys[ivaluesShift + index];
                }
            }
        }
        // Key must exist
        Context.codeBug();
        return 0;
    }

    public void put(int key, Object value) {
        if (!(key >= 0 && value != null)) Context.codeBug();
        int index = ensureIndex(key, false);
        if (values == null) {
            values = new Object[1 << power];
        }
        values[index] = value;
    }

    public void put(int key, int value) {
        if (key < 0) Context.codeBug();
        int index = ensureIndex(key, true);
        if (ivaluesShift == 0) {
            int N = 1 << power;
            // keys.length cam be N * 2 after clear which set ivaluesShift to 0
            if (keys.length != N * 2) {
                int[] tmp = new int[N * 2];
                System.arraycopy(keys, 0, tmp, 0, N);
                keys = tmp;
            }
            ivaluesShift = N;
        }
        keys[ivaluesShift + index] = value;
        if (values != null) { values[index] = null; }
    }

    public void remove(int key) {
        if (key < 0) Context.codeBug();
        int index = findIndex(key);
        if (0 <= index) {
            keys[index] = DELETED;
            --keyCount;
            if (values != null) { values[index] = null; }
        }
    }

    public void clear() {
        int N = 1 << power;
        if (keys != null) {
            for (int i = 0; i != N; ++i) {
                keys[i] = EMPTY;
            }
            if (values != null) {
                for (int i = 0; i != N; ++i) {
                    values[i] = null;
                }
            }
        }
        ivaluesShift = 0;
        keyCount = 0;
        occupiedCount = 0;
    }

    /** Return array of present keys */
    public int[] getKeys() {
        int[] keys = this.keys;
        int n = keyCount;
        int[] result = new int[n];
        for (int i = 0; n != 0; ++i) {
            int entry = keys[i];
            if (entry != EMPTY && entry != DELETED) {
                result[--n] = entry;
            }
        }
        return result;
    }

    private static int tableLookupStep(int fraction, int mask, int power) {
        int shift = 32 - 2 * power;
        if (shift >= 0) {
            return ((fraction >>> shift) & mask) | 1;
        }
        else {
            return (fraction & (mask >>> -shift)) | 1;
        }
    }

    private int findIndex(int key) {
        int[] keys = this.keys;
        if (keys != null) {
            int fraction = key * A;
            int index = fraction >>> (32 - power);
            int entry = keys[index];
            if (entry == key) { return index; }
            if (entry != EMPTY) {
                // Search in table after first failed attempt
                int mask = (1 << power) - 1;
                int step = tableLookupStep(fraction, mask, power);
                int n = 0;
                do {
                    if (check) {
                        if (n >= occupiedCount) Context.codeBug();
                        ++n;
                    }
                    index = (index + step) & mask;
                    entry = keys[index];
                    if (entry == key) { return index; }
                } while (entry != EMPTY);
            }
        }
        return -1;
    }

// Insert key that is not present to table without deleted entries
// and enough free space
    private int insertNewKey(int key) {
        if (check && occupiedCount != keyCount) Context.codeBug();
        if (check && keyCount == 1 << power) Context.codeBug();
        int[] keys = this.keys;
        int fraction = key * A;
        int index = fraction >>> (32 - power);
        if (keys[index] != EMPTY) {
            int mask = (1 << power) - 1;
            int step = tableLookupStep(fraction, mask, power);
            int firstIndex = index;
            do {
                if (check && keys[index] == DELETED) Context.codeBug();
                index = (index + step) & mask;
                if (check && firstIndex == index) Context.codeBug();
            } while (keys[index] != EMPTY);
        }
        keys[index] = key;
        ++occupiedCount;
        ++keyCount;
        return index;
    }

    private void rehashTable(boolean ensureIntSpace) {
        if (keys != null) {
            // Check if removing deleted entries would free enough space
            if (keyCount * 2 >= occupiedCount) {
                // Need to grow: less then half of deleted entries
                ++power;
            }
        }
        int N = 1 << power;
        int[] old = keys;
        int oldShift = ivaluesShift;
        if (oldShift == 0 && !ensureIntSpace) {
            keys = new int[N];
        }
        else {
            ivaluesShift = N; keys = new int[N * 2];
        }
        for (int i = 0; i != N; ++i) { keys[i] = EMPTY; }

        Object[] oldValues = values;
        if (oldValues != null) { values = new Object[N]; }

        int oldCount = keyCount;
        occupiedCount = 0;
        if (oldCount != 0) {
            keyCount = 0;
            for (int i = 0, remaining = oldCount; remaining != 0; ++i) {
                int key = old[i];
                if (key != EMPTY && key != DELETED) {
                    int index = insertNewKey(key);
                    if (oldValues != null) {
                        values[index] = oldValues[i];
                    }
                    if (oldShift != 0) {
                        keys[ivaluesShift + index] = old[oldShift + i];
                    }
                    --remaining;
                }
            }
        }
    }

// Ensure key index creating one if necessary
    private int ensureIndex(int key, boolean intType) {
        int index = -1;
        int firstDeleted = -1;
        int[] keys = this.keys;
        if (keys != null) {
            int fraction = key * A;
            index = fraction >>> (32 - power);
            int entry = keys[index];
            if (entry == key) { return index; }
            if (entry != EMPTY) {
                if (entry == DELETED) { firstDeleted = index; }
                // Search in table after first failed attempt
                int mask = (1 << power) - 1;
                int step = tableLookupStep(fraction, mask, power);
                int n = 0;
                do {
                    if (check) {
                        if (n >= occupiedCount) Context.codeBug();
                        ++n;
                    }
                    index = (index + step) & mask;
                    entry = keys[index];
                    if (entry == key) { return index; }
                    if (entry == DELETED && firstDeleted < 0) {
                        firstDeleted = index;
                    }
                } while (entry != EMPTY);
            }
        }
        // Inserting of new key
        if (check && keys != null && keys[index] != EMPTY)
            Context.codeBug();
        if (firstDeleted >= 0) {
            index = firstDeleted;
        }
        else {
            // Need to consume empty entry: check occupation level
            if (keys == null || occupiedCount * 4 >= (1 << power) * 3) {
                // Too litle unused entries: rehash
                rehashTable(intType);
                keys = this.keys;
                return insertNewKey(key);
            }
            ++occupiedCount;
        }
        keys[index] = key;
        ++keyCount;
        return index;
    }

    private boolean isObjectTypeValue(int index) {
        if (check && !(index >= 0 && index < (1 << power)))
            Context.codeBug();
        return values != null && values[index] != null;
    }

    public void writeExternal(ObjectOutput out) throws IOException {
        out.writeInt(power);
        out.writeInt(keyCount);
        boolean hasIntValues = (ivaluesShift != 0);
        boolean hasObjectValues = (values != null);
        out.writeBoolean(hasIntValues);
        out.writeBoolean(hasObjectValues);

        int count = keyCount;
        for (int i = 0; count != 0; ++i) {
            int key = keys[i];
            if (key != EMPTY && key != DELETED) {
                --count;
                out.writeInt(key);
                if (hasIntValues) {
                    out.writeInt(keys[ivaluesShift + i]);
                }
                if (hasObjectValues) {
                    out.writeObject(values[i]);
                }
            }
        }
    }

    public void readExternal(ObjectInput in)
        throws IOException, ClassNotFoundException
    {
        power = in.readInt();
        int writtenKeyCount = in.readInt();
        boolean hasIntValues = in.readBoolean();
        boolean hasObjectValues = in.readBoolean();
        if (writtenKeyCount != 0) {
            int N = 1 << power;
            if (hasIntValues) {
                keys = new int[2 * N];
                ivaluesShift = N;
            }else {
                keys = new int[N];
            }

            for (int i = 0; i != N; ++i) {
                keys[i] = EMPTY;
            }
            if (hasObjectValues) {
                values = new Object[N];
            }
            for (int i = 0; i != writtenKeyCount; ++i) {
                int key = in.readInt();
                int index = insertNewKey(key);
                if (hasIntValues) {
                    int ivalue = in.readInt();
                    keys[ivaluesShift + index] = ivalue;
                }
                if (hasObjectValues) {
                    values[index] = in.readObject();
                }
            }
        }
    }


// A == golden_ratio * (1 << 32) = ((sqrt(5) - 1) / 2) * (1 << 32)
// See Knuth etc.
    private static final int A = 0x9e3779b9;

    private static final int EMPTY = -1;
    private static final int DELETED = -2;

// Structure of kyes and values arrays (N == 1 << power):
// keys[0 <= i < N]: key value or EMPTY or DELETED mark
// values[0 <= i < N]: value of key at keys[i]
// keys[N <= i < 2N]: int values of keys at keys[i - N]

    private int[] keys;
    private Object[] values;

    private int power;
    private int keyCount;
    private int occupiedCount; // == keyCount + deleted_count

    // If ivaluesShift != 0, keys[ivaluesShift + index] contains integer
    // values associated with keys
    private int ivaluesShift;

// If true, enables consitency checks
    private static final boolean check = false;

/* TEST START

    public static void main(String[] args) {
        if (!check) {
            System.err.println("Set check to true and re-run");
            throw new RuntimeException("Set check to true and re-run");
        }

        UintMap map;
        map = new UintMap();
        testHash(map, 2);
        map = new UintMap();
        testHash(map, 10 * 1000);
        map = new UintMap(30 * 1000);
        testHash(map, 10 * 100);
        map.clear();
        testHash(map, 4);
        map = new UintMap(0);
        testHash(map, 10 * 100);
    }

    private static void testHash(UintMap map, int N) {
        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            map.put(i, i);
            check(i == map.getInt(i, -1));
        }

        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            map.put(i, i);
            check(i == map.getInt(i, -1));
        }

        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            map.put(i, new Integer(i));
            check(-1 == map.getInt(i, -1));
            Integer obj = (Integer)map.getObject(i);
            check(obj != null && i == obj.intValue());
        }

        check(map.size() == N);

        System.out.print("."); System.out.flush();
        int[] keys = map.getKeys();
        check(keys.length == N);
        for (int i = 0; i != N; ++i) {
            int key = keys[i];
            check(map.has(key));
            check(!map.isIntType(key));
            check(map.isObjectType(key));
            Integer obj = (Integer) map.getObject(key);
            check(obj != null && key == obj.intValue());
        }


        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            check(-1 == map.getInt(i, -1));
        }

        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            map.put(i * i, i);
            check(i == map.getInt(i * i, -1));
        }

        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            check(i == map.getInt(i * i, -1));
        }

        System.out.print("."); System.out.flush();
        for (int i = 0; i != N; ++i) {
            map.put(i * i, new Integer(i));
            check(-1 == map.getInt(i * i, -1));
            map.remove(i * i);
            check(!map.has(i * i));
            map.put(i * i, i);
            check(map.isIntType(i * i));
            check(null == map.getObject(i * i));
            map.remove(i * i);
            check(!map.isObjectType(i * i));
            check(!map.isIntType(i * i));
        }

        int old_size = map.size();
        for (int i = 0; i != N; ++i) {
            map.remove(i * i);
            check(map.size() == old_size);
        }

        System.out.print("."); System.out.flush();
        map.clear();
        check(map.size() == 0);
        for (int i = 0; i != N; ++i) {
            map.put(i * i, i);
            map.put(i * i + 1, new Double(i+0.5));
        }
        checkSameMaps(map, (UintMap)writeAndRead(map));

        System.out.print("."); System.out.flush();
        map = new UintMap(0);
        checkSameMaps(map, (UintMap)writeAndRead(map));
        map = new UintMap(1);
        checkSameMaps(map, (UintMap)writeAndRead(map));
        map = new UintMap(1000);
        checkSameMaps(map, (UintMap)writeAndRead(map));

        System.out.print("."); System.out.flush();
        map = new UintMap(N / 10);
        for (int i = 0; i != N; ++i) {
            map.put(2*i+1, i);
        }
        checkSameMaps(map, (UintMap)writeAndRead(map));

        System.out.print("."); System.out.flush();
        map = new UintMap(N / 10);
        for (int i = 0; i != N; ++i) {
            map.put(2*i+1, i);
        }
        for (int i = 0; i != N / 2; ++i) {
            map.remove(2*i+1);
        }
        checkSameMaps(map, (UintMap)writeAndRead(map));

        System.out.print("."); System.out.flush();
        map = new UintMap();
        for (int i = 0; i != N; ++i) {
            map.put(2*i+1, new Double(i + 10));
        }
        for (int i = 0; i != N / 2; ++i) {
            map.remove(2*i+1);
        }
        checkSameMaps(map, (UintMap)writeAndRead(map));

        System.out.println(); System.out.flush();

    }

    private static void checkSameMaps(UintMap map1, UintMap map2) {
        check(map1.size() == map2.size());
        int[] keys = map1.getKeys();
        check(keys.length == map1.size());
        for (int i = 0; i != keys.length; ++i) {
            int key = keys[i];
            check(map2.has(key));
            check(map1.isObjectType(key) == map2.isObjectType(key));
            check(map1.isIntType(key) == map2.isIntType(key));
            Object o1 = map1.getObject(key);
            Object o2 = map2.getObject(key);
            if (map1.isObjectType(key)) {
                check(o1.equals(o2));
            }else {
                check(map1.getObject(key) == null);
                check(map2.getObject(key) == null);
            }
            if (map1.isIntType(key)) {
                check(map1.getExistingInt(key) == map2.getExistingInt(key));
            }else {
                check(map1.getInt(key, -10) == -10);
                check(map1.getInt(key, -11) == -11);
                check(map2.getInt(key, -10) == -10);
                check(map2.getInt(key, -11) == -11);
            }
        }
    }

    private static void check(boolean condition) {
        if (!condition) Context.codeBug();
    }

    private static Object writeAndRead(Object obj) {
        try {
            java.io.ByteArrayOutputStream
                bos = new java.io.ByteArrayOutputStream();
            java.io.ObjectOutputStream
                out = new java.io.ObjectOutputStream(bos);
            out.writeObject(obj);
            out.close();
            byte[] data = bos.toByteArray();
            java.io.ByteArrayInputStream
                bis = new java.io.ByteArrayInputStream(data);
            java.io.ObjectInputStream
                in = new java.io.ObjectInputStream(bis);
            Object result = in.readObject();
            in.close();
            return result;
        }catch (Exception ex) {
            ex.printStackTrace();
            throw new RuntimeException("Unexpected");
        }
    }

// TEST END */
}
