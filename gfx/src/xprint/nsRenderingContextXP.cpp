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
 * Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *
 */

#include <math.h>
#include <strings.h>
#include "nsRenderingContextXP.h"
#include "nsFontMetricsXP.h"
#include "libimg.h"
#include "nsDeviceContextXP.h"
#include "nsXPrintContext.h"
#include "nsICharRepresentable.h"
#include "nsIRegion.h"      
#include "prmem.h"


static NS_DEFINE_IID(kIRenderingContextIID, NS_IRENDERING_CONTEXT_IID);

#ifdef PR_LOGGING 
static PRLogModuleInfo *RenderingContextXpLM = PR_NewLogModule("nsRenderingContextXp");
#endif /* PR_LOGGING */ 


// Macro to convert from TWIPS (1440 per inch) to POINTS (72 per inch)

#define FLAG_CLIP_VALID       0x0001
#define FLAG_CLIP_CHANGED     0x0002
#define FLAG_LOCAL_CLIP_VALID 0x0004

#define FLAGS_ALL             (FLAG_CLIP_VALID | FLAG_CLIP_CHANGED | FLAG_LOCAL_CLIP_VALID)

/** ---------------------------------------------------
 * Do we really need to maintain the state here
 */
class XP_State
{
public:
  XP_State();
  XP_State(XP_State &aState);
  ~XP_State();

  XP_State        *mNext;
  nsTransform2D   mMatrix;
  nsRect          mLocalClip;
  nsIFontMetrics  *mFontMetrics;
  nscolor         mCurrentColor;
  nscolor         mTextColor;
  nsLineStyle     mLineStyle;
  PRInt32         mFlags;
};

/** ---------------------------------------------------
 *  Default Constructor for the state
 */
XP_State :: XP_State()
{
  mNext = nsnull;
  mMatrix.SetToIdentity();  
  // mMatrix.SetToScale(1.0, 1.0);  
  mLocalClip.x = mLocalClip.y = mLocalClip.width = mLocalClip.height = 0;
  mFontMetrics = nsnull;
  mCurrentColor = NS_RGB(0, 0, 0); /* X11 intial bg color is always _black_...
                                    * ...but we should query that from 
                                    * Xserver instead of guessing that...
                                    */
  mTextColor = NS_RGB(0, 0, 0);
  mLineStyle = nsLineStyle_kSolid;
}

/** ---------------------------------------------------
 *  Constructor of a state using the passed in state to initialize to
 *  Default Constructor for the state
 */
XP_State :: XP_State(XP_State &aState):mMatrix(&aState.mMatrix),mLocalClip(aState.mLocalClip)
{
  mNext = &aState;
  // mClipRegion = NULL;
  mCurrentColor = aState.mCurrentColor;
  mFontMetrics = nsnull;
  //mFont = NULL;
  mFlags = ~FLAGS_ALL;
  mTextColor = aState.mTextColor;
  mLineStyle = aState.mLineStyle;
}

/** ---------------------------------------------------
 *  Destructor for a state
 */
XP_State :: ~XP_State()
{
  //if (NULL != mClipRegion){
    //VERIFY(::DeleteObject(mClipRegion));
    //mClipRegion = NULL;
  //}

  //don't delete this because it lives in the font metrics
  //mFont = NULL;
}

// This is not being used
static NS_DEFINE_IID(kRenderingContextIID, NS_IRENDERING_CONTEXT_IID);


/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
nsRenderingContextXp :: nsRenderingContextXp()
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::nsRenderingContextXp()\n"));

  NS_INIT_REFCNT();

  mContext = nsnull;
  mFontMetrics = nsnull;

  mStateCache = new nsVoidArray();
  mPrintContext = nsnull;
  mCurrentColor = NS_RGB(0, 0, 0); /* X11 intial bg color is always _black_...
                                    * ...but we should query that from 
                                    * Xserver instead of guessing that...
                                    */
  PushState();
  mP2T = 1.0f;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
nsRenderingContextXp :: ~nsRenderingContextXp()
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::~nsRenderingContextXp()\n"));

  if (nsnull != mStateCache){
    PRInt32 cnt = mStateCache->Count();

    while (--cnt >= 0){
      XP_State *state = (XP_State *)mStateCache->ElementAt(cnt);
      mStateCache->RemoveElementAt(cnt);

      if (nsnull != state)
        delete state;
    }

    delete mStateCache;
    mStateCache = nsnull;
  }

  mTranMatrix = nsnull;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
nsresult
nsRenderingContextXp :: QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  if (nsnull == aInstancePtr)
    return NS_ERROR_NULL_POINTER;

  if (aIID.Equals(kIRenderingContextIID)){
    nsIRenderingContext* tmp = this;
    *aInstancePtr = (void*) tmp;
    NS_ADDREF_THIS();
    return NS_OK;
  }

  static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

  if (aIID.Equals(kISupportsIID)){
    nsIRenderingContext* tmp = this;
    nsISupports* tmp2 = tmp;
    *aInstancePtr = (void*) tmp2;
    NS_ADDREF_THIS();
    return NS_OK;
  }

  return NS_NOINTERFACE;
}

NS_IMPL_ADDREF(nsRenderingContextXp)
NS_IMPL_RELEASE(nsRenderingContextXp)

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP
nsRenderingContextXp :: Init(nsIDeviceContext* aContext)
{
  mContext = do_QueryInterface(aContext);
  if (mContext) {
     mContext->GetPrintContext(mPrintContext);
  }
  NS_ASSERTION(nsnull != mContext,"Device context is null.");


  int x, y;
  unsigned int width, height, border, depth;
  Window root_win;

  
  XGetGeometry(mPrintContext->GetDisplay(),
               mPrintContext->GetDrawable(),
               &root_win,
               &x,
               &y,
               &width,
               &height,
               &border,
               &depth);
  mClipRegion = new nsRegionXlib();
  mClipRegion->Init();
  mClipRegion->SetTo(0, 0, width, height);

  mContext->GetDevUnitsToAppUnits(mP2T);
  float app2dev;
  mContext->GetAppUnitsToDevUnits(app2dev);
  mTranMatrix->AddScale(app2dev, app2dev);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: LockDrawingSurface(PRInt32 aX, PRInt32 aY,
                                             PRUint32 aWidth, PRUint32 aHeight,
                                             void **aBits, PRInt32 *aStride,
                                             PRInt32 *aWidthBytes, 
                                                PRUint32 aFlags)
{
  PushState();
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: UnlockDrawingSurface(void)
{
  PRBool clipstate;
  PopState(clipstate);
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP
nsRenderingContextXp :: SelectOffScreenDrawingSurface(nsDrawingSurface aSurface)
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::SelectOffScreenDrawingSurface()\n"));

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP
nsRenderingContextXp :: GetDrawingSurface(nsDrawingSurface *aSurface)
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::GetDrawingSurface()\n"));
  *aSurface = nsnull;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP
nsRenderingContextXp :: GetHints(PRUint32& aResult)
{
  PRUint32 result = 0;
  // Most X servers implement 8 bit text rendering alot faster than
  // XChar2b rendering. In addition, we can avoid the PRUnichar to
  // XChar2b conversion. So we set this bit...
  result |= NS_RENDERING_HINT_FAST_8BIT_TEXT;
  aResult = result;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: Reset()
{
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetDeviceContext(nsIDeviceContext *&aContext)
{
  aContext = mContext;
  NS_IF_ADDREF(aContext);
  return NS_OK;
}


/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: PushState(void)
{
  PRInt32 cnt = mStateCache->Count();

  if (cnt == 0){
    if (nsnull == mStates)
      mStates = new XP_State();
    else
      mStates = new XP_State(*mStates);
  } else {
    XP_State *state = (XP_State *)mStateCache->ElementAt(cnt - 1);
    mStateCache->RemoveElementAt(cnt - 1);

    state->mNext = mStates;

    //clone state info
    state->mMatrix = mStates->mMatrix;
    state->mLocalClip = mStates->mLocalClip;
    state->mCurrentColor = mStates->mCurrentColor;
    state->mFontMetrics = mStates->mFontMetrics;
    state->mTextColor = mStates->mTextColor;
    state->mLineStyle = mStates->mLineStyle;

    mStates = state;
  }

  mTranMatrix = &mStates->mMatrix;

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: PopState(PRBool &aClipEmpty)
{
  PRBool  retval = PR_FALSE;

  if (nsnull == mStates){
    NS_ASSERTION(!(nsnull == mStates), "state underflow");
  } else {
    XP_State *oldstate = mStates;

    mStates = mStates->mNext;

    mStateCache->AppendElement(oldstate);

    if (nsnull != mStates){
      mTranMatrix = &mStates->mMatrix;
      SetLineStyle(mStates->mLineStyle);
    }
    else
      mTranMatrix = nsnull;
  }

  aClipEmpty = retval;

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: IsVisibleRect(const nsRect& aRect, PRBool &aVisible)
{
  aVisible = PR_TRUE;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: SetClipRect(const nsRect& aRect, nsClipCombine aCombine, PRBool &aClipEmpty)
{
nsRect  trect = aRect;
#ifdef XP_PC
PRInt32     cliptype;
#endif
  Region rgn;
  mStates->mLocalClip = aRect;

  mTranMatrix->TransformCoord(&trect.x, &trect.y,&trect.width, &trect.height);
  mStates->mFlags |= FLAG_LOCAL_CLIP_VALID;

  switch(aCombine) {
   case nsClipCombine_kIntersect:
        mClipRegion->Intersect(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kUnion:
    mClipRegion->Union(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kSubtract:
    mClipRegion->Subtract(trect.x,trect.y,trect.width,trect.height);
    break;
  case nsClipCombine_kReplace:
    mClipRegion->SetTo(trect.x,trect.y,trect.width,trect.height);
    break;
  }
  aClipEmpty = mClipRegion->IsEmpty();

  mClipRegion->GetNativeRegion((void*&)rgn);
  XSetRegion(mPrintContext->GetDisplay(), mPrintContext->GetGC(), rgn);

#ifdef XP_PC
  if (cliptype == NULLREGION)
    aClipEmpty = PR_TRUE;
  else
    aClipEmpty = PR_FALSE;
#endif

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetClipRect(nsRect &aRect, PRBool &aClipValid)
{
  PRInt32 x, y, w, h;
  if (!mClipRegion->IsEmpty()) {
    mClipRegion->GetBoundingBox(&x,&y,&w,&h);
    aRect.SetRect(x,y,w,h);
    aClipValid = PR_TRUE;
  } else {
    aRect.SetRect(0,0,0,0);
    aClipValid = PR_FALSE;
  }

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: SetClipRegion(const nsIRegion& aRegion, nsClipCombine aCombine, PRBool &aClipEmpty)
{
  nsRect rect;
  nsIRegion* pRegion = (nsIRegion*)&aRegion;
  pRegion->GetBoundingBox(&rect.x, &rect.y, &rect.width, &rect.height);
  SetClipRect(rect, aCombine, aClipEmpty);

  return NS_OK; 
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: CopyClipRegion(nsIRegion &aRegion)
{
  aRegion.SetTo(*NS_STATIC_CAST(nsIRegion*, mClipRegion));
  return NS_OK; 
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetClipRegion(nsIRegion **aRegion)
{
  NS_ASSERTION(!(nsnull == aRegion), "no region ptr");

  if (*aRegion) {
    nsIRegion *nRegion = (nsIRegion *)mClipRegion;
    (*aRegion)->SetTo(*nRegion);
  }
  return NS_OK; 
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: SetColor(nscolor aColor)
{
  if (nsnull == mContext)
    return NS_ERROR_FAILURE;

  if (mCurrentColor != aColor) {
      mPrintContext->SetForegroundColor(aColor);
      mCurrentColor = aColor;
  }
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: GetColor(nscolor &aColor) const
{
  aColor = mCurrentColor;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: SetLineStyle(nsLineStyle aLineStyle)
{
  if (aLineStyle != mCurrLineStyle) {
    switch(aLineStyle)
      {
        case nsLineStyle_kSolid:
           XSetLineAttributes(mPrintContext->GetDisplay(), mPrintContext->GetGC(),
                           1, // width
                           LineSolid, // line style
                           CapNotLast,// cap style
                           JoinMiter);// join style
        break;
      case nsLineStyle_kDashed:
        {
          static char dashed[2] = {4,4};
          XSetDashes(mPrintContext->GetDisplay(), mPrintContext->GetGC(),
                     0, dashed, 2);
        }
        break;

      case nsLineStyle_kDotted:
        {
          static char dotted[2] = {3,1};
          XSetDashes(mPrintContext->GetDisplay(), mPrintContext->GetGC(),
                     0, dotted, 2);
        }
        break;

    default:
        break;

    }

    mCurrLineStyle = aLineStyle ;
  }
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetLineStyle(nsLineStyle &aLineStyle)
{
  aLineStyle = mCurrLineStyle;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXp::SetFont(const nsFont& aFont)
{
  //PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::SetFont(nsFont)\n"));
  NS_IF_RELEASE(mFontMetrics);
  mContext->GetMetricsFor(aFont, mFontMetrics);
  return SetFont(mFontMetrics);
}

NS_IMETHODIMP
nsRenderingContextXp::SetFont(nsIFontMetrics *aFontMetrics)
{
  //PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::SetFont(nsIFontMetrics)\n"));
  NS_IF_RELEASE(mFontMetrics);
  mFontMetrics = aFontMetrics;
  NS_IF_ADDREF(mFontMetrics);

  if (mFontMetrics)
  {
    nsFontHandle  fontHandle;
    mFontMetrics->GetFontHandle(fontHandle);
    mCurrentFont = (XFontStruct *)fontHandle;
    XSetFont(mPrintContext->GetDisplay(), mPrintContext->GetGC(), mCurrentFont->fid);
  }
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetFontMetrics(nsIFontMetrics *&aFontMetrics)
{

  NS_IF_ADDREF(mFontMetrics);
  aFontMetrics = mFontMetrics;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: Translate(nscoord aX, nscoord aY)
{
  mTranMatrix->AddTranslation((float)aX,(float)aY);
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: Scale(float aSx, float aSy)
{
  mTranMatrix->AddScale(aSx, aSy);
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetCurrentTransform(nsTransform2D *&aTransform)
{
  aTransform = mTranMatrix;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: CreateDrawingSurface(nsRect *aBounds, PRUint32 aSurfFlags, nsDrawingSurface &aSurface)
{
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DestroyDrawingSurface(nsDrawingSurface aDS)
{
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawLine(nscoord aX0, nscoord aY0, nscoord aX1, nscoord aY1)
{
  if (nsnull == mTranMatrix )
    return NS_ERROR_FAILURE;

  mTranMatrix->TransformCoord(&aX0,&aY0);
  mTranMatrix->TransformCoord(&aX1,&aY1);

  
  ::XDrawLine(mPrintContext->GetDisplay(), mPrintContext->GetDrawable(),
              mPrintContext->GetGC(), aX0, aY0, aX1, aY1);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawPolyline(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  if (nsnull == mTranMatrix) {
    return NS_ERROR_FAILURE;
  }

  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;

  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);

  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    thispoint->x = aPoints[i].x;
    thispoint->y = aPoints[i].y;
    mTranMatrix->TransformCoord((PRInt32*)&thispoint->x,(PRInt32*)&thispoint->y);
  }

  ::XDrawLines(mPrintContext->GetDisplay(),
               mPrintContext->GetDrawable(),
               mPrintContext->GetGC(),
               xpoints, aNumPoints, CoordModeOrigin);

  PR_Free((void *)xpoints);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawRect(const nsRect& aRect)
{

  return DrawRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }

  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);
  
  // Don't draw empty rectangles; also, w/h are adjusted down by one
  // so that the right number of pixels are drawn.
  if (w && h)
  {
    ::XDrawRectangle(mPrintContext->GetDisplay(),
                     mPrintContext->GetDrawable(),
                     mPrintContext->GetGC(),
                     x,
                     y,
                     w - 1,
                     h - 1);
  }

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: FillRect(const nsRect& aRect)
{
  return FillRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: FillRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;
  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);
  // Hack for background page
  if ((x == 0) && (y == 0)) {
     return NS_OK;
  }
  ::XFillRectangle(mPrintContext->GetDisplay(),
                   mPrintContext->GetDrawable(),
                   mPrintContext->GetGC(),
                   x,y,w,h);

  return NS_OK;
}

NS_IMETHODIMP 
nsRenderingContextXp :: InvertRect(const nsRect& aRect)
{
  return InvertRect(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP 
nsRenderingContextXp :: InvertRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsnull == mTranMatrix )
    return NS_ERROR_FAILURE;

  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  // Set XOR drawing mode
  ::XSetFunction(mPrintContext->GetDisplay(),
                 mPrintContext->GetGC(),
                 GXxor);

  ::XFillRectangle(mPrintContext->GetDisplay(),
                   mPrintContext->GetDrawable(), 
                   mPrintContext->GetGC(),
                   x,
                   y,
                   w,
                   h);

  // Back to normal copy drawing mode
  ::XSetFunction(mPrintContext->GetDisplay(),
                 mPrintContext->GetGC(),
                 GXcopy);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawPolygon(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;

  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);

  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    thispoint->x = aPoints[i].x;
    thispoint->y = aPoints[i].y;
    mTranMatrix->TransformCoord((PRInt32*)&thispoint->x,(PRInt32*)&thispoint->y);
  }

  ::XDrawLines(mPrintContext->GetDisplay(),
               mPrintContext->GetDrawable(),
               mPrintContext->GetGC(),
               xpoints, aNumPoints, CoordModeOrigin);

  PR_Free((void *)xpoints);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: FillPolygon(const nsPoint aPoints[], PRInt32 aNumPoints)
{
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  PRInt32 i ;
  XPoint * xpoints;
  XPoint * thispoint;
  nscoord x,y;

  xpoints = (XPoint *) PR_Malloc(sizeof(XPoint) * aNumPoints);

  for (i = 0; i < aNumPoints; i++){
    thispoint = (xpoints+i);
    x = aPoints[i].x;
    y = aPoints[i].y;
    mTranMatrix->TransformCoord(&x,&y);
    thispoint->x = x;
    thispoint->y = y;
  }

  ::XFillPolygon(mPrintContext->GetDisplay(),
                 mPrintContext->GetDrawable(),
                 mPrintContext->GetGC(),
                 xpoints, aNumPoints, Convex, CoordModeOrigin);

  PR_Free((void *)xpoints);

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawEllipse(const nsRect& aRect)
{
  return DrawEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsLineStyle_kNone == mCurrLineStyle)
    return NS_OK;

  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  ::XDrawArc(mPrintContext->GetDisplay(),
             mPrintContext->GetDrawable(),
             mPrintContext->GetGC(),
             x,y,w,h, 0, 360 * 64);

  return NS_OK;
}

NS_IMETHODIMP 
nsRenderingContextXp :: FillEllipse(const nsRect& aRect)
{
  return FillEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: FillEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  ::XFillArc(mPrintContext->GetDisplay(),
             mPrintContext->GetDrawable(),
             mPrintContext->GetGC(),
             x,y,w,h, 0, 360 * 64);
  
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXp::DrawTile(nsIImage *aImage, nscoord aX0, nscoord aY0,
                                        nscoord aX1, nscoord aY1,
                                        nscoord aWidth, nscoord aHeight)
{
  NS_NOTREACHED("nsRenderingContextXp::DrawTile() not yet implemented");
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawArc(const nsRect& aRect,
                                 float aStartAngle, float aEndAngle)
{
  return DrawArc(aRect.x,aRect.y,aRect.width,aRect.height,aStartAngle,aEndAngle);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawArc(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight,
                                 float aStartAngle, float aEndAngle)
{
  if (nsLineStyle_kNone == mCurrLineStyle)
    return NS_OK;

  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  ::XDrawArc(mPrintContext->GetDisplay(),
             mPrintContext->GetDrawable(),
             mPrintContext->GetGC(),
             x,y,w,h, NSToIntRound(aStartAngle * 64.0f),
             NSToIntRound(aEndAngle * 64.0f));

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: FillArc(const nsRect& aRect,
                                 float aStartAngle, float aEndAngle)
{
  return FillArc(aRect.x, aRect.y, aRect.width, aRect.height, aStartAngle, aEndAngle);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: FillArc(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight,
                                 float aStartAngle, float aEndAngle)
{
  if (nsLineStyle_kNone == mCurrLineStyle)
    return NS_OK;
  if (nsnull == mTranMatrix ) {
    return NS_ERROR_FAILURE;
  }
  nscoord x,y,w,h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x,&y,&w,&h);

  ::XFillArc(mPrintContext->GetDisplay(),
             mPrintContext->GetDrawable(),
             mPrintContext->GetGC(),
             x,y,w,h, NSToIntRound(aStartAngle * 64.0f),
             NSToIntRound(aEndAngle * 64.0f));

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(char ch, nscoord& aWidth)
{
  // Check for common case of getting the width of single space
  if ((ch == ' ') && (nsnull != mFontMetrics)) {
    nsFontMetricsXp* fontMetricsXP = (nsFontMetricsXp*)mFontMetrics;
    return fontMetricsXP->GetSpaceWidth(aWidth);
  }
  char buf[1];
  buf[0] = ch;
  return GetWidth(buf, 1, aWidth);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(PRUnichar ch, nscoord &aWidth, PRInt32 *aFontID)
{
  PRUnichar buf[1];
  buf[0] = ch;
  return GetWidth(buf, 1, aWidth, aFontID);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(const char* aString, nscoord& aWidth)
{
  return GetWidth(aString, strlen(aString),aWidth);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(const char* aString, PRUint32 aLength,
                                 nscoord& aWidth)
{

  if (0 == aLength) {
    aWidth = 0;
  } else {
    int rawWidth;
    if ((mCurrentFont->min_byte1 == 0) && (mCurrentFont->max_byte1 == 0))
      rawWidth = XTextWidth(mCurrentFont, aString, aLength);
    else
      rawWidth = XTextWidth16(mCurrentFont, (XChar2b *)aString, aLength/2);
    aWidth = NSToCoordRound(rawWidth * mP2T);
  }
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(const nsString& aString, nscoord& aWidth, PRInt32 *aFontID)
{
  return GetWidth(aString.GetUnicode(), aString.Length(), aWidth, aFontID);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: GetWidth(const PRUnichar *aString,PRUint32 aLength,nscoord &aWidth, PRInt32 *aFontID)
{
  if (0 == aLength) {
    aWidth = 0;
  } else {
    nsFontMetricsXp* metrics = (nsFontMetricsXp*) mFontMetrics;
    nsFontXp *prevFont = nsnull;
    int rawWidth = 0;
    PRUint32 start = 0;
    PRUint32 i;
    for (i = 0; i < aLength; i++) {
      PRUnichar c = aString[i];
      nsFontXp* currFont = nsnull;
      nsFontXp** font = metrics->mLoadedFonts;
      nsFontXp** end = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
      while (font < end) {
        if (IS_REPRESENTABLE((*font)->mMap, c)) {
          currFont = *font;
          goto FoundFont; // for speed -- avoid "if" statement
        }
        font++;
      }
      currFont = metrics->FindFont(c);
FoundFont:
      // XXX avoid this test by duplicating code -- erik
      if (prevFont) {
        if (currFont != prevFont) {
          rawWidth += prevFont->GetWidth(&aString[start],
                                                  i - start);
          prevFont = currFont;
          start = i;
        }
      }
      else {
        prevFont = currFont;
        start = i;
      }
    }

    if (prevFont) {
      rawWidth += prevFont->GetWidth(&aString[start],
                                              i - start);
    }

    aWidth = NSToCoordRound(rawWidth * mP2T);
  }
  if (nsnull != aFontID)
    *aFontID = 0;
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawString(const char *aString, PRUint32 aLength,
                        nscoord aX, nscoord aY,
                        const nscoord* aSpacing)
{
  if (0 == aLength)
    return NS_OK;
  if (mTranMatrix == nsnull)
      return NS_ERROR_FAILURE;
  if (aString == nsnull)
      return NS_ERROR_FAILURE;

  nscoord x = aX;
  nscoord y = aY;

  // Substract xFontStruct ascent since drawing specifies baseline
  if (mFontMetrics) {
      mFontMetrics->GetMaxAscent(y);
      y += aY;
  }

  if (nsnull != aSpacing) {
     // Render the string, one character at a time...
     const char* end = aString + aLength;
     while (aString < end) {
        char ch = *aString++;
        nscoord xx = x;
        nscoord yy = y;
        mTranMatrix->TransformCoord(&xx, &yy);
        XDrawString(mPrintContext->GetDisplay(),
                    mPrintContext->GetDrawable(),
                    mPrintContext->GetGC(),
                    xx, yy, (char*)&ch, 1);
        x += *aSpacing++;
     }
  } else {
    mTranMatrix->TransformCoord(&x, &y);
    if ((mCurrentFont->min_byte1 == 0) && (mCurrentFont->max_byte1 == 0))
      XDrawString(mPrintContext->GetDisplay(),
                  mPrintContext->GetDrawable(),
                  mPrintContext->GetGC(),
                  x, y, aString, aLength);
    else
      XDrawString16(mPrintContext->GetDisplay(),
                    mPrintContext->GetDrawable(),
                    mPrintContext->GetGC(),
                    x, y, (XChar2b*)aString, aLength/2);
  }
#if 0
  //this doesn't need to happen here anymore, but a
  //new api will come along that will need this stuff. MMP
  if (nsnull != mFontMetrics){
    nsFont *font;
    mFontMetrics->GetFont(font);
    PRUint8 decorations = font->decorations;

    if (decorations & NS_FONT_DECORATION_OVERLINE){
      nscoord offset;
      nscoord size;
      mFontMetrics->GetUnderline(offset, size);
      FillRect(aX, aY, aWidth, size);
    }
  }

#endif
  // XFlush(mPrintContext->GetDisplay());
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawString(const PRUnichar *aString, PRUint32 aLength,
                                    nscoord aX, nscoord aY, PRInt32 aFontID,
                                    const nscoord* aSpacing)
{
  if (0 == aLength)
    return NS_OK;
  if (nsnull == mFontMetrics)
      return NS_ERROR_FAILURE;
  if (mTranMatrix == nsnull)
      return NS_ERROR_FAILURE;
  if (aString == nsnull)
      return NS_ERROR_FAILURE;

    nscoord x = aX;
    nscoord y;

    // Substract xFontStruct ascent since drawing specifies baseline
    mFontMetrics->GetMaxAscent(y);
    y += aY;
    aY = y;

    mTranMatrix->TransformCoord(&x, &y);

    nsFontMetricsXp* metrics = (nsFontMetricsXp*) mFontMetrics;
    nsFontXp* prevFont = nsnull;
    PRUint32 start = 0;
    PRUint32 i;
    for (i = 0; i < aLength; i++) {
      PRUnichar c = aString[i];
      nsFontXp* currFont = nsnull;
      nsFontXp** font = metrics->mLoadedFonts;
      nsFontXp** lastFont = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
      while (font < lastFont) {
        if (IS_REPRESENTABLE((*font)->mMap, c)) {
          currFont = *font;
          goto FoundFont; // for speed -- avoid "if" statement
        }
        font++;
      }
      currFont = metrics->FindFont(c);
FoundFont:
      // XXX avoid this test by duplicating code -- erik
      if (prevFont) {
        if (currFont != prevFont) {
          if (aSpacing) {
            const PRUnichar* str = &aString[start];
            const PRUnichar* end = &aString[i];
            while (str < end) {
              x = aX;
              y = aY;
              mTranMatrix->TransformCoord(&x, &y);
              prevFont->DrawString(mPrintContext, x, y, str, 1);
              aX += *aSpacing++;
              str++;
            }
          }
          else {
            x += prevFont->DrawString(mPrintContext, x, y, 
                                      &aString[start], i - start);
          }
          prevFont = currFont;
          start = i;
        }
      }
      else {
        prevFont = currFont;
        start = i;
      }
    }

    if (prevFont) {
      if (aSpacing) {
        const PRUnichar* str = &aString[start];
        const PRUnichar* end = &aString[i];
        while (str < end) {
          x = aX;
          y = aY;
          mTranMatrix->TransformCoord(&x, &y);
          prevFont->DrawString(mPrintContext, x, y, str, 1);
          aX += *aSpacing++;
          str++;
        }
      }
      else {
        // mTranMatrix->TransformCoord(&x, &y);
        prevFont->DrawString(mPrintContext, x, y, &aString[start],
          i - start);
      }
    }
  // XFlush(mPrintContext->GetDisplay());
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawString(const nsString& aString,nscoord aX, nscoord aY, PRInt32 aFontID,
                                    const nscoord* aSpacing)
{
  return DrawString(aString.GetUnicode(), aString.Length(), aX, aY, aFontID, aSpacing);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawImage(nsIImage *aImage, nscoord aX, nscoord aY)
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::DrawImage(%d/%d)\n", (int)aX, (int)aY));

  nscoord width, height;

  // we have to do this here because we are doing a transform below
  width = NSToCoordRound(mP2T * aImage->GetWidth());
  height = NSToCoordRound(mP2T * aImage->GetHeight());

  return DrawImage(aImage, aX, aY, width, height);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawImage(nsIImage *aImage, nscoord aX, nscoord aY,
                                        nscoord aWidth, nscoord aHeight) 
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::DrawImage(%d/%d/%d/%d)\n", 
         (int)aX, (int)aY, (int)aWidth, (int)aHeight));

  nscoord x, y, w, h;

  x = aX;
  y = aY;
  w = aWidth;
  h = aHeight;

  mTranMatrix->TransformCoord(&x, &y, &w, &h);

  return mPrintContext->DrawImage(aImage, x, y, w, h);
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawImage(nsIImage *aImage, const nsRect& aSRect, const nsRect& aDRect)
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::DrawImage(aSRect,aDRect)\n"));
  nsRect sr,dr;

  sr = aSRect;
  mTranMatrix->TransformCoord(&sr.x, &sr.y, &sr.width, &sr.height);
  sr.x = aSRect.x;
  sr.y = aSRect.y;
  mTranMatrix->TransformNoXLateCoord(&sr.x, &sr.y);

  dr = aDRect;
  mTranMatrix->TransformCoord(&dr.x, &dr.y, &dr.width, &dr.height);
  return mPrintContext->DrawImage(aImage,
                     sr.x, sr.y,
                     sr.width, sr.height,
                     dr.x, dr.y,
                     dr.width, dr.height);
}

/** ---------------------------------------------------
 *  See documentation ndow_private/in nsIRenderingContext.h
 */
NS_IMETHODIMP 
nsRenderingContextXp :: DrawImage(nsIImage *aImage, const nsRect& aRect)
{
  PR_LOG(RenderingContextXpLM, PR_LOG_DEBUG, ("nsRenderingContextXp::DrawImage(aRect)\n"));

  return DrawImage(aImage, aRect.x, aRect.y,
                           aRect.width, aRect.height);
}


#ifdef USE_IMG2
#include "imgIContainer.h"
#include "gfxIImageFrame.h"
#include "nsIInterfaceRequestor.h"

/* [noscript] void drawImage (in imgIContainer aImage, [const] in nsRect aSrcRect, [const] in nsPoint aDestPoint); */
NS_IMETHODIMP nsRenderingContextXp::DrawImage(imgIContainer *aImage, const nsRect * aSrcRect, const nsPoint * aDestPoint)
{
  nsPoint pt;
  nsRect  sr;

  pt = *aDestPoint;
  mTranMatrix->TransformCoord(&pt.x, &pt.y);

  sr = *aSrcRect;
  mTranMatrix->TransformNoXLateCoord(&sr.x, &sr.y);

  nsCOMPtr<gfxIImageFrame> iframe;
  aImage->GetCurrentFrame(getter_AddRefs(iframe));
  if (!iframe) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIImage> img(do_GetInterface(iframe));
  if (!img) return NS_ERROR_FAILURE;

  // doesn't it seem like we should use more of the params here?
  // img->Draw(*this, surface, sr.x, sr.y, sr.width, sr.height,
  //           pt.x + sr.x, pt.y + sr.y, sr.width, sr.height);
  return mPrintContext->DrawImage(img, pt.x, pt.y, sr.width, sr.height);
}

/* [noscript] void drawScaledImage (in imgIContainer aImage, [const] in nsRect aSrcRect, [const] in nsRect aDestRect); */
NS_IMETHODIMP nsRenderingContextXp::DrawScaledImage(imgIContainer *aImage, const nsRect * aSrcRect, const nsRect * aDestRect)
{
  nsRect dr;

  dr = *aDestRect;
  mTranMatrix->TransformCoord(&dr.x, &dr.y, &dr.width, &dr.height);

  nsCOMPtr<gfxIImageFrame> iframe;
  aImage->GetCurrentFrame(getter_AddRefs(iframe));
  if (!iframe) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIImage> img(do_GetInterface(iframe));
  if (!img) return NS_ERROR_FAILURE;

  // doesn't it seem like we should use more of the params here?
  // img->Draw(*this, surface, sr.x, sr.y, sr.width, sr.height, dr.x, dr.y, dr.width, dr.height);

  nsRect sr;

  sr = *aSrcRect;
  mTranMatrix->TransformCoord(&sr.x, &sr.y, &sr.width, &sr.height);
  sr.x = aSrcRect->x;
  sr.y = aSrcRect->y;
  mTranMatrix->TransformNoXLateCoord(&sr.x, &sr.y);

  return mPrintContext->DrawImage(img,
                                  sr.x, sr.y,
                                  sr.width, sr.height,
                                  dr.x, dr.y,
                                  dr.width, dr.height);
}
#endif /* USE_IMG2 */

#ifdef MOZ_MATHML
  /**
   * Returns metrics (in app units) of an 8-bit character string
   */
NS_IMETHODIMP 
nsRenderingContextXp::GetBoundingMetrics(const char*        aString,
                                         PRUint32           aLength,
                                         nsBoundingMetrics& aBoundingMetrics)
{
  aBoundingMetrics.Clear();
  if (!aString || 0 > aLength) {
    return NS_OK;
  }

  XFontStruct *font_struct = mCurrentFont;
  XCharStruct overall;
  int direction, font_ascent, font_descent;

  if ((font_struct->min_byte1 == 0) && (font_struct->max_byte1 == 0))
    XTextExtents(font_struct, (char *)aString, aLength,
                 &direction, &font_ascent, &font_descent,
                 &overall);
  else
    XTextExtents16(font_struct, (XChar2b*)aString, aLength/2,
                   &direction, &font_ascent, &font_descent,
                   &overall);
  aBoundingMetrics.leftBearing  =  overall.lbearing;
  aBoundingMetrics.rightBearing =  overall.rbearing;
  aBoundingMetrics.width        =  overall.width;
  aBoundingMetrics.ascent       =  overall.ascent;
  aBoundingMetrics.descent      =  overall.descent;

  aBoundingMetrics.leftBearing = NSToCoordRound(aBoundingMetrics.leftBearing * mP2T);
  aBoundingMetrics.rightBearing = NSToCoordRound(aBoundingMetrics.rightBearing * mP2T);
  aBoundingMetrics.width = NSToCoordRound(aBoundingMetrics.width * mP2T);
  aBoundingMetrics.ascent = NSToCoordRound(aBoundingMetrics.ascent * mP2T);
  aBoundingMetrics.descent = NSToCoordRound(aBoundingMetrics.descent * mP2T);

  return NS_OK;
}

  /**
   * Returns metrics (in app units) of a Unicode character string
   */
NS_IMETHODIMP 
nsRenderingContextXp::GetBoundingMetrics(const PRUnichar*   aString,
                                         PRUint32           aLength,
                                         nsBoundingMetrics& aBoundingMetrics,
                                         PRInt32*           aFontID)
{
  aBoundingMetrics.Clear();
  if (!aString || 0 > aLength) {
    return NS_OK;
  }
  nsFontMetricsXp* metrics = (nsFontMetricsXp*) mFontMetrics;
  nsFontXp* prevFont = nsnull;

  nsBoundingMetrics rawbm;
  PRBool firstTime = PR_TRUE;
  PRUint32 start = 0;
  PRUint32 i;
  for (i = 0; i < aLength; i++) {
    PRUnichar c = aString[i];
    nsFontXp* currFont = nsnull;
    nsFontXp** font = metrics->mLoadedFonts;
    nsFontXp** end = &metrics->mLoadedFonts[metrics->mLoadedFontsCount];
    while (font < end) {
      if (IS_REPRESENTABLE((*font)->mMap, c)) {
        currFont = *font;
        goto FoundFont; // for speed -- avoid "if" statement
      }
      font++;
    }
    currFont = metrics->FindFont(c);
FoundFont:
    // XXX avoid this test by duplicating code -- erik
    if (prevFont) {
      if (currFont != prevFont) {
        prevFont->GetBoundingMetrics((const PRUnichar*) &aString[start],
                                     i - start, rawbm);
        if (firstTime) {
          firstTime = PR_FALSE;
          aBoundingMetrics = rawbm;
        }
        else {
          aBoundingMetrics += rawbm;
        }
        prevFont = currFont;
        start = i;
      }
    }
    else {
      prevFont = currFont;
      start = i;
    }
  }
  if (prevFont) {
    prevFont->GetBoundingMetrics((const PRUnichar*) &aString[start],
                                 i - start, rawbm);
    if (firstTime) {
      aBoundingMetrics = rawbm;
    }
    else {
      aBoundingMetrics += rawbm;
    }
  }

  // convert to app units
  aBoundingMetrics.leftBearing = NSToCoordRound(aBoundingMetrics.leftBearing * mP2T);
  aBoundingMetrics.rightBearing = NSToCoordRound(aBoundingMetrics.rightBearing * mP2T);
  aBoundingMetrics.width = NSToCoordRound(aBoundingMetrics.width * mP2T);
  aBoundingMetrics.ascent = NSToCoordRound(aBoundingMetrics.ascent * mP2T);
  aBoundingMetrics.descent = NSToCoordRound(aBoundingMetrics.descent * mP2T);
 
  if (nsnull != aFontID)
    *aFontID = 0;

  return NS_OK;
}
#endif /* MOZ_MATHML */

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
NS_IMETHODIMP nsRenderingContextXp :: CopyOffScreenBits(nsDrawingSurface aSrcSurf,
                                                         PRInt32 aSrcX, PRInt32 aSrcY,
                                                         const nsRect &aDestBounds,
                                                         PRUint32 aCopyFlags)
{
  NS_NOTREACHED("nsRenderingContextXp::CopyOffScreenBits() not yet implemented");

  return NS_OK;
}

NS_IMETHODIMP nsRenderingContextXp::RetrieveCurrentNativeGraphicData(PRUint32 * ngd)
{
  NS_NOTREACHED("nsRenderingContextXp::RetrieveCurrentNativeGraphicData() not yet implemented");
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 */
void 
nsRenderingContextXp :: SetupFontAndColor(void)
{
  return;
}

#ifdef NOTNOW
HPEN nsRenderingContextXp :: SetupSolidPen(void)
{
  return mCurrPen;
}

HPEN nsRenderingContextXp :: SetupDashedPen(void)
{
  return mCurrPen;
}

HPEN nsRenderingContextXp :: SetupDottedPen(void)
{
  return mCurrPen;
}
#endif /* NOTNOW */


#ifdef DC
NS_IMETHODIMP
nsRenderingContextXp::GetColor(nsString& aColor)
{
  char cbuf[40];
  PR_snprintf(cbuf, sizeof(cbuf), "#%02x%02x%02x",
              NS_GET_R(mCurrentColor),
              NS_GET_G(mCurrentColor),
              NS_GET_B(mCurrentColor));
  aColor = cbuf;
  return NS_OK;
}

NS_IMETHODIMP
nsRenderingContextXp::SetColor(const nsString& aColor)
{
  nscolor rgb;
  if (NS_ColorNameToRGB(aColor, &rgb)) {
    SetColor(rgb);
  }
  else if (NS_HexToRGB(aColor, &rgb)) {
    SetColor(rgb);
  }
  return NS_OK;
}
#endif /* DC */


