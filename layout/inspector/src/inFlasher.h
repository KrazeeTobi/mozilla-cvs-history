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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (original author)
 *
 *
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

#ifndef __inFlasher_h__
#define __inFlasher_h__

#include "inIFlasher.h"

#include "nsIInspectorCSSUtils.h"
#include "nsIDOMElement.h"
#include "nsIPresShell.h"
#include "nsIFrame.h"
#include "nsIRenderingContext.h"

#define BOUND_INNER 0
#define BOUND_OUTER 1

#define DIR_VERTICAL 0
#define DIR_HORIZONTAL 1 

class inFlasher : public inIFlasher
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_INIFLASHER

  inFlasher();
  virtual ~inFlasher();

protected:
  nsIFrame* GetFrameFor(nsIDOMElement* aElement, nsIPresShell* aShell);
  nsIPresShell* GetPresShellFor(nsISupports* aThing);

  NS_IMETHOD DrawOutline(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight, nscolor aColor, 
                           PRUint32 aThickness, float aP2T, nsIRenderingContext* aRenderContext);
  NS_IMETHOD DrawLine(nscoord aX, nscoord aY, nscoord aLength, PRUint32 aThickness, 
                        PRBool aDir, PRBool aBounds, float aP2T, nsIRenderingContext* aRenderContext);

  nsCOMPtr<nsIInspectorCSSUtils> mCSSUtils;
};

#endif // __inFlasher_h__
