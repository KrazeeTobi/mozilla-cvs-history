/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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


/*----------------------------------------------------------------------*/
/*																		*/
/* Name:		PaneTestFour.c											*/
/* Description:	Test for XfePane widget.								*/
/* Author:		Ramiro Estrugo <ramiro@netscape.com>					*/
/*																		*/
/*----------------------------------------------------------------------*/

#include <Xfe/XfeTest.h>

/*----------------------------------------------------------------------*/
int
main(int argc,char *argv[])
{
	Widget		frame;
	Widget		main_form;

	Widget		nav_center_pane;
	Widget		rdf_pane;
	Widget		selector;
	Widget		content;
	Widget		tree;

	XfeAppCreateSimple("PaneTestFour",&argc,argv,"Frame",&frame,&main_form);
	
	nav_center_pane = 
		XtVaCreateManagedWidget("mainPane",
								xfePaneWidgetClass,
								main_form,
								XmNorientation,		XmHORIZONTAL,
								NULL);

	rdf_pane = 
		XtVaCreateManagedWidget("rdfPane",
								xfePaneWidgetClass,
								nav_center_pane,
								XmNorientation,		XmHORIZONTAL,
								NULL);

	selector = 
		XtVaCreateManagedWidget("rdfPane",
								xfeToolScrollWidgetClass,
								rdf_pane,
								NULL);

	tree = 
		XtVaCreateManagedWidget("tree",
								xmFormWidgetClass,
								rdf_pane,
								NULL);


	content = 
		XtVaCreateManagedWidget("content",
								xmTextWidgetClass,
								nav_center_pane,
								NULL);

	XtPopup(frame,XtGrabNone);

    XfeAppMainLoop();

	return 0;
}
/*----------------------------------------------------------------------*/
