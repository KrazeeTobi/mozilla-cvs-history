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

#include "rdf.h"

int
main(int argc, char** argv)
{
  nsIRDFService* pRDF = 0;

  NS_GetRDFService( &pRDF );
  PR_ASSERT( pRDF != 0 );

  nsIRDFDataBase* pDB;
  char* url[] = { "file:///sitemap.rdf", NULL };

  /* turn on logging */

  pRDF->CreateDatabase( url, &pDB );
  PR_ASSERT( pDB != 0 );

  /* execute queries */

  pDB->Release(); /* destroy the DB */

  pRDF->Release(); /* shutdown the RDF system */

  return( 0 );
}

