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
/* Name:		<Xfe/ShellUtil.h>										*/
/* Description:	Shell misc utilities header.							*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#ifndef _XfeShellUtil_h_						/* start ShellUtil.h	*/
#define _XfeShellUtil_h_

#include <X11/Intrinsic.h>						/* Xt public defs		*/

XFE_BEGIN_CPLUSPLUS_PROTECTION

/*----------------------------------------------------------------------*/
/*																		*/
/* XmRShellPlacement													*/
/*																		*/
/*----------------------------------------------------------------------*/
enum
{
	XmSHELL_PLACEMENT_BOTTOM,					/*						*/
	XmSHELL_PLACEMENT_LEFT,						/*						*/
	XmSHELL_PLACEMENT_RIGHT,					/*						*/
	XmSHELL_PLACEMENT_TOP						/*						*/
};
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*																		*/
/* Shell																*/
/*																		*/
/*----------------------------------------------------------------------*/
extern Boolean
XfeShellIsPoppedUp					(Widget			shell);
/*----------------------------------------------------------------------*/
extern void
XfeShellAddCloseCallback			(Widget			shell,
									 XtCallbackProc	callback,
									 XtPointer		data);
/*----------------------------------------------------------------------*/
extern void
XfeShellRemoveCloseCallback			(Widget			shell,
									 XtCallbackProc	callback,
									 XtPointer		data);
/*----------------------------------------------------------------------*/
extern Boolean
XfeShellGetDecorationOffset			(Widget			shell,
									 Position *		x_out,
									 Position *		y_out);
/*----------------------------------------------------------------------*/
extern int
XfeShellGetGeometryFromResource		(Widget			shell,
									 Position *		x_out,
									 Position *		y_out,
									 Dimension *	width_out,
									 Dimension *	height_out);
/*----------------------------------------------------------------------*/
extern void
XfeShellSetIconicState				(Widget			shell,
									 Boolean		state);
/*----------------------------------------------------------------------*/
extern Boolean
XfeShellGetIconicState				(Widget			shell);
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*																		*/
/* Shell placement														*/
/*																		*/
/*----------------------------------------------------------------------*/
extern void
XfeShellPlaceAtLocation				(Widget			shell,
									 Widget			relative,
									 unsigned char	location,
									 Dimension		dx,
									 Dimension		dy);
/*----------------------------------------------------------------------*/

XFE_END_CPLUSPLUS_PROTECTION

#endif											/* end ShellUtil.h		*/
