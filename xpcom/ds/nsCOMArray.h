/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is a COM aware array class.
 *
 * The Initial Developer of the Original Code
 * Netscape Communications Corp.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alec Flett <alecf@netscape.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsCOMArray_h__
#define nsCOMArray_h__

#include "nsVoidArray.h"
#include "nsISupports.h"

// See below for the definition of nsCOMArray<T>

// a class that's nsISupports-specific, so that we can contain the
// work of this class in the XPCOM dll
class NS_COM nsCOMArray_base
{
protected:
    nsCOMArray_base() {}
    nsCOMArray_base(PRInt32 aCount) : mArray(aCount) {}
    nsCOMArray_base(const nsCOMArray_base& other);

    nsISupports* ObjectAt(PRInt32 aIndex) const {
        return NS_STATIC_CAST(nsISupports*, mArray.ElementAt(aIndex));
    }
    
    nsISupports* operator[](PRInt32 aIndex) const {
        return ObjectAt(aIndex);
    }

    PRInt32 IndexOf(nsISupports* aObject) {
        return mArray.IndexOf(aObject);
    }

    // override nsVoidArray stuff so that they can be accessed by
    // consumers of nsCOMArray
    PRInt32 Count() const {
        return mArray.Count();
    }

    PRBool EnumerateForwards(nsVoidArrayEnumFunc aFunc, void* aData) {
        return mArray.EnumerateForwards(aFunc, aData);
    }
    
    // any method which is not a direct forward to mArray should
    // avoid inline bodies, so that the compiler doesn't inline them
    // all over the place
    void Clear();
    PRBool InsertObjectAt(nsISupports* aObject, PRInt32 aIndex);
    PRBool ReplaceObjectAt(nsISupports* aObject, PRInt32 aIndex);
    PRBool AppendObject(nsISupports *aObject);
    PRBool RemoveObject(nsISupports *aObject);
    PRBool RemoveObjectAt(PRInt32 aIndex);

 private:
    
    // the actual storage
    nsVoidArray mArray;

    // don't implement these, defaults will muck with refcounts!
    nsCOMArray_base& operator=(const nsCOMArray_base& other);
};

// a non-XPCOM, refcounting array of XPCOM objects
// used as a member variable or stack variable - this object is NOT
// refcounted, but the objects that it holds are
//
// most of the read-only accessors like ObjectAt()/etc do NOT refcount
// on the way out. This means that you can do one of two things:
//
// * does an addref, but holds onto a reference
// nsCOMPtr<T> foo = array[i];
//
// * avoids the refcount, but foo might go stale if array[i] is ever
// * modified/removed. Be careful not to NS_RELEASE(foo)!
// T* foo = array[i];
//
// This array will accept null as an argument for any object, and will
// store null in the array, just like nsVoidArray. But that also means
// that methods like ObjectAt() may return null when refering to an
// existing, but null entry in the array.
template <class T>
class nsCOMArray : public nsCOMArray_base
{
 public:
    nsCOMArray() {}
    nsCOMArray(PRInt32 aCount) : nsCOMArray_base(aCount) {}
    
    nsCOMArray(const nsCOMArray_base& aOther) : nsCOMArray_base(aOther) { }

    ~nsCOMArray() {}

    // these do NOT refcount on the way out, for speed
    T* ObjectAt(PRInt32 aIndex) const {
        return NS_STATIC_CAST(T*,nsCOMArray_base::ObjectAt(aIndex));
    }

    // indexing operator for syntactic sugar
    T* operator[](PRInt32 aIndex) const {
        return ObjectAt(aIndex);
    }

    // index of the element in question.. does NOT refcount
    PRInt32 IndexOf(T* aObject) {
        return nsCOMArray_base::IndexOf(aObject);
    }

    // inserts the object at aIndex, and move all objects after aIndex
    // to the right
    PRBool InsertObjectAt(T* aObject, PRInt32 aIndex) {
        return nsCOMArray_base::InsertObjectAt(aObject, aIndex);
    }

    // replaces an existing element. Warning: if the array grows,
    // the newly created entries will all be null
    PRBool ReplaceObjectAt(T* aObject, PRInt32 aIndex) {
        return nsCOMArray_base::ReplaceObjectAt(aObject, aIndex);
    }

    // override nsVoidArray stuff so that they can be accessed by
    // other methods

    // elements in the array (including null elements!)
    PRInt32 Count() const {
        return nsCOMArray_base::Count();
    }

    // remove all elements in the array, and call NS_RELEASE on each one
    void Clear() {
        nsCOMArray_base::Clear();
    }

    // Enumerator callback function. Return PR_FALSE to stop
    // Here's a more readable form:
    // PRBool PR_CALLBACK enumerate(T* aElement, void* aData)
    typedef PRBool (* PR_CALLBACK nsCOMArrayEnumFunc)
        (T* aElement, void *aData);
    
    // enumerate through the array with a callback. 
    PRBool EnumerateForwards(nsCOMArrayEnumFunc aFunc, void* aData) {
        return nsCOMArray_base::EnumerateForwards(nsVoidArrayEnumFunc(aFunc),
                                                  aData);
    }

    // append an object, growing the array as necessary
    PRBool AppendObject(T *aObject) {
        return nsCOMArray_base::AppendObject(aObject);
    }

    // remove the first instance of the given object and shrink the
    // array as necessary
    // Warning: if you pass null here, it will remove the first null element
    PRBool RemoveObject(T *aObject) {
        return nsCOMArray_base::RemoveObject(aObject);
    }

    // remove an element at a specific position, shrinking the array
    // as necessary
    PRBool RemoveObjectAt(PRInt32 aIndex) {
        return nsCOMArray_base::RemoveObjectAt(aIndex);
    }

 private:

    // don't implement these!
    nsCOMArray(const nsCOMArray& other);
    nsCOMArray& operator=(const nsCOMArray& other);
};


#endif
