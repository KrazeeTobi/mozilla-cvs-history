/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nsViewManager_h___
#define nsViewManager_h___

#include "nsIViewManager.h"
#include "nsCRT.h"
#include "nsIPresContext.h"
#include "nsIWidget.h"
#include "nsITimer.h"
#include "prtime.h"

class nsViewManager : public nsIViewManager
{
public:
  nsViewManager();

  void* operator new(size_t sz) {
    void* rv = new char[sz];
    nsCRT::zero(rv, sz);
    return rv;
  }

  NS_DECL_ISUPPORTS

  virtual nsresult Init(nsIDeviceContext* aContext);

  virtual nsIView *GetRootView();
  virtual void SetRootView(nsIView *aView);

  virtual PRUint32 GetFrameRate();
  virtual nsresult SetFrameRate(PRUint32 frameRate);

  virtual void GetWindowDimensions(nscoord *width, nscoord *height);
  virtual void SetWindowDimensions(nscoord width, nscoord height);

  virtual void GetWindowOffsets(nscoord *xoffset, nscoord *yoffset);
  virtual void SetWindowOffsets(nscoord xoffset, nscoord yoffset);

  virtual void ResetScrolling(void);

  virtual void Composite();

  virtual void UpdateView(nsIView *aView, nsIRegion *aRegion,
                          PRUint32 aUpdateFlags);
  virtual void UpdateView(nsIView *aView, const nsRect &aRect, PRUint32 aUpdateFlags);

  virtual nsEventStatus DispatchEvent(nsGUIEvent *aEvent);

  virtual PRBool GrabMouseEvents(nsIView *aView);
  virtual PRBool GrabKeyEvents(nsIView *aView);

  virtual nsIView* GetMouseEventGrabber();
  virtual nsIView* GetKeyEventGrabber();

  virtual void InsertChild(nsIView *parent, nsIView *child, nsIView *sibling,
                           PRBool above);

  virtual void InsertChild(nsIView *parent, nsIView *child,
                           PRInt32 zindex);

  virtual void RemoveChild(nsIView *parent, nsIView *child);

  virtual void MoveViewBy(nsIView *aView, nscoord x, nscoord y);

  virtual void MoveViewTo(nsIView *aView, nscoord x, nscoord y);

  virtual void ResizeView(nsIView *aView, nscoord width, nscoord height);

  virtual void SetViewClip(nsIView *aView, nsRect *rect);

  virtual void SetViewVisibility(nsIView *aView, nsViewVisibility visible);

  virtual void SetViewZindex(nsIView *aView, PRInt32 zindex);

  virtual void MoveViewAbove(nsIView *aView, nsIView *other);
  virtual void MoveViewBelow(nsIView *aView, nsIView *other);

  virtual PRBool IsViewShown(nsIView *aView);

  virtual PRBool GetViewClipAbsolute(nsIView *aView, nsRect *rect);

  virtual void SetViewContentTransparency(nsIView *aView, PRBool aTransparent);
  virtual void SetViewOpacity(nsIView *aView, float aOpacity);

  NS_IMETHOD SetViewObserver(nsIViewObserver *aObserver);
  NS_IMETHOD GetViewObserver(nsIViewObserver *&aObserver);

  virtual nsIDeviceContext* GetDeviceContext();

  virtual void ClearDirtyRegion();

  nsDrawingSurface GetDrawingSurface(nsIRenderingContext &aContext, nsRect& aBounds);

  virtual void ShowQuality(PRBool aShow);
  virtual PRBool GetShowQuality(void);
  virtual void SetQuality(nsContentQuality aQuality);

  NS_IMETHOD DisableRefresh(void);
  NS_IMETHOD EnableRefresh(void);

private:
  virtual ~nsViewManager();
  nsIRenderingContext *CreateRenderingContext(nsIView &aView);
  void AddRectToDirtyRegion(nsRect &aRect);
  void UpdateTransCnt(nsIView *oldview, nsIView *newview);

  void Refresh(nsIView *aView, nsIRenderingContext *aContext,
                       nsIRegion *region, PRUint32 aUpdateFlags);
  void Refresh(nsIView* aView, nsIRenderingContext *aContext,
                       nsRect *rect, PRUint32 aUpdateFlags);

  nsIDeviceContext  *mContext;
  nsIViewObserver   *mObserver;
  nsIWidget         *mRootWindow;
  PRTime            mLastRefresh;
  nsIRegion         *mDirtyRegion;
  PRInt32           mTransCnt;
  PRBool            mRefreshEnabled;
  PRBool            mPainting;
  nsIView           *mMouseGrabber;
  nsIView           *mKeyGrabber;

  static PRUint32          mVMCount;        //number of viewmanagers
  static nsDrawingSurface  mDrawingSurface; //single drawing surface
  static nsRect            mDSBounds;       //for all VMs

public:
  //these are public so that our timer callback can poke them.
  nsITimer          *mTimer;
  nsRect            mDirtyRect;
  nsIView           *mRootView;
  PRUint32          mFrameRate;
};

#endif

