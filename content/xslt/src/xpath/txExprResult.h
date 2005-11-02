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
 * The Original Code is TransforMiiX XSLT processor.
 * 
 * The Initial Developer of the Original Code is The MITRE Corporation.
 * Portions created by MITRE are Copyright (C) 1999 The MITRE Corporation.
 *
 * Portions created by Keith Visco as a Non MITRE employee,
 * (C) 1999 Keith Visco. All Rights Reserved.
 * 
 * Contributor(s): 
 * Keith Visco, kvisco@ziplink.net
 *   -- original author.
 * Larry Fitzpatrick, OpenText, lef@opentext.com
 *   -- changed constant short result types to enum
 *
 */

#ifndef TRANSFRMX_EXPRRESULT_H
#define TRANSFRMX_EXPRRESULT_H

#include "primitives.h"
#include "TxObject.h"
#include "nsString.h"

/*
 * ExprResult
 *
 * Classes Represented:
 * BooleanResult, ExprResult, NumberResult, StringResult
 *
 * Note: for NodeSet, see NodeSet.h
*/

class ExprResult : public TxObject {

public:

    //-- ResultTypes
    enum ResultType {
        NODESET,
        BOOLEAN,
        NUMBER,
        STRING,
        RESULT_TREE_FRAGMENT
    };

    virtual ~ExprResult() {};

    /*
     * Clones this ExprResult
     * @return clone of this ExprResult
     */
    virtual ExprResult* clone()      = 0;

    /**
     * Returns the type of ExprResult represented
     * @return the type of ExprResult represented
    **/
    virtual short getResultType()      = 0;

    /**
     * Creates a String representation of this ExprResult
     * @param str the destination string to append the String representation to.
    **/
    virtual void stringValue(nsAString& str) = 0;

    /**
     * Returns a pointer to the stringvalue if possible. Otherwise null is
     * returned.
     */
    virtual nsAString* stringValuePointer() = 0;

    /**
     * Converts this ExprResult to a Boolean (MBool) value
     * @return the Boolean value
    **/
    virtual MBool booleanValue()          = 0;

    /**
     * Converts this ExprResult to a Number (double) value
     * @return the Number value
    **/
    virtual double numberValue()          = 0;

};

#define TX_DECL_EXPRRESULT                                        \
    virtual ExprResult* clone();                                  \
    virtual short getResultType();                                \
    virtual void stringValue(nsAString& str);                     \
    virtual nsAString* stringValuePointer();                      \
    virtual PRBool booleanValue();                                \
    virtual double numberValue();                                 \


class BooleanResult : public ExprResult {

public:

    BooleanResult();
    BooleanResult(MBool boolean);

    TX_DECL_EXPRRESULT

private:
    MBool value;
};

class NumberResult : public ExprResult {

public:

    NumberResult();
    NumberResult(double dbl);

    TX_DECL_EXPRRESULT

private:
    double value;

};


class StringResult : public ExprResult {
public:
    StringResult();
    StringResult(const nsAString& str);

    TX_DECL_EXPRRESULT

    nsString mValue;
};

#endif

