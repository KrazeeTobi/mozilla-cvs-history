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

#ifndef nsDeviceContextWin_h___
#define nsDeviceContextWin_h___

#include "nsDeviceContext.h"
#include "nsIScreenManager.h"
#include <windows.h>

class nsIScreen;


class nsDeviceContextWin : public DeviceContextImpl
{
public:
  nsDeviceContextWin();

  NS_IMETHOD  Init(nsNativeWidget aWidget);

  NS_IMETHOD  CreateRenderingContext(nsIRenderingContext *&aContext);

  NS_IMETHOD  SupportsNativeWidgets(PRBool &aSupportsWidgets);

  NS_IMETHOD  GetCanonicalPixelScale(float &aScale) const;

  NS_IMETHOD  GetScrollBarDimensions(float &aWidth, float &aHeight) const;
  NS_IMETHOD  GetSystemAttribute(nsSystemAttrID anID, SystemAttrStruct * aInfo) const;

  //get a low level drawing surface for rendering. the rendering context
  //that is passed in is used to create the drawing surface if there isn't
  //already one in the device context. the drawing surface is then cached
  //in the device context for re-use.
  NS_IMETHOD  GetDrawingSurface(nsIRenderingContext &aContext, nsDrawingSurface &aSurface);

  NS_IMETHOD  CheckFontExistence(const nsString& aFontName);

  NS_IMETHOD  GetDepth(PRUint32& aDepth);

  NS_IMETHOD  GetILColorSpace(IL_ColorSpace*& aColorSpace);

  NS_IMETHOD  GetPaletteInfo(nsPaletteInfo&);

  NS_IMETHOD ConvertPixel(nscolor aColor, PRUint32 & aPixel);

  NS_IMETHOD GetDeviceSurfaceDimensions(PRInt32 &aWidth, PRInt32 &aHeight);
  NS_IMETHOD GetRect(nsRect &aRect);
  NS_IMETHOD GetClientRect(nsRect &aRect);

  NS_IMETHOD GetDeviceContextFor(nsIDeviceContextSpec *aDevice,
                                 nsIDeviceContext *&aContext);

  NS_IMETHOD BeginDocument(void);
  NS_IMETHOD EndDocument(void);

  NS_IMETHOD BeginPage(void);
  NS_IMETHOD EndPage(void);

protected:
  virtual ~nsDeviceContextWin();
  void CommonInit(HDC aDC);
  nsresult Init(nsNativeDeviceContext aContext, nsIDeviceContext *aOrigContext);
  void FindScreen ( nsIScreen** outScreen ) ;
  void ComputeClientRectUsingScreen ( nsRect* outRect ) ;
  void ComputeFullAreaUsingScreen ( nsRect* outRect ) ;

  PRBool mCachedClientRect;
  PRBool mCachedFullRect;

  nsDrawingSurface      mSurface;
  PRUint32              mDepth;  // bit depth of device
  nsPaletteInfo         mPaletteInfo;
  float                 mPixelScale;
  PRInt32               mWidth;
  PRInt32               mHeight;
  nsRect                mClientRect;
  nsIDeviceContextSpec  *mSpec;

  nsCOMPtr<nsIScreenManager> mScreenManager;
  static PRUint32 sNumberOfScreens;

public:
  HDC                   mDC;

  static PRBool gRound;
  static int    PrefChanged(const char* aPref, void* aClosure);
};

#endif /* nsDeviceContextWin_h___ */
