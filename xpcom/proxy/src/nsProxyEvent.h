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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef nsProxyEvent_h__
#define nsProxyEvent_h__

#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nscore.h"
#include "nsISupports.h"
#include "nsIFactory.h"
#include "nsIEventTarget.h"

#include "prtypes.h"
#include "xptcall.h"
#include "xptinfo.h"
    
class nsProxyObjectCallInfo;
class nsIRunnable;

//#define AUTOPROXIFICATION

// WARNING about NS_PROXY_ASYNC:  
//
// If the calling thread goes away, any function which accesses the calling stack 
// will blow up.
//
//  example:
//
//     myFoo->bar(&x)
//
//     ... thread goes away ...
//
//    bar(PRInt32 *x)
//    {
//         *x = 0;   <-----  You will blow up here.
//
//   
//  So what gets saved?  
//
//  You can safely pass base types by value.  You can also pass interface pointers.
//  I will make sure that the interface pointers are addrefed while they are being 
//  proxied.  You can also pass string and wstring.  These I will copy and free.
//
//  I do **NOT** copy arrays or strings with size.  If you are using these either
//  change your interface, or contact me about this feature request.


class nsProxyObject
{
public:
    nsProxyObject(nsIEventTarget *destQueue, PRInt32 proxyType,
                  nsISupports *realObject, nsISupports *rootObject);

    void AddRef();
    void Release();

                        ~nsProxyObject();

    nsresult            Post( PRUint32            methodIndex, 
                              nsXPTMethodInfo   * info, 
                              nsXPTCMiniVariant * params, 
                              nsIInterfaceInfo  * interfaceInfo);
    
    nsresult            PostAndWait(nsProxyObjectCallInfo *proxyInfo);
    nsISupports*        GetRealObject() const { return mRealObject; }
    nsIEventTarget*     GetTarget() const { return mTarget; }
    PRInt32             GetProxyType() const { return mProxyType; }

    friend class nsProxyEventObject;
private:
    
    nsAutoRefCnt              mRefCnt;

    PRInt32                   mProxyType;
    
    nsCOMPtr<nsIEventTarget>  mTarget;           /* event target */
    
    nsCOMPtr<nsISupports>     mRealObject;       /* the non-proxy object that this event is referring to. 
                                                    This is a strong ref. */
    nsISupports              *mRootObject;       /* weak pointer to identity object */

    nsresult convertMiniVariantToVariant(nsXPTMethodInfo   * methodInfo, 
                                         nsXPTCMiniVariant * params, 
                                         nsXPTCVariant     **fullParam, 
                                         uint8 *paramCount);
};


class nsProxyObjectCallInfo
{
public:
    
    nsProxyObjectCallInfo(nsProxyObject* owner,
                          nsXPTMethodInfo *methodInfo,
                          PRUint32 methodIndex, 
                          nsXPTCVariant* parameterList, 
                          PRUint32 parameterCount, 
                          nsIRunnable *event);

    ~nsProxyObjectCallInfo();

    PRUint32            GetMethodIndex() const { return mMethodIndex; }
    nsXPTCVariant*      GetParameterList() const { return mParameterList; }
    PRUint32            GetParameterCount() const { return mParameterCount; }
    nsIRunnable*        GetEvent() const { return mEvent; }
    nsresult            GetResult() const { return mResult; }
    nsProxyObject*      GetProxyObject() const { return mOwner; }
    
    PRBool              GetCompleted();
    void                SetCompleted();
    void                PostCompleted();

    void                SetResult(nsresult rv) { mResult = rv; }
    
    nsIEventTarget*     GetCallersTarget();
    void                SetCallersTarget(nsIEventTarget* target);

private:
    
    nsresult         mResult;                    /* this is the return result of the called function */
    nsXPTMethodInfo *mMethodInfo;
    PRUint32         mMethodIndex;               /* which method to be called? */
    nsXPTCVariant   *mParameterList;             /* marshalled in parameter buffer */
    PRUint32         mParameterCount;            /* number of params */
    nsIRunnable     *mEvent;                     /* the current runnable */
    PRInt32          mCompleted;                 /* is true when the method has been called. */
       
    nsCOMPtr<nsIEventTarget> mCallersTarget;     /* this is the dispatch target that we must post a message back to 
                                                    when we are done invoking the method (only NS_PROXY_SYNC). */

    nsRefPtr<nsProxyObject>   mOwner;            /* this is the strong referenced nsProxyObject */
   
    void RefCountInInterfacePointers(PRBool addRef);
    void CopyStrings(PRBool copy);
};

#endif  // nsProxyEvent_h__
