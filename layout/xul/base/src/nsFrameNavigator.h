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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/**

  Eric D Vaughan
  This class lays out its children either vertically or horizontally
 
**/

#ifndef nsGrippyFrame_h___
#define nsGrippyFrame_h___

#include "nsIFrame.h"
class nsIBox;

class nsFrameNavigator 
{
public:

  static nsIBox* GetChildBeforeAfter(nsIPresContext* aPresContext, nsIBox* start, PRBool before);
  static nsIBox* GetChildAt(nsIPresContext* aPresContext, nsIBox* parent, PRInt32 index);
  static PRInt32 IndexOf(nsIPresContext* aPresContext, nsIBox* parent, nsIBox* child);
  static PRInt32 CountFrames(nsIPresContext* aPresContext, nsIBox* aFrame);
  static nsIAtom* GetTag(nsIBox* frame);

}; 



#endif

