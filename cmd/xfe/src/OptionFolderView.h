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
   OptionFolderView.h -- class definition for OptionFolderView
   Created: Dora Hsu<dora@netscape.com>, 23-Sept-96.
 */



#ifndef _xfe_optionfolderview_h
#define _xfe_optionfolderview_h

#include "rosetta.h"
#include "MNView.h"
#include "msgcom.h"
#include <Xm/Xm.h>

typedef enum
{
 MSG_TextEncodingNotSet = 0,
 MSG_8bit = 1,
 MSG_MimeCompliant
}MSG_TEXT_ENCODING;

typedef enum
{
 MSG_BinaryEncodingNotSet = 0,
 MSG_Mime = 1,
 MSG_UUencode
}MSG_BINARY_ENCODING;


class XFE_OptionFolderView: public XFE_MNView
{
  public:
	XFE_OptionFolderView(XFE_Component *toplevel_component, 
				XFE_View *parent_view,
                        	MSG_Pane *p = NULL, 
				MWContext *context = NULL);

	virtual ~XFE_OptionFolderView();
	void	createWidgets(Widget parent);
	Boolean isPrioritySelected(MSG_PRIORITY priority);
	void    selectPriority(MSG_PRIORITY priority);
	void    selectTextEncoding(MSG_TEXT_ENCODING priority);
	void    selectBinaryEncoding(MSG_BINARY_ENCODING priority);
 	void 	updateAllOptions();
  protected:
	static void 	selectPriorityCallback(Widget w, XtPointer client, XtPointer);
	static void 	returnReceiptChangeCallback(Widget w, XtPointer client, XtPointer);
	static void 	encryptedChangeCallback(Widget w, XtPointer client, XtPointer);
	static void 	signedChangeCallback(Widget w, XtPointer client, XtPointer);
	static void 	selectTextEncodingCallback(Widget w, XtPointer client, XtPointer);
	static void 	selectBinaryEncodingCallback(Widget w, XtPointer client, XtPointer);
	void handleReturnReceipt(Boolean set);
	void handleEncrypted(Boolean set);
	void handleSigned(Boolean set);
  private:
	Boolean m_bSigned;		// Not used for now
	Boolean m_bEncrypted; 		// Not used for now
	Boolean m_bReturnReceipt;	// Not used for now
	Widget  m_optionW;
	Widget  m_textEncodingOptionW;
	Widget  m_binaryEncodingOptionW;
	Widget  m_returnReceiptW;
	HG21182
	MSG_PRIORITY m_selectedPriority;
	XFE_CALLBACK_DECL(updateSecurityOption)
};

#endif

