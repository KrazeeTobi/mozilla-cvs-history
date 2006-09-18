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
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsSVGElement.h"
#include "nsSVGAnimatedString.h"
#include "nsSVGLength.h"
#include "nsSVGAtoms.h"
#include "nsSVGNumber2.h"
#include "nsGkAtoms.h"
#include "nsIDOMSVGFilters.h"
#include "nsCOMPtr.h"
#include "nsISVGFilter.h"
#include "nsSVGFilterInstance.h"
#include "nsSVGValue.h"
#include "nsISVGValueObserver.h"
#include "nsWeakReference.h"
#include "nsIDOMSVGFilterElement.h"
#include "nsSVGEnum.h"
#include "nsSVGAnimatedEnumeration.h"
#include "nsSVGNumberList.h"
#include "nsSVGAnimatedNumberList.h"
#include "nsISVGValueUtils.h"
#include "nsSVGFilters.h"
#include "nsSVGUtils.h"

nsSVGElement::LengthInfo nsSVGFE::sLengthInfo[4] =
{
  { &nsGkAtoms::x, 0, nsIDOMSVGLength::SVG_LENGTHTYPE_PERCENTAGE, nsSVGUtils::X },
  { &nsGkAtoms::y, 0, nsIDOMSVGLength::SVG_LENGTHTYPE_PERCENTAGE, nsSVGUtils::Y },
  { &nsGkAtoms::width, 100, nsIDOMSVGLength::SVG_LENGTHTYPE_PERCENTAGE, nsSVGUtils::X },
  { &nsGkAtoms::height, 100, nsIDOMSVGLength::SVG_LENGTHTYPE_PERCENTAGE, nsSVGUtils::Y },
};

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFE,nsSVGFEBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFE,nsSVGFEBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFE)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFE::nsSVGFE(nsINodeInfo* aNodeInfo) : nsSVGFEBase(aNodeInfo)
{
}

nsresult
nsSVGFE::Init()
{
  nsresult rv = nsSVGFEBase::Init();
  NS_ENSURE_SUCCESS(rv,rv);

  // DOM property: result ,  #REQUIRED  attrib: result
  {
    rv = NS_NewSVGAnimatedString(getter_AddRefs(mResult));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::result, mResult);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSVGFE::WillModifySVGObservable(nsISVGValue* observable,
                                 nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->BeginBatchUpdate();
  return nsSVGFEBase::WillModifySVGObservable(observable, aModType);
}

NS_IMETHODIMP
nsSVGFE::DidModifySVGObservable (nsISVGValue* observable,
                                 nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->EndBatchUpdate();
  return nsSVGFEBase::DidModifySVGObservable(observable, aModType);
}


//----------------------------------------------------------------------
// nsIDOMSVGFilterPrimitiveStandardAttributes methods

/* readonly attribute nsIDOMSVGAnimatedLength x; */
NS_IMETHODIMP nsSVGFE::GetX(nsIDOMSVGAnimatedLength * *aX)
{
  return mLengthAttributes[X].ToDOMAnimatedLength(aX, this);
}

/* readonly attribute nsIDOMSVGAnimatedLength y; */
NS_IMETHODIMP nsSVGFE::GetY(nsIDOMSVGAnimatedLength * *aY)
{
  return mLengthAttributes[Y].ToDOMAnimatedLength(aY, this);
}

/* readonly attribute nsIDOMSVGAnimatedLength width; */
NS_IMETHODIMP nsSVGFE::GetWidth(nsIDOMSVGAnimatedLength * *aWidth)
{
  return mLengthAttributes[WIDTH].ToDOMAnimatedLength(aWidth, this);
}

/* readonly attribute nsIDOMSVGAnimatedLength height; */
NS_IMETHODIMP nsSVGFE::GetHeight(nsIDOMSVGAnimatedLength * *aHeight)
{
  return mLengthAttributes[HEIGHT].ToDOMAnimatedLength(aHeight, this);
}

/* readonly attribute nsIDOMSVGAnimatedString result; */
NS_IMETHODIMP nsSVGFE::GetResult(nsIDOMSVGAnimatedString * *aResult)
{
  *aResult = mResult;
  NS_IF_ADDREF(*aResult);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGElement methods

void
nsSVGFE::DidChangeLength(PRUint8 aAttrEnum, PRBool aDoSetAttr)
{
  nsSVGFEBase::DidChangeLength(aAttrEnum, aDoSetAttr);

  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (value) {
    value->BeginBatchUpdate();
    value->EndBatchUpdate();
  }
}

void
nsSVGFE::DidChangeNumber(PRUint8 aAttrEnum, PRBool aDoSetAttr)
{
  nsSVGFEBase::DidChangeNumber(aAttrEnum, aDoSetAttr);

  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (value) {
    value->BeginBatchUpdate();
    value->EndBatchUpdate();
  }
}

nsSVGElement::LengthAttributesInfo
nsSVGFE::GetLengthInfo()
{
  return LengthAttributesInfo(mLengthAttributes, sLengthInfo,
                              NS_ARRAY_LENGTH(sLengthInfo));
}

//--------------------Filter Resource-----------------------
/**
 * nsSVGFilterResource provides functionality for obtaining and releasing
 * source and target images.  Images are automatically released when the
 * Filter Resource is destroyed, but methods to manually release images are
 * provided so that multiple source images may be used for a single filter.
 * PLEASE NOTE that nsSVGFilterResource should ONLY be used on the stack because
 * it has nsAutoString members
 */
class nsSVGFilterResource
{

public:
  nsSVGFilterResource(nsSVGFilterInstance* aInstance);
  ~nsSVGFilterResource();

  /*
   * Acquiring a source image will lock that image
   * and prepare its data to be read.
   * aIn:         the name of the filter primitive to use as the source
   * aFilter:     the filter that is calling AcquireImage
   * aSourceData: out parameter - the image data of the filter primitive
   *              specified by aIn
   */
  nsresult AcquireSourceImage(nsIDOMSVGAnimatedString* aIn,
                              nsSVGFE* aFilter, PRUint8** aSourceData);
  /*
   * Acquiring a target image will create and lock
   * a new surface to be used as the target image.
   * aResult:     the name by which the resulting filter primitive image will be
   *              identified
   * aTargetData: out parameter - the resulting filter primitive image data
   */
  nsresult AcquireTargetImage(nsIDOMSVGAnimatedString* aResult,
                              PRUint8** aTargetData);
  const nsRect& GetRect() {
    return mRect;
  }

  /*
   * Returns total length of data buffer in bytes
   */
  PRUint32 GetDataLength() {
    return mLength;
  }

  /*
   * Returns the total number of bytes per row of image data
   */
  PRInt32 GetDataStride() {
    return mStride;
  }

private:
  /*
   * Unlocks the source image if it exists
   */
  void ReleaseSource();

  /*
   * If the target image exists then it is unlocked and FixupTarget() is called
   */
  void ReleaseTarget();

  /*
   * FixupTarget copies the parts of the source image that lie outside of the
   * filter subregion into the target image.  If no source image exists, then
   * the area outside the filter region is filled with transparent black.
   */
  void FixupTarget();

  nsAutoString mInput, mResult;
  nsRect mRect;
  nsCOMPtr<nsISVGRendererSurface> mSourceImage, mTargetImage;
  nsSVGFilterInstance* mInstance;
  PRUint8 *mSourceData, *mTargetData;
  PRUint32 mWidth, mHeight, mLength;
  PRInt32 mStride;
};

nsSVGFilterResource::nsSVGFilterResource(nsSVGFilterInstance* aInstance):
  mInstance(aInstance),
  mSourceData(nsnull),
  mTargetData(nsnull),
  mWidth(0),
  mHeight(0),
  mLength(0),
  mStride(0)
{
}

nsSVGFilterResource::~nsSVGFilterResource()
{
  ReleaseTarget();
  ReleaseSource();
}

nsresult
nsSVGFilterResource::AcquireSourceImage(nsIDOMSVGAnimatedString* aIn,
                                        nsSVGFE* aFilter, PRUint8** aSourceData)
{
  ReleaseSource();
  nsRect defaultRect;
  aIn->GetAnimVal(mInput);
  mInstance->LookupRegion(mInput, &defaultRect);

  mInstance->GetFilterSubregion(aFilter, defaultRect, &mRect);
  mInstance->LookupImage(mInput, getter_AddRefs(mSourceImage));
  if (!mSourceImage) {
    return NS_ERROR_FAILURE;
  }
  mSourceImage->Lock();
  mSourceImage->GetData(&mSourceData, &mLength, &mStride);
  *aSourceData = mSourceData;
  return NS_OK;
}

nsresult
nsSVGFilterResource::AcquireTargetImage(nsIDOMSVGAnimatedString* aResult,
                                        PRUint8** aTargetData)
{
  aResult->GetAnimVal(mResult);
  mInstance->GetImage(getter_AddRefs(mTargetImage));
  if (!mTargetImage) {
    return NS_ERROR_FAILURE;
  }
  mTargetImage->Lock();
  mTargetImage->GetData(&mTargetData, &mLength, &mStride);
  mTargetImage->GetWidth(&mWidth);
  mTargetImage->GetHeight(&mHeight);
  *aTargetData = mTargetData;
  return NS_OK;
}

void
nsSVGFilterResource::ReleaseSource()
{
  if (!mSourceImage) {
    return;
  }
  mSourceImage->Unlock();
  mSourceImage = nsnull;
}

void
nsSVGFilterResource::ReleaseTarget()
{
  if (!mTargetImage) {
    return;
  }
  FixupTarget();
  mTargetImage->Unlock();
  mInstance->DefineImage(mResult, mTargetImage);
  mInstance->DefineRegion(mResult, mRect);
  mTargetImage = nsnull;
}

void
nsSVGFilterResource::FixupTarget()
{
  // top
  if (mRect.y > 0) {
    for (PRInt32 y = 0; y < mRect.y; y++)
      if (mSourceData) {
        memcpy(mTargetData + y * mStride, mSourceData + y * mStride, mStride);
      } else {
        memset(mTargetData + y * mStride, 0, mStride);
      }
  }

  // bottom
  if (mRect.y + mRect.height < mHeight) {
    for (PRInt32 y = mRect.y + mRect.height; y < mHeight; y++)
      if (mSourceData) {
        memcpy(mTargetData + y * mStride, mSourceData + y * mStride, mStride);
      } else {
        memset(mTargetData + y * mStride, 0, mStride);
      }
  }

  // left
  if (mRect.x > 0) {
    for (PRInt32 y = mRect.y; y < mRect.y + mRect.height; y++)
      memcpy(mTargetData + y * mStride, mSourceData + y * mStride, 4 * mRect.x);
  }

  // right
  if (mRect.x + mRect.width < mWidth) {
    for (PRInt32 y = mRect.y; y < mRect.y + mRect.height; y++)
      if (mSourceData) {
        memcpy(mTargetData + y * mStride + 4 * (mRect.x + mRect.width),
               mSourceData + y * mStride + 4 * (mRect.x + mRect.width),
               4 * (mWidth - mRect.x - mRect.width));
      } else {
        memset(mTargetData + y * mStride + 4 * (mRect.x + mRect.width),
               0,
               4 * (mWidth - mRect.x - mRect.width));
      }
  }
}

//---------------------Gaussian Blur------------------------

typedef nsSVGFE nsSVGFEGaussianBlurElementBase;

class nsSVGFEGaussianBlurElement : public nsSVGFEGaussianBlurElementBase,
                                   public nsIDOMSVGFEGaussianBlurElement,
                                   public nsISVGFilter
{
protected:
  friend nsresult NS_NewSVGFEGaussianBlurElement(nsIContent **aResult,
                                                 nsINodeInfo *aNodeInfo);
  nsSVGFEGaussianBlurElement(nsINodeInfo* aNodeInfo);
  nsresult Init();

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  // FE Base
  NS_FORWARD_NSIDOMSVGFILTERPRIMITIVESTANDARDATTRIBUTES(nsSVGFEGaussianBlurElementBase::)

  // nsISVGFilter
  NS_IMETHOD Filter(nsSVGFilterInstance *instance);
  NS_IMETHOD GetRequirements(PRUint32 *aRequirements);

  // Gaussian
  NS_DECL_NSIDOMSVGFEGAUSSIANBLURELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEGaussianBlurElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEGaussianBlurElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEGaussianBlurElementBase::)

  virtual nsresult SetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                           nsIAtom* aPrefix, const nsAString& aValue,
                           PRBool aNotify);
  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;

protected:
  virtual NumberAttributesInfo GetNumberInfo();

  enum { STD_DEV_X, STD_DEV_Y };
  nsSVGNumber2 mNumberAttributes[2];
  static NumberInfo sNumberInfo[2];

  nsCOMPtr<nsIDOMSVGAnimatedString> mIn1;
};

nsSVGElement::NumberInfo nsSVGFEGaussianBlurElement::sNumberInfo[2] =
{
  { &nsGkAtoms::stdDeviation, 0 },
  { &nsGkAtoms::stdDeviation, 0 }
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEGaussianBlur)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEGaussianBlurElement,nsSVGFEGaussianBlurElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEGaussianBlurElement,nsSVGFEGaussianBlurElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEGaussianBlurElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFilterPrimitiveStandardAttributes)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEGaussianBlurElement)
  NS_INTERFACE_MAP_ENTRY(nsISVGFilter)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEGaussianBlurElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEGaussianBlurElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEGaussianBlurElement::nsSVGFEGaussianBlurElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEGaussianBlurElementBase(aNodeInfo)
{
}

nsresult
nsSVGFEGaussianBlurElement::Init()
{
  nsresult rv = nsSVGFEGaussianBlurElementBase::Init();
  NS_ENSURE_SUCCESS(rv,rv);

  // DOM property: in1 , #IMPLIED attrib: in
  {
    rv = NS_NewSVGAnimatedString(getter_AddRefs(mIn1));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::in, mIn1);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return rv;
}

//----------------------------------------------------------------------
// nsIDOMNode methods


NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEGaussianBlurElement)


//----------------------------------------------------------------------
// nsIDOMSVGFEGaussianBlurElement methods

/* readonly attribute nsIDOMSVGAnimatedString in1; */
NS_IMETHODIMP nsSVGFEGaussianBlurElement::GetIn1(nsIDOMSVGAnimatedString * *aIn)
{
  *aIn = mIn1;
  NS_IF_ADDREF(*aIn);
  return NS_OK;
}

/* readonly attribute nsIDOMSVGAnimatedNumber stdDeviationX; */
NS_IMETHODIMP nsSVGFEGaussianBlurElement::GetStdDeviationX(nsIDOMSVGAnimatedNumber * *aX)
{
  return mNumberAttributes[STD_DEV_X].ToDOMAnimatedNumber(aX, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber stdDeviationY; */
NS_IMETHODIMP nsSVGFEGaussianBlurElement::GetStdDeviationY(nsIDOMSVGAnimatedNumber * *aY)
{
  return mNumberAttributes[STD_DEV_Y].ToDOMAnimatedNumber(aY, this);
}

NS_IMETHODIMP
nsSVGFEGaussianBlurElement::SetStdDeviation(float stdDeviationX, float stdDeviationY)
{
  mNumberAttributes[STD_DEV_X].SetBaseValue(stdDeviationX, this, PR_TRUE);
  mNumberAttributes[STD_DEV_Y].SetBaseValue(stdDeviationY, this, PR_TRUE);
  return NS_OK;
}

nsresult
nsSVGFEGaussianBlurElement::SetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                                    nsIAtom* aPrefix, const nsAString& aValue,
                                    PRBool aNotify)
{
  nsresult rv = nsSVGFEGaussianBlurElementBase::SetAttr(aNameSpaceID, aName, aPrefix,
                                                        aValue, aNotify);

  if (aName == nsSVGAtoms::stdDeviation && aNameSpaceID == kNameSpaceID_None) {
    float stdX = 0.0f, stdY = 0.0f;
    char *str;
    str = ToNewCString(aValue);
    int num = sscanf(str, "%f %f\n", &stdX, &stdY);
    if (num == 1)
      stdY = stdX;
    mNumberAttributes[STD_DEV_X].SetBaseValue(stdX, this, PR_FALSE);
    mNumberAttributes[STD_DEV_Y].SetBaseValue(stdY, this, PR_FALSE);
    nsMemory::Free(str);
  }

  return rv;
}

static void
boxBlurH(PRUint8 *aInput, PRUint8 *aOutput,
         PRInt32 aStride, nsRect aRegion,
         PRUint32 leftLobe, PRUint32 rightLobe)
{
  PRInt32 boxSize = leftLobe + rightLobe + 1;

  for (PRInt32 y = aRegion.y; y < aRegion.y + aRegion.height; y++) {
    PRUint32 sums[4] = {0, 0, 0, 0};
    for (PRInt32 i=0; i < boxSize; i++) {
      PRInt32 pos = aRegion.x - leftLobe + i;
      pos = PR_MAX(pos, aRegion.x);
      pos = PR_MIN(pos, aRegion.x + aRegion.width - 1);
      sums[0] += aInput[aStride*y + 4*pos    ];
      sums[1] += aInput[aStride*y + 4*pos + 1];
      sums[2] += aInput[aStride*y + 4*pos + 2];
      sums[3] += aInput[aStride*y + 4*pos + 3];
    }
    for (PRInt32 x = aRegion.x; x < aRegion.x + aRegion.width; x++) {
      PRInt32 tmp = x - leftLobe;
      PRInt32 last = PR_MAX(tmp, aRegion.x);
      PRInt32 next = PR_MIN(tmp + boxSize, aRegion.x + aRegion.width - 1);

      aOutput[aStride*y + 4*x    ] = sums[0]/boxSize;
      aOutput[aStride*y + 4*x + 1] = sums[1]/boxSize;
      aOutput[aStride*y + 4*x + 2] = sums[2]/boxSize;
      aOutput[aStride*y + 4*x + 3] = sums[3]/boxSize;

      sums[0] += aInput[aStride*y + 4*next    ] -
                 aInput[aStride*y + 4*last    ];
      sums[1] += aInput[aStride*y + 4*next + 1] -
                 aInput[aStride*y + 4*last + 1];
      sums[2] += aInput[aStride*y + 4*next + 2] -
                 aInput[aStride*y + 4*last + 2];
      sums[3] += aInput[aStride*y + 4*next + 3] -
                 aInput[aStride*y + 4*last + 3];
    }
  }
}

static void
boxBlurV(PRUint8 *aInput, PRUint8 *aOutput,
         PRInt32 aStride, nsRect aRegion,
         unsigned topLobe, unsigned bottomLobe)
{
  PRInt32 boxSize = topLobe + bottomLobe + 1;

  for (PRInt32 x = aRegion.x; x < aRegion.x + aRegion.width; x++) {
    PRUint32 sums[4] = {0, 0, 0, 0};
    for (PRInt32 i=0; i < boxSize; i++) {
      PRInt32 pos = aRegion.y - topLobe + i;
      pos = PR_MAX(pos, aRegion.y);
      pos = PR_MIN(pos, aRegion.y + aRegion.height - 1);
      sums[0] += aInput[aStride*pos + 4*x    ];
      sums[1] += aInput[aStride*pos + 4*x + 1];
      sums[2] += aInput[aStride*pos + 4*x + 2];
      sums[3] += aInput[aStride*pos + 4*x + 3];
    }
    for (PRInt32 y = aRegion.y; y < aRegion.y + aRegion.height; y++) {
      PRInt32 tmp = y - topLobe;
      PRInt32 last = PR_MAX(tmp, aRegion.y);
      PRInt32 next = PR_MIN(tmp + boxSize, aRegion.y + aRegion.height - 1);

      aOutput[aStride*y + 4*x    ] = sums[0]/boxSize;
      aOutput[aStride*y + 4*x + 1] = sums[1]/boxSize;
      aOutput[aStride*y + 4*x + 2] = sums[2]/boxSize;
      aOutput[aStride*y + 4*x + 3] = sums[3]/boxSize;

      sums[0] += aInput[aStride*next + 4*x    ] -
                 aInput[aStride*last + 4*x    ];
      sums[1] += aInput[aStride*next + 4*x + 1] -
                 aInput[aStride*last + 4*x + 1];
      sums[2] += aInput[aStride*next + 4*x + 2] -
                 aInput[aStride*last + 4*x + 2];
      sums[3] += aInput[aStride*next + 4*x + 3] -
                 aInput[aStride*last + 4*x + 3];
    }
  }
}

static nsresult
gaussianBlur(PRUint8 *aInput, PRUint8 *aOutput,
             PRUint32 aLength, PRInt32 aStride, nsRect aRegion,
             float aStdX, float aStdY)
{
  if (aStdX < 0 || aStdY < 0) {
    return NS_ERROR_FAILURE;
  }

  if (aStdX == 0 || aStdY == 0) {
    memset(aOutput, 0, aLength);
    return NS_OK;
  }

  PRUint32 dX, dY;
  dX = (PRUint32) floor(aStdX * 3*sqrt(2*M_PI)/4 + 0.5);
  dY = (PRUint32) floor(aStdY * 3*sqrt(2*M_PI)/4 + 0.5);

  PRUint8 *tmp = new PRUint8[aLength];

  if (dX & 1) {
    // odd
    boxBlurH(aInput, tmp,  aStride, aRegion, dX/2, dX/2);
    boxBlurH(tmp, aOutput, aStride, aRegion, dX/2, dX/2);
    boxBlurH(aOutput, tmp, aStride, aRegion, dX/2, dX/2);
  } else {
    // even
    if (dX == 0) {
      memcpy(tmp, aInput, aLength);
    } else {
      boxBlurH(aInput, tmp,  aStride, aRegion, dX/2,     dX/2 - 1);
      boxBlurH(tmp, aOutput, aStride, aRegion, dX/2 - 1, dX/2);
      boxBlurH(aOutput, tmp, aStride, aRegion, dX/2,     dX/2);
    }
  }

  if (dY & 1) {
    // odd
    boxBlurV(tmp, aOutput, aStride, aRegion, dY/2, dY/2);
    boxBlurV(aOutput, tmp, aStride, aRegion, dY/2, dY/2);
    boxBlurV(tmp, aOutput, aStride, aRegion, dY/2, dY/2);
  } else {
    // even
    if (dY == 0) {
      memcpy(aOutput, tmp, aLength);
    } else {
      boxBlurV(tmp, aOutput, aStride, aRegion, dY/2,     dY/2 - 1);
      boxBlurV(aOutput, tmp, aStride, aRegion, dY/2 - 1, dY/2);
      boxBlurV(tmp, aOutput, aStride, aRegion, dY/2,     dY/2);
    }
  }

  delete [] tmp;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGFEGaussianBlurElement::Filter(nsSVGFilterInstance *instance)
{
  nsresult rv;
  PRUint8 *sourceData, *targetData;
  nsSVGFilterResource fr(instance);

  rv = fr.AcquireSourceImage(mIn1, this, &sourceData);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = fr.AcquireTargetImage(mResult, &targetData);
  NS_ENSURE_SUCCESS(rv, rv);
  nsRect rect = fr.GetRect();

#ifdef DEBUG_tor
  fprintf(stderr, "FILTER GAUSS rect: %d,%d  %dx%d\n",
          rect.x, rect.y, rect.width, rect.height);
#endif

  float stdX, stdY;
  nsSVGLength2 val;

  GetAnimatedNumberValues(&stdX, &stdY, nsnull);
  val.Init(nsSVGUtils::X, 0xff, stdX, nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER);
  stdX = instance->GetPrimitiveLength(&val);

  val.Init(nsSVGUtils::Y, 0xff, stdY, nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER);
  stdY = instance->GetPrimitiveLength(&val);

  rv = gaussianBlur(sourceData, targetData, fr.GetDataLength(),
                    fr.GetDataStride(), rect, stdX, stdY);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

static PRUint32
CheckStandardNames(nsIDOMSVGAnimatedString *aStr)
{
  nsAutoString str;
  aStr->GetAnimVal(str);

  if (str.EqualsLiteral("SourceGraphic"))
    return NS_FE_SOURCEGRAPHIC;
  if (str.EqualsLiteral("SourceAlpha"))
    return NS_FE_SOURCEALPHA;
  if (str.EqualsLiteral("BackgroundImage"))
    return NS_FE_BACKGROUNDIMAGE;
  if (str.EqualsLiteral("BackgroundAlpha"))
    return NS_FE_BACKGROUNDALPHA;
  if (str.EqualsLiteral("FillPaint"))
    return NS_FE_FILLPAINT;
  if (str.EqualsLiteral("StrokePaint"))
    return NS_FE_STROKEPAINT;
  return 0;
}

NS_IMETHODIMP
nsSVGFEGaussianBlurElement::GetRequirements(PRUint32 *aRequirements)
{
  *aRequirements = CheckStandardNames(mIn1);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGElement methods

nsSVGElement::NumberAttributesInfo
nsSVGFEGaussianBlurElement::GetNumberInfo()
{
  return NumberAttributesInfo(mNumberAttributes, sNumberInfo,
                              NS_ARRAY_LENGTH(sNumberInfo));
}

//---------------------Component Transfer------------------------

typedef nsSVGFE nsSVGFEComponentTransferElementBase;

class nsSVGFEComponentTransferElement : public nsSVGFEComponentTransferElementBase,
                                        public nsIDOMSVGFEComponentTransferElement,
                                        public nsISVGFilter
{
protected:
  friend nsresult NS_NewSVGFEComponentTransferElement(nsIContent **aResult,
                                                      nsINodeInfo *aNodeInfo);
  nsSVGFEComponentTransferElement(nsINodeInfo* aNodeInfo);
  nsresult Init();

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  // FE Base
  NS_FORWARD_NSIDOMSVGFILTERPRIMITIVESTANDARDATTRIBUTES(nsSVGFEComponentTransferElementBase::)

  // nsISVGFilter
  NS_IMETHOD Filter(nsSVGFilterInstance *instance);
  NS_IMETHOD GetRequirements(PRUint32 *aRequirements);

  // Component Transfer
  NS_DECL_NSIDOMSVGFECOMPONENTTRANSFERELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEComponentTransferElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEComponentTransferElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEComponentTransferElementBase::)

  // nsIContent
  virtual nsresult InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                 PRBool aNotify);
  virtual nsresult RemoveChildAt(PRUint32 aIndex, PRBool aNotify);

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;

protected:

  nsCOMPtr<nsIDOMSVGAnimatedString> mIn1;
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEComponentTransfer)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEComponentTransferElement,nsSVGFEComponentTransferElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEComponentTransferElement,nsSVGFEComponentTransferElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEComponentTransferElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFilterPrimitiveStandardAttributes)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEComponentTransferElement)
  NS_INTERFACE_MAP_ENTRY(nsISVGFilter)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEComponentTransferElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEComponentTransferElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEComponentTransferElement::nsSVGFEComponentTransferElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEComponentTransferElementBase(aNodeInfo)
{
}

nsresult
nsSVGFEComponentTransferElement::Init()
{
  nsresult rv = nsSVGFEComponentTransferElementBase::Init();
  NS_ENSURE_SUCCESS(rv,rv);

  // DOM property: in1 , #IMPLIED attrib: in
  {
    rv = NS_NewSVGAnimatedString(getter_AddRefs(mIn1));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::in, mIn1);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return rv;
}

//----------------------------------------------------------------------
// nsIDOMNode methods

NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEComponentTransferElement)

//----------------------------------------------------------------------
// nsIDOMSVGFEComponentTransferElement methods

/* readonly attribute nsIDOMSVGAnimatedString in1; */
NS_IMETHODIMP
nsSVGFEComponentTransferElement::GetIn1(nsIDOMSVGAnimatedString * *aIn)
{
  *aIn = mIn1;
  NS_IF_ADDREF(*aIn);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGFEComponentTransferElement::Filter(nsSVGFilterInstance *instance)
{
  nsresult rv;
  PRUint8 *sourceData, *targetData;
  nsSVGFilterResource fr(instance);

  rv = fr.AcquireSourceImage(mIn1, this, &sourceData);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = fr.AcquireTargetImage(mResult, &targetData);
  NS_ENSURE_SUCCESS(rv, rv);
  nsRect rect = fr.GetRect();

#ifdef DEBUG_tor
  fprintf(stderr, "FILTER COMPONENT rect: %d,%d  %dx%d\n",
          rect.x, rect.y, rect.width, rect.height);
#endif

  PRUint8 tableR[256], tableG[256], tableB[256], tableA[256];
  for (int i=0; i<256; i++)
    tableR[i] = tableG[i] = tableB[i] = tableA[i] = i;
  PRUint32 count = GetChildCount();
  for (PRUint32 k = 0; k < count; k++) {
    nsCOMPtr<nsIContent> child = GetChildAt(k);
    nsCOMPtr<nsIDOMSVGFEFuncRElement> elementR = do_QueryInterface(child);
    nsCOMPtr<nsIDOMSVGFEFuncGElement> elementG = do_QueryInterface(child);
    nsCOMPtr<nsIDOMSVGFEFuncBElement> elementB = do_QueryInterface(child);
    nsCOMPtr<nsIDOMSVGFEFuncAElement> elementA = do_QueryInterface(child);
    if (elementR)
      elementR->GenerateLookupTable(tableR);
    if (elementG)
      elementG->GenerateLookupTable(tableG);
    if (elementB)
      elementB->GenerateLookupTable(tableB);
    if (elementA)
      elementA->GenerateLookupTable(tableA);
  }

  PRInt32 stride = fr.GetDataStride();
  for (PRInt32 y = rect.y; y < rect.y + rect.height; y++)
    for (PRInt32 x = rect.x; x < rect.x + rect.width; x++) {
      PRUint32 r, g, b, a;
      a = sourceData[y * stride + 4 * x + 3];
      if (a) {
        b = tableB[(255 * (PRUint32)sourceData[y * stride + 4 * x])     / a];
        g = tableG[(255 * (PRUint32)sourceData[y * stride + 4 * x + 1]) / a];
        r = tableR[(255 * (PRUint32)sourceData[y * stride + 4 * x + 2]) / a];
      } else {
        b = g = r = 0;
      }
      a = tableA[a];
      targetData[y * stride + 4 * x]     = (b * a) / 255;
      targetData[y * stride + 4 * x + 1] = (g * a) / 255;
      targetData[y * stride + 4 * x + 2] = (r * a) / 255;
      targetData[y * stride + 4 * x + 3] = a;
    }
  return NS_OK;
}

NS_IMETHODIMP
nsSVGFEComponentTransferElement::GetRequirements(PRUint32 *aRequirements)
{
  *aRequirements = CheckStandardNames(mIn1);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsIContent methods

nsresult
nsSVGFEComponentTransferElement::InsertChildAt(nsIContent* aKid,
                                               PRUint32 aIndex,
                                               PRBool aNotify)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->BeginBatchUpdate();
  nsresult rv = nsSVGFEComponentTransferElementBase::InsertChildAt(aKid,
                                                                   aIndex,
                                                                   aNotify);
  if (filter && value)
    value->EndBatchUpdate();

  return rv;
}

nsresult
nsSVGFEComponentTransferElement::RemoveChildAt(PRUint32 aIndex, PRBool aNotify)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->BeginBatchUpdate();
  nsresult rv = nsSVGFEComponentTransferElementBase::RemoveChildAt(aIndex,
                                                                   aNotify);
  if (filter && value)
    value->EndBatchUpdate();

  return rv;
}

//--------------------------------------------

typedef nsSVGElement nsSVGComponentTransferFunctionElementBase;

class nsSVGComponentTransferFunctionElement : public nsSVGComponentTransferFunctionElementBase
{
protected:
  friend nsresult NS_NewSVGComponentTransferFunctionElement(nsIContent **aResult,
                                       nsINodeInfo *aNodeInfo);
  nsSVGComponentTransferFunctionElement(nsINodeInfo* aNodeInfo);
  nsresult Init();

  // nsISVGValueObserver interface:
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable (nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);

public:
  // interfaces:

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIDOMSVGCOMPONENTTRANSFERFUNCTIONELEMENT

protected:
  virtual NumberAttributesInfo GetNumberInfo();

  // nsIDOMSVGComponentTransferFunctionElement properties:
  nsCOMPtr<nsIDOMSVGAnimatedEnumeration> mType;
  nsCOMPtr<nsIDOMSVGAnimatedNumberList>  mTableValues;

  enum { SLOPE, INTERCEPT, AMPLITUDE, EXPONENT, OFFSET };
  nsSVGNumber2 mNumberAttributes[5];
  static NumberInfo sNumberInfo[5];

};

nsSVGElement::NumberInfo nsSVGComponentTransferFunctionElement::sNumberInfo[5] =
{
  { &nsGkAtoms::slope,     0 },
  { &nsGkAtoms::intercept, 0 },
  { &nsGkAtoms::amplitude, 0 },
  { &nsGkAtoms::exponent,  0 },
  { &nsGkAtoms::offset,    0 }
};

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGComponentTransferFunctionElement,nsSVGComponentTransferFunctionElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGComponentTransferFunctionElement,nsSVGComponentTransferFunctionElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGComponentTransferFunctionElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGComponentTransferFunctionElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGComponentTransferFunctionElement::nsSVGComponentTransferFunctionElement(nsINodeInfo* aNodeInfo)
  : nsSVGComponentTransferFunctionElementBase(aNodeInfo)
{
}

nsresult
nsSVGComponentTransferFunctionElement::Init()
{
  nsresult rv;

  // enumeration mappings
  static struct nsSVGEnumMapping gComponentTransferTypes[] = {
    {&nsSVGAtoms::identity,
     nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY},
    {&nsSVGAtoms::table,
     nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_TABLE},
    {&nsSVGAtoms::discrete,
     nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE},
    {&nsSVGAtoms::linear,
     nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_LINEAR},
    {&nsSVGAtoms::gamma,
     nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_GAMMA},
    {nsnull, 0}
  };

  // Create mapped properties:

  // DOM property: type, #IMPLIED attrib: type
  {
    nsCOMPtr<nsISVGEnum> types;
    rv = NS_NewSVGEnum(getter_AddRefs(types),
                       nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY,
                       gComponentTransferTypes);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = NS_NewSVGAnimatedEnumeration(getter_AddRefs(mType), types);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::type, mType);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  // DOM property: tableValues, #IMPLIED attrib: tableValues
  {
    nsCOMPtr<nsIDOMSVGNumberList> values;
    rv = NS_NewSVGNumberList(getter_AddRefs(values));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = NS_NewSVGAnimatedNumberList(getter_AddRefs(mTableValues), values);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::tableValues, mTableValues);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSVGComponentTransferFunctionElement::WillModifySVGObservable(nsISVGValue* observable,
                                 nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFEComponentTransferElement> element;
  nsCOMPtr<nsIDOMSVGFilterElement> filter;
  element = do_QueryInterface(GetParent());
  if (GetParent())
    filter = do_QueryInterface(GetParent()->GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(filter);

  if (element && filter && value)
    value->BeginBatchUpdate();
  return nsSVGComponentTransferFunctionElementBase::WillModifySVGObservable(observable, aModType);
}

NS_IMETHODIMP
nsSVGComponentTransferFunctionElement::DidModifySVGObservable (nsISVGValue* observable,
                                 nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFEComponentTransferElement> element;
  nsCOMPtr<nsIDOMSVGFilterElement> filter;
  element = do_QueryInterface(GetParent());
  if (GetParent())
    filter = do_QueryInterface(GetParent()->GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(filter);

  if (element && filter && value)
    value->EndBatchUpdate();
  return nsSVGComponentTransferFunctionElementBase::DidModifySVGObservable(observable, aModType);
}

//----------------------------------------------------------------------
// nsIDOMSVGComponentTransferFunctionElement methods

/* readonly attribute nsIDOMSVGAnimatedEnumeration type; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetType(nsIDOMSVGAnimatedEnumeration * *aType)
{
  *aType = mType;
  NS_IF_ADDREF(*aType);
  return NS_OK;
}

/* readonly attribute nsIDOMSVGAnimatedNumberList tableValues; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetTableValues(nsIDOMSVGAnimatedNumberList * *aTableValues)
{
  *aTableValues = mTableValues;
  NS_IF_ADDREF(*aTableValues);
  return NS_OK;
}

/* readonly attribute nsIDOMSVGAnimatedNumber slope; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetSlope(nsIDOMSVGAnimatedNumber * *aSlope)
{
  return mNumberAttributes[SLOPE].ToDOMAnimatedNumber(aSlope, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber intercept; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetIntercept(nsIDOMSVGAnimatedNumber * *aIntercept)
{
  return mNumberAttributes[INTERCEPT].ToDOMAnimatedNumber(aIntercept, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber amplitude; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetAmplitude(nsIDOMSVGAnimatedNumber * *aAmplitude)
{
  return mNumberAttributes[AMPLITUDE].ToDOMAnimatedNumber(aAmplitude, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber exponent; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetExponent(nsIDOMSVGAnimatedNumber * *aExponent)
{
  return mNumberAttributes[EXPONENT].ToDOMAnimatedNumber(aExponent, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber offset; */
NS_IMETHODIMP nsSVGComponentTransferFunctionElement::GetOffset(nsIDOMSVGAnimatedNumber * *aOffset)
{
  return mNumberAttributes[OFFSET].ToDOMAnimatedNumber(aOffset, this);
}

NS_IMETHODIMP
nsSVGComponentTransferFunctionElement::GenerateLookupTable(PRUint8 *aTable)
{
  PRUint16 type;
  mType->GetAnimVal(&type);

  float slope, intercept, amplitude, exponent, offset;
  GetAnimatedNumberValues(&slope, &intercept, &amplitude, 
                          &exponent, &offset, nsnull);

  PRUint32 i;

  switch (type) {
  case nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_TABLE:
  {
    nsCOMPtr<nsIDOMSVGNumberList> list;
    nsCOMPtr<nsIDOMSVGNumber> number;
    mTableValues->GetAnimVal(getter_AddRefs(list));
    PRUint32 num = 0;
    if (list)
      list->GetNumberOfItems(&num);
    if (num <= 1)
      break;

    for (i = 0; i < 256; i++) {
      PRInt32 k = (i * (num - 1)) / 255;
      float v1, v2;
      list->GetItem(k, getter_AddRefs(number));
      number->GetValue(&v1);
      list->GetItem(PR_MIN(k + 1, num - 1), getter_AddRefs(number));
      number->GetValue(&v2);
      PRInt32 val =
        PRInt32(255 * (v1 + (i/255.0f - k/float(num-1))*(num - 1)*(v2 - v1)));
      val = PR_MIN(255, val);
      val = PR_MAX(0, val);
      aTable[i] = val;
    }
    break;
  }

  case nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE:
  {
    nsCOMPtr<nsIDOMSVGNumberList> list;
    nsCOMPtr<nsIDOMSVGNumber> number;
    mTableValues->GetAnimVal(getter_AddRefs(list));
    PRUint32 num = 0;
    if (list)
      list->GetNumberOfItems(&num);
    if (num <= 1)
      break;

    for (i = 0; i < 256; i++) {
      PRInt32 k = (i * num) / 255;
      k = PR_MIN(k, num - 1);
      float v;
      list->GetItem(k, getter_AddRefs(number));
      number->GetValue(&v);
      PRInt32 val = PRInt32(255 * v);
      val = PR_MIN(255, val);
      val = PR_MAX(0, val);
      aTable[i] = val;
    }
    break;
  }

  case nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_LINEAR:
  {
    for (i = 0; i < 256; i++) {
      PRInt32 val = PRInt32(slope * i + 255 * intercept);
      val = PR_MIN(255, val);
      val = PR_MAX(0, val);
      aTable[i] = val;
    }
    break;
  }

  case nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_GAMMA:
  {
    for (i = 0; i < 256; i++) {
      PRInt32 val = PRInt32(255 * (amplitude * pow(i / 255.0f, exponent) + offset));
      val = PR_MIN(255, val);
      val = PR_MAX(0, val);
      aTable[i] = val;
    }
    break;
  }

  case nsIDOMSVGComponentTransferFunctionElement::SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY:
  default:
    break;
  }

  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGElement methods

nsSVGElement::NumberAttributesInfo
nsSVGComponentTransferFunctionElement::GetNumberInfo()
{
  return NumberAttributesInfo(mNumberAttributes, sNumberInfo,
                              NS_ARRAY_LENGTH(sNumberInfo));
}

class nsSVGFEFuncRElement : public nsSVGComponentTransferFunctionElement,
                            public nsIDOMSVGFEFuncRElement
{
protected:
  friend nsresult NS_NewSVGFEFuncRElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEFuncRElement(nsINodeInfo* aNodeInfo) 
    : nsSVGComponentTransferFunctionElement(aNodeInfo) {}

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  NS_FORWARD_NSIDOMSVGCOMPONENTTRANSFERFUNCTIONELEMENT(nsSVGComponentTransferFunctionElement::)

  NS_DECL_NSIDOMSVGFEFUNCRELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMNODE(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGComponentTransferFunctionElement::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_ADDREF_INHERITED(nsSVGFEFuncRElement,nsSVGComponentTransferFunctionElement)
NS_IMPL_RELEASE_INHERITED(nsSVGFEFuncRElement,nsSVGComponentTransferFunctionElement)

NS_INTERFACE_MAP_BEGIN(nsSVGFEFuncRElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGComponentTransferFunctionElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEFuncRElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEFuncRElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGComponentTransferFunctionElement)

NS_IMPL_NS_NEW_SVG_ELEMENT(FEFuncR)
NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEFuncRElement)


class nsSVGFEFuncGElement : public nsSVGComponentTransferFunctionElement,
                            public nsIDOMSVGFEFuncGElement
{
protected:
  friend nsresult NS_NewSVGFEFuncGElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEFuncGElement(nsINodeInfo* aNodeInfo) 
    : nsSVGComponentTransferFunctionElement(aNodeInfo) {}

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  NS_FORWARD_NSIDOMSVGCOMPONENTTRANSFERFUNCTIONELEMENT(nsSVGComponentTransferFunctionElement::)

  NS_DECL_NSIDOMSVGFEFUNCGELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMNODE(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGComponentTransferFunctionElement::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_ADDREF_INHERITED(nsSVGFEFuncGElement,nsSVGComponentTransferFunctionElement)
NS_IMPL_RELEASE_INHERITED(nsSVGFEFuncGElement,nsSVGComponentTransferFunctionElement)

NS_INTERFACE_MAP_BEGIN(nsSVGFEFuncGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGComponentTransferFunctionElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEFuncGElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEFuncGElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGComponentTransferFunctionElement)

NS_IMPL_NS_NEW_SVG_ELEMENT(FEFuncG)
NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEFuncGElement)


class nsSVGFEFuncBElement : public nsSVGComponentTransferFunctionElement,
                            public nsIDOMSVGFEFuncBElement
{
protected:
  friend nsresult NS_NewSVGFEFuncBElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEFuncBElement(nsINodeInfo* aNodeInfo) 
    : nsSVGComponentTransferFunctionElement(aNodeInfo) {}

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  NS_FORWARD_NSIDOMSVGCOMPONENTTRANSFERFUNCTIONELEMENT(nsSVGComponentTransferFunctionElement::)

  NS_DECL_NSIDOMSVGFEFUNCBELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMNODE(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGComponentTransferFunctionElement::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_ADDREF_INHERITED(nsSVGFEFuncBElement,nsSVGComponentTransferFunctionElement)
NS_IMPL_RELEASE_INHERITED(nsSVGFEFuncBElement,nsSVGComponentTransferFunctionElement)

NS_INTERFACE_MAP_BEGIN(nsSVGFEFuncBElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGComponentTransferFunctionElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEFuncBElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEFuncBElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGComponentTransferFunctionElement)

NS_IMPL_NS_NEW_SVG_ELEMENT(FEFuncB)
NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEFuncBElement)


class nsSVGFEFuncAElement : public nsSVGComponentTransferFunctionElement,
                            public nsIDOMSVGFEFuncAElement
{
protected:
  friend nsresult NS_NewSVGFEFuncAElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEFuncAElement(nsINodeInfo* aNodeInfo) 
    : nsSVGComponentTransferFunctionElement(aNodeInfo) {}

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  NS_FORWARD_NSIDOMSVGCOMPONENTTRANSFERFUNCTIONELEMENT(nsSVGComponentTransferFunctionElement::)

  NS_DECL_NSIDOMSVGFEFUNCAELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMNODE(nsSVGComponentTransferFunctionElement::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGComponentTransferFunctionElement::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_ADDREF_INHERITED(nsSVGFEFuncAElement,nsSVGComponentTransferFunctionElement)
NS_IMPL_RELEASE_INHERITED(nsSVGFEFuncAElement,nsSVGComponentTransferFunctionElement)

NS_INTERFACE_MAP_BEGIN(nsSVGFEFuncAElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGComponentTransferFunctionElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEFuncAElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEFuncAElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGComponentTransferFunctionElement)

NS_IMPL_NS_NEW_SVG_ELEMENT(FEFuncA)
NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEFuncAElement)

//---------------------Merge------------------------

typedef nsSVGFE nsSVGFEMergeElementBase;

class nsSVGFEMergeElement : public nsSVGFEMergeElementBase,
                            public nsIDOMSVGFEMergeElement,
                            public nsISVGFilter
{
protected:
  friend nsresult NS_NewSVGFEMergeElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEMergeElement(nsINodeInfo* aNodeInfo);
  virtual ~nsSVGFEMergeElement();

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  // FE Base
  NS_FORWARD_NSIDOMSVGFILTERPRIMITIVESTANDARDATTRIBUTES(nsSVGFEMergeElementBase::)

  // nsISVGFilter
  NS_IMETHOD Filter(nsSVGFilterInstance *instance);
  NS_IMETHOD GetRequirements(PRUint32 *aRequirements);

  // Gaussian
  NS_DECL_NSIDOMSVGFEMERGEELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEMergeElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEMergeElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEMergeElementBase::)

  // nsIContent
  virtual nsresult InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                 PRBool aNotify);
  virtual nsresult RemoveChildAt(PRUint32 aIndex, PRBool aNotify);

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEMerge)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEMergeElement,nsSVGFEMergeElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEMergeElement,nsSVGFEMergeElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEMergeElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFilterPrimitiveStandardAttributes)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEMergeElement)
  NS_INTERFACE_MAP_ENTRY(nsISVGFilter)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEMergeElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEMergeElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEMergeElement::nsSVGFEMergeElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEMergeElementBase(aNodeInfo)
{
}

nsSVGFEMergeElement::~nsSVGFEMergeElement()
{
}

//----------------------------------------------------------------------
// nsIDOMNode methods

NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEMergeElement)

NS_IMETHODIMP
nsSVGFEMergeElement::Filter(nsSVGFilterInstance *instance)
{
  nsresult rv;
  PRUint8 *sourceData, *targetData;

  nsSVGFilterResource fr(instance);

  rv = fr.AcquireTargetImage(mResult, &targetData);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 count = GetChildCount();
  for (PRUint32 i = 0; i < count; i++) {
    nsCOMPtr<nsIContent> child = GetChildAt(i);
    nsCOMPtr<nsIDOMSVGFEMergeNodeElement> node = do_QueryInterface(child);
    if (!node)
      continue;
    nsCOMPtr<nsIDOMSVGAnimatedString> str;
    node->GetIn1(getter_AddRefs(str));

    rv = fr.AcquireSourceImage(str, this, &sourceData);
    NS_ENSURE_SUCCESS(rv, rv);
    nsRect rect = fr.GetRect();

#ifdef DEBUG_tor
    fprintf(stderr, "FILTER MERGE rect: %d,%d  %dx%d\n",
            rect.x, rect.y, rect.width, rect.height);
#endif

#define BLEND(target, source, alpha) \
    target = PR_MIN(255, (target * (255 - alpha))/255 + source)

    PRInt32 stride = fr.GetDataStride();
    for (PRInt32 y = rect.y; y < rect.y + rect.height; y++)
      for (PRInt32 x = rect.x; x < rect.x + rect.width; x++) {
        PRUint32 a = sourceData[y * stride + 4 * x + 3];
        BLEND(targetData[y * stride + 4 * x    ],
              sourceData[y * stride + 4 * x    ], a);
        BLEND(targetData[y * stride + 4 * x + 1],
              sourceData[y * stride + 4 * x + 1], a);
        BLEND(targetData[y * stride + 4 * x + 2],
              sourceData[y * stride + 4 * x + 2], a);
        BLEND(targetData[y * stride + 4 * x + 3],
              sourceData[y * stride + 4 * x + 3], a);
      }
#undef BLEND
  }
  return NS_OK;
}


NS_IMETHODIMP
nsSVGFEMergeElement::GetRequirements(PRUint32 *aRequirements)
{
  *aRequirements = 0;

  PRUint32 count = GetChildCount();
  for (PRUint32 i = 0; i < count; i++) {
    nsCOMPtr<nsIContent> child = GetChildAt(i);
    nsCOMPtr<nsIDOMSVGFEMergeNodeElement> node = do_QueryInterface(child);
    if (node) {
      nsCOMPtr<nsIDOMSVGAnimatedString> str;
      node->GetIn1(getter_AddRefs(str));
      *aRequirements |= CheckStandardNames(str);
    }
  }

  return NS_OK;
}

//----------------------------------------------------------------------
// nsIContent methods

nsresult
nsSVGFEMergeElement::InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                   PRBool aNotify)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->BeginBatchUpdate();
  nsresult rv = nsSVGFEMergeElementBase::InsertChildAt(aKid, aIndex, aNotify);
  if (filter && value)
    value->EndBatchUpdate();

  return rv;
}

nsresult
nsSVGFEMergeElement::RemoveChildAt(PRUint32 aIndex, PRBool aNotify)
{
  nsCOMPtr<nsIDOMSVGFilterElement> filter = do_QueryInterface(GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(GetParent());
  if (filter && value)
    value->BeginBatchUpdate();
  nsresult rv = nsSVGFEMergeElementBase::RemoveChildAt(aIndex, aNotify);
  if (filter && value)
    value->EndBatchUpdate();

  return rv;
}

//---------------------Merge Node------------------------

typedef nsSVGStylableElement nsSVGFEMergeNodeElementBase;

class nsSVGFEMergeNodeElement : public nsSVGFEMergeNodeElementBase,
                                public nsIDOMSVGFEMergeNodeElement
{
protected:
  friend nsresult NS_NewSVGFEMergeNodeElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEMergeNodeElement(nsINodeInfo* aNodeInfo);
  nsresult Init();

  // nsISVGValueObserver interface:
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable (nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  NS_DECL_NSIDOMSVGFEMERGENODEELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEMergeNodeElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEMergeNodeElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEMergeNodeElementBase::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;

protected:
  nsCOMPtr<nsIDOMSVGAnimatedString> mIn1;
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEMergeNode)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEMergeNodeElement,nsSVGFEMergeNodeElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEMergeNodeElement,nsSVGFEMergeNodeElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEMergeNodeElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEMergeNodeElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEMergeNodeElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEMergeNodeElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEMergeNodeElement::nsSVGFEMergeNodeElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEMergeNodeElementBase(aNodeInfo)
{
}

nsresult
nsSVGFEMergeNodeElement::Init()
{
  nsresult rv = nsSVGFEMergeNodeElementBase::Init();
  NS_ENSURE_SUCCESS(rv,rv);

  // DOM property: in1 , #IMPLIED attrib: in
  {
    rv = NS_NewSVGAnimatedString(getter_AddRefs(mIn1));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::in, mIn1);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return rv;
}

//----------------------------------------------------------------------
// nsIDOMNode methods

NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEMergeNodeElement)

NS_IMETHODIMP
nsSVGFEMergeNodeElement::WillModifySVGObservable(nsISVGValue* observable,
                                                 nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFEMergeElement> element;
  nsCOMPtr<nsIDOMSVGFilterElement> filter;
  element = do_QueryInterface(GetParent());
  if (GetParent())
    filter = do_QueryInterface(GetParent()->GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(filter);

  if (element && filter && value)
    value->BeginBatchUpdate();
  return nsSVGFEMergeNodeElementBase::WillModifySVGObservable(observable, aModType);
}

NS_IMETHODIMP
nsSVGFEMergeNodeElement::DidModifySVGObservable(nsISVGValue* observable,
                                                nsISVGValue::modificationType aModType)
{
  nsCOMPtr<nsIDOMSVGFEMergeElement> element;
  nsCOMPtr<nsIDOMSVGFilterElement> filter;
  element = do_QueryInterface(GetParent());
  if (GetParent())
    filter = do_QueryInterface(GetParent()->GetParent());
  nsCOMPtr<nsISVGValue> value = do_QueryInterface(filter);

  if (element && filter && value)
    value->EndBatchUpdate();
  return nsSVGFEMergeNodeElementBase::DidModifySVGObservable(observable, aModType);
}

//----------------------------------------------------------------------
// nsIDOMSVGFEMergeNodeElement methods

/* readonly attribute nsIDOMSVGAnimatedString in1; */
NS_IMETHODIMP nsSVGFEMergeNodeElement::GetIn1(nsIDOMSVGAnimatedString * *aIn)
{
  *aIn = mIn1;
  NS_IF_ADDREF(*aIn);
  return NS_OK;
}


//---------------------Offset------------------------

typedef nsSVGFE nsSVGFEOffsetElementBase;

class nsSVGFEOffsetElement : public nsSVGFEOffsetElementBase,
                             public nsIDOMSVGFEOffsetElement,
                             public nsISVGFilter
{
protected:
  friend nsresult NS_NewSVGFEOffsetElement(nsIContent **aResult,
                                           nsINodeInfo *aNodeInfo);
  nsSVGFEOffsetElement(nsINodeInfo* aNodeInfo);
  nsresult Init();

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  // FE Base
  NS_FORWARD_NSIDOMSVGFILTERPRIMITIVESTANDARDATTRIBUTES(nsSVGFEOffsetElementBase::)

  // nsISVGFilter
  NS_IMETHOD Filter(nsSVGFilterInstance *instance);
  NS_IMETHOD GetRequirements(PRUint32 *aRequirements);

  // Offset
  NS_DECL_NSIDOMSVGFEOFFSETELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEOffsetElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEOffsetElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEOffsetElementBase::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;

protected:
  virtual NumberAttributesInfo GetNumberInfo();

  enum { DX, DY };
  nsSVGNumber2 mNumberAttributes[2];
  static NumberInfo sNumberInfo[2];
  nsCOMPtr<nsIDOMSVGAnimatedString> mIn1;
};

nsSVGElement::NumberInfo nsSVGFEOffsetElement::sNumberInfo[2] =
{
  { &nsGkAtoms::dx, 0 },
  { &nsGkAtoms::dy, 0 }
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEOffset)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEOffsetElement,nsSVGFEOffsetElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEOffsetElement,nsSVGFEOffsetElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEOffsetElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFilterPrimitiveStandardAttributes)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEOffsetElement)
  NS_INTERFACE_MAP_ENTRY(nsISVGFilter)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEOffsetElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEOffsetElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEOffsetElement::nsSVGFEOffsetElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEOffsetElementBase(aNodeInfo)
{
}

nsresult
nsSVGFEOffsetElement::Init()
{
  nsresult rv = nsSVGFEOffsetElementBase::Init();
  NS_ENSURE_SUCCESS(rv,rv);

  // DOM property: in1 , #IMPLIED attrib: in
  {
    rv = NS_NewSVGAnimatedString(getter_AddRefs(mIn1));
    NS_ENSURE_SUCCESS(rv,rv);
    rv = AddMappedSVGValue(nsSVGAtoms::in, mIn1);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  return rv;
}

//----------------------------------------------------------------------
// nsIDOMNode methods


NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEOffsetElement)


//----------------------------------------------------------------------
// nsIDOMSVGFEOffsetElement methods

/* readonly attribute nsIDOMSVGAnimatedString in1; */
NS_IMETHODIMP nsSVGFEOffsetElement::GetIn1(nsIDOMSVGAnimatedString * *aIn)
{
  *aIn = mIn1;
  NS_IF_ADDREF(*aIn);
  return NS_OK;
}

/* readonly attribute nsIDOMSVGAnimatedNumber dx; */
NS_IMETHODIMP nsSVGFEOffsetElement::GetDx(nsIDOMSVGAnimatedNumber * *aDx)
{
  return mNumberAttributes[DX].ToDOMAnimatedNumber(aDx, this);
}

/* readonly attribute nsIDOMSVGAnimatedNumber dy; */
NS_IMETHODIMP nsSVGFEOffsetElement::GetDy(nsIDOMSVGAnimatedNumber * *aDy)
{
  return mNumberAttributes[DY].ToDOMAnimatedNumber(aDy, this);
}

NS_IMETHODIMP
nsSVGFEOffsetElement::Filter(nsSVGFilterInstance *instance)
{
  nsresult rv;
  PRUint8 *sourceData, *targetData;
  nsSVGFilterResource fr(instance);

  rv = fr.AcquireSourceImage(mIn1, this, &sourceData);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = fr.AcquireTargetImage(mResult, &targetData);
  NS_ENSURE_SUCCESS(rv, rv);
  nsRect rect = fr.GetRect();

#ifdef DEBUG_tor
  fprintf(stderr, "FILTER OFFSET rect: %d,%d  %dx%d\n",
          rect.x, rect.y, rect.width, rect.height);
#endif

  PRInt32 offsetX, offsetY;
  float fltX, fltY;
  nsSVGLength2 val;

  GetAnimatedNumberValues(&fltX, &fltY, nsnull);
  val.Init(nsSVGUtils::X, 0xff, fltX, nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER);
  offsetX = (PRInt32) instance->GetPrimitiveLength(&val);

  val.Init(nsSVGUtils::Y, 0xff, fltY, nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER);
  offsetY = (PRInt32) instance->GetPrimitiveLength(&val);

  for (PRInt32 y = rect.y; y < rect.y + rect.height; y++) {
    PRInt32 targetRow = y + offsetY;
    if (targetRow < rect.y || targetRow >= rect.y + rect.height)
      continue;

    PRInt32 targetColumn = rect.x + offsetX;
    PRInt32 stride = fr.GetDataStride();
    if (targetColumn < rect.x)
      memcpy(targetData + stride * targetRow + 4 * rect.x,
             sourceData + stride * y - 4 * offsetX,
             4 * (rect.width + offsetX));
    else
      memcpy(targetData + stride * targetRow + 4 * targetColumn,
             sourceData + stride * y + 4 * rect.x,
             4 * (rect.width - offsetX));
  }
  return NS_OK;
}

NS_IMETHODIMP
nsSVGFEOffsetElement::GetRequirements(PRUint32 *aRequirements)
{
  *aRequirements = CheckStandardNames(mIn1);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGElement methods

nsSVGElement::NumberAttributesInfo
nsSVGFEOffsetElement::GetNumberInfo()
{
  return NumberAttributesInfo(mNumberAttributes, sNumberInfo,
                              NS_ARRAY_LENGTH(sNumberInfo));
}


//---------------------UnimplementedMOZ------------------------

typedef nsSVGFE nsSVGFEUnimplementedMOZElementBase;

class nsSVGFEUnimplementedMOZElement : public nsSVGFEUnimplementedMOZElementBase,
                                       public nsIDOMSVGFEUnimplementedMOZElement,
                                       public nsISVGFilter
{
protected:
  friend nsresult NS_NewSVGFEUnimplementedMOZElement(nsIContent **aResult,
                                          nsINodeInfo *aNodeInfo);
  nsSVGFEUnimplementedMOZElement(nsINodeInfo* aNodeInfo);
  virtual ~nsSVGFEUnimplementedMOZElement();

public:
  // interfaces:
  NS_DECL_ISUPPORTS_INHERITED

  // FE Base
  NS_FORWARD_NSIDOMSVGFILTERPRIMITIVESTANDARDATTRIBUTES(nsSVGFEUnimplementedMOZElementBase::)

  // nsISVGFilter
  NS_IMETHOD Filter(nsSVGFilterInstance *instance);
  NS_IMETHOD GetRequirements(PRUint32 *aRequirements);

  // Gaussian
  NS_DECL_NSIDOMSVGFEMERGEELEMENT

  NS_FORWARD_NSIDOMSVGELEMENT(nsSVGFEUnimplementedMOZElementBase::)

  NS_FORWARD_NSIDOMNODE(nsSVGFEUnimplementedMOZElementBase::)
  NS_FORWARD_NSIDOMELEMENT(nsSVGFEUnimplementedMOZElementBase::)

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_IMPL_NS_NEW_SVG_ELEMENT(FEUnimplementedMOZ)

//----------------------------------------------------------------------
// nsISupports methods

NS_IMPL_ADDREF_INHERITED(nsSVGFEUnimplementedMOZElement,nsSVGFEUnimplementedMOZElementBase)
NS_IMPL_RELEASE_INHERITED(nsSVGFEUnimplementedMOZElement,nsSVGFEUnimplementedMOZElementBase)

NS_INTERFACE_MAP_BEGIN(nsSVGFEUnimplementedMOZElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFilterPrimitiveStandardAttributes)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGFEUnimplementedMOZElement)
  NS_INTERFACE_MAP_ENTRY(nsISVGFilter)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGFEUnimplementedMOZElement)
NS_INTERFACE_MAP_END_INHERITING(nsSVGFEUnimplementedMOZElementBase)

//----------------------------------------------------------------------
// Implementation

nsSVGFEUnimplementedMOZElement::nsSVGFEUnimplementedMOZElement(nsINodeInfo *aNodeInfo)
  : nsSVGFEUnimplementedMOZElementBase(aNodeInfo)
{
}

nsSVGFEUnimplementedMOZElement::~nsSVGFEUnimplementedMOZElement()
{
}

//----------------------------------------------------------------------
// nsIDOMNode methods

NS_IMPL_ELEMENT_CLONE_WITH_INIT(nsSVGFEUnimplementedMOZElement)

NS_IMETHODIMP
nsSVGFEUnimplementedMOZElement::Filter(nsSVGFilterInstance *instance)
{
  /* should never be called */
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsSVGFEUnimplementedMOZElement::GetRequirements(PRUint32 *aRequirements)
{
  /* should never be called */
  return NS_ERROR_NOT_IMPLEMENTED;
}
