/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape security libraries.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1994-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

#ifdef DEBUG
static const char CVS_ID[] = "@(#) $Source: /cvs/cvsroot/mozilla/security/nss/lib/pkix/src/Attribute/Attic/PEncode.c,v $ $Revision: 1.1 $ $Date: 2000/03/31 19:11:03 $ $Name:  $";
#endif /* DEBUG */

#ifndef PKIX_H
#include "pkix.h"
#endif /* PKIX_H */

#ifndef ASN1_H
#include "asn1.h"
#endif /* ASN1_H */

/*
 * nssPKIXAttribute_Encode
 *
 * This routine returns an ASN.1 encoding of the specified 
 * NSSPKIXAttribute.  {usual rules about itemOpt and arenaOpt}
 * This routine indicates an error (NSS_ERROR_INVALID_DATA) 
 * if there are no attribute values (i.e., the last one was removed).
 *
 * The error value may be one of the following values:
 *  NSS_ERROR_INVALID_PKIX_ATTRIBUTE
 *  NSS_ERROR_INVALID_ARENA
 *  NSS_ERROR_NO_MEMORY
 *  NSS_ERROR_INVALID_DATA
 *
 * Return value:
 *  A valid NSSBER pointer upon success
 *  NULL upon failure
 */

NSS_IMPLEMENT NSSBER *
nssPKIXAttribute_Encode
(
  NSSPKIXAttribute *a,
  NSSASN1EncodingType encoding,
  NSSBER *rvOpt,
  NSSArena *arenaOpt
)
{
  NSSBER *it;

#ifdef NSSDEBUG
  if( PR_SUCCESS != nssPKIXAttribute_verifyPointer(a) ) {
    return (NSSBER *)NULL;
  }

  if( (NSSArena *)NULL != arenaOpt ) {
    if( PR_SUCCESS != nssArena_verifyPointer(arenaOpt) ) {
      return (NSSBER *)NULL;
    }
  }
#endif /* NSSDEBUG */

  switch( encoding ) {
  case NSSASN1BER:
    if( (NSSBER *)NULL != a->ber ) {
      it = a->ber;
      goto done;
    }
    /*FALLTHROUGH*/
  case NSSASN1DER:
    if( (NSSDER *)NULL != a->der ) {
      it = a->der;
      goto done;
    }
    break;
  default:
    nss_SetError(NSS_ERROR_UNSUPPORTED_ENCODING);
    return (NSSBER *)NULL;
  }

  it = nssASN1_EncodeItem(a->arena, (NSSItem *)NULL, a, 
                          nssPKIXAttribute_template, encoding);
  if( (NSSBER *)NULL == it ) {
    return (NSSBER *)NULL;
  }

  switch( encoding ) {
  case NSSASN1BER:
    a->ber = it;
    break;
  case NSSASN1DER:
    a->der = it;
    break;
  default:
    PR_ASSERT(0);
    break;
  }

 done:
  return nssItem_Duplicate(it, arenaOpt, rvOpt);
}
