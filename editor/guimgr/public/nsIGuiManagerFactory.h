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

#ifndef nsIGuiManagerFactory_h__
#define nsIGuiManagerFactory_h__
#include "nsISupports.h"

/*
Gui Manager interface to outside world
*/

#define NS_IGUIMANAGERFACTORY_IID \
{ /* 6279AC00-8BD7-11d2-9821-00805F8AA8B8*/ \
0x6279ac00, 0x8bd7, 0x11d2, \
{ 0x98, 0x21, 0x0, 0x80, 0x5f, 0x8a, 0xa8, 0xb8 } }


/**
 * A Gui manager specific interface. 
 */
class nsIGuiManagerFactory  : public nsISupports{
public:

};

#endif // nsIGuiManagerFactory_h__

