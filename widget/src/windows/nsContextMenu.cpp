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

#include "nsContextMenu.h"
#include "nsIContextMenu.h"
#include "nsIMenu.h"
#include "nsIMenuItem.h"

#include "nsToolkit.h"
#include "nsColor.h"
#include "nsGUIEvent.h"
#include "nsString.h"
#include "nsStringUtil.h"
#include <windows.h>

#include "nsIAppShell.h"
#include "nsGUIEvent.h"
#include "nsIDeviceContext.h"
#include "nsRect.h"
#include "nsGfxCIID.h"
#include "nsMenuItem.h"
#include "nsCOMPtr.h"
#include "nsIMenuListener.h"
#include "nsIComponentManager.h"

// IIDs
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIContextMenuIID,  NS_ICONTEXTMENU_IID);
static NS_DEFINE_IID(kIMenuIID,     NS_IMENU_IID);
static NS_DEFINE_IID(kIMenuItemIID, NS_IMENUITEM_IID);
static NS_DEFINE_IID(kIWidgetIID,   NS_IWIDGET_IID);

// CIDs
#include "nsWidgetsCID.h"
static NS_DEFINE_IID(kContextMenuCID,  NS_CONTEXTMENU_CID);
static NS_DEFINE_IID(kMenuCID,     NS_MENU_CID);
static NS_DEFINE_IID(kMenuItemCID, NS_MENUITEM_CID);

//-------------------------------------------------------------------------
nsresult nsContextMenu::QueryInterface(REFNSIID aIID, void** aInstancePtr)      
{                                                                        
  if (NULL == aInstancePtr) {                                            
    return NS_ERROR_NULL_POINTER;                                        
  }                                                                      
                                                                         
  *aInstancePtr = NULL;                                                  
                                                                                        
  if (aIID.Equals(kIContextMenuIID)) {                                         
    *aInstancePtr = (void*)(nsIContextMenu*) this;                                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }                                                                      
  if (aIID.Equals(kIMenuListenerIID)) {                                      
    *aInstancePtr = (void*)(nsIMenuListener*)this;                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }                                                     
  if (aIID.Equals(kISupportsIID)) {                                      
    *aInstancePtr = (void*)(nsISupports*)(nsIContextMenu*)this;                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }
  return NS_NOINTERFACE;                                                 
}

//-------------------------------------------------------------------------
NS_IMPL_ADDREF(nsContextMenu)
NS_IMPL_RELEASE(nsContextMenu)

//-------------------------------------------------------------------------
//
// nsContextMenu constructor
//
//-------------------------------------------------------------------------
nsContextMenu::nsContextMenu()
{
  NS_INIT_REFCNT();
  mMenu           = nsnull;
  mListener       = nsnull;
  nsresult result = NS_NewISupportsArray(&mItems);
  mDOMNode        = nsnull;
  mDOMElement     = nsnull;
  mWebShell       = nsnull;
  mConstructed    = false;
}

//-------------------------------------------------------------------------
//
// nsContextMenu destructor
//
//-------------------------------------------------------------------------
nsContextMenu::~nsContextMenu()
{
  NS_IF_RELEASE(mListener);

  // Remove all references to the items
  mItems->Clear();
}


//-------------------------------------------------------------------------
//
// Create the proper widget
//
//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::Create(nsISupports *aParent, const nsString& anAlignment)
{
  if(aParent)
  {
      nsIWidget * parent = nsnull;
      aParent->QueryInterface(kIWidgetIID, (void**) &parent); // This does the addref
      if(parent)
	  {
        mParentWindow = parent;
	  }
  }

  mMenu = CreatePopupMenu();
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::GetParent(nsISupports*& aParent)
{
  aParent = nsnull;
  if (nsnull != mParentWindow) {
    return mParentWindow->QueryInterface(kISupportsIID,(void**)&aParent);
  } 

  return NS_ERROR_FAILURE;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::AddItem(nsISupports * aItem)

{
  if(aItem)
  {
    // Figure out what we're adding
		nsIMenuItem * menuitem = nsnull;
		aItem->QueryInterface(kIMenuItemIID, (void**) &menuitem);
		if(menuitem)
		{
			// case menuitem
			AddMenuItem(menuitem);
			NS_RELEASE(menuitem);
		}
		else
		{
			nsIMenu * menu = nsnull;
			aItem->QueryInterface(kIMenuIID, (void**) &menu);
			if(menu)
			{
				// case menu
				AddMenu(menu);
				NS_RELEASE(menu);
			}
		}
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
// This does not return a ref counted object
// This is NOT an nsIMenu method
nsIWidget *  nsContextMenu::GetParentWidget()
{
  return mParentWindow;
}


//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::AddMenuItem(nsIMenuItem * aMenuItem)
{
  PRUint32 cnt;
  mItems->Count(&cnt);
  return InsertItemAt(cnt, (nsISupports *)aMenuItem);
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::AddMenu(nsIMenu * aMenu)
{
  PRUint32 cnt;
  mItems->Count(&cnt);
  return InsertItemAt(cnt, (nsISupports *)aMenu);
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::AddSeparator() 
{
  PRUint32 cnt;
  mItems->Count(&cnt);
  InsertSeparator(cnt);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::GetItemCount(PRUint32 &aCount)
{
  PRUint32 cnt;
  mItems->Count(&cnt);
  aCount = cnt;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::GetItemAt(const PRUint32 aCount, nsISupports *& aMenuItem)
{
  aMenuItem = (nsISupports *)mItems->ElementAt(aCount);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::InsertItemAt(const PRUint32 aCount, nsISupports * aMenuItem)
{
  nsString name;
  BOOL status = FALSE;

  mItems->InsertElementAt(aMenuItem, (PRInt32)aCount);

  nsCOMPtr<nsIMenuItem> menuItem(do_QueryInterface(aMenuItem));
  if (menuItem) {
    menuItem->GetLabel(name);
    nsIWidget * win = GetParentWidget();
    PRInt32 id = ((nsWindow *)win)->GetNewCmdMenuId();
    ((nsMenuItem *)((nsIMenuItem *)menuItem))->SetCmdId(id);

    char * nameStr = GetACPString(name);

    MENUITEMINFO menuInfo;
    menuInfo.cbSize     = sizeof(menuInfo);
    menuInfo.fMask      = MIIM_TYPE | MIIM_ID;
    menuInfo.fType      = MFT_STRING;
    menuInfo.dwTypeData = nameStr;
    menuInfo.wID        = (DWORD)id;
    menuInfo.cch        = strlen(nameStr);

    status = ::InsertMenuItem(mMenu, aCount, TRUE, &menuInfo);

    delete[] nameStr;
  } else {
    nsCOMPtr<nsIMenu> menu(do_QueryInterface(aMenuItem));
    if (menu) {
      nsString name;
      menu->GetLabel(name);
      //mItems->AppendElement((nsISupports *)(nsIMenu *)menu);

      char * nameStr = GetACPString(name);

      HMENU nativeMenuHandle;
      void * voidData;
      menu->GetNativeData(&voidData);

      nativeMenuHandle = (HMENU)voidData;

      MENUITEMINFO menuInfo;

      menuInfo.cbSize     = sizeof(menuInfo);
      menuInfo.fMask      = MIIM_SUBMENU | MIIM_TYPE;
      menuInfo.hSubMenu   = nativeMenuHandle;
      menuInfo.fType      = MFT_STRING;
      menuInfo.dwTypeData = nameStr;

      BOOL status = ::InsertMenuItem(mMenu, aCount, TRUE, &menuInfo);

      delete[] nameStr;
    }
  }

  return (status ? NS_OK : NS_ERROR_FAILURE);
}


//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::InsertSeparator(const PRUint32 aCount)
{
  nsISupports * supports = nsnull;
  QueryInterface(kISupportsIID, (void**) &supports);
  nsMenuItem * item = new nsMenuItem();
  if(supports) {	
    item->Create(supports, "", PR_FALSE);
	NS_RELEASE(supports);
  }
    mItems->InsertElementAt((nsISupports *)(nsIMenuItem *)item, (PRInt32)aCount);

    MENUITEMINFO menuInfo;

    menuInfo.cbSize = sizeof(menuInfo);
    menuInfo.fMask  = MIIM_TYPE;
    menuInfo.fType  = MFT_SEPARATOR;

    BOOL status = ::InsertMenuItem(mMenu, aCount, TRUE, &menuInfo);
 
  return (status ? NS_OK : NS_ERROR_FAILURE);
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::RemoveItem(const PRUint32 aCount)
{

  //nsISupports * supports = (nsISupports *)mItems->ElementAt(aCount);
  mItems->RemoveElementAt(aCount);

  return (::RemoveMenu(mMenu, aCount, MF_BYPOSITION) ? NS_OK:NS_ERROR_FAILURE);
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::RemoveAll()
{
  PRUint32 cnt;
  mItems->Count(&cnt);
  while (cnt) {
    mItems->RemoveElementAt(0);
    ::RemoveMenu(mMenu, 0, MF_BYPOSITION);
	mItems->Count(&cnt);
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::GetNativeData(void ** aData)

{
  *aData = (void *)mMenu;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::AddMenuListener(nsIMenuListener * aMenuListener)
{
  NS_IF_RELEASE(mListener);
  mListener = aMenuListener;
  NS_IF_ADDREF(mListener);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::RemoveMenuListener(nsIMenuListener * aMenuListener)
{
  if (aMenuListener == mListener) {
    NS_IF_RELEASE(mListener);
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
// nsIMenuListener interface
//-------------------------------------------------------------------------
nsEventStatus nsContextMenu::MenuItemSelected(const nsMenuEvent & aMenuEvent)
{
#ifdef saari_debug
  char* menuLabel = GetACPString(mLabel);
  printf("Menu Item Selected %s\n", menuLabel);
  delete[] menuLabel;
#endif
  if (nsnull != mListener) {
    NS_ASSERTION(false, "get debugger");
    mListener->MenuSelected(aMenuEvent);
  }
  return nsEventStatus_eIgnore;
}


nsEventStatus nsContextMenu::MenuSelected(const nsMenuEvent & aMenuEvent)
{
  //printf("nsMenu::MenuSelected called\n");
  
  if(mConstructed){
	MenuDestruct(aMenuEvent);
    mConstructed = false;
  }

  if(!mConstructed) {
    MenuConstruct(
      aMenuEvent,
      mParentWindow, 
      mDOMNode,
	  mWebShell);
	mConstructed = true;
  }
  
  HWND pWnd = (HWND)mParentWindow->GetNativeData(NS_NATIVE_WIDGET);

  // Track the menu
  PRInt32 identifier = ::TrackPopupMenu(
	  mMenu, 
	  TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, 
	  mX, mY, 0, pWnd, NULL);

  if (identifier > 0) {
    // Dispatch to MenuItemSelected
    nsIContextMenu * menu;
    QueryInterface(kIContextMenuIID, (void**) &menu);
    nsIMenuItem * menuItem = FindMenuItem(menu, identifier);
    NS_RELEASE(menu);
    if (menuItem) {
      nsIMenuListener * listener;
      if (NS_OK == menuItem->QueryInterface(kIMenuListenerIID, (void **)&listener)) {
        listener->MenuItemSelected(aMenuEvent);
        NS_RELEASE(listener);
      }
    }
  }
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsIMenuItem * nsContextMenu::FindMenuItem(nsIContextMenu * aMenu, PRUint32 aId)
{
  PRUint32 i, count;
  aMenu->GetItemCount(count);
  for (i=0;i<count;i++) {
    nsISupports * item;
    nsIMenuItem * menuItem;
    nsIContextMenu     * menu;

    aMenu->GetItemAt(i, item);
    if (NS_OK == item->QueryInterface(kIMenuItemIID, (void **)&menuItem)) {
      if (((nsMenuItem *)menuItem)->GetCmdId() == (PRInt32)aId) {
        NS_RELEASE(item);
        return menuItem;
      }
    } else if (NS_OK == item->QueryInterface(kIContextMenuIID, (void **)&menu)) {
      nsIMenuItem * fndItem = FindMenuItem(menu, aId);
      NS_RELEASE(menu);
      if (nsnull != fndItem) {
        NS_RELEASE(item);
        return fndItem;
      }
    }
    NS_RELEASE(item);
  }
  return nsnull;
}

//-------------------------------------------------------------------------
nsEventStatus nsContextMenu::MenuDeselected(const nsMenuEvent & aMenuEvent)
{
  //printf("nsMenu::MenuDeselected called\n");  
  //MenuDestruct(aMenuEvent);
  //mConstructed = false;

  if (nsnull != mListener) {
    NS_ASSERTION(false, "get debugger");
    mListener->MenuDeselected(aMenuEvent);
  }
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsContextMenu::MenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * menuNode,
	void              * aWebShell)
{
   //printf("nsMenu::MenuConstruct called \n");
   // Begin menuitem inner loop
    nsCOMPtr<nsIDOMNode> menuitemNode;
    ((nsIDOMNode*)mDOMNode)->GetFirstChild(getter_AddRefs(menuitemNode));

	unsigned short menuIndex = 0;

    while (menuitemNode) {
      nsCOMPtr<nsIDOMElement> menuitemElement(do_QueryInterface(menuitemNode));
      if (menuitemElement) {
        nsString menuitemNodeType;
        nsString menuitemName;
        menuitemElement->GetNodeName(menuitemNodeType);
        if (menuitemNodeType.Equals("menuitem")) {
          // LoadMenuItem
          LoadMenuItem(this, menuitemElement, menuitemNode, menuIndex, (nsIWebShell*)aWebShell);
        } else if (menuitemNodeType.Equals("separator")) {
          AddSeparator();
        } else if (menuitemNodeType.Equals("menu")) {
          // Load a submenu
          LoadSubMenu(this, menuitemElement, menuitemNode);
        }
      }
	  ++menuIndex;
      nsCOMPtr<nsIDOMNode> oldmenuitemNode(menuitemNode);
      oldmenuitemNode->GetNextSibling(getter_AddRefs(menuitemNode));
    } // end menu item innner loop
    
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsContextMenu::MenuDestruct(const nsMenuEvent & aMenuEvent)
{
  //printf("nsMenu::MenuDestruct called \n");
  // We cannot call RemoveAll() yet because menu item selection may need it
  //RemoveAll();
  
  PRUint32 cnt;
  mItems->Count(&cnt);
  while (cnt) {
    mItems->RemoveElementAt(0);
    ::RemoveMenu(mMenu, 0, MF_BYPOSITION);
	mItems->Count(&cnt);
  }
  return nsEventStatus_eIgnore;
}

//----------------------------------------
void nsContextMenu::LoadMenuItem(
  nsIMenu *    pParentMenu,
  nsIDOMElement * menuitemElement,
  nsIDOMNode *    menuitemNode,
  unsigned short  menuitemIndex,
  nsIWebShell *   aWebShell)
{
  static const char* NS_STRING_TRUE = "true";
  nsString disabled;
  nsString menuitemName;
  nsString menuitemCmd;

  menuitemElement->GetAttribute(nsAutoString("disabled"), disabled);
  menuitemElement->GetAttribute(nsAutoString("name"), menuitemName);
  menuitemElement->GetAttribute(nsAutoString("cmd"), menuitemCmd);
  // Create nsMenuItem
  nsIMenuItem * pnsMenuItem = nsnull;
  nsresult rv = nsComponentManager::CreateInstance(kMenuItemCID, nsnull, kIMenuItemIID, (void**)&pnsMenuItem);
  if (NS_OK == rv) {
    pnsMenuItem->Create(pParentMenu, menuitemName, 0);   
	
    nsISupports * supports = nsnull;
    pnsMenuItem->QueryInterface(kISupportsIID, (void**) &supports);
    pParentMenu->AddItem(supports); // Parent should now own menu item
    NS_RELEASE(supports);
          
    // Create MenuDelegate - this is the intermediator inbetween 
    // the DOM node and the nsIMenuItem
    // The nsWebShellWindow wacthes for Document changes and then notifies the 
    // the appropriate nsMenuDelegate object
    nsCOMPtr<nsIDOMElement> domElement(do_QueryInterface(menuitemNode));
    if (!domElement) {
		return;
    }
    
    nsAutoString cmdAtom("onclick");
    nsString cmdName;

    domElement->GetAttribute(cmdAtom, cmdName);

    pnsMenuItem->SetCommand(cmdName);
	// DO NOT use passed in wehshell because of messed up windows dynamic loading
	// code. 
    pnsMenuItem->SetWebShell(mWebShell);
    pnsMenuItem->SetDOMElement(domElement);

	if(disabled == NS_STRING_TRUE )
		::EnableMenuItem(mMenu, menuitemIndex, MF_BYPOSITION | MF_GRAYED);

	NS_RELEASE(pnsMenuItem);
  } 
  return;
}

//----------------------------------------
void nsContextMenu::LoadSubMenu(
  nsIMenu *       pParentMenu,
  nsIDOMElement * menuElement,
  nsIDOMNode *    menuNode)
{
  nsString menuName;
  menuElement->GetAttribute(nsAutoString("name"), menuName);
  //printf("Creating Menu [%s] \n", menuName.ToNewCString()); // this leaks

  // Create nsMenu
  nsIMenu * pnsMenu = nsnull;
  nsresult rv = nsComponentManager::CreateInstance(kMenuCID, nsnull, kIMenuIID, (void**)&pnsMenu);
  if (NS_OK == rv) {
    // Call Create
    nsISupports * supports = nsnull;
    pParentMenu->QueryInterface(kISupportsIID, (void**) &supports);
    pnsMenu->Create(supports, menuName);
    NS_RELEASE(supports); // Balance QI

    // Set nsMenu Name
    pnsMenu->SetLabel(menuName); 

    // Make nsMenu a child of parent nsMenu. The parent takes ownership
    supports = nsnull;
    pnsMenu->QueryInterface(kISupportsIID, (void**) &supports);
	pParentMenu->AddItem(supports);
	NS_RELEASE(supports);

	pnsMenu->SetWebShell(mWebShell);
	pnsMenu->SetDOMNode(menuNode);
	pnsMenu->SetDOMElement(menuElement);

	// We're done with the menu
	NS_RELEASE(pnsMenu);
  }     
}

//----------------------------------------
void nsContextMenu::LoadMenuItem(
  nsIContextMenu * pParentMenu,
  nsIDOMElement *  menuitemElement,
  nsIDOMNode *     menuitemNode,
  unsigned short   menuitemIndex,
  nsIWebShell *    aWebShell)
{
  static const char* NS_STRING_TRUE = "true";
  nsString disabled;
  nsString menuitemName;
  nsString menuitemCmd;

  menuitemElement->GetAttribute(nsAutoString("disabled"), disabled);
  menuitemElement->GetAttribute(nsAutoString("name"), menuitemName);
  menuitemElement->GetAttribute(nsAutoString("cmd"), menuitemCmd);
  // Create nsMenuItem
  nsIMenuItem * pnsMenuItem = nsnull;
  nsresult rv = nsComponentManager::CreateInstance(kMenuItemCID, nsnull, kIMenuItemIID, (void**)&pnsMenuItem);
  if (NS_OK == rv) {
    pnsMenuItem->Create(pParentMenu, menuitemName, 0);   
	
    nsISupports * supports = nsnull;
    pnsMenuItem->QueryInterface(kISupportsIID, (void**) &supports);
    pParentMenu->AddItem(supports); // Parent should now own menu item
    NS_RELEASE(supports);
          
    // Create MenuDelegate - this is the intermediator inbetween 
    // the DOM node and the nsIMenuItem
    // The nsWebShellWindow wacthes for Document changes and then notifies the 
    // the appropriate nsMenuDelegate object
    nsCOMPtr<nsIDOMElement> domElement(do_QueryInterface(menuitemNode));
    if (!domElement) {
		return;
    }
    
    nsAutoString cmdAtom("onclick");
    nsString cmdName;

    domElement->GetAttribute(cmdAtom, cmdName);

    pnsMenuItem->SetCommand(cmdName);
	// DO NOT use passed in wehshell because of messed up windows dynamic loading
	// code. 
    pnsMenuItem->SetWebShell(mWebShell);
    pnsMenuItem->SetDOMElement(domElement);

	if(disabled == NS_STRING_TRUE )
		::EnableMenuItem(mMenu, menuitemIndex, MF_BYPOSITION | MF_GRAYED);

	NS_RELEASE(pnsMenuItem);
  } 
  return;
}

//----------------------------------------
void nsContextMenu::LoadSubMenu(
  nsIContextMenu *       pParentMenu,
  nsIDOMElement * menuElement,
  nsIDOMNode *    menuNode)
{
  nsString menuName;
  menuElement->GetAttribute(nsAutoString("name"), menuName);
  //printf("Creating Menu [%s] \n", menuName.ToNewCString()); // this leaks

  // Create nsMenu
  nsIMenu * pnsMenu = nsnull;
  nsresult rv = nsComponentManager::CreateInstance(kMenuCID, nsnull, kIMenuIID, (void**)&pnsMenu);
  if (NS_OK == rv) {
    // Call Create
    nsISupports * supports = nsnull;
    pParentMenu->QueryInterface(kISupportsIID, (void**) &supports);
    pnsMenu->Create(supports, menuName);
    NS_RELEASE(supports); // Balance QI

    // Set nsMenu Name
    pnsMenu->SetLabel(menuName); 

    // Make nsMenu a child of parent nsMenu. The parent takes ownership
    supports = nsnull;
    pnsMenu->QueryInterface(kISupportsIID, (void**) &supports);
	pParentMenu->AddItem(supports);
	NS_RELEASE(supports);

	pnsMenu->SetWebShell(mWebShell);
	pnsMenu->SetDOMNode(menuNode);
	pnsMenu->SetDOMElement(menuElement);

	// We're done with the menu
	NS_RELEASE(pnsMenu);
  }     
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::SetLocation(PRInt32 aX, PRInt32 aY)
{
  mX = aX;
  mY = aY;
  return NS_OK;
}


//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::SetDOMNode(nsIDOMNode * menuNode)
{
  mDOMNode = menuNode;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::SetDOMElement(nsIDOMElement * menuElement)
{
  mDOMElement = menuElement;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsContextMenu::SetWebShell(nsIWebShell * aWebShell)
{
  mWebShell = aWebShell;
  return NS_OK;
}

char* nsContextMenu::GetACPString(nsString& aStr)
{
   int acplen = aStr.Length() * 2 + 1;
   char * acp = new char[acplen];
   if(acp)
   {
      int outlen = ::WideCharToMultiByte( CP_ACP, 0, 
                      aStr.GetUnicode(), aStr.Length(),
                      acp, acplen, NULL, NULL);
      if ( outlen > 0)
         acp[outlen] = '\0';  // null terminate
   }
   return acp;
}


