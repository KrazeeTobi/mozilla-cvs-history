/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/**
 
  Author:
  Eric D Vaughan

**/

#ifndef nsGridLayout2_h___
#define nsGridLayout2_h___

#include "nsStackLayout.h"
#include "nsIGridPart.h"
#include "nsCOMPtr.h"
#include "nsIBox.h"
#include "nsGrid.h"

class nsGridRowGroupLayout;
class nsGridRowLayout;
class nsGridRow;
class nsBoxLayoutState;
class nsGridCell;

class nsGridLayout2 : public nsStackLayout, 
                      public nsIGridPart
{
public:

  friend nsresult NS_NewGridLayout2(nsIPresShell* aPresShell, nsIBoxLayout** aNewLayout);

  NS_DECL_ISUPPORTS_INHERITED

  NS_IMETHOD Layout(nsIBox* aBox, nsBoxLayoutState& aBoxLayoutState);

  NS_IMETHOD CastToRowGroupLayout(nsGridRowGroupLayout** aRowGroup);
  NS_IMETHOD CastToGridLayout(nsGridLayout2** aGrid);
  NS_IMETHOD GetGrid(nsIBox* aBox, nsGrid** aList, PRInt32* aIndex, nsGridRowLayout* aRequestor=nsnull);
  NS_IMETHOD GetParentGridPart(nsIBox* aBox, nsIBox** aParentBox, nsIGridPart** aParentGridPart);
  NS_IMETHOD GetMinSize(nsIBox* aBox, nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetPrefSize(nsIBox* aBox, nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetMaxSize(nsIBox* aBox, nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD CountRowsColumns(nsIBox* aBox, PRInt32& aRowCount, PRInt32& aComputedColumnCount);
  NS_IMETHOD DirtyRows(nsIBox* aBox, nsBoxLayoutState& aState);
  NS_IMETHOD BuildRows(nsIBox* aBox, nsGridRow* aRows, PRInt32* aCount);
  NS_IMETHOD GetTotalMargin(nsIBox* aBox, nsMargin& aMargin, PRBool aIsHorizontal);
  NS_IMETHOD GetRowCount(PRInt32& aRowCount);

protected:

  nsGridLayout2(nsIPresShell* aShell);
  nsGrid mGrid;

private:
  void AddWidth(nsSize& aSize, nscoord aSize2, PRBool aIsHorizontal);


}; // class nsGridLayout2


#endif

