/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Terry Hayes <thayes@netscape.com>
 *   Javier Delgadillo <javi@netscape.com>
 *   Oleg Romashin <romaxa@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/**
 * Derived from nsNSSDialogs http://landfill.mozilla.org/mxr-test/seamonkey/source/security/manager/pki/src/nsNSSDialogs.h
 */
#ifndef __EmbedCertificates_h
#define __EmbedCertificates_h
#include "nsITokenPasswordDialogs.h"
#include "nsIBadCertListener.h"
#ifdef BAD_CERT_LISTENER2
#include "nsIBadCertListener2.h"
#endif
#include "nsICertificateDialogs.h"
#include "nsIClientAuthDialogs.h"
#include "nsICertPickDialogs.h"
#include "nsITokenDialogs.h"
#include "nsIDOMCryptoDialogs.h"
#include "nsIGenKeypairInfoDlg.h"
#include "nsCOMPtr.h"
#include "nsIStringBundle.h"
#define EMBED_CERTIFICATES_CID \
  { 0x518e071f, 0x1dd2, 0x11b2, \
  { 0x93, 0x7e, 0xc4, 0x5f, 0x14, 0xde, 0xf7, 0x78 }}
#define EMBED_CERTIFICATES_DESCRIPTION "Certificates Listener Impl"
class EmbedPrivate;
class EmbedCertificates
: public nsITokenPasswordDialogs,
  public nsIBadCertListener,
#ifdef BAD_CERT_LISTENER2
  public nsIBadCertListener2,
#endif
  public nsICertificateDialogs,
  public nsIClientAuthDialogs,
  public nsICertPickDialogs,
  public nsITokenDialogs,
  public nsIDOMCryptoDialogs,
  public nsIGeneratingKeypairInfoDialogs
{
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSITOKENPASSWORDDIALOGS
    NS_DECL_NSIBADCERTLISTENER
#ifdef BAD_CERT_LISTENER2
    NS_DECL_NSIBADCERTLISTENER2
#endif
    NS_DECL_NSICERTIFICATEDIALOGS
    NS_DECL_NSICLIENTAUTHDIALOGS
    NS_DECL_NSICERTPICKDIALOGS
    NS_DECL_NSITOKENDIALOGS
    NS_DECL_NSIDOMCRYPTODIALOGS
    NS_DECL_NSIGENERATINGKEYPAIRINFODIALOGS
    EmbedCertificates();
    virtual ~EmbedCertificates();
    nsresult Init(void);
  protected:
  nsCOMPtr<nsIStringBundle> mPIPStringBundle;
};
#endif /* __EmbedCertificates_h */
