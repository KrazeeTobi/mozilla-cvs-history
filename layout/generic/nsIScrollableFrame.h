/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#ifndef nsIScrollFrame_h___
#define nsIScrollFrame_h___

#include "nsISupports.h"
#include "nsCoord.h"

class nsIFrame;
class nsIPresContext;

// IID for the nsIScrollableFrame interface
#define NS_ISCROLLABLE_FRAME_IID    \
{ 0xc95f1831, 0xc372, 0x11d1, \
{ 0xb7, 0x21, 0x0, 0x64, 0x9, 0x92, 0xd8, 0xc9 } }


class nsIScrollableFrame : public nsISupports {
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCROLLABLE_FRAME_IID)

  /**
   * Set the view that we are scrolling within the scrolling view. 
   */
  NS_IMETHOD SetScrolledFrame(nsIPresContext* aPresContext, 
                              nsIFrame *aScrolledFrame) = 0;

  /**
   * Get the view that we are scrolling within the scrollable frame. 
   * @result child view
   */
  NS_IMETHOD GetScrolledFrame(nsIPresContext* aPresContext,
                              nsIFrame *&aScrolledFrame) const = 0;

 /**
  * Gets the size of the area that lies inside the scrollbars but clips the scrolled frame
  */
  NS_IMETHOD GetClipSize(nsIPresContext* aPresContext, 
                         nscoord *aWidth, 
                         nscoord *aHeight) const = 0;
  /**
   * Get information about whether the vertical and horizontal scrollbars
   * are currently visible
   */
  NS_IMETHOD GetScrollbarVisibility(nsIPresContext* aPresContext,
                                    PRBool *aVerticalVisible,
                                    PRBool *aHorizontalVisible) const = 0;

  /**
   * Set information about whether the vertical and horizontal scrollbars
   * are currently visible
   */
  NS_IMETHOD SetScrollbarVisibility(nsIPresContext* aPresContext,
                                    PRBool aVerticalVisible,
                                    PRBool aHorizontalVisible) = 0;

};

#endif
