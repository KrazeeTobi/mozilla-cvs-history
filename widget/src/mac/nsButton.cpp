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

#include "nsButton.h"
#include "nsIButton.h"
#include "nsToolkit.h"
#include "nsColor.h"
#include "nsGUIEvent.h"
#include "nsString.h"
#include "nsStringUtil.h"

#include <Controls.h>


#define DBG 0
//-------------------------------------------------------------------------
//
// nsButton constructor
//
//-------------------------------------------------------------------------
nsButton::nsButton(nsISupports *aOuter) : nsWindow(aOuter)
{
}

void nsButton::Create(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData) 
{
  aParent->AddChild(this);

  if (DBG) fprintf(stderr, "aParent 0x%x\n", aParent);
	
	WindowPtr window = nsnull;

  if (aParent) {
    window = (WindowPtr) aParent->GetNativeData(NS_NATIVE_WIDGET);
  } else if (aAppShell) {
    window = (WindowPtr) aAppShell->GetNativeData(NS_NATIVE_SHELL);
  }
  
  NS_ASSERTION(window!=nsnull,"The WindowPtr for the widget cannot be null")
	if (window)
	{
	  InitToolkit(aToolkit, aParent);
	  // InitDeviceContext(aContext, parentWidget);

	  if (DBG) fprintf(stderr, "Parent 0x%x\n", window);

		// NOTE: CREATE MACINTOSH CONTROL HERE
		Str255  title = "";
		Boolean visible = PR_TRUE;
		PRInt16 initialValue = 0;
		PRInt16 minValue = 0;
		PRInt16 maxValue = 1;
		PRInt16 ctrlType = pushButProc;
		
		Rect r;
		r.left = aRect.x;
		r.top = aRect.y;
		r.right = aRect.x + aRect.width;
		r.bottom = aRect.y + aRect.height;
		
		mControl = NewControl ( window, &r, title, visible, 
												    initialValue, minValue, maxValue, 
												    ctrlType, (long)this);

	  if (DBG) fprintf(stderr, "Button 0x%x  this 0x%x\n", mControl, this);

	  // save the event callback function
	  mEventCallback = aHandleEventFunction;

	  //InitCallbacks("nsButton");
	}
}

void nsButton::Create(nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
	NS_ERROR("This Widget must not use this Create method");
}

//-------------------------------------------------------------------------
//
// nsButton destructor
//
//-------------------------------------------------------------------------
nsButton::~nsButton()
{
}

//-------------------------------------------------------------------------
//
// Query interface implementation
//
//-------------------------------------------------------------------------
nsresult nsButton::QueryObject(REFNSIID aIID, void** aInstancePtr)
{
  static NS_DEFINE_IID(kIButtonIID,    NS_IBUTTON_IID);

  if (aIID.Equals(kIButtonIID)) {
    AddRef();
    *aInstancePtr = (void**) &mAggWidget;
    return NS_OK;
  }
  return nsWindow::QueryObject(aIID, aInstancePtr);
}

//-------------------------------------------------------------------------
//
// Convert a nsString to a PascalStr255
//
//-------------------------------------------------------------------------
void nsButton::StringToStr255(const nsString& aText, Str255& aStr255)
{
	char buffer[256];
	
	aText.ToCString(buffer,255);
	
	PRInt32 len = strlen(buffer);
	for (PRInt32 i = 0; i < len; i++)
		aStr255[i+1] = buffer[i];
	aStr255[0] = len;
}



//-------------------------------------------------------------------------
//
// Set this button label
//
//-------------------------------------------------------------------------
void nsButton::SetLabel(const nsString& aText)
{
	NS_ASSERTION(mControl != nsnull,"Control must not be null");
	if (mControl != nsnull)
	{
		Str255 s;
		StringToStr255(aText,s);
		SetControlTitle(mControl,s);
	}
}

//-------------------------------------------------------------------------
//
// Get this button label
//
//-------------------------------------------------------------------------
void nsButton::GetLabel(nsString& aBuffer)
{

}

//-------------------------------------------------------------------------
//
// paint message. Don't send the paint out
//
//-------------------------------------------------------------------------
PRBool nsButton::OnPaint(nsPaintEvent &aEvent)
{
  //printf("** nsButton::OnPaint **\n");
  return PR_FALSE;
}

PRBool nsButton::OnResize(nsSizeEvent &aEvent)
{
    return PR_FALSE;
}


#define GET_OUTER() ((nsButton*) ((char*)this - nsButton::GetOuterOffset()))

void nsButton::AggButton::GetLabel(nsString& aBuffer)
{
  GET_OUTER()->GetLabel(aBuffer);
}

void nsButton::AggButton::SetLabel(const nsString& aText)
{
  GET_OUTER()->SetLabel(aText);
}

//----------------------------------------------------------------------

BASE_IWIDGET_IMPL(nsButton, AggButton);


