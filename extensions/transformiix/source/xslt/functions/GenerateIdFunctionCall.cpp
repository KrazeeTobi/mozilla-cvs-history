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
 * The Original Code is XSL:P XSLT processor.
 *
 * The Initial Developer of the Original Code is Keith Visco.
 * Portions created by Keith Visco (C) 1999, 2000 Keith Visco.
 * All Rights Reserved.
 *
 * Contributor(s):
 *
 * Keith Visco, kvisco@ziplink.net
 *    -- original author.
 *
 * $Id: GenerateIdFunctionCall.cpp,v 1.1 2000/04/19 10:39:27 kvisco%ziplink.net Exp $
 */

#include "XSLTFunctions.h"

/*
  Implementation of XSLT 1.0 extension function: generate-id
*/

/**
 * Creates a new generate-id function call
**/
GenerateIdFunctionCall::GenerateIdFunctionCall(DOMHelper* domHelper) : FunctionCall(GENERATE_ID_FN)
{
    this->domHelper = domHelper;
} //-- GenerateIdFunctionCall

/**
 * Evaluates this Expr based on the given context node and processor state
 * @param context the context node for evaluation of this Expr
 * @param ps the ContextState containing the stack information needed
 * for evaluation
 * @return the result of the evaluation
 * @see FunctionCall.h
**/
ExprResult* GenerateIdFunctionCall::evaluate(Node* context, ContextState* cs) {

    Node* node = context;

    String id;

    int argc = params.getLength();


    domHelper->generateId(node, id);

    return new StringResult(id);

} //-- evaluate

