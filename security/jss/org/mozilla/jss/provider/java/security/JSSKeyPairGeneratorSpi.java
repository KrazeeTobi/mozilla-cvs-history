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
 * Portions created by the Initial Developer are Copyright (C) 2001
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

import org.mozilla.jss.crypto.*;
import java.security.KeyPair;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;
import java.security.InvalidAlgorithmParameterException;


class JSSKeyPairGeneratorSpi
    extends java.security.KeyPairGeneratorSpi
{

    private KeyPairGenerator kpg;

    private JSSKeyPairGeneratorSpi() { super(); }

    protected JSSKeyPairGeneratorSpi(KeyPairAlgorithm alg) {
        super();
        CryptoToken token =
            TokenSupplierManager.getTokenSupplier().getThreadToken();
        try {
          try {
            kpg = token.getKeyPairGenerator(alg);
          } catch(java.security.NoSuchAlgorithmException e) {
            throw new UnsupportedOperationException(
                "Token '" + token.getName() + "' does not support algorithm " +
                alg.toString());
          }
        } catch(TokenException e) {
            throw new TokenRuntimeException(e.getMessage());
        }
    }

    public void initialize(AlgorithmParameterSpec params,
        SecureRandom random) throws InvalidAlgorithmParameterException
    {
        kpg.initialize(params, random);
    }

    public void initialize(int keysize, SecureRandom random) {
        kpg.initialize(keysize, random);
    }
        
    public KeyPair generateKeyPair()  {
      try {
        return kpg.genKeyPair();
      } catch(TokenException e) { 
        throw new TokenRuntimeException(e.getMessage());
      }
    }

    public static class RSA extends JSSKeyPairGeneratorSpi {
        public RSA() {
            super(KeyPairAlgorithm.RSA);
        }
    }
    public static class DSA extends JSSKeyPairGeneratorSpi {
        public DSA() {
            super(KeyPairAlgorithm.DSA);
        }
    }
    public static class EC extends JSSKeyPairGeneratorSpi {
        public EC() {
            super(KeyPairAlgorithm.EC);
        }
    }
}
