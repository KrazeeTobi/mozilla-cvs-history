/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Mozilla browser.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Stuart Parmenter <pavlov@netscape.com>
 */


#ifndef __nsStdColorPicker_h__
#define __nsStdColorPicker_h__

#include "nsISupports.h"
#include "nsrootidl.h"
#include "nsIPresContext.h"
#include "nsIRenderingContext.h"
#include "nsIColorPicker.h"

class nsStdColorPicker : public nsIColorPicker {
public: 
  nsStdColorPicker();
  virtual ~nsStdColorPicker();

  NS_DECL_ISUPPORTS

  NS_DECL_NSICOLORPICKER

private:
  PRInt32 mNumCols;
  PRInt32 mNumRows;
  PRInt32 mBlockWidth;
  PRInt32 mBlockHeight;
  PRInt32 mFrameWidth;
  PRInt32 mFrameHeight;
};

#endif /* __nsStdColorPicker_h__ */
