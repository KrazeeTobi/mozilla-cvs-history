/* -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; -*- */
/*
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

#include "nsFileWidget.h"
#include "nsStringUtil.h"


NS_IMPL_ADDREF(nsFileWidget)
NS_IMPL_RELEASE(nsFileWidget)

//-------------------------------------------------------------------------
//
// nsFileWidget constructor
//
//-------------------------------------------------------------------------
nsFileWidget::nsFileWidget() : nsWidget(), nsIFileWidget()
{
  NS_INIT_REFCNT();
  mNumberOfFilters = 0;
}

NS_METHOD nsFileWidget::Create(nsIWidget        *aParent,
                          const nsRect     &aRect,
                          EVENT_CALLBACK   aHandleEventFunction,
                          nsIDeviceContext *aContext,
                          nsIAppShell      *aAppShell,
                          nsIToolkit       *aToolkit,
                          nsWidgetInitData *aInitData)
{
  nsString title("Load");
  Create(aParent, title, eMode_load, aContext, aAppShell, aToolkit, aInitData);
  return NS_OK;
}


//-------------------------------------------------------------------------
NS_METHOD   nsFileWidget:: Create(nsIWidget  *aParent,
                             nsString&   aTitle,
                             nsMode      aMode,
                             nsIDeviceContext *aContext,
                             nsIAppShell *aAppShell,
                             nsIToolkit *aToolkit,
                             void       *aInitData)
{
  mTitle.SetLength(0);
  mTitle.Append(aTitle);
  mMode = aMode;

  nsRect tRect(0,0,0,0);

  //  BaseCreate(aParent, tRect, (EVENT_CALLBACK)nsnull, aContext, 
  //            aAppShell, aToolkit, aInitData);
 
  NS_ALLOC_STR_BUF(title, aTitle, 256);
  mWidget = gtk_file_selection_new(title);
  NS_FREE_STR_BUF(title);
  /*
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(mWidget)->ok_button),
                            "clicked",
                            GTK_SIGNAL_FUNC(nsFileWidget::OnOk),
                            this);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(mWidget->)cancel_button),
                            "clicked",
                            GTK_SIGNAL_FUNC(nsFileWidget::OnCancel),
                            this);
  */
  return NS_OK;
}

NS_METHOD nsFileWidget::Create(nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
  return NS_ERROR_FAILURE;
}

//-------------------------------------------------------------------------
//
// Query interface implementation
//
//-------------------------------------------------------------------------
nsresult nsFileWidget::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
    nsresult result = NS_NOINTERFACE;
    static NS_DEFINE_IID(kInsFileWidgetIID, NS_IFILEWIDGET_IID);
    if (result == NS_NOINTERFACE && aIID.Equals(kInsFileWidgetIID)) {
        *aInstancePtr = (void*) ((nsIFileWidget*)this);
        AddRef();
        result = NS_OK;
    }

    return result;
}



//-------------------------------------------------------------------------
//
// Ok's the dialog
//
//-------------------------------------------------------------------------
NS_METHOD nsFileWidget::OnOk()
{
#if 0
  XtUnmanageChild(mWidget);
  mWasCancelled  = PR_FALSE;
  mIOwnEventLoop = PR_FALSE;
#endif
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Cancel the dialog
//
//-------------------------------------------------------------------------
NS_METHOD nsFileWidget::OnCancel()
{
#if 0
  XtUnmanageChild(mWidget);
  mWasCancelled  = PR_TRUE;
  mIOwnEventLoop = PR_FALSE;
#endif
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Show - Display the file dialog
//
//-------------------------------------------------------------------------
PRBool nsFileWidget::Show()
{
#if 0
  nsresult result = nsEventStatus_eIgnore;
  XtManageChild(mWidget);

  // XXX Kludge: gAppContext is a global set in nsAppShell
  XEvent event;
  mIOwnEventLoop = PR_TRUE;
  while (mIOwnEventLoop) {
    XtAppNextEvent(gAppContext, &event);
    XtDispatchEvent(&event);
  }

  if (!mWasCancelled) {
    XmString str;
    char *   fileBuf;
    XtVaGetValues(mWidget, XmNdirSpec, &str, nsnull);
    if (XmStringGetLtoR(str, XmFONTLIST_DEFAULT_TAG, &fileBuf)) {
      // Set user-selected location of file or directory
      mFile.SetLength(0);
      mFile.Append(fileBuf);
      XmStringFree(str);
      XtFree(fileBuf);
    }
  }
#endif
 return PR_TRUE;
}

//-------------------------------------------------------------------------
//
// Convert filter titles + filters into a Windows filter string
//
//-------------------------------------------------------------------------

void nsFileWidget::GetFilterListArray(nsString& aFilterList)
{
  aFilterList.SetLength(0);
  for (PRUint32 i = 0; i < mNumberOfFilters; i++) {
    const nsString& title = mTitles[i];
    const nsString& filter = mFilters[i];

    aFilterList.Append(title);
    aFilterList.Append('\0');
    aFilterList.Append(filter);
    aFilterList.Append('\0');
  }
  aFilterList.Append('\0');
}

//-------------------------------------------------------------------------
//
// Set the list of filters
//
//-------------------------------------------------------------------------

NS_METHOD nsFileWidget::SetFilterList(PRUint32 aNumberOfFilters,const nsString aTitles[],const nsString aFilters[])
{
  mNumberOfFilters  = aNumberOfFilters;
  mTitles           = aTitles;
  mFilters          = aFilters;
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get the file + path
//
//-------------------------------------------------------------------------

NS_METHOD  nsFileWidget::GetFile(nsString& aFile)
{
  aFile.SetLength(0);
  aFile.Append(mFile);
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get the file + path
//
//-------------------------------------------------------------------------

NS_METHOD  nsFileWidget::SetDefaultString(nsString& aString)
{
  mDefault = aString;
  return NS_OK;
}



//-------------------------------------------------------------------------
//
// nsFileWidget destructor
//
//-------------------------------------------------------------------------
nsFileWidget::~nsFileWidget()
{
}
