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
 * The Original Code is mozilla.org code.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 2001 Netscape Communications Corporation. All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Stuart Parmenter <pavlov@netscape.com>
 */

#include "gfxImageFrame.h"

#include "nsIServiceManager.h"

NS_IMPL_ISUPPORTS2(gfxImageFrame, gfxIImageFrame, nsIInterfaceRequestor)

gfxImageFrame::gfxImageFrame() :
  mTimeout(0),
  mInitalized(PR_FALSE)
{
  NS_INIT_ISUPPORTS();
  /* member initializers and constructor code */
}

gfxImageFrame::~gfxImageFrame()
{
  /* destructor code */
}

/* void init (in nscoord aX, in nscoord aY, in nscoord aWidth, in nscoord aHeight, in gfx_format aFormat); */
NS_IMETHODIMP gfxImageFrame::Init(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight, gfx_format aFormat)
{
  if (aWidth <= 0 || aHeight <= 0) {
    printf("error - negative image size\n");
    return NS_ERROR_FAILURE;
  }

  if (mInitalized)
    return NS_ERROR_FAILURE;

  mInitalized = PR_TRUE;

  mOffset.MoveTo(aX, aY);
  mSize.SizeTo(aWidth, aHeight);

  mFormat = aFormat;

  mImage = do_CreateInstance("@mozilla.org/gfx/image;1");

  gfx_depth depth = 24;
  nsMaskRequirements maskReq;

  switch (aFormat) {
  case gfxIFormats::BGR:
  case gfxIFormats::RGB:
    maskReq = nsMaskRequirements_kNoMask;
    break;

  case gfxIFormats::BGRA:
  case gfxIFormats::RGBA:
    printf("we can't do this with the old image code\n");
    maskReq = nsMaskRequirements_kNeeds8Bit;
    break;

  case gfxIFormats::BGR_A1:
  case gfxIFormats::RGB_A1:
    maskReq = nsMaskRequirements_kNeeds1Bit;
    break;

  case gfxIFormats::BGR_A8:
  case gfxIFormats::RGB_A8:
    maskReq = nsMaskRequirements_kNeeds8Bit;
    break;

  default:
    printf("unsupposed gfx_format\n");
    break;
  }

  mImage->Init(aWidth, aHeight, depth, maskReq);

  return NS_OK;
}

/* readonly attribute nscoord x; */
NS_IMETHODIMP gfxImageFrame::GetX(nscoord *aX)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aX = mOffset.x;
  return NS_OK;
}

/* readonly attribute nscoord y; */
NS_IMETHODIMP gfxImageFrame::GetY(nscoord *aY)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aY = mOffset.y;
  return NS_OK;
}


/* readonly attribute nscoord width; */
NS_IMETHODIMP gfxImageFrame::GetWidth(nscoord *aWidth)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aWidth = mSize.width;
  return NS_OK;
}

/* readonly attribute nscoord height; */
NS_IMETHODIMP gfxImageFrame::GetHeight(nscoord *aHeight)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aHeight = mSize.height;
  return NS_OK;
}

/* readonly attribute nsRect rect; */
NS_IMETHODIMP gfxImageFrame::GetRect(nsRect **aRect)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  nsRect rect(mOffset.x, mOffset.y, mSize.width, mSize.height);
  *aRect = &rect;
  return NS_OK;
}

/* readonly attribute gfx_format format; */
NS_IMETHODIMP gfxImageFrame::GetFormat(gfx_format *aFormat)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aFormat = mFormat;
  return NS_OK;
}

/* attribute long timeout; */
NS_IMETHODIMP gfxImageFrame::GetTimeout(PRInt32 *aTimeout)
{
  *aTimeout = mTimeout;
  return NS_OK;
}

NS_IMETHODIMP gfxImageFrame::SetTimeout(PRInt32 aTimeout)
{
  mTimeout = aTimeout;
  return NS_OK;
}

/* readonly attribute unsigned long imageBytesPerRow; */
NS_IMETHODIMP gfxImageFrame::GetImageBytesPerRow(PRUint32 *aBytesPerRow)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aBytesPerRow = mImage->GetLineStride();
  return NS_OK;
}

/* readonly attribute unsigned long imageDataLength; */
NS_IMETHODIMP gfxImageFrame::GetImageDataLength(PRUint32 *aBitsLength)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aBitsLength = mImage->GetBytesPix();
  return NS_OK;
}

/* void getImageData([array, size_is(length)] out PRUint8 bits, out unsigned long length); */
NS_IMETHODIMP gfxImageFrame::GetImageData(PRUint8 **aData, PRUint32 *length)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  *aData = mImage->GetBits();
  *length = mImage->GetBytesPix() * mSize.height;

  return NS_OK;
}

/* void setImageData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
NS_IMETHODIMP gfxImageFrame::SetImageData(const PRUint8 *aData, PRUint32 aLength, PRInt32 aOffset)
{
  if (!mInitalized)
    return NS_ERROR_NOT_INITIALIZED;

  PRUint8 *imgData = mImage->GetBits();
  PRInt32 imgLen = mImage->GetBytesPix();

  if (((aOffset + (PRInt32)aLength) > imgLen) || !imgData)
    return NS_ERROR_FAILURE;

  memcpy(imgData + aOffset, aData, aLength);

  return NS_OK;
}

/* readonly attribute unsigned long alphaBytesPerRow; */
NS_IMETHODIMP gfxImageFrame::GetAlphaBytesPerRow(PRUint32 *aBytesPerRow)
{
  if (!mInitalized || !mImage->GetHasAlphaMask())
    return NS_ERROR_NOT_INITIALIZED;

  *aBytesPerRow = mImage->GetAlphaLineStride();
  return NS_OK;
}

/* readonly attribute unsigned long alphaDataLength; */
NS_IMETHODIMP gfxImageFrame::GetAlphaDataLength(PRUint32 *aBitsLength)
{
  if (!mInitalized || !mImage->GetHasAlphaMask())
    return NS_ERROR_NOT_INITIALIZED;

  *aBitsLength = mImage->GetAlphaLineStride() * mSize.height;
  return NS_OK;
}

/* void getAlphaData([array, size_is(length)] out PRUint8 bits, out unsigned long length); */
NS_IMETHODIMP gfxImageFrame::GetAlphaData(PRUint8 **aData, PRUint32 *length)
{
  if (!mInitalized || !mImage->GetHasAlphaMask())
    return NS_ERROR_NOT_INITIALIZED;

  *aData = mImage->GetAlphaBits();
  *length = mImage->GetAlphaLineStride() * mSize.height;

  return NS_OK;
}

/* void setAlphaData ([array, size_is (length), const] in PRUint8 data, in unsigned long length, in long offset); */
NS_IMETHODIMP gfxImageFrame::SetAlphaData(const PRUint8 *aData, PRUint32 aLength, PRInt32 aOffset)
{
  if (!mInitalized || !mImage->GetHasAlphaMask())
    return NS_ERROR_NOT_INITIALIZED;

  PRUint8 *alphaData = mImage->GetAlphaBits();
  PRInt32 alphaLen = mImage->GetAlphaLineStride() * mSize.height;

  if (((aOffset + (PRInt32)aLength) > alphaLen) || !alphaData)
    return NS_ERROR_FAILURE;

  memcpy(alphaData + aOffset, aData, aLength);
  return NS_OK;
}


NS_IMETHODIMP gfxImageFrame::GetInterface(const nsIID & aIID, void * *result)
{
  NS_ENSURE_ARG_POINTER(result);

  if (NS_SUCCEEDED(QueryInterface(aIID, result)))
    return NS_OK;
  if (mImage && aIID.Equals(NS_GET_IID(nsIImage)))
    return mImage->QueryInterface(aIID, result);

  return NS_NOINTERFACE;
}
 