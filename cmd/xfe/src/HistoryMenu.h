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
   HistoryMenu.h -- class for doing the dynamic bookmark menus
   Created: Chris Toshok <toshok@netscape.com>, 21-Dec-1996.
 */



#include "NotificationCenter.h"
#include "Frame.h"

#ifndef _xfe_historymenu_h
#define _xfe_historymenu_h

// This class can be used with a DYNA_CASCADEBUTTON or
// DYNA_MENUITEMS.

class XFE_HistoryMenu : public XFE_NotificationCenter
{
public:
  virtual ~XFE_HistoryMenu();

  // this function occupies the generateCallback slot in a menuspec.
  static void generate(Widget, void *, XFE_Frame *);

  XFE_CALLBACK_DECL(historyHasChanged)
private:
  XFE_HistoryMenu(Widget, XFE_Frame *);

  // the toplevel component -- the thing we dispatch our events to.
  XFE_Frame *m_parentFrame;

  // the cascade button we're tied to.
  Widget m_cascade;

  // the row column we're in.
  Widget m_submenu;

  // this is the first slot we are using the code assumes that the 
  // dynamic portion of the menu is at the end, so we can destroy from
  // this index down.
  int m_firstslot;

  // this function is substituted as the cascadingCallback once the
  // object has been created, and is used from then on.
  static void update_cb(Widget, XtPointer, XtPointer);

  // callback added in generate() that destroys this object when the cascade button
  // is destroyed.
  static void destroy_cb(Widget, XtPointer, XtPointer);

  static void free_cons_cb(Widget, XtPointer, XtPointer);
  static void arm_cb(Widget, XtPointer, XtPointer);
  static void activate_cb(Widget, XtPointer, XtPointer);
  static void disarm_cb(Widget, XtPointer, XtPointer);

  static void fillin_menu(Widget, XtPointer, XtPointer);
  void add_menu_items(Widget, MWContext*, BM_Entry *);

  // non-callback versions of the update callback
  void update();
};

#endif /* _xfe_historymenu_h */
