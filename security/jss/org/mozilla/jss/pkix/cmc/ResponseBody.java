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
 * The Original Code is the Netscape Security Services for Java.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
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

package com.netscape.jss.pkix.cmc;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import com.netscape.jss.asn1.*;
import com.netscape.jss.pkix.primitive.*;
import com.netscape.jss.util.Assert;

/**
 * A ResponseBody for CMC full enrollment request.
 *  ResponseBody ::= SEQUENCE { 
 *        controlSequence    SEQUENCE SIZE(0..MAX) OF TaggedAttribute, 
 *        cmsSequence        SEQUENCE SIZE(0..MAX) OF TaggedContentInfo, 
 *        otherMsgSequence   SEQUENCE SIZE(0..MAX) OF OtherMsg 
 *  } 
 */
public class ResponseBody implements ASN1Value {

    ///////////////////////////////////////////////////////////////////////
    // Members
    ///////////////////////////////////////////////////////////////////////
    private SEQUENCE sequence;
    private SEQUENCE controlSequence;
	private SEQUENCE cmsSequence;
	private SEQUENCE otherMsgSequence;

    ///////////////////////////////////////////////////////////////////////
    // Construction
    ///////////////////////////////////////////////////////////////////////

    // no default constructor
    private ResponseBody() { }

    /** 
     * Constructs a ResponseBody from its components.
     *
     * @param controlSequence Sequence of TagggedAttribute.
     * @param cmsSequence Sequence of TagggedContentInfo.
     * @param otherMsgSequence Sequence of OtherMsg.
     */
    public ResponseBody(SEQUENCE controlSequence, SEQUENCE
			cmsSequence, SEQUENCE otherMsgSequence) {
        sequence = new SEQUENCE();
        this.controlSequence = controlSequence;
        sequence.addElement(controlSequence);
        this.cmsSequence = cmsSequence;
        sequence.addElement(cmsSequence);
        this.otherMsgSequence = otherMsgSequence;
        sequence.addElement(otherMsgSequence);
    }


    ///////////////////////////////////////////////////////////////////////
    // accessors
    ///////////////////////////////////////////////////////////////////////

    public SEQUENCE getControlSequence() {
        return controlSequence;
    }

    public SEQUENCE getCmsSequence() {
        return cmsSequence;
    }

    public SEQUENCE getOtherMsgSequence() {
        return otherMsgSequence;
    }


    ///////////////////////////////////////////////////////////////////////
    // DER encoding/decoding
    ///////////////////////////////////////////////////////////////////////
    static Tag TAG = SEQUENCE.TAG;
    public Tag getTag() {
        return TAG;
    }

    public void encode(OutputStream ostream) throws IOException {
        encode(TAG, ostream);
    }

    public void encode(Tag implicitTag, OutputStream ostream)
        throws IOException
    {
        sequence.encode(implicitTag, ostream);
    }

    private static final Template templateInstance = new Template();
    public static Template getTemplate() {
        return templateInstance;
    }

    /**
     * A template for decoding an ResponseBody from its BER encoding.
     */
    public static class Template implements ASN1Template {
        private SEQUENCE.Template seqt;

        public Template() {
            seqt = new SEQUENCE.Template();
            seqt.addElement(new SEQUENCE.OF_Template(TaggedAttribute.getTemplate()) );
            seqt.addElement(new SEQUENCE.OF_Template(ANY.getTemplate()) );
            seqt.addElement(new SEQUENCE.OF_Template(ANY.getTemplate()) );
        }

        public boolean tagMatch(Tag tag) {
            return TAG.equals(tag);
        }

        public ASN1Value decode(InputStream istream)
            throws InvalidBERException, IOException
        {
            return decode(TAG, istream);
        }

        public ASN1Value decode(Tag implicitTag, InputStream istream)
            throws InvalidBERException, IOException
        {
            SEQUENCE seq = (SEQUENCE) seqt.decode(implicitTag, istream);

            Assert.assert(seq.size() == 3);

            return new ResponseBody(
                            (SEQUENCE)      seq.elementAt(0),
                            (SEQUENCE)      seq.elementAt(1),
                            (SEQUENCE)      seq.elementAt(2));
        }
    }
}

