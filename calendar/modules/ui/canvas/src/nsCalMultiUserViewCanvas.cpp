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

#include "nsCalMultiUserViewCanvas.h"
#include "nsCalTimebarTimeHeading.h"
#include "nsBoxLayout.h"
#include "nsCalUICIID.h"
#include "nsIVector.h"
#include "nsIIterator.h"
#include "nsCalToolkit.h"
#include "nsCalDayListCommand.h"
#include "nsCalNewModelCommand.h"
#include "nscalstrings.h"


static NS_DEFINE_IID(kISupportsIID,           NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kCalMultiViewCanvasCID,  NS_CAL_MULTIVIEWCANVAS_CID);
static NS_DEFINE_IID(kIXPFCCanvasIID,         NS_IXPFC_CANVAS_IID);
static NS_DEFINE_IID(kCalTimebarCanvasCID,    NS_CAL_TIMEBARCANVAS_CID);

nsCalMultiUserViewCanvas :: nsCalMultiUserViewCanvas(nsISupports* outer) : nsCalMultiViewCanvas(outer)
{
  NS_INIT_REFCNT();
}

nsCalMultiUserViewCanvas :: ~nsCalMultiUserViewCanvas()
{
}

nsresult nsCalMultiUserViewCanvas::QueryInterface(REFNSIID aIID, void** aInstancePtr)      
{                                                                        
  if (NULL == aInstancePtr) {                                            
    return NS_ERROR_NULL_POINTER;                                        
  }                                                                      
  static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);                 
  static NS_DEFINE_IID(kClassIID, kCalMultiViewCanvasCID);                         
  if (aIID.Equals(kClassIID)) {                                          
    *aInstancePtr = (void*) this;                                        
    AddRef();                                                            
    return NS_OK;                                                        
  }                                                                      
  if (aIID.Equals(kISupportsIID)) {                                      
    *aInstancePtr = (void*) (this);                        
    AddRef();                                                            
    return NS_OK;                                                        
  }                                                                      
  if (aIID.Equals(kIXPFCCanvasIID)) {                                      
    *aInstancePtr = (void*) (this);                        
    AddRef();                                                            
    return NS_OK;                                                        
  }                                                                      
  return (nsCalMultiViewCanvas::QueryInterface(aIID, aInstancePtr));
}

NS_IMPL_ADDREF(nsCalMultiUserViewCanvas)
NS_IMPL_RELEASE(nsCalMultiUserViewCanvas)

/*
 *
 */

nsresult nsCalMultiUserViewCanvas :: Init()
{
  return (nsCalMultiViewCanvas::Init());
}

