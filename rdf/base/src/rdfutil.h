/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */


/*

  A bunch of useful RDF utility routines.  Many of these will
  eventually be exported outside of RDF.DLL via the nsIRDFService
  interface.

  TO DO

  1) Move the anonymous resource stuff to nsIRDFService?

  2) All that's left is rdf_PossiblyMakeRelative() and
     -Absolute(). Maybe those go on nsIRDFService, too.

 */

#ifndef rdfutil_h__
#define rdfutil_h__

#include "prtypes.h"

class nsCString;
class nsString;
class nsIURI;

nsresult
rdf_MakeRelativeRef(const nsString& aBaseURI, nsString& aURI);

nsresult
rdf_MakeRelativeName(const nsString& aBaseURI, nsString& aURI);

nsresult
rdf_MakeAbsoluteURI(const nsString& aBaseURI, nsString& aURI);

nsresult
rdf_MakeAbsoluteURI(nsIURI* aBaseURL, nsString& aURI);

nsresult
rdf_MakeAbsoluteURI(nsIURI* aBaseURL, nsCString& aURI);

#endif // rdfutil_h__


