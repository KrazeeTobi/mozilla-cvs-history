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

/* a really lame-ass last resort for putting objects in the tree
 * use only if there really is no object */

#include "nsIRDFDOMViewerObject.h"
#include "nsHashtable.h"

class nsDOMViewerObject : nsIRDFDOMViewerObject {
 public:
  nsDOMViewerObject();
  virtual ~nsDOMViewerObject();
  
  NS_DECL_ISUPPORTS
  NS_DECL_NSIRDFDOMVIEWEROBJECT

private:
  nsSupportsHashtable targets;
  
};

nsresult
NS_NewDOMViewerObject(const nsIID& iid, void ** result);
