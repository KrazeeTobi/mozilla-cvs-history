/*
 * (C) Copyright The MITRE Corporation 1999  All rights reserved.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * The program provided "as is" without any warranty express or
 * implied, including the warranty of non-infringement and the implied
 * warranties of merchantibility and fitness for a particular purpose.
 * The Copyright owner will not be liable for any damages suffered by
 * you as a result of using the Program. In no event will the Copyright
 * owner be liable for any special, indirect or consequential damages or
 * lost profits even if the Copyright owner has been advised of the
 * possibility of their occurrence.
 *
 * Please see release.txt distributed with this file for more information.
 *
 */

/**
 * StringResult
 * Represents a String as a Result of evaluating an Expr
 * @author <a href="mailto:kvisco@mitre.org">Keith Visco</a>
**/
#include "ExprResult.h"


/**
 * Default Constructor
**/
StringResult::StringResult() {
} //-- StringResult

/**
 * Creates a new StringResult with the value of the given String parameter
 * @param str the String to use for initialization of this StringResult's value
**/
StringResult::StringResult(String& str) {
    //-- copy str
    this->value = str;
} //-- StringResult

/**
 * Creates a new StringResult with the value of the given String parameter
 * @param str the String to use for initialization of this StringResult's value
**/
StringResult::StringResult(const String& str) {
    //-- copy str
    this->value = str;
} //-- StringResult

/**
 * Returns the value of this StringResult
**/
String& StringResult::getValue() {
    return this->value;
} //-- getValue

/**
 * Sets the value of this StringResult
 * @param str the String to use for this StringResult's value
**/
void StringResult::setValue(const String& str){
    // copy str
    this->value = str;
} //-- setValue

/*
 * Virtual Methods from ExprResult
*/

short StringResult::getResultType() {
    return ExprResult::STRING;
} //-- getResultType

void StringResult::stringValue(String& str)  {
    str.append(this->value);
} //-- stringValue

MBool StringResult::booleanValue() {
   return (MBool)(this->value.length());
} //-- booleanValue

double StringResult::numberValue() {
    Double dbl(value);
    return dbl.doubleValue();
} //-- numberValue

