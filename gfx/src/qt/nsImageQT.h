/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 *		John C. Griggs <johng@corel.com>
 *
 */

#ifndef nsImageQT_h___
#define nsImageQT_h___

#include "nsIImage.h"

#include <qpixmap.h>
#include <qimage.h>

#undef Bool

class nsImageQT : public nsIImage
{
public:
  nsImageQT();
  virtual ~nsImageQT();

  NS_DECL_ISUPPORTS

  /**
   *  @see nsIImage.h
   */
  virtual PRInt32     GetBytesPix();
  virtual PRBool      GetIsRowOrderTopToBottom();
  virtual PRInt32     GetHeight();
  virtual PRInt32     GetWidth();
  virtual PRUint8     *GetBits();
  virtual void        *GetBitInfo();
  virtual PRInt32     GetLineStride();

  NS_IMETHOD SetNaturalWidth(PRInt32 naturalwidth)
  {
    mNaturalWidth = naturalwidth;
    return NS_OK;
  }
  NS_IMETHOD SetNaturalHeight(PRInt32 naturalheight)
  {
    mNaturalHeight = naturalheight;
    return NS_OK;
  }
  virtual PRInt32     GetNaturalWidth() {return mNaturalWidth;}
  virtual PRInt32     GetNaturalHeight() {return mNaturalHeight;}
 
  NS_IMETHOD          SetDecodedRect(PRInt32 x1,PRInt32 y1,
                                     PRInt32 x2,PRInt32 y2);        
  virtual PRInt32     GetDecodedX1() {return mDecodedX1;}
  virtual PRInt32     GetDecodedY1() {return mDecodedY1;}
  virtual PRInt32     GetDecodedX2() {return mDecodedX2;}
  virtual PRInt32     GetDecodedY2() {return mDecodedY2;}

  virtual nsColorMap *GetColorMap();
  NS_IMETHOD Draw(nsIRenderingContext &aContext,
                  nsDrawingSurface aSurface,
                  PRInt32 aX, PRInt32 aY,
                  PRInt32 aWidth, PRInt32 aHeight);
  NS_IMETHOD Draw(nsIRenderingContext &aContext,
                  nsDrawingSurface aSurface,
                  PRInt32 aSX, PRInt32 aSY, 
                  PRInt32 aSWidth, PRInt32 aSHeight,
                  PRInt32 aDX, PRInt32 aDY, 
                  PRInt32 aDWidth, PRInt32 aDHeight);

#ifdef USE_IMG2
  void CreateOffscreenPixmap(PRInt32 aWidth,PRInt32 aHeight);
  NS_IMETHOD DrawToImage(nsIImage *aDstImage,nscoord aDX,nscoord aDY,
                         nscoord aDWidth,nscoord aDHeight);
#endif

  NS_IMETHOD DrawTile(nsIRenderingContext &aContext,
                      nsDrawingSurface aSurface,
                      nsRect &aSrcRect,
                      nsRect &aTileRect);
 
  NS_IMETHOD DrawTile(nsIRenderingContext &aContext,
                      nsDrawingSurface aSurface,
                      PRInt32 aSXOffset, PRInt32 aSYOffset,
                      const nsRect &aTileRect);

  virtual void ImageUpdated(nsIDeviceContext *aContext,
                            PRUint8 aFlags,nsRect *aUpdateRect);
  virtual nsresult    Init(PRInt32 aWidth,PRInt32 aHeight,
                           PRInt32 aDepth,
                           nsMaskRequirements aMaskRequirements);
  virtual PRBool      IsOptimized();

  virtual nsresult    Optimize(nsIDeviceContext *aContext);
  virtual PRBool      GetHasAlphaMask() {return mAlphaBits != nsnull;} 
  virtual PRUint8     *GetAlphaBits();
  virtual PRInt32     GetAlphaWidth();
  virtual PRInt32     GetAlphaHeight();
  virtual PRInt32     GetAlphaLineStride();
  virtual nsIImage    *DuplicateImage();

  /**
   * Calculate the number of bytes spaned for this image for a given width
   * @param aWidth is the width to calculate the number of bytes for
   * @return the number of bytes in this span
   */
  PRInt32  CalcBytesSpan(PRUint32 aWidth);
  virtual void  SetAlphaLevel(PRInt32 aAlphaLevel);
  virtual PRInt32 GetAlphaLevel();
  virtual void  MoveAlphaMask(PRInt32 aX,PRInt32 aY);

  NS_IMETHOD   LockImagePixels(PRBool aMaskPixels);
  NS_IMETHOD   UnlockImagePixels(PRBool aMaskPixels);    

private:
  /**
   * Calculate the amount of memory needed for the initialization of the 
   * image
   */
  void ComputeMetrics();
  void ComputePaletteSize(PRIntn nBitCount);
  void CreateImagePixmap(PRInt32 aWidth,PRInt32 aHeight);

private:
  PRInt32   mWidth;
  PRInt32   mHeight;
  PRInt32   mDepth;         // actual bits per pixel
  PRInt32   mRequestDepth;  // requested bits per pixel
  PRInt32   mRowBytes;
  PRUint8   *mImageBits;
  PRInt32   mSizeImage;
  PRBool    mIsTopToBottom;

  PRInt8    mNumBytesPixel;

  PRInt32   mNaturalWidth;
  PRInt32   mNaturalHeight;

  PRInt32   mDecodedX1;     //Keeps track of what part of image
  PRInt32   mDecodedY1;     // has been decoded.
  PRInt32   mDecodedX2; 
  PRInt32   mDecodedY2;    

  // alpha layer members
  PRUint8   *mAlphaBits;
  QImage    *mAlphaPixmap;
  PRInt8    mAlphaDepth;    // alpha layer depth
  PRInt16   mAlphaRowBytes; // alpha bytes per row
  PRInt16   mAlphaWidth;    // alpha layer width
  PRInt16   mAlphaHeight;   // alpha layer height
  QImage    *mImagePixmap;

  PRUint32  mID;
};

#endif
