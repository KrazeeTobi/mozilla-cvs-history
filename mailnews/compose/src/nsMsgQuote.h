/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
#ifndef __nsMsgQuote_h__
#define __nsMsgQuote_h__

#include "nsIMsgQuote.h" 
#include "nsFileSpec.h"
#include "nsIFileSpec.h"
#include "nsIMsgMessageService.h"
#include "nsIStreamListener.h"
#include "nsIMimeStreamConverter.h"

#define NS_MSGQUOTE_CID \
  {0x1C7ABF0C, 0x21E5, 0x11d3, \
    { 0x8E, 0xF1, 0x00, 0xA0, 0x24, 0xA7, 0xD1, 0x44 }}

class nsMsgQuote;

class nsMsgQuoteListener: public nsIMimeStreamConverterListener
{
public:
	nsMsgQuoteListener();
	virtual     ~nsMsgQuoteListener();

	NS_DECL_ISUPPORTS

	// nsIMimeStreamConverterListener support
	NS_DECL_NSIMIMESTREAMCONVERTERLISTENER

	void SetMsgQuote(nsMsgQuote * msgQuote);
private:
	nsMsgQuote * mMsgQuote;
};

class nsMsgQuote: public nsIMsgQuote {
public: 
  nsMsgQuote();
  virtual ~nsMsgQuote();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIMSGQUOTE

  // 
  // Implementation data...
  //
  nsFileSpec      *mTmpFileSpec;
  nsIFileSpec     *mTmpIFileSpec;
//  nsCOMPtr<nsIStreamListener> mStreamListener;
  nsIStreamListener* mStreamListener;
  char            *mURI;
  nsIMsgMessageService    *mMessageService;
  PRBool			mQuoteHeaders;
  nsMsgQuoteListener *mQuoteListener;
};

// Will be used by factory to generate a nsMsgQuote class...
nsresult      NS_NewMsgQuote(const nsIID &aIID, void ** aInstancePtrResult);

#endif /* __nsMsgQuote_h__ */
