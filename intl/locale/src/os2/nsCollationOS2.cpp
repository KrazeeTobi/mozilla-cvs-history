/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Mozilla OS/2 libraries.
 *
 * The Initial Developer of the Original Code is John Fairhurst,
 * <john_fairhurst@iname.com>.  Portions created by John Fairhurst are
 * Copyright (C) 1999 John Fairhurst. All Rights Reserved.
 *
 * Contributor(s): 
 *
 */

#define INCL_DOS
#include <os2.h>
#include "unidef.h"

#include "nscore.h"
#include "nsCollationOS2.h"

#include "nsIOS2Locale.h"

static NS_DEFINE_IID(kILocaleOS2IID, NS_IOS2LOCALE_IID);
static NS_DEFINE_IID(kICollationIID, NS_ICOLLATION_IID);
static NS_DEFINE_IID(kICollationFactoryIID, NS_ICOLLATIONFACTORY_IID);

// nsISupports implementation
NS_IMPL_ISUPPORTS(nsCollationOS2, kICollationIID)

// init/term
nsCollationOS2::nsCollationOS2() : mLocaleObject(0), mLocale(nsnull)
{
   NS_INIT_REFCNT();
}

nsCollationOS2::~nsCollationOS2()
{
   NS_IF_RELEASE(mLocale);
}

nsresult nsCollationOS2::Initialize( nsILocale *aLocale)
{
   nsIOS2Locale *mOS2Locale = nsnull;
   nsresult rc = aLocale->QueryInterface( kILocaleOS2IID,
                                          (void**) &mOS2Locale);
   if( NS_SUCCEEDED(rc))
   {
      mLocale = aLocale;
      NS_ADDREF(mLocale);
      mOS2Locale->GetLocaleObject( &mLocaleObject);
      NS_RELEASE(mOS2Locale);
   }

   return rc;
}

// sort-key creation
// get a length (of character) of a sort key to be generated by an
// input string length is a byte length 
nsresult nsCollationOS2::GetSortKeyLen( const nsCollationStrength  aStrength, 
                                        const nsString            &aStringIn,
                                        PRUint32                  *aOutLen)
{
   if( !aOutLen)
      return NS_ERROR_NULL_POINTER;

   // XXXX M15 LAUNCH HACK: TODO: track down why we get here before Initialize has been called...OS2TODO 
   if (!mLocaleObject)
     UniCreateLocaleObject(UNI_UCS_STRING_POINTER, (UniChar*)L"", &mLocaleObject);

   size_t num_elems = UniStrxfrm( mLocaleObject, nsnull,
                                  aStringIn.GetUnicode(), 0);

   *aOutLen = 2 * num_elems; // out is in bytes

   return NS_OK;
}

// create sort key from input string
// length is a byte length, caller should allocate a memory for a key
nsresult nsCollationOS2::CreateRawSortKey( const nsCollationStrength aStrength, 
                                           const nsString           &aStringIn,
                                           PRUint8                  *aKey,
                                           PRUint32                 *aOutLen)
{
   if( !aKey || !aOutLen)
      return NS_ERROR_NULL_POINTER;

   UniStrxfrm( mLocaleObject, (UniChar*) aKey,
               aStringIn.GetUnicode(), *aOutLen / 2 + 1);

   // XXX hmm
   if( aStrength & kCollationCaseInsensitiveAscii)
   {
#if 0
      // XXX this is correct, but is it guaranteed not to alter the
      //     length of key needed?  Ought we to do this in the 'getkeylen'
      //     function above as well?
      //
      // XXX Don't currently support the 'no accent' comparison; may have
      //     to bite the bullet & use the mozilla function, if it ever
      //     arrives.
      //
      XformObject xform_object = 0;
      int         iSize = -1, oSize = aStringIn.Length() + 1;
      UniChar    *xformed = new UniChar [ oSize];

      UniCreateTransformObject( mLocaleObject, L"lower", &xform_object);
      UniTransformStr( xform_object,
                       (const UniChar *) aStringIn.GetUnicode(),
                       &iSize, xformed, &oSize);
      UniFreeTransformObject( xform_object);
      UniStrxfrm( mLocaleObject, (UniChar*) aKey, xformed, *aOutLen / 2 + 1);
      delete [] xformed;
#else
      UniStrlwr( (UniChar*) aKey);
#endif
   }

   return NS_OK;
}

// compare two sort keys
// length is a byte length, result is same as strcmp
nsresult nsCollationOS2::CompareRawSortKey( const PRUint8  *aKey1,
                                            const PRUint32  aLen1, 
                                            const PRUint8  *aKey2,
                                            const PRUint32  aLen2, 
                                            PRInt32        *aResult)
{
   if( !aKey1 || !aKey2 || !aResult)
      return NS_ERROR_NULL_POINTER;

   *aResult = UniStrcmp( (UniChar*) aKey1, (UniChar*) aKey2);

   return NS_OK;
}

// create a sort key (nsString)
nsresult nsCollationOS2::CreateSortKey( const nsCollationStrength  aStrength, 
                                        const nsString            &aStringIn,
                                        nsString                  &aKey)
{
   PRUint32 length = 0;
   PRUint8 *key = nsnull;

   nsresult rc = GetSortKeyLen( aStrength, aStringIn, &length);

   if( NS_SUCCEEDED(rc))
   {
      key = new PRUint8[ length];
      CreateRawSortKey( aStrength, aStringIn, key, &length);
      aKey.SetString( (PRUnichar*) key);
      delete [] key;
   }
 
   return rc;
}

// compare two sort keys (nsString)
nsresult nsCollationOS2::CompareSortKey( const nsString &aKey1,
                                         const nsString &aKey2,
                                         PRInt32        *aResult)
{
   return CompareRawSortKey( (const PRUint8 *) aKey1.GetUnicode(),
                             (aKey1.Length() + 1) * 2,
                             (const PRUint8 *) aKey2.GetUnicode(),
                             (aKey2.Length() + 1) * 2,
                             aResult);
}

// compare two strings; result is same as strcmp
nsresult nsCollationOS2::CompareString( const nsCollationStrength  aStrength, 
                                        const nsString            &aString1,
                                        const nsString            &aString2,
                                        PRInt32                   *aResult)
{
   if( !aResult)
      return NS_ERROR_NULL_POINTER;

   nsString key1, key2;

   CreateSortKey( aStrength, aString1, key1);
   CreateSortKey( aStrength, aString2, key2);

   return CompareSortKey( key1, key2, aResult);
}

// nsCollationFactoryOS2 implementation

// nsISupports implementation
NS_IMPL_ISUPPORTS(nsCollationFactoryOS2,kICollationFactoryIID)

// init/term
nsCollationFactoryOS2::nsCollationFactoryOS2()
{
   NS_INIT_REFCNT();
}

nsCollationFactoryOS2::~nsCollationFactoryOS2()
{
}

// strange factory method
nsresult nsCollationFactoryOS2::CreateCollation( nsILocale *aLocale,
                                                 nsICollation **aInstance)
{
   if( !aLocale || !aInstance)
      return NS_ERROR_NULL_POINTER;

   *aInstance = nsnull;

   nsCollationOS2 *coll = new nsCollationOS2;

   if(nsnull == coll)
      return NS_ERROR_OUT_OF_MEMORY;

   nsresult rc = coll->Initialize( aLocale);

   if( NS_SUCCEEDED(rc))
   {
      NS_ADDREF(coll);
      *aInstance = coll;
   }
   else
      delete coll;

   return rc;
}
