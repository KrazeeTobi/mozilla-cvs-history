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
/* Name:		<Xfe/LogoP.h>											*/
/* Description:	XfeLogo widget private header file.						*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#ifndef _XfeLogoP_h_							/* start LogoP.h		*/
#define _XfeLogoP_h_

#include <Xfe/Logo.h>
#include <Xfe/ButtonP.h>

XFE_BEGIN_CPLUSPLUS_PROTECTION
   
/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogoClassPart														*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct
{
    XtPointer		extension;					/* Extension			*/
} XfeLogoClassPart;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogoClassRec														*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeLogoClassRec
{
    CoreClassPart			core_class;
    XmPrimitiveClassPart	primitive_class;
    XfePrimitiveClassPart	xfe_primitive_class;
    XfeLabelClassPart		xfe_label_class;
    XfeButtonClassPart		xfe_button_class;
    XfeLogoClassPart		xfe_logo_class;
} XfeLogoClassRec;

externalref XfeLogoClassRec xfeLogoClassRec;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogoPart															*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeLogoPart
{
    /* Callback resources */
    XtCallbackList		animation_callback;		/* Animation callback	*/

    /* Pixmap resources */
	Cardinal			current_pixmap_index;	/* Current pixmap index	*/
    XfePixmapTable		animation_pixmaps;		/* Logo pixmaps			*/
	Cardinal			num_animation_pixmaps;	/* Num logo pixmaps		*/
	int					animation_interval;		/* Animation interval	*/
	Boolean				animation_running;		/* Animation running	*/
	Boolean				reset_when_idle;		/* Reset when idle		*/

    /* Private Data Members */
    GC					copy_GC;				/* Copy gc				*/
	Dimension			animation_width;		/* Animation width		*/
	Dimension			animation_height;		/* Animation height		*/

	XtIntervalId		timer_id;				/* Timer id				*/
} XfeLogoPart;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogoRec															*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeLogoRec
{
    CorePart			core;					/* Core Part			*/
    XmPrimitivePart		primitive;				/* XmPrimitive Part		*/
    XfePrimitivePart	xfe_primitive;			/* XfePrimitive Part	*/
    XfeLabelPart		xfe_label;				/* XfeLabel Part		*/
    XfeButtonPart		xfe_button;				/* XfeButton Part		*/
    XfeLogoPart			xfe_logo;				/* XfeLogo Part			*/
} XfeLogoRec;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogoPart Access Macro												*/
/*																		*/
/*----------------------------------------------------------------------*/
#define _XfeLogoPart(w) &(((XfeLogoWidget) w) -> xfe_logo)

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeLogo - superclass = XfeButton										*/
/*																		*/
/*----------------------------------------------------------------------*/
#define _XFE_PREPARE_LOGO_ANIMATION					XfePrepare6

XFE_END_CPLUSPLUS_PROTECTION

#endif											/* end LogoP.h			*/
