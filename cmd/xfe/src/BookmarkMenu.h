/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
/* 
   BookmarkMenu.h -- class for doing the dynamic bookmark menus
   Created: Chris Toshok <toshok@netscape.com>, 19-Dec-1996.
 */



#include "RDFMenuToolbarBase.h"
#include "xp_core.h"
#include <X11/Intrinsic.h>

#ifndef _xfe_bookmarkmenu_h
#define _xfe_bookmarkmenu_h

// This class can be used with a DYNA_CASCADEBUTTON or
// DYNA_MENUITEMS.

class XFE_Frame;

class XFE_BookmarkMenu : public XFE_RDFMenuToolbarBase
{
public:
	
	// this function occupies the generateCallback slot in a menuspec.
	static void generate		(Widget, void *, XFE_Frame *);

	static void generateQuickfile	(Widget, void *, XFE_Frame *);

	virtual void	enableDropping			();
	virtual void	disableDropping			();

	virtual void	enableFiling			();
	virtual void	disableFiling			();

protected:

	virtual void	prepareToUpdateRoot		();
	virtual void	updateRoot      		();

private:

	XFE_BookmarkMenu		(Widget			cascade,
							 XFE_Frame *	frame,
							 XP_Bool		onlyHeaders,
							 XP_Bool		fancyItems);

	Widget				_cascade;			// Cascade widget to popup from
	Widget				_subMenu;			// Sub menu
	Cardinal			_firstSlot;			// First slot for bookmark items

	static void		destroy_cb			(Widget,XtPointer,XtPointer);
	static void		update_cb			(Widget,XtPointer,XtPointer);

	void			setFixedItemSensitive		(XP_Bool);

};

#endif /* _xfe_bookmarkmenu_h */
