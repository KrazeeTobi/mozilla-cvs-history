/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun
 * Microsystems, Inc.  Portions created by Sun are
 * Copyright (C) 2001 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Created by Cyrille Moureaux <Cyrille.Moureaux@sun.com>
 */
#include "nsMapiAddressBook.h"
#include "nsAbUtils.h"
#include "nsAutoLock.h"

#include "nslog.h"

NS_IMPL_LOG(nsMapiAddressBookLog)

#define PRINTF NS_LOG_PRINTF(nsMapiAddressBookLog)
#define FLUSH  NS_LOG_FLUSH(nsMapiAddressBookLog)


HMODULE nsMapiAddressBook::mLibrary = NULL ;
PRInt32 nsMapiAddressBook::mLibUsage = 0 ;
LPMAPIINITIALIZE nsMapiAddressBook::mMAPIInitialize = NULL ;
LPMAPIUNINITIALIZE nsMapiAddressBook::mMAPIUninitialize = NULL ;
LPMAPIALLOCATEBUFFER nsMapiAddressBook::mMAPIAllocateBuffer = NULL ;
LPMAPIFREEBUFFER nsMapiAddressBook::mMAPIFreeBuffer = NULL ;
LPMAPILOGONEX nsMapiAddressBook::mMAPILogonEx = NULL ;

BOOL nsMapiAddressBook::mInitialized = FALSE ;
BOOL nsMapiAddressBook::mLogonDone = FALSE ;
LPMAPISESSION nsMapiAddressBook::mRootSession = NULL ;
LPADRBOOK nsMapiAddressBook::mRootBook = NULL ;

BOOL nsMapiAddressBook::LoadMapiLibrary(void)
{
    if (mLibrary) { ++ mLibUsage ; return TRUE ; }
    HMODULE libraryHandle = LoadLibrary("MAPI32.DLL") ;

    if (!libraryHandle) { return FALSE ; }
    FARPROC entryPoint = GetProcAddress(libraryHandle, "MAPIGetNetscapeVersion") ;

    if (entryPoint) {
        FreeLibrary(libraryHandle) ;
        libraryHandle = LoadLibrary("MAPI32BAK.DLL") ;
        if (!libraryHandle) { return FALSE ; }
    }
    mLibrary = libraryHandle ;
    ++ mLibUsage ;
    mMAPIInitialize = NS_REINTERPRET_CAST(LPMAPIINITIALIZE, 
        GetProcAddress(mLibrary, "MAPIInitialize")) ;
    if (!mMAPIInitialize) { return FALSE ; }
    mMAPIUninitialize = NS_REINTERPRET_CAST(LPMAPIUNINITIALIZE, 
        GetProcAddress(mLibrary, "MAPIUninitialize")) ;
    if (!mMAPIUninitialize) { return FALSE ; }
    mMAPIAllocateBuffer = NS_REINTERPRET_CAST(LPMAPIALLOCATEBUFFER, 
        GetProcAddress(mLibrary, "MAPIAllocateBuffer")) ;
    if (!mMAPIAllocateBuffer) { return FALSE ; }
    mMAPIFreeBuffer = NS_REINTERPRET_CAST(LPMAPIFREEBUFFER, 
        GetProcAddress(mLibrary, "MAPIFreeBuffer")) ;
    if (!mMAPIFreeBuffer) { return FALSE ; }
    mMAPILogonEx = NS_REINTERPRET_CAST(LPMAPILOGONEX, 
        GetProcAddress(mLibrary, "MAPILogonEx")) ;
    if (!mMAPILogonEx) { return FALSE ; }
    MAPIINIT_0 mapiInit = { MAPI_INIT_VERSION, MAPI_MULTITHREAD_NOTIFICATIONS } ;
    HRESULT retCode = mMAPIInitialize(&mapiInit) ;

    if (HR_FAILED(retCode)) { 
        PRINTF("Cannot initialize MAPI %08x.\n", retCode) ; return FALSE ;
    }
    mInitialized = TRUE ;
    retCode = mMAPILogonEx(0, NULL, NULL,
                           MAPI_NO_MAIL | 
                           MAPI_USE_DEFAULT | 
                           MAPI_EXTENDED | 
                           MAPI_NEW_SESSION,
                           &mRootSession) ;
    if (HR_FAILED(retCode)) { 
        PRINTF("Cannot logon to MAPI %08x.\n", retCode) ; return FALSE ;
    }
    mLogonDone = TRUE ;
    retCode = mRootSession->OpenAddressBook(0, NULL, 0, &mRootBook) ;
    if (HR_FAILED(retCode)) { 
        PRINTF("Cannot open MAPI address book %08x.\n", retCode) ;
    }
    return HR_SUCCEEDED(retCode) ;
}

void nsMapiAddressBook::FreeMapiLibrary(void)
{
    if (mLibrary) {
        if (-- mLibUsage == 0) {
            {
                if (mRootBook) { mRootBook->Release() ; }
                if (mRootSession) {
                    if (mLogonDone) { 
                        mRootSession->Logoff(NULL, 0, 0) ; 
                        mLogonDone = FALSE ;
                    }
                    mRootSession->Release() ;
                }
                if (mInitialized) { 
                    mMAPIUninitialize() ; 
                    mInitialized = FALSE ;
                }
            }  
            FreeLibrary(mLibrary) ;
            mLibrary = NULL ; 
        }
    }
}

MOZ_DECL_CTOR_COUNTER(nsMapiAddressBook)

nsMapiAddressBook::nsMapiAddressBook(void)
: nsAbWinHelper()
{
    BOOL result = Initialize() ;

    NS_ASSERTION(result == TRUE, "Couldn't initialize Mapi Helper") ;
    MOZ_COUNT_CTOR(nsMapiAddressBook) ;
}

nsMapiAddressBook::~nsMapiAddressBook(void)
{
    nsAutoLock guard(mMutex) ;

    FreeMapiLibrary() ;
    MOZ_COUNT_DTOR(nsMapiAddressBook) ;
}

BOOL nsMapiAddressBook::Initialize(void)
{
    if (mAddressBook) { return TRUE ; }
    nsAutoLock guard(mMutex) ;

    if (!LoadMapiLibrary()) {
        PRINTF("Cannot load library.\n") ;
        return FALSE ;
    }
    mAddressBook = mRootBook ; 
    return TRUE ;
}

void nsMapiAddressBook::AllocateBuffer(ULONG aByteCount, LPVOID *aBuffer)
{
    mMAPIAllocateBuffer(aByteCount, aBuffer) ;
}

void nsMapiAddressBook::FreeBuffer(LPVOID aBuffer)
{
    mMAPIFreeBuffer(aBuffer) ;
}





