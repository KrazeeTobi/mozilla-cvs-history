/*
 * (C) Copyright The MITRE Corporation 1999  All rights reserved.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * The program provided "as is" without any warranty express or
 * implied, including the warranty of non-infringement and the implied
 * warranties of merchantibility and fitness for a particular purpose.
 * The Copyright owner will not be liable for any damages suffered by
 * you as a result of using the Program. In no event will the Copyright
 * owner be liable for any special, indirect or consequential damages or
 * lost profits even if the Copyright owner has been advised of the
 * possibility of their occurrence.
 *
 * Please see release.txt distributed with this file for more information.
 *
 */
// Tom Kneeland (02/02/2000)
//
//  Implementation of the wrapper class to convert a Mozilla 
//  nsIDOMDOMImplementation into a TransforMIIX DOMImplementation interface.
//
// Modification History:
// Who  When      What
//

#include "mozilladom.h"

DOMImplementation::DOMImplementation(nsIDOMDOMImplementation* domImpl,
                     Document* owner)
{
  nsDOMImpl = domImpl;
  ownerDocument = owner;
}

DOMImplementation::~DOMImplementation()
{
}

//
//Query the mozilla object for the requested feature, and return the 
//result to the caller.
//
MBool DOMImplementation::hasFeature(const String& feature,
                                    const String& version) const
{
  MBool bHasFeature = MB_FALSE;

  nsDOMImpl->HasFeature(feature.getConstNSString(), version.getConstNSString(),
            &bHasFeature);

  return bHasFeature;
}
