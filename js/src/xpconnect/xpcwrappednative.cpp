/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

/* Class that wraps each native interface instance. */

#include "xpcprivate.h"

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

NS_IMPL_QUERY_INTERFACE(nsXPCWrappedNative, NS_IXPCONNECT_WRAPPED_NATIVE_IID)

// do chained ref counting

nsrefcnt
nsXPCWrappedNative::AddRef(void)
{
    NS_PRECONDITION(mRoot, "bad root");
    ++mRefCnt;

    if(1 == mRefCnt && mRoot && mRoot != this)
        NS_ADDREF(mRoot);
    else if(2 == mRefCnt)
        JS_AddRoot(mClass->GetXPCContext()->GetJSContext(), &mJSObj);

    return mRefCnt;
}

nsrefcnt
nsXPCWrappedNative::Release(void)
{
    NS_PRECONDITION(mRoot, "bad root");
    NS_PRECONDITION(0 != mRefCnt, "dup release");
    --mRefCnt;
    if(0 == mRefCnt)
    {
        if(mRoot == this)
        {
            NS_DELETEXPCOM(this);   // cascaded delete
        }
        else
        {
            NS_RELEASE(mRoot);
        }
        return 0;
    }
    if(1 == mRefCnt)
        JS_RemoveRoot(mClass->GetXPCContext()->GetJSContext(), &mJSObj);
    return mRefCnt;
}

void
nsXPCWrappedNative::JSObjectFinalized()
{
    NS_PRECONDITION(1 == mRefCnt, "bad JSObject finalization");
    Release();
}

// static
nsXPCWrappedNative*
nsXPCWrappedNative::GetNewOrUsedWrapper(XPCContext* xpcc,
                                       nsISupports* aObj,
                                       REFNSIID aIID)
{
    Native2WrappedNativeMap* map;
    nsISupports* rootObj = NULL;
    nsXPCWrappedNative* root;
    nsXPCWrappedNative* wrapper = NULL;
    nsXPCWrappedNativeClass* clazz = NULL;

    NS_PRECONDITION(xpcc, "bad param");
    NS_PRECONDITION(aObj, "bad param");

    map = xpcc->GetWrappedNativeMap();
    if(!map)
    {
        NS_ASSERTION(map,"bad map");
        return NULL;
    }

    // always find the native root
    if(NS_FAILED(aObj->QueryInterface(kISupportsIID, (void**)&rootObj)))
        return NULL;

    // look for the root wrapper
    root = map->Find(rootObj);
    if(root)
    {
        wrapper = root->Find(aIID);
        if(wrapper)
        {
            NS_ADDREF(wrapper);
            goto return_wrapper;
        }
    }

    clazz = nsXPCWrappedNativeClass::GetNewOrUsedClass(xpcc, aIID);
    if(!clazz)
        goto return_wrapper;

    // build the root wrapper

    if(!root)
    {
        if(rootObj == aObj)
        {
            // the root will do double duty as the interface wrapper
            wrapper = root = new nsXPCWrappedNative(aObj, clazz, NULL);
            if(!wrapper)
                goto return_wrapper;
            if(!wrapper->mJSObj)
            {
                NS_RELEASE(wrapper);    // sets wrapper to NULL
                goto return_wrapper;
            }

            map->Add(root);
            goto return_wrapper;
        }
        else
        {
            // just a root wrapper
            nsXPCWrappedNativeClass* rootClazz;
            rootClazz = nsXPCWrappedNativeClass::GetNewOrUsedClass(xpcc, kISupportsIID);
            if(!rootClazz)
                goto return_wrapper;

            root = new nsXPCWrappedNative(rootObj, rootClazz, NULL);
            NS_RELEASE(rootClazz);

            if(!root)
                goto return_wrapper;
            if(!root->mJSObj)
            {
                NS_RELEASE(root);    // sets root to NULL
                goto return_wrapper;
            }
            map->Add(root);
        }
    }

    // at this point we have a root and may need to build the specific wrapper
    NS_ASSERTION(root,"bad root");
    NS_ASSERTION(clazz,"bad clazz");

    if(!wrapper)
    {
        wrapper = new nsXPCWrappedNative(aObj, clazz, root);
        if(!wrapper)
            goto return_wrapper;
        if(!wrapper->mJSObj)
        {
            NS_RELEASE(wrapper);    // sets wrapper to NULL
            goto return_wrapper;
        }
    }

    wrapper->mNext = root->mNext;
    root->mNext = wrapper;

return_wrapper:
    if(rootObj)
        NS_RELEASE(rootObj);
    if(clazz)
        NS_RELEASE(clazz);
    return wrapper;
}

#ifdef WIN32
#pragma warning(disable : 4355) // OK to pass "this" in member initializer
#endif

nsXPCWrappedNative::nsXPCWrappedNative(nsISupports* aObj,
                                     nsXPCWrappedNativeClass* aClass,
                                     nsXPCWrappedNative* root)
    : mObj(aObj),
      mClass(aClass),
      mJSObj(NULL),
      mRoot(root ? root : this),
      mNext(NULL)

{
    NS_INIT_REFCNT();
    NS_ADDREF_THIS();
    NS_ADDREF(aClass);
    NS_ADDREF(aObj);

    mJSObj = aClass->NewInstanceJSObject(this);
    if(mJSObj)
    {
        // intentional second addref to be released when mJSObj is gc'd
        NS_ADDREF_THIS();
    }

}

nsXPCWrappedNative::~nsXPCWrappedNative()
{
    NS_PRECONDITION(0 == mRefCnt, "refcounting error");
    NS_RELEASE(mClass);
    NS_RELEASE(mObj);
    if(mNext)
        NS_DELETEXPCOM(mNext);  // cascaded delete
}

nsXPCWrappedNative*
nsXPCWrappedNative::Find(REFNSIID aIID)
{
    if(aIID.Equals(kISupportsIID))
        return mRoot;

    nsXPCWrappedNative* cur = mRoot;
    do
    {
        if(aIID.Equals(GetIID()))
            return cur;

    } while(NULL != (cur = cur->mNext));

    return NULL;
}

