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
/* 
   prefs.h -- the C api to preferences dialogs
   Created: Chris Toshok <toshok@netscape.com>, 12-Feb-97
 */



#ifndef _xfe_prefs_h_
#define _xfe_prefs_h_

#include "Component.h"

XP_BEGIN_PROTOS

void fe_showPreferences(XFE_Component *topLevel, MWContext *context);

void fe_showMailNewsPreferences(XFE_Component *topLevel, MWContext *context);
void fe_showMailServerPreferences(XFE_Component *toplevel, MWContext *context);

XP_END_PROTOS

#endif /* _xfe_prefs_h_ */
