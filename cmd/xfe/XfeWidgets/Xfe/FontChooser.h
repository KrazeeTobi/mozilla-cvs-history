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
/* Name:		<Xfe/FontChooser.h>										*/
/* Description:	XfeFontChooser widget public header file.				*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#ifndef _XfeFontChooser_h_						/* start FontChooser.h	*/
#define _XfeFontChooser_h_

#include <Xfe/Cascade.h>

XFE_BEGIN_CPLUSPLUS_PROTECTION

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeFontChooser resource names										*/
/*																		*/
/*----------------------------------------------------------------------*/
#define XmNfontItemFonts				"fontItemFonts"
#define XmNfontItemLabels				"fontItemLabels"
#define XmNnumFontItems					"numFontItems"

#define XmCFontItemFonts				"FontItemFonts"
#define XmCNumFontItems					"NumFontItems"

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeFontChooser class names											*/
/*																		*/
/*----------------------------------------------------------------------*/
externalref WidgetClass xfeFontChooserWidgetClass;
    
typedef struct _XfeFontChooserClassRec *	XfeFontChooserWidgetClass;
typedef struct _XfeFontChooserRec *			XfeFontChooserWidget;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeFontChooser subclass test macro									*/
/*																		*/
/*----------------------------------------------------------------------*/
#define XfeIsFontChooser(w)	XtIsSubclass(w,xfeFontChooserWidgetClass)

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeFontChooser public functions										*/
/*																		*/
/*----------------------------------------------------------------------*/
extern Widget
XfeCreateFontChooser		(Widget		pw,
							 String		name,
							 Arg *		av,
							 Cardinal	ac);
/*----------------------------------------------------------------------*/
extern void
XfeFontChooserDestroyChildren	(Widget		w);
/*----------------------------------------------------------------------*/

XFE_END_CPLUSPLUS_PROTECTION

#endif											/* end FontChooser.h	*/
