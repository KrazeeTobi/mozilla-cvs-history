/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef _IdlAttribute_h__
#define _IdlAttribute_h__

#include "IdlVariable.h"

class IdlAttribute : public IdlVariable {

private:
  int mReadOnly;

public:
                  IdlAttribute();
                  ~IdlAttribute();

  void            SetReadOnly(int aReadOnlyFlag);
  int             GetReadOnly();

};

#ifdef XP_MAC
#include <ostream.h>			// required for namespace resolution
#else
class ostream;
#endif

ostream& operator<<(ostream &s, IdlAttribute &aAttribute);

#endif

