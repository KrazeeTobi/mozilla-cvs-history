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
   EditorBar.cpp
   Created: Richard Hess <rhess@netscape.com>, 10-Dec-96.
   */



#include "mozilla.h"
#include "xfe.h"
#include "EditorBar.h"

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/TextF.h>
#include "DtWidgets/ComboBox.h"

extern "C" {
  Widget fe_EditorCreatePropertiesToolbar(MWContext*, Widget, char*);
  Widget fe_EditorCreateComposeToolbar(MWContext*, Widget, char*);
  XtPointer fe_tooltip_mappee(Widget widget, XtPointer data);
}

XFE_EditorBar::XFE_EditorBar(XFE_Component *toplevel_component,
			     Widget parent, MWContext* context)
: XFE_Component(toplevel_component), m_eToolbar(NULL)
{
  Widget url_frame;

  url_frame = XmCreateFrame(parent, "editBarFrame", NULL, 0);
  XtVaSetValues(url_frame,
				XmNshadowType,      XmSHADOW_OUT,
				XmNshadowThickness, 1,
				NULL);

  m_eToolbar = fe_EditorCreatePropertiesToolbar(context, url_frame, "editBar");

  XtManageChild(m_eToolbar);

  /* Add tooltip
   */
  fe_WidgetTreeWalk(m_eToolbar, fe_tooltip_mappee, NULL);

  setBaseWidget(url_frame);
}

XFE_EditorBar::~XFE_EditorBar()
{
}

