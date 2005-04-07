/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   John Gaunt (jgaunt@netscape.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _nsAccessible_H_
#define _nsAccessible_H_

#include "nsAccessNodeWrap.h"
#include "nsIAccessible.h"
#include "nsPIAccessible.h"
#include "nsWeakReference.h"
#include "nsIDOMNodeList.h"
#include "nsString.h"

struct nsRect;
class nsIContent;
class nsIFrame;
class nsIPresShell;
class nsIDOMNode;
class nsIAtom;

// When mNextSibling is set to this, it indicates there ar eno more siblings
#define DEAD_END_ACCESSIBLE NS_STATIC_CAST(nsIAccessible*, (void*)1)

struct nsStateMapEntry
{
  const char* attributeName;  // magic value of nsnull means last entry in map
  const char* attributeValue; // magic value of nsnull means any value
  PRUint32 state;       // OR state with this
};

enum ENameRule {
  eNoName,
  eNameFromSubtree,  // Collect name from text & img descendents; use title if resulting name is "".
  eNameFromTitle     // Use the title attribute for a name
};

enum EValueRule {
  eNoValue,
  eHasValueMinMax    // Supports value, min and max from waistate:valuenow, valuemin and valuemax
};

#define eNoReqStates 0
#define END_ENTRY {0, 0, 0}  // To fill in array of state mappings
#define BOOL_STATE 0

struct nsRoleMapEntry
{
  const char *roleString; // such as "button"
  PRUint32 role;   // use this role
  ENameRule nameRule;  // how to compute name
  EValueRule valueRule;  // how to compute name
  PRUint32 state;  // always OR state with this
  // For this role with a DOM attribute/value match definined in
  // nsStateMapEntry.attributeName && .attributeValue, OR accessible state with
  // nsStateMapEntry.state
  // Currently you can have up to 3 DOM attributes with accessible state mappings.
  // A variable sized array would not allow use of C++'s struct initialization feature.
  nsStateMapEntry attributeMap1;
  nsStateMapEntry attributeMap2;
  nsStateMapEntry attributeMap3;
  nsStateMapEntry attributeMap4;
  nsStateMapEntry attributeMap5;
};

class nsAccessible : public nsAccessNodeWrap, 
                     public nsIAccessible, 
                     public nsPIAccessible
{
public:
  // to eliminate the confusion of "magic numbers" -- if ( 0 ){ foo; }
  enum { eAction_Switch=0, eAction_Jump=0, eAction_Click=0, eAction_Select=0, eAction_Expand=1 };
  // how many actions
  enum { eNo_Action=0, eSingle_Action=1, eDouble_Action=2 }; 

  nsAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell);
  virtual ~nsAccessible();

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIACCESSIBLE
  NS_DECL_NSPIACCESSIBLE

  // nsIAccessNode
  NS_IMETHOD Init();
  NS_IMETHOD Shutdown();

  // Support GetFinalState(), GetFinalValue()
  NS_IMETHOD GetState(PRUint32 *aState);
  NS_IMETHOD GetValue(nsAString & aValue);

#ifdef MOZ_ACCESSIBILITY_ATK
  static nsresult GetParentBlockNode(nsIPresShell *aPresShell, nsIDOMNode *aCurrentNode, nsIDOMNode **aBlockNode);
  static nsIFrame* GetParentBlockFrame(nsIFrame *aFrame);
  static PRBool FindTextFrame(PRInt32 &index, nsPresContext *aPresContext, nsIFrame *aCurFrame, 
                                   nsIFrame **aFirstTextFrame, const nsIFrame *aTextFrame);
#endif

  static PRBool IsCorrectFrameType(nsIFrame* aFrame, nsIAtom* aAtom);

protected:
  PRBool MappedAttrState(nsIContent *aContent, PRUint32 *aStateInOut, nsStateMapEntry *aStateMapEntry);
  virtual nsIFrame* GetBoundsFrame();
  virtual void GetBoundsRect(nsRect& aRect, nsIFrame** aRelativeFrame);
  PRBool IsPartiallyVisible(PRBool *aIsOffscreen); 
  static nsIContent *GetLabelForId(nsIContent *aLookContent, nsIAtom *forAttrib,
                                const nsAString *aId);
  static nsIContent *GetXULLabelContent(nsIContent *aForNode);
  static nsIContent *GetHTMLLabelContent(nsIContent *aForNode);
  nsresult GetHTMLName(nsAString& _retval, PRBool aCanAggregateSubtree = PR_TRUE);
  nsresult GetXULName(nsAString& aName, PRBool aCanAggregateSubtree = PR_TRUE);
  // For accessibles that are not lists of choices, the name of the subtree should be the 
  // sum of names in the subtree
  nsresult AppendFlatStringFromSubtree(nsIContent *aContent, nsAString *aFlatString);
  nsresult AppendNameFromAccessibleFor(nsIContent *aContent, nsAString *aFlatString);
  nsresult AppendFlatStringFromContentNode(nsIContent *aContent, nsAString *aFlatString);
  nsresult AppendStringWithSpaces(nsAString *aFlatString, const nsAString& textEquivalent);

  // helper method to verify frames
  static nsresult GetFullKeyName(const nsAString& aModifierName, const nsAString& aKeyName, nsAString& aStringOut);
  static nsresult GetTranslatedString(const nsAString& aKey, nsAString& aStringOut);
  void GetScreenOrigin(nsPresContext *aPresContext, nsIFrame *aFrame, nsRect *aRect);
  nsresult AppendFlatStringFromSubtreeRecurse(nsIContent *aContent, nsAString *aFlatString);
  virtual void CacheChildren(PRBool aWalkAnonContent);

  // For accessibles that have actions
  static void DoCommandCallback(nsITimer *aTimer, void *aClosure);
  nsresult DoCommand();

  // Data Members
  nsCOMPtr<nsIAccessible> mParent;
  nsIAccessible *mFirstChild, *mNextSibling;
  nsRoleMapEntry *mRoleMapEntry; // Non-null indicates author-supplied role; possibly state & value as well

  static nsRoleMapEntry gWAIRoleMap[];
  static nsStateMapEntry gDisabledStateMap;
};


#endif  

