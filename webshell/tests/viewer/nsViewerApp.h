/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#ifndef nsViewerApp_h___
#define nsViewerApp_h___

#include "nsINetContainerApplication.h"
#include "nsIAppShell.h"
#include "nsString.h"
#include "nsCRT.h"
#include "nsVoidArray.h"

class nsIPref;
class nsDocLoader;
class nsBrowserWindow;

class nsViewerApp : public nsINetContainerApplication,
                    public nsDispatchListener
{
public:
  void* operator new(size_t sz) {
    void* rv = new char[sz];
    nsCRT::zero(rv, sz);
    return rv;
  }

  virtual ~nsViewerApp();

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsINetContainerApplication
  NS_IMETHOD GetAppCodeName(nsString& aAppCodeName);
  NS_IMETHOD GetAppVersion(nsString& aAppVersion);
  NS_IMETHOD GetAppName(nsString& aAppName);
  NS_IMETHOD GetLanguage(nsString& aLanguage);
  NS_IMETHOD GetPlatform(nsString& aPlatform);

  // nsDispatchListener
  virtual void AfterDispatch();

  // nsViewerApp
  NS_IMETHOD SetupRegistry();
  NS_IMETHOD Initialize(int argc, char** argv);
  NS_IMETHOD ProcessArguments(int argc, char** argv);
  NS_IMETHOD OpenWindow();
  NS_IMETHOD CreateRobot(nsBrowserWindow* aWindow);
  NS_IMETHOD CreateSiteWalker(nsBrowserWindow* aWindow);
  NS_IMETHOD CreateJSConsole(nsBrowserWindow* aWindow);
  NS_IMETHOD Exit();

  virtual int Run() = 0;

protected:
  nsViewerApp();

  nsIAppShell* mAppShell;
  nsIPref* mPrefs;
  nsString mStartURL;
  PRBool mDoPurify;
  PRBool mDoQuantify;
  PRBool mLoadTestFromFile;
  nsString mInputFileName;
  PRInt32 mNumSamples;
  nsVoidArray mInputFiles;
  PRInt32 mDelay;
  PRInt32 mRepeatCount;
  nsDocLoader* mDocLoader;
};

class nsNativeViewerApp : public nsViewerApp {
public:
  nsNativeViewerApp();
  ~nsNativeViewerApp();

  virtual int Run();
};

#endif /* nsViewerApp_h___ */
