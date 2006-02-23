/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * Portions created by the Initial Developer are Copyright (C) 2000-2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@netscape.com>
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

#include "gfxIImageFrame.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"

#include "nsIImage.h"

#include "nsPoint.h"
#include "nsSize.h"

#include "nsCOMPtr.h"

#define GFX_IMAGEFRAME_CID \
{ /* aa699204-1dd1-11b2-84a9-a280c268e4fb */         \
     0xaa699204,                                     \
     0x1dd1,                                         \
     0x11b2,                                         \
    {0x84, 0xa9, 0xa2, 0x80, 0xc2, 0x68, 0xe4, 0xfb} \
}

class gfxImageFrame : public gfxIImageFrame,
                      public nsIInterfaceRequestor
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_GFXIIMAGEFRAME
  NS_DECL_NSIINTERFACEREQUESTOR

  gfxImageFrame();
  virtual ~gfxImageFrame();

protected:
  nsIntSize mSize;

private:
  nsresult  SetData(const PRUint8 *aData, PRUint32 aLength, 
                    PRInt32 aOffset, PRBool aSetAlpha);

  nsCOMPtr<nsIImage> mImage;

  PRPackedBool mInitialized;
  PRPackedBool mMutable;
  PRPackedBool mHasBackgroundColor;
  PRPackedBool mTopToBottom;
  gfx_format   mFormat;

  PRInt32 mTimeout; // -1 means display forever
  nsIntPoint mOffset;

  gfx_color mBackgroundColor;

  PRInt32   mDisposalMethod;
};
