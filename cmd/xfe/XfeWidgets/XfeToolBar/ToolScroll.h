/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

/*----------------------------------------------------------------------*/
/*																		*/
/* Name:		<Xfe/ToolScroll.h>										*/
/* Description:	XfeToolScroll widget public header file.				*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#ifndef _XfeToolScroll_h_						/* start ToolScroll.h	*/
#define _XfeToolScroll_h_

#include <Xfe/Oriented.h>
#include <Xfe/Logo.h>
#include <Xfe/ToolBar.h>

XFE_BEGIN_CPLUSPLUS_PROTECTION

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeToolScroll resource names											*/
/*																		*/
/*----------------------------------------------------------------------*/
#define XmNarrowDisplayPolicy				"arrowDisplayPolicy"
#define XmNarrowPlacement					"arrowPlacement"
#define XmNbackwardArrow					"backwardArrow"
#define XmNclipArea							"clipArea"
#define XmNclipShadowThickness				"clipShadowThickness"
#define XmNclipShadowType					"clipShadowType"
#define XmNforwardArrow						"forwardArrow"
#define XmNtoolBarPosition					"toolBarPosition"

#define XmCArrowDisplayPolicy				"ArrowDisplayPolicy"
#define XmCArrowPlacement					"ArrowPlacement"
#define XmCBackwardArrow					"BackwardArrow"
#define XmCClipArea							"ClipArea"
#define XmCForwardArrow						"ForwardArrow"
#define XmCToolBarPosition					"ToolBarPosition"

#define XmRArrowDisplayPolicy				XmRScrollBarDisplayPolicy
#define XmRToolScrollArrowPlacement			"ToolScrollArrowPlacement"

/*----------------------------------------------------------------------*/
/*																		*/
/* XmRarrowPlacement													*/
/*																		*/
/*----------------------------------------------------------------------*/
enum
{
	XmTOOL_SCROLL_ARROW_PLACEMENT_BOTH,
	XmTOOL_SCROLL_ARROW_PLACEMENT_END,
	XmTOOL_SCROLL_ARROW_PLACEMENT_START
};

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeBox class names													*/
/*																		*/
/*----------------------------------------------------------------------*/
externalref WidgetClass xfeToolScrollWidgetClass;

typedef struct _XfeToolScrollClassRec *		XfeToolScrollWidgetClass;
typedef struct _XfeToolScrollRec *			XfeToolScrollWidget;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeBox subclass test macro											*/
/*																		*/
/*----------------------------------------------------------------------*/
#define XfeIsToolScroll(w)	XtIsSubclass(w,xfeToolScrollWidgetClass)

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeToolScroll Public Methods											*/
/*																		*/
/*----------------------------------------------------------------------*/
extern Widget
XfeCreateToolScroll				(Widget		pw,
								 String		name,
								 Arg *		av,
								 Cardinal	ac);
/*----------------------------------------------------------------------*/

XFE_END_CPLUSPLUS_PROTECTION

#endif											/* end ToolScroll.h		*/
