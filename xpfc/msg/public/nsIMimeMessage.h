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

#ifndef nsIMIMEMessage_h___
#define nsIMIMEMessage_h___

#include "nsISupports.h"
#include "nsString.h"
#include "nsIMIMEBodyPart.h"

//d69d9a40-7027-11d2-8dbc-00805f8a7ab6
#define NS_IMIME_MESSAGE_IID   \
{ 0xd69d9a40, 0x7027, 0x11d2,    \
{ 0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

class nsIMIMEMessage : public nsISupports
{

public:

  NS_IMETHOD Init() = 0;

  NS_IMETHOD AddHeader(nsString& aHeaderName, nsString& aHeaderValue) = 0;
  NS_IMETHOD RemoveHeader(nsString& aHeaderName) = 0;
  NS_IMETHOD GetHeader(nsString& aHeaderName, nsString& aHeaderValue) = 0;

  NS_IMETHOD AddAttachment(nsString& aAttachment, nsMIMEEncoding aMIMEEncoding = nsMIMEEncoding_default) = 0;
  NS_IMETHOD AddText(nsString& aText, nsMIMEEncoding aMIMEEncoding = nsMIMEEncoding_default) = 0;

  NS_IMETHOD AddBodyPart(nsIMIMEBodyPart& aBodyPart) = 0;

};

#endif /* nsIMIMEMessage_h___ */
