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
/* Name:		<Xfe/ArrowP.h>											*/
/* Description:	XfeArrow widget private header file.					*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#ifndef _XfeArrowP_h_							/* start ArrowP.h		*/
#define _XfeArrowP_h_

#include <Xfe/Arrow.h>
#include <Xfe/ButtonP.h>

XFE_BEGIN_CPLUSPLUS_PROTECTION
   
/*----------------------------------------------------------------------*/
/*																		*/
/* XfeArrowClassPart													*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct
{
    XtPointer		extension;					/* Extension			*/
} XfeArrowClassPart;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeArrowClassRec														*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeArrowClassRec
{
    CoreClassPart				core_class;
    XmPrimitiveClassPart		primitive_class;
    XfePrimitiveClassPart		xfe_primitive_class;
    XfeLabelClassPart			xfe_label_class;
    XfeButtonClassPart			xfe_button_class;
    XfeArrowClassPart			xfe_arrow_class;
} XfeArrowClassRec;

externalref XfeArrowClassRec xfeArrowClassRec;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeArrowPart															*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeArrowPart
{
    /* Arrow resources */
	unsigned char		arrow_direction;		/* arrow_direction		*/
	Dimension			arrow_width;			/* arrow_width			*/
	Dimension			arrow_height;			/* arrow_height			*/

    /* Private data -- Dont even look past this comment -- */
	GC					arrow_insens_GC;		/* Arrow insens GC		*/

} XfeArrowPart;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeArrowRec															*/
/*																		*/
/*----------------------------------------------------------------------*/
typedef struct _XfeArrowRec
{
    CorePart				core;				/* Core Part			*/
    XmPrimitivePart			primitive;			/* XmPrimitive Part		*/
    XfePrimitivePart		xfe_primitive;		/* XfePrimitive Part	*/
    XfeLabelPart			xfe_label;			/* XfeLabel Part		*/
    XfeButtonPart			xfe_button;			/* XfeButton Part		*/
    XfeArrowPart			xfe_arrow;			/* XfeArrow Part		*/
} XfeArrowRec;

/*----------------------------------------------------------------------*/
/*																		*/
/* XfeArrowPart Access Macro											*/
/*																		*/
/*----------------------------------------------------------------------*/
#define _XfeArrowPart(w) &(((XfeArrowWidget) w)->xfe_arrow)

XFE_END_CPLUSPLUS_PROTECTION

#endif											/* end ArrowP.h			*/
