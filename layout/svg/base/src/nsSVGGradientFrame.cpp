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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Scooter Morris.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Scooter Morris <scootermorris@comcast.net>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsContainerFrame.h"
#include "nsSVGGradient.h"
#include "nsWeakReference.h"
#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIDOMSVGStopElement.h"
#include "nsSVGAtoms.h"
#include "nsIDOMSVGLength.h"
#include "nsIDOMSVGAnimatedEnum.h"
#include "nsIDOMSVGAnimatedLength.h"
#include "nsIDOMSVGAnimatedNumber.h"
#include "nsIDOMSVGAnimatedString.h"
#include "nsIDOMSVGAnimTransformList.h"
#include "nsIDOMSVGTransformList.h"
#include "nsIDOMSVGNumber.h"
#include "nsIDOMSVGGradientElement.h"
#include "nsIDOMSVGURIReference.h"
#include "nsISVGValue.h"
#include "nsISVGValueUtils.h"
#include "nsStyleContext.h"
#include "nsISVGValueObserver.h"
#include "nsSVGValue.h"
#include "nsNetUtil.h"
#include "nsINameSpaceManager.h"
#include "nsISVGChildFrame.h"
#include "nsIDOMSVGRect.h"
#include "nsSVGMatrix.h"
#include "nsISVGGeometrySource.h"
#include "nsISVGGradient.h"
#include "nsIURI.h"
#include "nsIContent.h"
#include "nsSVGNumber.h"
#include "nsIDOMSVGStopElement.h"
#include "nsLayoutAtoms.h"
  
typedef nsContainerFrame  nsSVGGradientFrameBase;

class nsSVGGradientFrame : public nsSVGGradientFrameBase,
                           public nsSVGValue,
                           public nsISVGValueObserver,
                           public nsSupportsWeakReference,
                           public nsISVGGradient
{
protected:
  friend nsresult NS_NewSVGLinearGradientFrame(nsIPresShell* aPresShell, 
                                               nsIContent*   aContent, 
                                               nsIFrame**    aNewFrame);

  friend nsresult NS_NewSVGRadialGradientFrame(nsIPresShell* aPresShell, 
                                               nsIContent*   aContent, 
                                               nsIFrame**    aNewFrame);

  friend nsresult NS_NewSVGStopFrame(nsIPresShell* aPresShell, 
                                     nsIContent*   aContent, 
                                     nsIFrame*     aParentFrame, 
                                     nsIFrame**    aNewFrame);

  friend nsresult NS_GetSVGGradientFrame(nsIFrame**      result, 
                                         nsIURI*         aURI, 
                                         nsIContent*     aContent,
                                         nsIPresShell*   aPresShell);


public:
  // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef() { return NS_OK; }
  NS_IMETHOD_(nsrefcnt) Release() { return NS_OK; }

  // nsISVGGradient interface:
  NS_DECL_NSISVGGRADIENT

  // nsISVGValue interface:
  NS_IMETHOD SetValueString(const nsAString &aValue) { return NS_OK; }
  NS_IMETHOD GetValueString(nsAString& aValue) { return NS_ERROR_NOT_IMPLEMENTED; }

  // nsISVGValueObserver interface:
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable, 
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable(nsISVGValue* observable, 
                                    nsISVGValue::modificationType aModType);

  // nsIFrame interface:
  NS_IMETHOD DidSetStyleContext(nsPresContext* aPresContext);

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgGradientFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGGradient"), aResult);
  }
#endif // DEBUG

protected:
  virtual ~nsSVGGradientFrame();

  // Internal methods for handling referenced gradients
  PRBool checkURITarget(nsIAtom *);
  PRBool checkURITarget();
  //
  NS_IMETHOD PrivateGetGradientUnits(nsIDOMSVGAnimatedEnumeration * *aEnum);
  NS_IMETHOD PrivateGetGradientTransform(nsIDOMSVGMatrix * *aValue, 
                                         nsISVGGeometrySource *aSource);
  NS_IMETHOD PrivateGetSpreadMethod(nsIDOMSVGAnimatedEnumeration * *aValue);
  //

  nsSVGGradientFrame                     *mNextGrad;

private:
  // Cached values
  nsCOMPtr<nsIDOMSVGAnimatedEnumeration>  mGradientUnits;
  nsCOMPtr<nsIDOMSVGMatrix>               mGradientTransform;
  nsCOMPtr<nsIDOMSVGAnimatedEnumeration>  mSpreadMethod;

  nsAutoString                            mNextGradStr;

  PRInt32 GetStopElement(PRInt32 aIndex, 
                         nsIDOMSVGStopElement * *aStopElement,
                         nsIFrame * *aStopFrame);
};

// -------------------------------------------------------------------------
// Linear Gradients
// -------------------------------------------------------------------------

typedef nsSVGGradientFrame nsSVGLinearGradientFrameBase;

class nsSVGLinearGradientFrame : public nsSVGLinearGradientFrameBase,
                                 public nsISVGLinearGradient
{
public:
  // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef() { return NS_OK; }
  NS_IMETHOD_(nsrefcnt) Release() { return NS_OK; }

  // nsISVGLinearGradient interface:
  NS_DECL_NSISVGLINEARGRADIENT

  // nsISVGGradient interface gets inherited from nsSVGGradientFrame

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgLinearGradientFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGLinearGradient"), aResult);
  }
#endif // DEBUG


protected:
  virtual ~nsSVGLinearGradientFrame();

  NS_IMETHOD PrivateGetX1(nsIDOMSVGLength * *aX1);
  NS_IMETHOD PrivateGetX2(nsIDOMSVGLength * *aX2);
  NS_IMETHOD PrivateGetY1(nsIDOMSVGLength * *aY1);
  NS_IMETHOD PrivateGetY2(nsIDOMSVGLength * *aY2);

private:
  nsCOMPtr<nsIDOMSVGLength>       mX1;
  nsCOMPtr<nsIDOMSVGLength>       mX2;
  nsCOMPtr<nsIDOMSVGLength>       mY1;
  nsCOMPtr<nsIDOMSVGLength>       mY2;
};

// -------------------------------------------------------------------------
// Radial Gradients
// -------------------------------------------------------------------------

typedef nsSVGGradientFrame nsSVGRadialGradientFrameBase;

class nsSVGRadialGradientFrame : public nsSVGRadialGradientFrameBase,
                                 public nsISVGRadialGradient
{
public:
   // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef() { return NS_OK; }
  NS_IMETHOD_(nsrefcnt) Release() { return NS_OK; }

  // nsISVGRadialGradient interface:
  NS_DECL_NSISVGRADIALGRADIENT

  // nsISVGGradient interface gets inherited from nsSVGGradientFrame

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgLinearGradientFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGRadialGradient"), aResult);
  }
#endif // DEBUG

protected:
  virtual ~nsSVGRadialGradientFrame();

  NS_IMETHOD PrivateGetCx(nsIDOMSVGLength * *aCx);
  NS_IMETHOD PrivateGetCy(nsIDOMSVGLength * *aCy);
  NS_IMETHOD PrivateGetFx(nsIDOMSVGLength * *aFx);
  NS_IMETHOD PrivateGetFy(nsIDOMSVGLength * *aFy);
  NS_IMETHOD PrivateGetR(nsIDOMSVGLength * *aR);

private:
  nsCOMPtr<nsIDOMSVGLength>       mCx;
  nsCOMPtr<nsIDOMSVGLength>       mCy;
  nsCOMPtr<nsIDOMSVGLength>       mFx;
  nsCOMPtr<nsIDOMSVGLength>       mFy;
  nsCOMPtr<nsIDOMSVGLength>       mR;

};

static nsresult GetSVGGradient(nsSVGGradientFrame **result, nsCAutoString& aSpec, 
                               nsIContent *aContent, nsIPresShell *aPresShell);

//----------------------------------------------------------------------
// Implementation

nsSVGGradientFrame::~nsSVGGradientFrame()
{
  WillModify();
  // Notify the world that we're dying
  DidModify(mod_die);

  // Remove observers on gradient attributes
  if (mGradientUnits) NS_REMOVE_SVGVALUE_OBSERVER(mGradientUnits);
  if (mGradientTransform) NS_REMOVE_SVGVALUE_OBSERVER(mGradientTransform);
  if (mSpreadMethod) NS_REMOVE_SVGVALUE_OBSERVER(mSpreadMethod);
}

//----------------------------------------------------------------------
// nsISupports methods:

NS_INTERFACE_MAP_BEGIN(nsSVGGradientFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGValue)
  NS_INTERFACE_MAP_ENTRY(nsISVGGradient)
  NS_INTERFACE_MAP_ENTRY(nsISVGValueObserver)
  NS_INTERFACE_MAP_ENTRY(nsSupportsWeakReference)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsISVGValue)
NS_INTERFACE_MAP_END_INHERITING(nsSVGGradientFrameBase)

//----------------------------------------------------------------------
// nsISVGValueObserver methods:
NS_IMETHODIMP
nsSVGGradientFrame::WillModifySVGObservable(nsISVGValue* observable,
                                            modificationType aModType)
{
  WillModify(aModType);
  return NS_OK;
}
                                                                                
NS_IMETHODIMP
nsSVGGradientFrame::DidModifySVGObservable(nsISVGValue* observable, 
                                           nsISVGValue::modificationType aModType)
{
  // Something we depend on was modified -- pass it on!
  DidModify(aModType);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsIFrame methods:

NS_IMETHODIMP
nsSVGGradientFrame::DidSetStyleContext(nsPresContext* aPresContext)
{
  WillModify(mod_other);
  DidModify(mod_other);
  return NS_OK;
}

nsIAtom*
nsSVGGradientFrame::GetType() const
{
  return nsLayoutAtoms::svgGradientFrame;
}

//----------------------------------------------------------------------
//  nsISVGGradient implementation
//----------------------------------------------------------------------

NS_IMETHODIMP
nsSVGGradientFrame::GetStopCount(PRUint32 *aStopCount)
{
  nsresult rv = NS_OK;
  nsIDOMSVGStopElement *stopElement = nsnull;
  *aStopCount = GetStopElement(-1, &stopElement, nsnull);
  if (*aStopCount == 0) {
    // No stops?  check for URI target
    if (checkURITarget())
      rv = mNextGrad->GetStopCount(aStopCount);
  }
  return rv;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetStopOffset(PRInt32 aIndex, float *aOffset)
{
  nsIDOMSVGStopElement *stopElement = nsnull;
  PRInt32 stopCount = GetStopElement(aIndex, &stopElement, nsnull);
  if (stopCount == 0 && !stopElement) {
    // No stops?  check for URI target
    if (checkURITarget())
      return mNextGrad->GetStopOffset(aIndex, aOffset);
  }
  if (!stopElement) {
    *aOffset = nsnull;
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<nsIDOMSVGAnimatedNumber> aNum;
  stopElement->GetOffset(getter_AddRefs(aNum));
  aNum->GetAnimVal(aOffset);
  if (*aOffset < 0.0f)
    *aOffset = 0.0f;
  if (*aOffset > 1.0f)
    *aOffset = 1.0f;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetStopColorType(PRInt32 aIndex, PRUint16 *aStopColorType) 
{
  nsIDOMSVGStopElement *stopElement = nsnull;
  nsIFrame *stopFrame = nsnull;
  PRInt32 stopCount = GetStopElement(aIndex, &stopElement, &stopFrame);
  if (stopCount == 0 && !stopElement) {
    // No stops?  check for URI target
    if (checkURITarget())
      return mNextGrad->GetStopColorType(aIndex, aStopColorType);
  }
  if (!stopElement) {
    *aStopColorType = 0;
    return NS_ERROR_FAILURE;
  }

  *aStopColorType = stopFrame->GetStyleSVGReset()->mStopColor.mType;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetStopColor(PRInt32 aIndex, nscolor *aStopColor) 
{
  nsIDOMSVGStopElement *stopElement = nsnull;
  nsIFrame *stopFrame = nsnull;
  PRInt32 stopCount = GetStopElement(aIndex, &stopElement, &stopFrame);
  if (stopCount == 0 && !stopElement) {
    // No stops?  check for URI target
    if (checkURITarget())
      return mNextGrad->GetStopColor(aIndex, aStopColor);
  }
  NS_ASSERTION(stopFrame, "No stop frame found!");
  if (!stopFrame) {
    *aStopColor = 0;
    return NS_ERROR_FAILURE;
  }
  *aStopColor = stopFrame->GetStyleSVGReset()->mStopColor.mPaint.mColor;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetStopOpacity(PRInt32 aIndex, float *aStopOpacity) 
{
  nsIDOMSVGStopElement *stopElement = nsnull;
  nsIFrame *stopFrame = nsnull;
  PRInt32 stopCount = GetStopElement(aIndex, &stopElement, &stopFrame);
  if (stopCount == 0 && !stopElement) {
    // No stops?  check for URI target
    if (checkURITarget())
      return mNextGrad->GetStopOpacity(aIndex, aStopOpacity);
  }
  if (!stopFrame) {
    *aStopOpacity = 1.0f;
    return NS_ERROR_FAILURE;
  }

  *aStopOpacity = stopFrame->GetStyleSVGReset()->mStopOpacity;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetGradientType(PRUint32 *aType)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> aLe = do_QueryInterface(mContent);
  if (aLe) {
    *aType = nsISVGGradient::SVG_LINEAR_GRADIENT;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRe = do_QueryInterface(mContent);
  if (aRe) {
    *aType = nsISVGGradient::SVG_RADIAL_GRADIENT;
    return NS_OK;
  }
  *aType = nsISVGGradient::SVG_UNKNOWN_GRADIENT;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetGradientUnits(PRUint16 *aUnits)
{
  if (!mGradientUnits) {
    PrivateGetGradientUnits(getter_AddRefs(mGradientUnits));
    if (!mGradientUnits)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mGradientUnits);
  }
  mGradientUnits->GetAnimVal(aUnits);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetGradientTransform(nsIDOMSVGMatrix **aGradientTransform,
                                         nsISVGGeometrySource *aSource)
{
  if (!mGradientTransform) {
    PrivateGetGradientTransform(getter_AddRefs(mGradientTransform),aSource);
    if (!mGradientTransform)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mGradientTransform);
  }
  *aGradientTransform = mGradientTransform;
  NS_ADDREF(*aGradientTransform);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetSpreadMethod(PRUint16 *aSpreadMethod)
{
  if (!mSpreadMethod) {
    PrivateGetSpreadMethod(getter_AddRefs(mSpreadMethod));
    if (!mSpreadMethod)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mSpreadMethod);
  }
  mSpreadMethod->GetAnimVal(aSpreadMethod);
  return NS_OK;
}

// -------------------------------------------------------------
// Protected versions of the various "Get" routines.  These need
// to be used to allow for the ability to delegate to referenced
// gradients
// -------------------------------------------------------------
NS_IMETHODIMP
nsSVGGradientFrame::PrivateGetGradientUnits(nsIDOMSVGAnimatedEnumeration * *aEnum)
{
  nsCOMPtr<nsIDOMSVGGradientElement> aGrad = do_QueryInterface(mContent);
  NS_ASSERTION(aGrad, "Wrong content element (not gradient)");
  if (aGrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (!checkURITarget(nsSVGAtoms::gradientUnits)) {
    // No, return the values
    aGrad->GetGradientUnits(aEnum);
  } else {
    // Yes, get it from the target
    mNextGrad->PrivateGetGradientUnits(aEnum);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::PrivateGetGradientTransform(nsIDOMSVGMatrix **aGradientTransform,
                                                nsISVGGeometrySource *aSource)
{
  *aGradientTransform = nsnull;
  nsCOMPtr<nsIDOMSVGAnimatedTransformList> aTrans;
  nsCOMPtr<nsIDOMSVGGradientElement> aGrad = do_QueryInterface(mContent);
  NS_ASSERTION(aGrad, "Wrong content element (not gradient)");
  if (aGrad == nsnull) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIDOMSVGMatrix> bboxTransform;
  PRUint16 bbox;
  GetGradientUnits(&bbox);
  if (bbox == nsIDOMSVGGradientElement::SVG_GRUNITS_OBJECTBOUNDINGBOX) {
    nsISVGChildFrame *frame = nsnull;
    if (aSource)
      CallQueryInterface(aSource, &frame);
    nsCOMPtr<nsIDOMSVGRect> rect;
    if (frame) {
      frame->SetMatrixPropagation(PR_FALSE);
      frame->NotifyCanvasTMChanged();
      frame->GetBBox(getter_AddRefs(rect));
      frame->SetMatrixPropagation(PR_TRUE);
      frame->NotifyCanvasTMChanged();
    }
    if (rect) {
      float x, y, width, height;
      rect->GetX(&x);
      rect->GetY(&y);
      rect->GetWidth(&width);
      rect->GetHeight(&height);
      NS_NewSVGMatrix(getter_AddRefs(bboxTransform),
                      width, 0, 0, height, x, y);
    }
  }

  if (!bboxTransform)
    NS_NewSVGMatrix(getter_AddRefs(bboxTransform));

  nsCOMPtr<nsIDOMSVGMatrix> gradientTransform;
  // See if we need to get the value from another gradient
  if (!checkURITarget(nsSVGAtoms::gradientTransform)) {
    // No, return the values
    aGrad->GetGradientTransform(getter_AddRefs(aTrans));
    nsCOMPtr<nsIDOMSVGTransformList> lTrans;
    aTrans->GetAnimVal(getter_AddRefs(lTrans));
    lTrans->GetConsolidationMatrix(getter_AddRefs(gradientTransform));
  } else {
    // Yes, get it from the target
    mNextGrad->PrivateGetGradientTransform(getter_AddRefs(gradientTransform), nsnull);
  }

  bboxTransform->Multiply(gradientTransform, aGradientTransform);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::PrivateGetSpreadMethod(nsIDOMSVGAnimatedEnumeration * *aEnum)
{
  nsCOMPtr<nsIDOMSVGGradientElement> aGrad = do_QueryInterface(mContent);
  NS_ASSERTION(aGrad, "Wrong content element (not gradient)");
  if (aGrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (!checkURITarget(nsSVGAtoms::spreadMethod)) {
    // No, return the values
    aGrad->GetSpreadMethod(aEnum);
  } else {
    // Yes, get it from the target
    mNextGrad->PrivateGetSpreadMethod(aEnum);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::GetNextGradient(nsISVGGradient * *aNextGrad, PRUint32 aType) {
  if (checkURITarget()) {
    // OK, we have a reference.  We need to find the next reference that points to
    // our type
    PRUint32 nextType;
    mNextGrad->GetGradientType(&nextType);
    if (nextType == aType) {
      *aNextGrad = mNextGrad;
      return NS_OK;
    } else {
      return mNextGrad->GetNextGradient(aNextGrad, aType);
    }
  } else {
    *aNextGrad = nsnull;
    return NS_ERROR_FAILURE;
  }
}

// Private (helper) methods
PRBool 
nsSVGGradientFrame::checkURITarget(nsIAtom *attr) {
  // Was the attribute explicitly set?
  if (mContent->HasAttr(kNameSpaceID_None, attr)) {
    // Yes, just return
    return PR_FALSE;
  }
  return checkURITarget();
}

PRBool
nsSVGGradientFrame::checkURITarget(void) {
  nsSVGGradientFrame *aNextGrad;
  // Have we already figured out the next Gradient?
  if (mNextGrad != nsnull) {
    return PR_TRUE;
  }

  // Do we have URI?
  if (mNextGradStr.Length() == 0) {
    return PR_FALSE; // No, return the default
  }

  // Get the Gradient
  nsCAutoString aGradStr;
  CopyUTF16toUTF8(mNextGradStr, aGradStr);
  // Note that we are using *our* frame tree for this call, otherwise we're going to have
  // to get the PresShell in each call
  if (GetSVGGradient(&aNextGrad, aGradStr, mContent, GetPresContext()->PresShell()) == NS_OK) {
    mNextGrad = aNextGrad;
    // Add ourselves to the observer list
    if (mNextGrad) {
      // Can't use the NS_ADD macro here because of nsISupports ambiguity
      mNextGrad->AddObserver(this);
    }
    return PR_TRUE;
  }
  return PR_FALSE;
}

// -------------------------------------------------------------------------
// Private helper method to simplify getting stop elements
// returns non-addrefed stop element
// -------------------------------------------------------------------------
PRInt32 
nsSVGGradientFrame::GetStopElement(PRInt32 aIndex, nsIDOMSVGStopElement * *aStopElement,
                                   nsIFrame * *aStopFrame)
{
  PRInt32 stopCount = 0;
  nsIFrame *stopFrame;
  for (stopFrame = mFrames.FirstChild(); stopFrame;
       stopFrame = stopFrame->GetNextSibling()) {
    nsCOMPtr<nsIDOMSVGStopElement>stopElement = do_QueryInterface(stopFrame->GetContent());
    if (stopElement) {
      // Is this the one we're looking for?
      if (stopCount == aIndex) {
        *aStopElement = stopElement;
        break; // Yes, break out of the loop
      }
      stopCount++;
    }
  }
  if (aStopFrame)
    *aStopFrame = stopFrame;
  return stopCount;
}

// -------------------------------------------------------------------------
// Linear Gradients
// -------------------------------------------------------------------------

nsIAtom*
nsSVGLinearGradientFrame::GetType() const
{
  return nsLayoutAtoms::svgLinearGradientFrame;
}

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGLinearGradientFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGLinearGradient)
NS_INTERFACE_MAP_END_INHERITING(nsSVGLinearGradientFrameBase)

// Implementation
nsSVGLinearGradientFrame::~nsSVGLinearGradientFrame()
{
  if (mX1) NS_REMOVE_SVGVALUE_OBSERVER(mX1);
  if (mY1) NS_REMOVE_SVGVALUE_OBSERVER(mY1);
  if (mX2) NS_REMOVE_SVGVALUE_OBSERVER(mX2);
  if (mY2) NS_REMOVE_SVGVALUE_OBSERVER(mY2);
}

nsresult
nsSVGLinearGradientFrame::PrivateGetX1(nsIDOMSVGLength * *aX1)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> aLgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aLgrad, "Wrong content element (not linear gradient)");
  if (aLgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::x1)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_LINEAR_GRADIENT) == NS_OK) {
      nsSVGLinearGradientFrame *aLNgrad = (nsSVGLinearGradientFrame *)aNextGrad;
      aLNgrad->PrivateGetX1(aX1);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aLgrad->GetX1(getter_AddRefs(aLen));
  aLen->GetAnimVal(aX1);
  return NS_OK;
}

nsresult
nsSVGLinearGradientFrame::PrivateGetY1(nsIDOMSVGLength * *aY1)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> aLgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aLgrad, "Wrong content element (not linear gradient)");
  if (aLgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::y1)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_LINEAR_GRADIENT) == NS_OK) {
      nsSVGLinearGradientFrame *aLNgrad = (nsSVGLinearGradientFrame *)aNextGrad;
      aLNgrad->PrivateGetY1(aY1);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aLgrad->GetY1(getter_AddRefs(aLen));
  aLen->GetAnimVal(aY1);
  return NS_OK;
}

nsresult
nsSVGLinearGradientFrame::PrivateGetX2(nsIDOMSVGLength * *aX2)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> aLgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aLgrad, "Wrong content element (not linear gradient)");
  if (aLgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::x2)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_LINEAR_GRADIENT) == NS_OK) {
      nsSVGLinearGradientFrame *aLNgrad = (nsSVGLinearGradientFrame *)aNextGrad;
      aLNgrad->PrivateGetX2(aX2);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aLgrad->GetX2(getter_AddRefs(aLen));
  aLen->GetAnimVal(aX2);
  return NS_OK;
}

nsresult
nsSVGLinearGradientFrame::PrivateGetY2(nsIDOMSVGLength * *aY2)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> aLgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aLgrad, "Wrong content element (not linear gradient)");
  if (aLgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::y2)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_LINEAR_GRADIENT) == NS_OK) {
      nsSVGLinearGradientFrame *aLNgrad = (nsSVGLinearGradientFrame *)aNextGrad;
      aLNgrad->PrivateGetY2(aY2);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aLgrad->GetY2(getter_AddRefs(aLen));
  aLen->GetAnimVal(aY2);
  return NS_OK;
}

// nsISVGLinearGradient
NS_IMETHODIMP
nsSVGLinearGradientFrame::GetX1(float *aX1)
{
  if (!mX1) {
    PrivateGetX1(getter_AddRefs(mX1));
    if (!mX1)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mX1);
  }
  mX1->GetValue(aX1);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGLinearGradientFrame::GetY1(float *aY1)
{
  if (!mY1) {
    PrivateGetY1(getter_AddRefs(mY1));
    if (!mY1)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mY1);
  }
  mY1->GetValue(aY1);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGLinearGradientFrame::GetX2(float *aX2)
{
  if (!mX2) {
    PrivateGetX2(getter_AddRefs(mX2));
    if (!mX2)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mX2);
  }
  mX2->GetValue(aX2);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGLinearGradientFrame::GetY2(float *aY2)
{
  if (!mY2) {
    PrivateGetY2(getter_AddRefs(mY2));
    if (!mY2)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mY2);
  }
  mY2->GetValue(aY2);
  return NS_OK;
}

// -------------------------------------------------------------------------
// Radial Gradients
// -------------------------------------------------------------------------

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGRadialGradientFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGRadialGradient)
NS_INTERFACE_MAP_END_INHERITING(nsSVGRadialGradientFrameBase)

nsIAtom*
nsSVGRadialGradientFrame::GetType() const
{
  return nsLayoutAtoms::svgRadialGradientFrame;
}

// Implementation
nsSVGRadialGradientFrame::~nsSVGRadialGradientFrame()
{
  if (mCx) NS_REMOVE_SVGVALUE_OBSERVER(mCx);
  if (mCy) NS_REMOVE_SVGVALUE_OBSERVER(mCy);
  if (mFx) NS_REMOVE_SVGVALUE_OBSERVER(mFx);
  if (mFy) NS_REMOVE_SVGVALUE_OBSERVER(mFy);
  if (mR) NS_REMOVE_SVGVALUE_OBSERVER(mR);
}


nsresult
nsSVGRadialGradientFrame::PrivateGetCx(nsIDOMSVGLength * *aCx)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aRgrad, "Wrong content element (not radial gradient)");
  if (aRgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::cx)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_RADIAL_GRADIENT) == NS_OK) {
      nsSVGRadialGradientFrame *aRNgrad = (nsSVGRadialGradientFrame *)aNextGrad;
      aRNgrad->PrivateGetCx(aCx);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aRgrad->GetCx(getter_AddRefs(aLen));
  aLen->GetAnimVal(aCx);
  return NS_OK;
}

nsresult
nsSVGRadialGradientFrame::PrivateGetCy(nsIDOMSVGLength * *aCy)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aRgrad, "Wrong content element (not radial gradient)");
  if (aRgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::cy)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_RADIAL_GRADIENT) == NS_OK) {
      nsSVGRadialGradientFrame *aRNgrad = (nsSVGRadialGradientFrame *)aNextGrad;
      aRNgrad->PrivateGetCy(aCy);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aRgrad->GetCy(getter_AddRefs(aLen));
  aLen->GetAnimVal(aCy);
  return NS_OK;
}

nsresult
nsSVGRadialGradientFrame::PrivateGetR(nsIDOMSVGLength * *aR)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aRgrad, "Wrong content element (not radial gradient)");
  if (aRgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::r)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_RADIAL_GRADIENT) == NS_OK) {
      nsSVGRadialGradientFrame *aRNgrad = (nsSVGRadialGradientFrame *)aNextGrad;
      aRNgrad->PrivateGetR(aR);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- fall through
    // and return our default value
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aRgrad->GetR(getter_AddRefs(aLen));
  aLen->GetAnimVal(aR);
  return NS_OK;
}

nsresult
nsSVGRadialGradientFrame::PrivateGetFx(nsIDOMSVGLength * *aFx)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aRgrad, "Wrong content element (not radial gradient)");
  if (aRgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::fx)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_RADIAL_GRADIENT) == NS_OK) {
      nsSVGRadialGradientFrame *aRNgrad = (nsSVGRadialGradientFrame *)aNextGrad;
      aRNgrad->PrivateGetFx(aFx);
      return NS_OK;
    }
    // There are no gradients in the list with our type --  the spec
    // states that if there is no explicit fx value, we return the cx value
    // see http://www.w3.org/TR/SVG11/pservers.html#RadialGradients
    return PrivateGetCx(aFx);
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aRgrad->GetFx(getter_AddRefs(aLen));
  aLen->GetAnimVal(aFx);
  return NS_OK;
}

nsresult
nsSVGRadialGradientFrame::PrivateGetFy(nsIDOMSVGLength * *aFy)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> aRgrad = do_QueryInterface(mContent);
  NS_ASSERTION(aRgrad, "Wrong content element (not radial gradient)");
  if (aRgrad == nsnull) {
    return NS_ERROR_FAILURE;
  }
  // See if we need to get the value from another gradient
  if (checkURITarget(nsSVGAtoms::fy)) {
    // Yes, get it from the target
    nsISVGGradient *aNextGrad;
    if (nsSVGGradientFrame::GetNextGradient(&aNextGrad, nsISVGGradient::SVG_RADIAL_GRADIENT) == NS_OK) {
      nsSVGRadialGradientFrame *aRNgrad = (nsSVGRadialGradientFrame *)aNextGrad;
      aRNgrad->PrivateGetFy(aFy);
      return NS_OK;
    }
    // There are no gradients in the list with our type -- the spec
    // states that if there is no explicit fy value, we return the cy value
    // see http://www.w3.org/TR/SVG11/pservers.html#RadialGradients
    return PrivateGetCy(aFy);
  }
  // No, return the values
  nsCOMPtr<nsIDOMSVGAnimatedLength> aLen;
  aRgrad->GetFy(getter_AddRefs(aLen));
  aLen->GetAnimVal(aFy);
  return NS_OK;
}

// nsISVGRadialGradient
NS_IMETHODIMP
nsSVGRadialGradientFrame::GetFx(float *aFx)
{
  if (!mFx) {
    PrivateGetFx(getter_AddRefs(mFx));
    if (!mFx)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mFx);
  }
  mFx->GetValue(aFx);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGRadialGradientFrame::GetFy(float *aFy)
{
  if (!mFy) {
    PrivateGetFy(getter_AddRefs(mFy));
    if (!mFy)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mFy);
  }
  mFy->GetValue(aFy);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGRadialGradientFrame::GetCx(float *aCx)
{
  if (!mCx) {
    PrivateGetCx(getter_AddRefs(mCx));
    if (!mCx)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mCx);
  }
  mCx->GetValue(aCx);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGRadialGradientFrame::GetCy(float *aCy)
{
  if (!mCy) {
    PrivateGetCy(getter_AddRefs(mCy));
    if (!mCy)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mCy);
  }
  mCy->GetValue(aCy);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGRadialGradientFrame::GetR(float *aR)
{
  if (!mR) {
    PrivateGetR(getter_AddRefs(mR));
    if (!mR)
      return NS_ERROR_FAILURE;
    NS_ADD_SVGVALUE_OBSERVER(mR);
  }
  mR->GetValue(aR);
  return NS_OK;
}

// -------------------------------------------------------------------------
// Public functions
// -------------------------------------------------------------------------

nsresult NS_NewSVGLinearGradientFrame(nsIPresShell* aPresShell, 
                                      nsIContent*   aContent, 
                                      nsIFrame**    aNewFrame)
{
  *aNewFrame = nsnull;
  
  nsCOMPtr<nsIDOMSVGLinearGradientElement> grad = do_QueryInterface(aContent);
  NS_ASSERTION(grad, "NS_NewSVGLinearGradientFrame -- Content doesn't support nsIDOMSVGLinearGradient");
  if (!grad)
    return NS_ERROR_FAILURE;
  
  nsSVGLinearGradientFrame* it = new (aPresShell) nsSVGLinearGradientFrame;
  if (nsnull == it)
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIDOMSVGURIReference> aRef = do_QueryInterface(aContent);
  NS_ASSERTION(aRef, "NS_NewSVGLinearGradientFrame -- Content doesn't support nsIDOMSVGURIReference");
  if (!aRef) {
    it->mNextGrad = nsnull;
  } else {
    // Get the hRef
    nsCOMPtr<nsIDOMSVGAnimatedString> aHref;
    aRef->GetHref(getter_AddRefs(aHref));

    nsAutoString aStr;
    aHref->GetAnimVal(aStr);
    it->mNextGradStr = aStr;
    it->mNextGrad = nsnull;
  }

  *aNewFrame = it;

  return NS_OK;
}

nsresult NS_NewSVGRadialGradientFrame(nsIPresShell* aPresShell, 
                                      nsIContent*   aContent, 
                                      nsIFrame**    aNewFrame)
{
  *aNewFrame = nsnull;
  
  nsCOMPtr<nsIDOMSVGRadialGradientElement> grad = do_QueryInterface(aContent);
  NS_ASSERTION(grad, "NS_NewSVGRadialGradientFrame -- Content doesn't support nsIDOMSVGRadialGradient");
  if (!grad)
    return NS_ERROR_FAILURE;
  
  nsSVGRadialGradientFrame* it = new (aPresShell) nsSVGRadialGradientFrame;
  if (nsnull == it)
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIDOMSVGURIReference> aRef = do_QueryInterface(aContent);
  NS_ASSERTION(aRef, "NS_NewSVGRadialGradientFrame -- Content doesn't support nsIDOMSVGURIReference");
  if (!aRef) {
    it->mNextGrad = nsnull;
  } else {
    // Get the hRef
    nsCOMPtr<nsIDOMSVGAnimatedString> aHref;
    aRef->GetHref(getter_AddRefs(aHref));

    nsAutoString aStr;
    aHref->GetAnimVal(aStr);
    it->mNextGradStr = aStr;
    it->mNextGrad = nsnull;
  }

  *aNewFrame = it;

  return NS_OK;
}

// Public function to locate the SVGGradientFrame structure pointed to by a URI
// and return a nsISVGGradient
nsresult NS_GetSVGGradient(nsISVGGradient **aGrad, nsIURI *aURI, nsIContent *aContent, 
                           nsIPresShell *aPresShell)
{
  *aGrad = nsnull;

  // Get the spec from the URI
  nsCAutoString uriSpec;
  aURI->GetSpec(uriSpec);
  nsSVGGradientFrame *result;
  nsresult rv = GetSVGGradient(&result, uriSpec, aContent, aPresShell);
  *aGrad = (nsISVGGradient*)result;
  
  return rv;
}

// Static (helper) function to get a gradient from URI spec
static nsresult GetSVGGradient(nsSVGGradientFrame **result, nsCAutoString& aSpec, nsIContent *aContent,
                               nsIPresShell *aPresShell)
{
  nsresult rv = NS_OK;
  *result = nsnull;

  // Get the ID from the spec (no ID = an error)
  PRInt32 pos = aSpec.FindChar('#');
  if (pos == -1) {
    NS_ASSERTION(pos != -1, "URI Spec not a reference");
    return NS_ERROR_FAILURE;
  }

  // Strip off the hash and get the name
  nsCAutoString aURICName;
  aSpec.Right(aURICName, aSpec.Length()-(pos + 1));

  // Get a unicode string
  nsAutoString  aURIName;
  CopyUTF8toUTF16(aURICName, aURIName);

  // Get the domDocument
  nsCOMPtr<nsIDOMDocument>domDoc = do_QueryInterface(aContent->GetDocument());
  NS_ASSERTION(domDoc, "Content doesn't reference a dom Document");
  if (domDoc == nsnull) {
    return NS_ERROR_FAILURE;
  }

  // Get the gradient element
  nsCOMPtr<nsIDOMElement> element;
  rv = domDoc->GetElementById(aURIName, getter_AddRefs(element));
  if (!NS_SUCCEEDED(rv) || element == nsnull) {
    return rv;  
  }

  // Note: the following queries are not really required, but we want to make sure
  // we're pointing to gradient elements.
  nsCOMPtr<nsIDOMSVGLinearGradientElement> lGradient = do_QueryInterface(element);
  if (!lGradient) {
    nsCOMPtr<nsIDOMSVGRadialGradientElement> rGradient = do_QueryInterface(element);
    if (!rGradient) {
      NS_ASSERTION(PR_FALSE, "Not a gradient element!");
      return NS_ERROR_FAILURE;
    } 
  } 

  // Get the Primary Frame
  nsCOMPtr<nsIContent> aGContent = do_QueryInterface(element);
  NS_ASSERTION(aPresShell, "svg get gradient -- no pres shell provided");
  if (!aPresShell)
    return NS_ERROR_FAILURE;
  nsIFrame *grad;
  rv = aPresShell->GetPrimaryFrameFor(aGContent, &grad);

  *result = (nsSVGGradientFrame *)grad;
  return rv;
}
