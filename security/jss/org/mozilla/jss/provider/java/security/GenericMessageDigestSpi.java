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
 * The Original Code is Network Security Services for Java.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

package org.mozilla.jss.provider.java.security;

import java.security.DigestException;
import org.mozilla.jss.crypto.*;


abstract class GenericMessageDigestSpi {

    private JSSMessageDigest digest;

    private GenericMessageDigestSpi() { }

    protected GenericMessageDigestSpi(DigestAlgorithm alg) {
        super();
        CryptoToken token =
            TokenSupplierManager.getTokenSupplier().getThreadToken();
        try {
            try {
              digest = token.getDigestContext(alg);
            } catch(java.security.NoSuchAlgorithmException e) {
                throw new UnsupportedOperationException(
                    "Token '" + token.getName() + "' does not support " +
                    "algorithm " + alg.toString());
            }
        } catch(TokenException e) {
            throw new TokenRuntimeException(e.getMessage());
        } catch(DigestException e1) {
            throw new TokenRuntimeException(e1.getMessage());
        }
    }

    public Object clone() throws CloneNotSupportedException {
        throw new CloneNotSupportedException();
    }

    public byte[] engineDigest() {
      try {
        return digest.digest();
      } catch(java.security.DigestException de) {
        throw new TokenRuntimeException(de.getMessage());
      }
    }

    public int engineDigest(byte[] buf, int offset, int len)
        throws DigestException
    {
        return digest.digest(buf, offset, len);
    }

    public int engineGetDigestLength() {
        return digest.getOutputSize();
    }

    public void engineReset() {
      try {
        digest.reset();
      } catch(java.security.DigestException de) {
        throw new TokenRuntimeException(de.getMessage());
      }
    }

    public void engineUpdate(byte input) {
      try {
        digest.update(input);
      } catch(java.security.DigestException de) {
        throw new TokenRuntimeException(de.getMessage());
      }
    }

    public void engineUpdate(byte[] input, int offset, int len) {
      try {
        digest.update(input,offset,len);
      } catch(java.security.DigestException de) {
        throw new TokenRuntimeException(de.getMessage());
      }
    }
}
