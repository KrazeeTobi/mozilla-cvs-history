/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef nsMenuItem_h__
#define nsMenuItem_h__

#include "nsdefs.h"
#include "nsISupports.h"
#include "nsIWidget.h"
#include "nsSwitchToUIThread.h"

#include "nsCOMPtr.h"

#include "nsIMenuItem.h"
#include "nsIMenuListener.h"

class nsIMenu;
class nsIPopUpMenu;
class nsIMenuListener;

/**
 * Native Win32 MenuItem wrapper
 */

class nsMenuItem : public nsIMenuItem, public nsIMenuListener
{

public:
  nsMenuItem();
  virtual ~nsMenuItem();

  // nsISupports
  NS_DECL_ISUPPORTS

  NS_IMETHOD Create(nsISupports * aParent, const nsString &aLabel, PRBool aIsSeparator);

  // nsIMenuItem Methods
  NS_IMETHOD SetDOMNode(nsIDOMNode * aDOMNode);
  NS_IMETHOD GetDOMNode(nsIDOMNode ** aDOMNode);
  NS_IMETHOD SetDOMElement(nsIDOMElement * aDOMElement);
  NS_IMETHOD GetDOMElement(nsIDOMElement ** aDOMElement);
  NS_IMETHOD SetWebShell(nsIWebShell * aWebShell);
  NS_IMETHOD SetCommand(const nsString & aStrCmd);
  NS_IMETHOD DoCommand();

  NS_IMETHOD GetLabel(nsString &aText);
  NS_IMETHOD SetLabel(nsString &aText);
  NS_IMETHOD SetEnabled(PRBool aIsEnabled);
  NS_IMETHOD GetEnabled(PRBool *aIsEnabled);
  NS_IMETHOD SetChecked(PRBool aIsEnabled);
  NS_IMETHOD GetChecked(PRBool *aIsEnabled);
  NS_IMETHOD SetCheckboxType(PRBool aIsCheckbox);
  NS_IMETHOD GetCheckboxType(PRBool *aIsCheckbox);
  NS_IMETHOD GetCommand(PRUint32 & aCommand);
  NS_IMETHOD GetTarget(nsIWidget *& aTarget);
  NS_IMETHOD GetNativeData(void*& aData);
  NS_IMETHOD AddMenuListener(nsIMenuListener * aMenuListener);
  NS_IMETHOD RemoveMenuListener(nsIMenuListener * aMenuListener);
  NS_IMETHOD IsSeparator(PRBool & aIsSep);

  NS_IMETHOD SetShortcutChar(const nsString &aText);
  NS_IMETHOD GetShortcutChar(nsString &aText);
  NS_IMETHOD SetModifiers(PRUint8 aModifiers);
  NS_IMETHOD GetModifiers(PRUint8 * aModifiers);
  
  // nsIMenuListener interface
  nsEventStatus MenuItemSelected(const nsMenuEvent & aMenuEvent);
  nsEventStatus MenuSelected(const nsMenuEvent & aMenuEvent);
  nsEventStatus MenuDeselected(const nsMenuEvent & aMenuEvent);
  nsEventStatus MenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * menubarNode,
	void              * aWebShell);
  nsEventStatus MenuDestruct(const nsMenuEvent & aMenuEvent);

  // Need for Native Impl
  void SetCmdId(PRInt32 aId);
  PRInt32 GetCmdId();

protected:
  NS_IMETHOD Create(nsIPopUpMenu * aParent, const nsString &aLabel, PRUint32 aCommand);
  NS_IMETHOD Create(nsIMenu * aParent);
  NS_IMETHOD Create(nsIPopUpMenu * aParent);

  nsIWidget * GetMenuBarParent(nsISupports * aParent);

  nsString          mLabel;
  nsString          mKeyEquivalent;
  PRUint8           mModifiers;
  PRUint32          mCommand;
  nsIWidget       * mTarget;
  nsIMenu         * mMenu;
  nsIMenuListener * mListener;
  PRInt32           mCmdId;
  PRBool            mIsSeparator;

  nsString          mCommandStr;
  nsIWebShell   *   mWebShell;
  nsIDOMElement *   mDOMElement;
};

#endif // nsMenuItem_h__
