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
 * Copyright (C) 1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsIMsgRDFFolder.h"

class nsMsgRDFFolder : public nsIMsgRDFFolder
{
private:
	char *	mURI;
	nsIMsgFolder *mFolder;
public: 
	nsMsgRDFFolder(const char* uri);
	virtual ~nsMsgRDFFolder();

	/* this macro defines QueryInterface, AddRef and Release for this class */
	NS_DECL_ISUPPORTS

	//nsIRDFNode
  NS_IMETHOD EqualsNode(nsIRDFNode* node, PRBool* result) const;

	//nsIRDFResource
	NS_IMETHOD GetValue(const char* *uri) const;
  NS_IMETHOD EqualsResource(const nsIRDFResource* resource, PRBool* result) const;
  NS_IMETHOD EqualsString(const char* uri, PRBool* result) const;

	//nsIMsgRDFFolder
	NS_IMETHOD GetFolder(nsIMsgFolder * *aFolder);
	NS_IMETHOD SetFolder(nsIMsgFolder * aFolder);


};