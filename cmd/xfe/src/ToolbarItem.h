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

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Name:        ToolbarItem.h                                           //
//                                                                      //
// Description:	XFE_ToolbarItem class header.                           //
//              Superclass for anything that goes in a toolbar.         //
//                                                                      //
// Author:		Ramiro Estrugo <ramiro@netscape.com>                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _xfe_toolbar_item_h_
#define _xfe_toolbar_item_h_

#include "Component.h"
#include "Frame.h"
#include "htrdf.h"

class XFE_ToolbarItem : public XFE_Component
{
public:

    XFE_ToolbarItem(XFE_Frame *		frame,
					Widget			parent,
                    HT_Resource		htResource,
					const String	name);

    virtual ~XFE_ToolbarItem();


	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// Accessors                                                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	const String	getName();
	Widget			getParent();
	XFE_Frame *		getAncestorFrame();
	MWContext *		getAncestorContext();
	HT_Resource		getHtResource();

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// initialize                                                       //
	//                                                                  //
	// This method should be called by the item creator to initialize   //
	// the item.  This is needed for abstract methods to be properly    //
	// called.  The alternative is to call initialize directlry from    //
	// the XFE_ToolbarItem ctor - but it does not work because abstract //
	// methods might not have been properly set up by then.             //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
 	virtual void		initialize				() = 0;	

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// Sensitive interface                                              //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	virtual void		setSensitive			(Boolean state);
	virtual Boolean		isSensitive				();

protected:
	
	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// XFE_Component methods                                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	virtual void	setBaseWidget		(Widget w);

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// Widget creation interface                                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	virtual Widget	createBaseWidget	(Widget			parent,
										 const String	name) = 0;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// configure                                                        //
	//                                                                  //
	// This method is called as soon as the base widget has been set    //
	// so that sub classes can configure the item according to their    //
	// needs.                                                           //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
 	virtual void	configure			() = 0;	

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// addCallbacks                                                     //
	//                                                                  //
	// This method is called right after configure().  Sub classes can  //
	// use it to install callbacks on the base widget.                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
 	virtual void	addCallbacks		();

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// ToolTip support                                                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	virtual void	addToolTipSupport	();

	XmString		getTipStringFromAppDefaults	();
	XmString		getDocStringFromAppDefaults	();

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// ToolTip interface                                                //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////

 	virtual void	tipStringObtain		(XmString *		stringReturn,
										 Boolean *		needToFreeString);
	
 	virtual void	docStringObtain		(XmString *		stringReturn,
										 Boolean *		needToFreeString);
	
 	virtual void	docStringSet		(XmString		string);

 	virtual void	docStringClear		(XmString		string);

private:

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// Private data                                                     //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	String			m_name;
	Widget			m_parent;
	XFE_Frame *		m_ancestorFrame;
	HT_Resource		m_htResource;

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	// Private callbacks                                                //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
 	static void		tipStringObtainCB	(Widget			w,
 										 XtPointer		clientData,
 										 XmString *		stringReturn,
 										 Boolean *		needToFreeString);

 	static void		docStringObtainCB	(Widget			w,
 										 XtPointer		clientData,
 										 XmString *		stringReturn,
 										 Boolean *		needToFreeString);

 	static void		docStringCB			(Widget			w,
 										 XtPointer		clientData,
 										 unsigned char	reason,
 										 XmString		string);


	static void		freeNameCB			(Widget			w,
 										 XtPointer		clientData,
 										 XtPointer		callData);
};

#endif // _xfe_toolbar_item_h_
