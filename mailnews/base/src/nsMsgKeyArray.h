/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
// nsMsgKeyArray.h: header file
//

#ifndef _nsMsgKeyArray_H_
#define _nsMsgKeyArray_H_ 

/////////////////////////////////////////////////////////////////////////////
// nsMsgKeyArray - Growable Array of MessageKeys's - Can contain up to uint - 1 ids. 
///////////////////////////////////////////////////////////////////////////// 
#include "nsUInt32Array.h"
#include "MailNewsTypes.h"

class nsMsgKeyArray : public nsUInt32Array
{
// constructors
public:
	nsMsgKeyArray() : nsUInt32Array() {}

// Public Operations on the MessageKey array - typesafe overrides
public:
	MessageKey operator[](PRUint32 nIndex) const {
	  return((MessageKey)nsUInt32Array::operator[](nIndex));
	}
	MessageKey GetKeyFromIndex(PRUint32 nIndex) {
	  return(operator[](nIndex));
	}
	MessageKey GetAt(PRUint32 nIndex) const {
	  return(operator[](nIndex));
	}
	void SetAt(PRUint32 nIndex, MessageKey key) {
	  nsUInt32Array::SetAt(nIndex, (PRUint32)key);
	}
	void SetAtGrow(PRUint32 nIndex, MessageKey key) {
	  nsUInt32Array::SetAtGrow(nIndex, (uint32)key);
	}
	void InsertAt(PRUint32 nIndex, MessageKey key, int nCount = 1) {
	  nsUInt32Array::InsertAt(nIndex, (PRUint32)key, nCount);
	}
	void InsertAt(PRUint32 nIndex, const nsMsgKeyArray *idArray) {
	  nsUInt32Array::InsertAt(nIndex, idArray);
	}
	PRUint32 Add(MessageKey id) {
	  return(nsUInt32Array::Add((uint32)id));
	}
	PRUint32 Add(MessageKey *elementPtr, PRUint32 numElements) {
	  return nsUInt32Array::Add((PRUint32 *) elementPtr, numElements);
	}
	void CopyArray(nsMsgKeyArray *oldA) { nsUInt32Array::CopyArray((nsUInt32Array*) oldA); }
	void CopyArray(nsMsgKeyArray &oldA) { nsUInt32Array::CopyArray(oldA); }
// new operations
public:
	MSG_ViewIndex					FindIndex(MessageKey key); // returns -1 if not found
	
	// use these next two carefully
	MessageKey*				GetArray(void) {
	  return((MessageKey*)m_pData);		// only valid until another function
										// called on the array (like
										// GetBuffer() in CString)
	}
	void				SetArray(MessageKey* pData, int numElements,
								 int numAllocated);
};
///////////////////////////////////////////////////////
#endif // _IDARRAY_H_
