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
 * Copyright (C) 2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 *  Ian McGreer <mcgreer@netscape.com>
 *  Javier Delgadillo <javi@netscape.com>
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
 *
 */

#include "nsNSSComponent.h"
#include "nsNSSCertificateDB.h"
#include "nsCOMPtr.h"
#include "nsNSSCertificate.h"
#include "nsNSSHelper.h"
#include "nsNSSCertHelper.h"
#include "nsNSSCertCache.h"
#include "nsCRT.h"
#include "nsICertificateDialogs.h"
#include "nsNSSCertTrust.h"
#include "nsILocalFile.h"
#include "nsPKCS12Blob.h"
#include "nsPK11TokenDB.h"
#include "nsOCSPResponder.h"
#include "nsReadableUtils.h"
#include "nsIWindowWatcher.h"
#include "nsIPrompt.h"

#include "nspr.h"
extern "C" {
#include "pk11func.h"
#include "certdb.h"
#include "cert.h"
#include "secerr.h"
#include "nssb64.h"
#include "secasn1.h"
#include "secder.h"
}
#include "ssl.h"
#include "ocsp.h"
#include "plbase64.h"

#ifdef PR_LOGGING
extern PRLogModuleInfo* gPIPNSSLog;
#endif

#include "nsNSSCleaner.h"
NSSCleanupAutoPtrClass(CERTCertificate, CERT_DestroyCertificate)
NSSCleanupAutoPtrClass(CERTCertList, CERT_DestroyCertList)

static NS_DEFINE_CID(kNSSComponentCID, NS_NSSCOMPONENT_CID);


NS_IMPL_ISUPPORTS1(nsNSSCertificateDB, nsIX509CertDB)

nsNSSCertificateDB::nsNSSCertificateDB()
{
  NS_INIT_ISUPPORTS();
}

nsNSSCertificateDB::~nsNSSCertificateDB()
{
}

NS_IMETHODIMP
nsNSSCertificateDB::GetCertByNickname(nsISupports *aToken,
                                      const PRUnichar *nickname,
                                      nsIX509Cert **_rvCert)
{
  CERTCertificate *cert = NULL;
  char *asciiname = NULL;
  NS_ConvertUCS2toUTF8 aUtf8Nickname(nickname);
  asciiname = NS_CONST_CAST(char*, aUtf8Nickname.get());
  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("Getting \"%s\"\n", asciiname));
#if 0
  // what it should be, but for now...
  if (aToken) {
    cert = PK11_FindCertFromNickname(asciiname, NULL);
  } else {
    cert = CERT_FindCertByNickname(CERT_GetDefaultCertDB(), asciiname);
  }
#endif
  cert = PK11_FindCertFromNickname(asciiname, NULL);
  if (!cert) {
    cert = CERT_FindCertByNickname(CERT_GetDefaultCertDB(), asciiname);
  }
  if (cert) {
    PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("got it\n"));
    nsCOMPtr<nsIX509Cert> pCert = new nsNSSCertificate(cert);
    *_rvCert = pCert;
    NS_ADDREF(*_rvCert);
    return NS_OK;
  }
  *_rvCert = nsnull;
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP 
nsNSSCertificateDB::GetCertByDBKey(const char *aDBkey, nsISupports *aToken,
                                   nsIX509Cert **_cert)
{
  SECItem keyItem = {siBuffer, nsnull, 0};
  SECItem *dummy;
  CERTIssuerAndSN issuerSN;
  unsigned long moduleID,slotID;
  *_cert = nsnull; 
  if (!aDBkey) return NS_ERROR_FAILURE;
  dummy = NSSBase64_DecodeBuffer(nsnull, &keyItem, aDBkey,
                                 (PRUint32)PL_strlen(aDBkey)); 
  CERTCertificate *cert;

  // someday maybe we can speed up the search using the moduleID and slotID
  moduleID = NS_NSS_GET_LONG(keyItem.data);
  slotID = NS_NSS_GET_LONG(&keyItem.data[NS_NSS_LONG]);

  // build the issuer/SN structure
  issuerSN.serialNumber.len = NS_NSS_GET_LONG(&keyItem.data[NS_NSS_LONG*2]);
  issuerSN.derIssuer.len = NS_NSS_GET_LONG(&keyItem.data[NS_NSS_LONG*3]);
  issuerSN.serialNumber.data= &keyItem.data[NS_NSS_LONG*4];
  issuerSN.derIssuer.data= &keyItem.data[NS_NSS_LONG*4+
                                              issuerSN.serialNumber.len];

  cert = CERT_FindCertByIssuerAndSN(CERT_GetDefaultCertDB(), &issuerSN);
  PR_FREEIF(keyItem.data);
  if (cert) {
    nsNSSCertificate *nssCert = new nsNSSCertificate(cert);
    if (nssCert == nsnull)
      return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(nssCert);
    *_cert = NS_STATIC_CAST(nsIX509Cert*, nssCert);
  }
  return NS_OK;
}

NS_IMETHODIMP 
nsNSSCertificateDB::GetCertNicknames(nsISupports *aToken, 
                                     PRUint32      aType,
                                     PRUint32     *_count,
                                     PRUnichar  ***_certNames)
{
  nsresult rv = NS_ERROR_FAILURE;
  /*
   * obtain the cert list from NSS
   */
  CERTCertList *certList = NULL;
  PK11CertListType pk11type;
#if 0
  // this would seem right, but it didn't work...
  // oh, I know why - bonks out on internal slot certs
  if (aType == nsIX509Cert::USER_CERT)
    pk11type = PK11CertListUser;
  else 
#endif
    pk11type = PK11CertListUnique;
  certList = PK11_ListCerts(pk11type, NULL);
  if (!certList)
    goto cleanup;
  /*
   * get list of cert names from list of certs
   * XXX also cull the list (NSS only distinguishes based on user/non-user
   */
  getCertNames(certList, aType, _count, _certNames);
  rv = NS_OK;
  /*
   * finish up
   */
cleanup:
  if (certList)
    CERT_DestroyCertList(certList);
  return rv;
}

SECStatus PR_CALLBACK
collect_certs(void *arg, SECItem **certs, int numcerts)
{
  CERTDERCerts *collectArgs;
  SECItem *cert;
  SECStatus rv;

  collectArgs = (CERTDERCerts *)arg;

  collectArgs->numcerts = numcerts;
  collectArgs->rawCerts = (SECItem *) PORT_ArenaZAlloc(collectArgs->arena,
                                           sizeof(SECItem) * numcerts);
  if ( collectArgs->rawCerts == NULL )
    return(SECFailure);

  cert = collectArgs->rawCerts;

  while ( numcerts-- ) {
    rv = SECITEM_CopyItem(collectArgs->arena, cert, *certs);
    if ( rv == SECFailure )
      return(SECFailure);
    cert++;
    certs++;
  }

  return (SECSuccess);
}

CERTDERCerts*
nsNSSCertificateDB::getCertsFromPackage(PRArenaPool *arena, PRUint8 *data, 
                                        PRUint32 length)
{
  CERTDERCerts *collectArgs = 
               (CERTDERCerts *)PORT_ArenaZAlloc(arena, sizeof(CERTDERCerts));
  if ( collectArgs == nsnull ) 
    return nsnull;

  collectArgs->arena = arena;
  SECStatus sec_rv = CERT_DecodeCertPackage(NS_REINTERPRET_CAST(char *, data), 
                                            length, collect_certs, 
                                            (void *)collectArgs);
  if (sec_rv != SECSuccess)
    return nsnull;

  return collectArgs;
}

nsresult
nsNSSCertificateDB::handleCACertDownload(nsISupportsArray *x509Certs,
                                         nsIInterfaceRequestor *ctx)
{
  // First thing we have to do is figure out which certificate we're 
  // gonna present to the user.  The CA may have sent down a list of 
  // certs which may or may not be a chained list of certs.  Until
  // the day we can design some solid UI for the general case, we'll
  // code to the > 90% case.  That case is where a CA sends down a
  // list that is a chain up to its root in either ascending or 
  // descending order.  What we're gonna do is compare the first 
  // 2 entries, if the first was signed by the second, we assume
  // the leaf cert is the first cert and display it.  If the second
  // cert was signed by the first cert, then we assume the first cert
  // is the root and the last cert in the array is the leaf.  In this
  // case we display the last cert.
  PRUint32 numCerts;

  x509Certs->Count(&numCerts);
  NS_ASSERTION(numCerts > 0, "Didn't get any certs to import.");
  if (numCerts == 0)
    return NS_OK; // Nothing to import, so nothing to do.

  nsCOMPtr<nsIX509Cert> certToShow;
  nsCOMPtr<nsISupports> isupports;
  PRUint32 selCertIndex;
  if (numCerts == 1) {
    // There's only one cert, so let's show it.
    selCertIndex = 0;
    isupports = dont_AddRef(x509Certs->ElementAt(selCertIndex));
    certToShow = do_QueryInterface(isupports);
  } else {
    nsCOMPtr<nsIX509Cert> cert0;
    nsCOMPtr<nsIX509Cert> cert1;

    isupports = dont_AddRef(x509Certs->ElementAt(0));
    cert0 = do_QueryInterface(isupports);

    isupports = dont_AddRef(x509Certs->ElementAt(1));
    cert1 = do_QueryInterface(isupports);

    nsXPIDLString cert0SubjectName;
    nsXPIDLString cert0IssuerName;
    nsXPIDLString cert1SubjectName;
    nsXPIDLString cert1IssuerName;

    cert0->GetIssuerName(getter_Copies(cert0IssuerName));
    cert0->GetSubjectName(getter_Copies(cert0SubjectName));

    cert1->GetIssuerName(getter_Copies(cert1IssuerName));
    cert1->GetSubjectName(getter_Copies(cert1SubjectName));

    if (nsCRT::strcmp(cert1IssuerName.get(), cert0SubjectName.get()) == 0) {
      // In this case, the first cert in the list signed the second,
      // so the first cert is the root.  Let's display the last cert 
      // in the list.
      selCertIndex = numCerts-1;
      isupports = dont_AddRef(x509Certs->ElementAt(selCertIndex));
      certToShow = do_QueryInterface(isupports);
    } else 
    if (nsCRT::strcmp(cert0IssuerName.get(), cert1SubjectName.get()) == 0) { 
      // In this case the second cert has signed the first cert.  The 
      // first cert is the leaf, so let's display it.
      selCertIndex = 0;
      certToShow = cert0;
    } else {
      // It's not a chain, so let's just show the first one in the 
      // downloaded list.
      selCertIndex = 0;
      certToShow = cert0;
    }
  }

  if (!certToShow)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsICertificateDialogs> dialogs;
  nsresult rv = ::getNSSDialogs(getter_AddRefs(dialogs), 
                                NS_GET_IID(nsICertificateDialogs),
                                NS_CERTIFICATEDIALOGS_CONTRACTID);
                       
  if (NS_FAILED(rv))
    return rv;
 
  SECItem der;
  rv=certToShow->GetRawDER(&der.len, (PRUint8 **)&der.data);

  if (NS_FAILED(rv))
    return rv;

  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("Creating temp cert\n"));
  CERTCertificate *tmpCert;
  CERTCertDBHandle *certdb = CERT_GetDefaultCertDB();
  tmpCert = CERT_FindCertByDERCert(certdb, &der);
  if (!tmpCert) {
    tmpCert = CERT_NewTempCertificate(certdb, &der,
                                      nsnull, PR_FALSE, PR_TRUE);
  }
  if (!tmpCert) {
    NS_ASSERTION(0,"Couldn't create cert from DER blob\n");
    return NS_ERROR_FAILURE;
  }

  CERTCertificateCleaner tmpCertCleaner(tmpCert);

  PRBool canceled;
  if (tmpCert->isperm) {
    dialogs->CACertExists(ctx, &canceled);
    return NS_ERROR_FAILURE;
  }

  PRUint32 trustBits;
  rv = dialogs->DownloadCACert(ctx, certToShow, &trustBits, &canceled);
  if (NS_FAILED(rv))
    return rv;

  if (canceled)
    return NS_ERROR_NOT_AVAILABLE;

  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("trust is %d\n", trustBits));
  nsXPIDLCString nickname;
  nickname.Adopt(CERT_MakeCANickname(tmpCert));

  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("Created nick \"%s\"\n", nickname.get()));

  nsNSSCertTrust trust;
  trust.SetValidCA();
  trust.AddCATrust(trustBits & nsIX509CertDB::TRUSTED_SSL,
                   trustBits & nsIX509CertDB::TRUSTED_EMAIL,
                   trustBits & nsIX509CertDB::TRUSTED_OBJSIGN);

  SECStatus srv = CERT_AddTempCertToPerm(tmpCert, 
                                         NS_CONST_CAST(char*,nickname.get()), 
                                         trust.GetTrust()); 

  if (srv != SECSuccess)
    return NS_ERROR_FAILURE;

  // Now it's time to add the rest of the certs we just downloaded.
  // Since we didn't prompt the user about any of these certs, we
  // won't set any trust bits for them.
  nsNSSCertTrust defaultTrust;
  defaultTrust.SetValidCA();
  defaultTrust.AddCATrust(0,0,0);
  for (PRUint32 i=0; i<numCerts; i++) {
    if (i == selCertIndex)
      continue;

    isupports = dont_AddRef(x509Certs->ElementAt(i));
    certToShow = do_QueryInterface(isupports);
    certToShow->GetRawDER(&der.len, (PRUint8 **)&der.data);

    CERTCertificate *tmpCert2 = 
      CERT_NewTempCertificate(certdb, &der, nsnull, PR_FALSE, PR_TRUE);

    if (!tmpCert2) {
      NS_ASSERTION(0, "Couldn't create temp cert from DER blob\n");
      continue;  // Let's try to import the rest of 'em
    }
    nickname.Adopt(CERT_MakeCANickname(tmpCert2));
    CERT_AddTempCertToPerm(tmpCert2, NS_CONST_CAST(char*,nickname.get()), 
                           defaultTrust.GetTrust());
    CERT_DestroyCertificate(tmpCert2);
  }
  
  return NS_OK;  
}

/*
 *  [noscript] void importCertificates(in charPtr data, in unsigned long length,
 *                                     in unsigned long type, 
 *                                     in nsIInterfaceRequestor ctx);
 */
NS_IMETHODIMP 
nsNSSCertificateDB::ImportCertificates(PRUint8 * data, PRUint32 length, 
                                       PRUint32 type, 
                                       nsIInterfaceRequestor *ctx)

{
  nsresult nsrv;

  PRArenaPool *arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
  if (!arena)
    return NS_ERROR_OUT_OF_MEMORY;

  CERTDERCerts *certCollection = getCertsFromPackage(arena, data, length);
  if (!certCollection) {
    PORT_FreeArena(arena, PR_FALSE);
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<nsISupportsArray> array;
  nsresult rv = NS_NewISupportsArray(getter_AddRefs(array));
  if (NS_FAILED(rv)) {
    PORT_FreeArena(arena, PR_FALSE);
    return rv;
  }

  // Now let's create some certs to work with
  nsCOMPtr<nsIX509Cert> x509Cert;
  nsNSSCertificate *nssCert;
  SECItem *currItem;
  for (int i=0; i<certCollection->numcerts; i++) {
     currItem = &certCollection->rawCerts[i];
     nssCert = nsNSSCertificate::ConstructFromDER((char*)currItem->data, currItem->len);
     if (!nssCert)
       return NS_ERROR_FAILURE;
     x509Cert = do_QueryInterface((nsIX509Cert*)nssCert);
     array->AppendElement(x509Cert);
  }
  switch (type) {
  case nsIX509Cert::CA_CERT:
    nsrv = handleCACertDownload(array, ctx);
    break;
  default:
    // We only deal with import CA certs in this method currently.
     nsrv = NS_ERROR_FAILURE;
     break;
  }  
  PORT_FreeArena(arena, PR_FALSE);
  return nsrv;
}


/*
 *  [noscript] void importEmailCertificates(in charPtr data, in unsigned long length,
 *                                     in nsIInterfaceRequestor ctx);
 */
NS_IMETHODIMP
nsNSSCertificateDB::ImportEmailCertificate(PRUint8 * data, PRUint32 length, 
                                       nsIInterfaceRequestor *ctx)

{
  SECStatus srv = SECFailure;
  nsresult nsrv = NS_OK;
  CERTCertificate * cert;
  SECItem **rawCerts;
  int numcerts;
  int i;
 
  PRArenaPool *arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
  if (!arena)
    return NS_ERROR_OUT_OF_MEMORY;

  CERTDERCerts *certCollection = getCertsFromPackage(arena, data, length);
  if (!certCollection) {
    PORT_FreeArena(arena, PR_FALSE);
    return NS_ERROR_FAILURE;
  }
  cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), certCollection->rawCerts,
                          (char *)NULL, PR_FALSE, PR_TRUE);
  if (!cert) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }
  numcerts = certCollection->numcerts;
  rawCerts = (SECItem **) PORT_Alloc(sizeof(SECItem *) * numcerts);
  if ( !rawCerts ) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }

  for ( i = 0; i < numcerts; i++ ) {
    rawCerts[i] = &certCollection->rawCerts[i];
  }
 
  srv = CERT_ImportCerts(CERT_GetDefaultCertDB(), certUsageEmailSigner,
             numcerts, rawCerts, NULL, PR_TRUE, PR_FALSE,
             NULL);
  if ( srv != SECSuccess ) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }
  srv = CERT_SaveSMimeProfile(cert, NULL, NULL);
  PORT_Free(rawCerts);
loser:
  if (arena) 
    PORT_FreeArena(arena, PR_TRUE);
  return nsrv;
}

NS_IMETHODIMP
nsNSSCertificateDB::ImportServerCertificate(PRUint8 * data, PRUint32 length, 
                                            nsIInterfaceRequestor *ctx)

{
  SECStatus srv = SECFailure;
  nsresult nsrv = NS_OK;
  CERTCertificate * cert;
  SECItem **rawCerts = nsnull;
  int numcerts;
  int i;
  nsNSSCertTrust trust;
  char *serverNickname = nsnull;
 
  PRArenaPool *arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
  if (!arena)
    return NS_ERROR_OUT_OF_MEMORY;

  CERTDERCerts *certCollection = getCertsFromPackage(arena, data, length);
  if (!certCollection) {
    PORT_FreeArena(arena, PR_FALSE);
    return NS_ERROR_FAILURE;
  }
  cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), certCollection->rawCerts,
                          (char *)NULL, PR_FALSE, PR_TRUE);
  if (!cert) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }
  numcerts = certCollection->numcerts;
  rawCerts = (SECItem **) PORT_Alloc(sizeof(SECItem *) * numcerts);
  if ( !rawCerts ) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }

  for ( i = 0; i < numcerts; i++ ) {
    rawCerts[i] = &certCollection->rawCerts[i];
  }

  serverNickname = nsNSSCertificate::defaultServerNickname(cert);
  srv = CERT_ImportCerts(CERT_GetDefaultCertDB(), certUsageSSLServer,
             numcerts, rawCerts, NULL, PR_TRUE, PR_FALSE,
             serverNickname);
  PR_FREEIF(serverNickname);
  if ( srv != SECSuccess ) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }

  trust.SetValidServerPeer();
  srv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), cert, trust.GetTrust());
  if ( srv != SECSuccess ) {
    nsrv = NS_ERROR_FAILURE;
    goto loser;
  }
loser:
  PORT_Free(rawCerts);
  if (arena) 
    PORT_FreeArena(arena, PR_TRUE);
  return nsrv;
}

NS_IMETHODIMP 
nsNSSCertificateDB::ImportUserCertificate(PRUint8 *data, PRUint32 length, nsIInterfaceRequestor *ctx)
{
  PK11SlotInfo *slot;
  char * nickname = NULL;
  nsresult rv = NS_ERROR_FAILURE;
  int numCACerts;
  SECItem *CACerts;
  CERTDERCerts * collectArgs;
  PRArenaPool *arena;
  CERTCertificate * cert=NULL;

  arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
  if ( arena == NULL ) {
    goto loser;
  }

  collectArgs = getCertsFromPackage(arena, data, length);
  if (!collectArgs) {
    goto loser;
  }

  cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), collectArgs->rawCerts,
                	       (char *)NULL, PR_FALSE, PR_TRUE);
  if (!cert) {
    goto loser;
  }

  slot = PK11_KeyForCertExists(cert, NULL, ctx);
  if ( slot == NULL ) {
    goto loser;
  }
  PK11_FreeSlot(slot);

  /* pick a nickname for the cert */
  if (cert->nickname) {
	/* sigh, we need a call to look up other certs with this subject and
	 * identify nicknames from them. We can no longer walk down internal
	 * database structures  rjr */
  	nickname = cert->nickname;
  }
  else {
    nickname = default_nickname(cert, ctx);
  }

  /* user wants to import the cert */
  slot = PK11_ImportCertForKey(cert, nickname, ctx);
  if (!slot) {
    goto loser;
  }
  PK11_FreeSlot(slot);
  numCACerts = collectArgs->numcerts - 1;

  if (numCACerts) {
    CACerts = collectArgs->rawCerts+1;
    if ( ! CERT_ImportCAChain(CACerts, numCACerts, certUsageUserCertImport) ) {
      rv = NS_OK;
    }
  }
  
loser:
  if (arena) {
    PORT_FreeArena(arena, PR_FALSE);
  }
  if ( cert ) {
    CERT_DestroyCertificate(cert);
  }
  return rv;
}

/*
 * void deleteCertificate(in nsIX509Cert aCert);
 */
NS_IMETHODIMP 
nsNSSCertificateDB::DeleteCertificate(nsIX509Cert *aCert)
{
  nsNSSCertificate *nssCert = NS_STATIC_CAST(nsNSSCertificate*, aCert);
  CERTCertificate *cert = nssCert->GetCert();
  if (!cert) return NS_ERROR_FAILURE;
  SECStatus srv = SECSuccess;

  PRUint32 certType = getCertType(cert);
  nssCert->SetCertType(certType);
  if (NS_FAILED(nssCert->MarkForPermDeletion()))
  {
    return NS_ERROR_FAILURE;
  }

  if (cert->slot && certType != nsIX509Cert::USER_CERT) {
    // To delete a cert of a slot (builtin, most likely), mark it as
    // completely untrusted.  This way we keep a copy cached in the
    // local database, and next time we try to load it off of the 
    // external token/slot, we'll know not to trust it.  We don't 
    // want to do that with user certs, because a user may  re-store
    // the cert onto the card again at which point we *will* want to 
    // trust that cert if it chains up properly.
    nsNSSCertTrust trust(0, 0, 0);
    srv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), 
                               cert, trust.GetTrust());
  }
  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("cert deleted: %d", srv));
  return (srv) ? NS_ERROR_FAILURE : NS_OK;
}

/*
 * void setCertTrust(in nsIX509Cert cert,
 *                   in unsigned long type,
 *                   in unsigned long trust);
 */
NS_IMETHODIMP 
nsNSSCertificateDB::SetCertTrust(nsIX509Cert *cert, 
                                 PRUint32 type,
                                 PRUint32 trusted)
{
  SECStatus srv;
  nsNSSCertTrust trust;
  nsNSSCertificate *pipCert = NS_STATIC_CAST(nsNSSCertificate *, cert);
  CERTCertificate *nsscert = pipCert->GetCert();
  if (type == nsIX509Cert::CA_CERT) {
    // always start with untrusted and move up
    trust.SetValidCA();
    trust.AddCATrust(trusted & nsIX509CertDB::TRUSTED_SSL,
                     trusted & nsIX509CertDB::TRUSTED_EMAIL,
                     trusted & nsIX509CertDB::TRUSTED_OBJSIGN);
    srv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), 
                               nsscert,
                               trust.GetTrust());
  } else if (type == nsIX509Cert::SERVER_CERT) {
    // always start with untrusted and move up
    trust.SetValidPeer();
    trust.AddPeerTrust(trusted & nsIX509CertDB::TRUSTED_SSL, 0, 0);
    srv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), 
                               nsscert,
                               trust.GetTrust());
  } else if (type == nsIX509Cert::EMAIL_CERT) {
    // always start with untrusted and move up
    trust.SetValidPeer();
    trust.AddPeerTrust(0, trusted & nsIX509CertDB::TRUSTED_EMAIL, 0);
    srv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), 
                               nsscert,
                               trust.GetTrust());
  } else {
    // ignore user certs
    return NS_OK;
  }
  return (srv) ? NS_ERROR_FAILURE : NS_OK;
}

/*
 * boolean getCertTrust(in nsIX509Cert cert,
 *                      in unsigned long certType,
 *                      in unsigned long trustType);
 */
NS_IMETHODIMP 
nsNSSCertificateDB::GetCertTrust(nsIX509Cert *cert, 
                                 PRUint32 certType,
                                 PRUint32 trustType,
                                 PRBool *_isTrusted)
{
  SECStatus srv;
  nsNSSCertificate *pipCert = NS_STATIC_CAST(nsNSSCertificate *, cert);
  CERTCertificate *nsscert = pipCert->GetCert();
  CERTCertTrust nsstrust;
  srv = CERT_GetCertTrust(nsscert, &nsstrust);
  nsNSSCertTrust trust(&nsstrust);
  if (certType == nsIX509Cert::CA_CERT) {
    if (trustType & nsIX509CertDB::TRUSTED_SSL) {
      *_isTrusted = trust.HasTrustedCA(PR_TRUE, PR_FALSE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_EMAIL) {
      *_isTrusted = trust.HasTrustedCA(PR_FALSE, PR_TRUE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_OBJSIGN) {
      *_isTrusted = trust.HasTrustedCA(PR_FALSE, PR_FALSE, PR_TRUE);
    } else {
      return NS_ERROR_FAILURE;
    }
  } else if (certType == nsIX509Cert::SERVER_CERT) {
    if (trustType & nsIX509CertDB::TRUSTED_SSL) {
      *_isTrusted = trust.HasTrustedPeer(PR_TRUE, PR_FALSE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_EMAIL) {
      *_isTrusted = trust.HasTrustedPeer(PR_FALSE, PR_TRUE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_OBJSIGN) {
      *_isTrusted = trust.HasTrustedPeer(PR_FALSE, PR_FALSE, PR_TRUE);
    } else {
      return NS_ERROR_FAILURE;
    }
  } else if (certType == nsIX509Cert::EMAIL_CERT) {
    if (trustType & nsIX509CertDB::TRUSTED_SSL) {
      *_isTrusted = trust.HasTrustedPeer(PR_TRUE, PR_FALSE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_EMAIL) {
      *_isTrusted = trust.HasTrustedPeer(PR_FALSE, PR_TRUE, PR_FALSE);
    } else if (trustType & nsIX509CertDB::TRUSTED_OBJSIGN) {
      *_isTrusted = trust.HasTrustedPeer(PR_FALSE, PR_FALSE, PR_TRUE);
    } else {
      return NS_ERROR_FAILURE;
    }
  } /* user: ignore */
  return NS_OK;
}


NS_IMETHODIMP 
nsNSSCertificateDB::ImportCertsFromFile(nsISupports *aToken, 
                                        nsILocalFile *aFile,
                                        PRUint32 aType)
{
  switch (aType) {
    case nsIX509Cert::CA_CERT:
    case nsIX509Cert::EMAIL_CERT:
    case nsIX509Cert::SERVER_CERT:
      // good
      break;
    
    default:
      // not supported (yet)
      return NS_ERROR_FAILURE;
  }

  nsresult rv;
  PRFileDesc *fd = nsnull;

  rv = aFile->OpenNSPRFileDesc(PR_RDONLY, 0, &fd);

  if (NS_FAILED(rv))
    return rv;

  if (!fd)
    return NS_ERROR_FAILURE;

  PRFileInfo file_info;
  if (PR_SUCCESS != PR_GetOpenFileInfo(fd, &file_info))
    return NS_ERROR_FAILURE;
  
  unsigned char *buf = new unsigned char[file_info.size];
  if (!buf)
    return NS_ERROR_OUT_OF_MEMORY;
  
  PRInt32 bytes_obtained = PR_Read(fd, buf, file_info.size);
  PR_Close(fd);
  
  if (bytes_obtained != file_info.size)
    rv = NS_ERROR_FAILURE;
  else {
	  nsCOMPtr<nsIInterfaceRequestor> cxt = new PipUIContext();

    switch (aType) {
      case nsIX509Cert::CA_CERT:
        rv = ImportCertificates(buf, bytes_obtained, aType, cxt);
        break;
        
      case nsIX509Cert::SERVER_CERT:
        rv = ImportServerCertificate(buf, bytes_obtained, cxt);
        break;

      case nsIX509Cert::EMAIL_CERT:
        rv = ImportEmailCertificate(buf, bytes_obtained, cxt);
        break;
      
      default:
        break;
    }
  }

  delete [] buf;
  return rv;  
}

NS_IMETHODIMP 
nsNSSCertificateDB::ImportPKCS12File(nsISupports *aToken, 
                                     nsILocalFile *aFile)
{
  NS_ENSURE_ARG(aFile);
  nsPKCS12Blob blob;
  if (aToken) {
    nsCOMPtr<nsIPK11Token> t = do_QueryInterface(aToken);
    blob.SetToken(t);
  }
  return blob.ImportFromFile(aFile);
}

NS_IMETHODIMP 
nsNSSCertificateDB::ExportPKCS12File(nsISupports     *aToken, 
                                     nsILocalFile     *aFile,
                                     PRUint32          count,
                                     nsIX509Cert     **certs)
                                     //const PRUnichar **aCertNames)
{
  NS_ENSURE_ARG(aFile);
  nsPKCS12Blob blob;
  if (count == 0) return NS_OK;
  nsCOMPtr<nsIPK11Token> localRef;
  if (!aToken) {
    PK11SlotInfo *keySlot = PK11_GetInternalKeySlot();
    NS_ASSERTION(keySlot,"Failed to get the internal key slot");
    localRef = new nsPK11Token(keySlot);
    PK11_FreeSlot(keySlot);
  }
  else {
    localRef = do_QueryInterface(aToken);
  }
  blob.SetToken(localRef);
  //blob.LoadCerts(aCertNames, count);
  //return blob.ExportToFile(aFile);
  return blob.ExportToFile(aFile, certs, count);
}


static SECStatus PR_CALLBACK 
GetOCSPResponders (CERTCertificate *aCert,
                   SECItem         *aDBKey,
                   void            *aArg)
{
  nsISupportsArray *array = NS_STATIC_CAST(nsISupportsArray*, aArg);
  PRUnichar* nn = nsnull;
  PRUnichar* url = nsnull;
  char *serviceURL = nsnull;
  char *nickname = nsnull;
  PRUint32 i, count;
  nsresult rv;

  // Are we interested in this cert //
  if (!nsOCSPResponder::IncludeCert(aCert)) {
    return SECSuccess;
  }

  // Get the AIA and nickname //
  serviceURL = CERT_GetOCSPAuthorityInfoAccessLocation(aCert);
  if (serviceURL) {
	url = ToNewUnicode(NS_ConvertUTF8toUCS2(serviceURL));
  }

  nickname = aCert->nickname;
  nn = ToNewUnicode(NS_ConvertUTF8toUCS2(nickname));

  nsCOMPtr<nsIOCSPResponder> new_entry = new nsOCSPResponder(nn, url);

  // Sort the items according to nickname //
  rv = array->Count(&count);
  for (i=0; i < count; ++i) {
    nsCOMPtr<nsISupports> isupport = dont_AddRef(array->ElementAt(i));
    nsCOMPtr<nsIOCSPResponder> entry = do_QueryInterface(isupport);
    if (nsOCSPResponder::CompareEntries(new_entry, entry) < 0) {
      array->InsertElementAt(new_entry, i);
      break;
    }
  }
  if (i == count) {
    array->AppendElement(new_entry);
  }
  return SECSuccess;
}



/*
 * getOCSPResponders
 *
 * Export a set of certs and keys from the database to a PKCS#12 file.
*/
NS_IMETHODIMP 
nsNSSCertificateDB::GetOCSPResponders(nsISupportsArray ** aResponders)
{
  SECStatus sec_rv;
  nsCOMPtr<nsISupportsArray> respondersArray;
  nsresult rv = NS_NewISupportsArray(getter_AddRefs(respondersArray));
  if (NS_FAILED(rv)) {
    return rv;
  }

  sec_rv = PK11_TraverseSlotCerts(::GetOCSPResponders,
                                  respondersArray,
                                  nsnull);
  if (sec_rv != SECSuccess) {
    goto loser;
  }

  *aResponders = respondersArray;
  NS_IF_ADDREF(*aResponders);
  return NS_OK;
loser:
  return NS_ERROR_FAILURE;
}

/*
 * NSS Helper Routines (private to nsNSSCertificateDB)
 */

#define DELIM '\001'

/*
 * GetSortedNameList
 *
 * Converts a CERTCertList to a list of certificate names
 */
void
nsNSSCertificateDB::getCertNames(CERTCertList *certList,
                                 PRUint32      type, 
                                 PRUint32     *_count,
                                 PRUnichar  ***_certNames)
{
  nsresult rv;
  CERTCertListNode *node;
  PRUint32 numcerts = 0, i=0;
  PRUnichar **tmpArray = NULL;
  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("List of certs %d:\n", type));
  for (node = CERT_LIST_HEAD(certList);
       !CERT_LIST_END(node, certList);
       node = CERT_LIST_NEXT(node)) {
    if (getCertType(node->cert) == type) {
      numcerts++;
    }
  }
  PR_LOG(gPIPNSSLog, PR_LOG_DEBUG, ("num certs: %d\n", numcerts));
  int nc = (numcerts == 0) ? 1 : numcerts;
  tmpArray = (PRUnichar **)nsMemory::Alloc(sizeof(PRUnichar *) * nc);
  if (numcerts == 0) goto finish;
  for (node = CERT_LIST_HEAD(certList);
       !CERT_LIST_END(node, certList);
       node = CERT_LIST_NEXT(node)) {
    if (getCertType(node->cert) == type) {
      nsNSSCertificate pipCert(node->cert);
      char *dbkey = NULL;
      char *namestr = NULL;
      nsAutoString certstr;
      rv = pipCert.GetDbKey(&dbkey);
      nsAutoString keystr = NS_ConvertASCIItoUCS2(dbkey);
      PR_FREEIF(dbkey);
      if (type == nsIX509Cert::EMAIL_CERT) {
        namestr = node->cert->emailAddr;
      } else {
        namestr = node->cert->nickname;
        char *sc = strchr(namestr, ':');
        if (sc) *sc = DELIM;
      }
      nsAutoString certname = NS_ConvertASCIItoUCS2(namestr);
      certstr.Append(PRUnichar(DELIM));
      certstr += certname;
      certstr.Append(PRUnichar(DELIM));
      certstr += keystr;
      tmpArray[i++] = ToNewUnicode(certstr);
    }
  }
finish:
  *_count = numcerts;
  *_certNames = tmpArray;
}

/* somewhat follows logic of cert_list_include_cert from PSM 1.x */


/* readonly attribute boolean ocspOn; */
NS_IMETHODIMP 
nsNSSCertificateDB::GetOcspOn(PRBool *aOcspOn)
{
  nsCOMPtr<nsIPref> prefService = do_GetService(NS_PREF_CONTRACTID);

  PRInt32 ocspEnabled;
  prefService->GetIntPref("security.OCSP.enabled", &ocspEnabled);
  *aOcspOn = ( ocspEnabled == 0 ) ? PR_FALSE : PR_TRUE; 
  return NS_OK;
}

/* void disableOCSP (); */
NS_IMETHODIMP 
nsNSSCertificateDB::DisableOCSP()
{
  nsresult rv;
  nsCOMPtr<nsINSSComponent> nssComponent(do_GetService(kNSSComponentCID, &rv));
  if (NS_FAILED(rv))
    return rv;

  return nssComponent->DisableOCSP();
}

/* void enableOCSP (); */
NS_IMETHODIMP 
nsNSSCertificateDB::EnableOCSP()
{
  nsresult rv;
  nsCOMPtr<nsINSSComponent> nssComponent(do_GetService(kNSSComponentCID, &rv));
  if (NS_FAILED(rv))
    return rv;

  return nssComponent->EnableOCSP();
}

/* nsIX509Cert getDefaultEmailEncryptionCert (); */
NS_IMETHODIMP
nsNSSCertificateDB::GetEmailEncryptionCert(const PRUnichar* aNickname, nsIX509Cert **_retval)
{
  if (!aNickname || !_retval)
    return NS_ERROR_FAILURE;

  *_retval = 0;

  nsDependentString aDepNickname(aNickname);
  if (aDepNickname.IsEmpty())
    return NS_OK;

  nsresult rv = NS_OK;
  CERTCertificate *cert = 0;
  nsCOMPtr<nsIInterfaceRequestor> ctx = new PipUIContext();
  nsNSSCertificate *nssCert = nsnull;
  char *asciiname = NULL;
  NS_ConvertUCS2toUTF8 aUtf8Nickname(aDepNickname);
  asciiname = NS_CONST_CAST(char*, aUtf8Nickname.get());

  /* Find a good cert in the user's database */
  cert = CERT_FindUserCertByUsage(CERT_GetDefaultCertDB(), asciiname, 
           certUsageEmailRecipient, PR_TRUE, ctx);

  if (!cert) { goto loser; }  

  nssCert = new nsNSSCertificate(cert);
  if (nssCert == nsnull) {
    rv = NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(nssCert);

  *_retval = NS_STATIC_CAST(nsIX509Cert*, nssCert);

loser:
  if (cert) CERT_DestroyCertificate(cert);
  return rv;
}

/* nsIX509Cert getDefaultEmailSigningCert (); */
NS_IMETHODIMP
nsNSSCertificateDB::GetEmailSigningCert(const PRUnichar* aNickname, nsIX509Cert **_retval)
{
  if (!aNickname || !_retval)
    return NS_ERROR_FAILURE;

  *_retval = 0;

  nsDependentString aDepNickname(aNickname);
  if (aDepNickname.IsEmpty())
    return NS_OK;

  nsresult rv = NS_OK;
  CERTCertificate *cert = 0;
  nsCOMPtr<nsIInterfaceRequestor> ctx = new PipUIContext();
  nsNSSCertificate *nssCert = nsnull;
  char *asciiname = NULL;
  NS_ConvertUCS2toUTF8 aUtf8Nickname(aDepNickname);
  asciiname = NS_CONST_CAST(char*, aUtf8Nickname.get());

  /* Find a good cert in the user's database */
  cert = CERT_FindUserCertByUsage(CERT_GetDefaultCertDB(), asciiname, 
           certUsageEmailSigner, PR_TRUE, ctx);

  if (!cert) { goto loser; }  

  nssCert = new nsNSSCertificate(cert);
  if (nssCert == nsnull) {
    rv = NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(nssCert);

  *_retval = NS_STATIC_CAST(nsIX509Cert*, nssCert);

loser:
  if (cert) CERT_DestroyCertificate(cert);
  return rv;
}

NS_IMETHODIMP
nsNSSCertificateDB::GetCertByEmailAddress(nsISupports *aToken, const char *aEmailAddress, nsIX509Cert **_retval)
{
  CERTCertificate *any_cert = CERT_FindCertByNicknameOrEmailAddr(CERT_GetDefaultCertDB(), (char*)aEmailAddress);
  if (!any_cert)
    return NS_ERROR_FAILURE;

  CERTCertificateCleaner certCleaner(any_cert);
    
  // any_cert now contains a cert with the right subject, but it might not have the correct usage
  CERTCertList *certlist = CERT_CreateSubjectCertList(
    nsnull, CERT_GetDefaultCertDB(), &any_cert->derSubject, PR_Now(), PR_TRUE);
  if (!certlist)
    return NS_ERROR_FAILURE;  

  CERTCertListCleaner listCleaner(certlist);

  if (SECSuccess != CERT_FilterCertListByUsage(certlist, certUsageEmailRecipient, PR_FALSE))
    return NS_ERROR_FAILURE;
  
  if (CERT_LIST_END(CERT_LIST_HEAD(certlist), certlist))
    return NS_ERROR_FAILURE;
  
  nsNSSCertificate *nssCert = new nsNSSCertificate(CERT_LIST_HEAD(certlist)->cert);
  if (!nssCert)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(nssCert);
  *_retval = NS_STATIC_CAST(nsIX509Cert*, nssCert);
  return NS_OK;
}

/* nsIX509Cert constructX509FromBase64 (in string base64); */
NS_IMETHODIMP
nsNSSCertificateDB::ConstructX509FromBase64(const char * base64, nsIX509Cert **_retval)
{
  if (!_retval) {
    return NS_ERROR_FAILURE;
  }

  PRUint32 len = PL_strlen(base64);
  int adjust = 0;

  /* Compute length adjustment */
  if (base64[len-1] == '=') {
    adjust++;
    if (base64[len-2] == '=') adjust++;
  }

  nsresult rv = NS_OK;
  char *certDER = 0;
  PRInt32 lengthDER = 0;

  certDER = PL_Base64Decode(base64, len, NULL);
  if (!certDER || !*certDER) {
    rv = NS_ERROR_ILLEGAL_VALUE;
  }
  else {
    lengthDER = (len*3)/4 - adjust;

    SECItem secitem_cert;
    secitem_cert.type = siDERCertBuffer;
    secitem_cert.data = (unsigned char*)certDER;
    secitem_cert.len = lengthDER;

    CERTCertificate *cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), &secitem_cert, nsnull, PR_FALSE, PR_TRUE);

    if (!cert) {
      rv = NS_ERROR_FAILURE;
    }
    else {
      nsNSSCertificate *nsNSS = new nsNSSCertificate(cert);
      if (!nsNSS) {
        rv = NS_ERROR_OUT_OF_MEMORY;
      }
      else {
        nsresult rv = nsNSS->QueryInterface(NS_GET_IID(nsIX509Cert), (void**)_retval);

        if (NS_SUCCEEDED(rv) && *_retval) {
          NS_ADDREF(*_retval);
        }
        
        NS_RELEASE(nsNSS);
      }
      CERT_DestroyCertificate(cert);
    }
  }
  
  if (certDER) {
    nsCRT::free(certDER);
  }
  return rv;
}



char *
nsNSSCertificateDB::default_nickname(CERTCertificate *cert, nsIInterfaceRequestor* ctx)
{   
  nsresult rv;
  char *username = NULL;
  char *caname = NULL;
  char *nickname = NULL;
  char *tmp = NULL;
  int count;
  char *nickFmt=NULL, *nickFmtWithNum = NULL;
  CERTCertificate *dummycert;
  PK11SlotInfo *slot=NULL;
  CK_OBJECT_HANDLE keyHandle;
  nsAutoString tmpNickFmt;
  nsAutoString tmpNickFmtWithNum;

  CERTCertDBHandle *defaultcertdb = CERT_GetDefaultCertDB();
  nsCOMPtr<nsINSSComponent> nssComponent(do_GetService(kNSSComponentCID, &rv));
  if (NS_FAILED(rv)) goto loser; 

  username = CERT_GetCommonName(&cert->subject);
  if ( username == NULL ) 
    username = PL_strdup("");

  if ( username == NULL ) 
    goto loser;
    
  caname = CERT_GetOrgName(&cert->issuer);
  if ( caname == NULL ) 
    caname = PL_strdup("");
  
  if ( caname == NULL ) 
    goto loser;
  
  count = 1;
  nssComponent->GetPIPNSSBundleString(
                              NS_LITERAL_STRING("nick_template").get(),
                              tmpNickFmt);
  nickFmt = ToNewUTF8String(tmpNickFmt);

  nssComponent->GetPIPNSSBundleString(
                              NS_LITERAL_STRING("nick_template_with_num").get(),
                              tmpNickFmtWithNum);
  nickFmtWithNum = ToNewUTF8String(tmpNickFmtWithNum);


  nickname = PR_smprintf(nickFmt, username, caname);
  /*
   * We need to see if the private key exists on a token, if it does
   * then we need to check for nicknames that already exist on the smart
   * card.
   */
  slot = PK11_KeyForCertExists(cert, &keyHandle, ctx);
  if (slot == NULL) {
    goto loser;
  }
  if (!PK11_IsInternal(slot)) {
    tmp = PR_smprintf("%s:%s", PK11_GetTokenName(slot), nickname);
    PR_Free(nickname);
    nickname = tmp;
    tmp = NULL;
  }
  tmp = nickname;
  while ( 1 ) {	
    if ( count > 1 ) {
      nickname = PR_smprintf("%s #%d", tmp, count);
    }
  
    if ( nickname == NULL ) 
      goto loser;
 
    if (PK11_IsInternal(slot)) {
      /* look up the nickname to make sure it isn't in use already */
      dummycert = CERT_FindCertByNickname(defaultcertdb, nickname);
      
    } else {
      /*
       * Check the cert against others that already live on the smart 
       * card.
       */
      dummycert = PK11_FindCertFromNickname(nickname, ctx);
      if (dummycert != NULL) {
	/*
	 * Make sure the subject names are different.
	 */ 
	if (CERT_CompareName(&cert->subject, &dummycert->subject) == SECEqual)
	{
	  /*
	   * There is another certificate with the same nickname and
	   * the same subject name on the smart card, so let's use this
	   * nickname.
	   */
	  CERT_DestroyCertificate(dummycert);
	  dummycert = NULL;
	}
      }
    }
    if ( dummycert == NULL ) 
      goto done;
    
    /* found a cert, destroy it and loop */
    CERT_DestroyCertificate(dummycert);
    if (tmp != nickname) PR_Free(nickname);
    count++;
  } /* end of while(1) */
    
loser:
  if ( nickname ) {
    PR_Free(nickname);
  }
  nickname = NULL;
done:
  if ( caname ) {
    PR_Free(caname);
  }
  if ( username )  {
    PR_Free(username);
  }
  if (slot != NULL) {
      PK11_FreeSlot(slot);
      if (nickname != NULL) {
	      tmp = nickname;
	      nickname = strchr(tmp, ':');
	      if (nickname != NULL) {
	        nickname++;
	        nickname = PL_strdup(nickname);
	        PR_Free(tmp);
             tmp = nsnull;
	      } else {
	        nickname = tmp;
	        tmp = NULL;
	      }
      }
    }
    PR_FREEIF(tmp);
    return(nickname);
}
