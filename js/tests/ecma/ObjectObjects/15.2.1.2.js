/* The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released March
 * 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation. Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 * 
 */
/**
    File Name:          15.2.1.2.js
    ECMA Section:       15.2.1.2  The Object Constructor Called as a Function:
                                  Object(value)
    Description:        When Object is called as a function rather than as a
                        constructor, the following steps are taken:

                        1.  If value is null or undefined, create and return a
                            new object with no proerties other than internal
                            properties exactly as if the object constructor
                            had been called on that same value (15.2.2.1).
                        2.  Return ToObject (value), whose rules are:

                        undefined   generate a runtime error
                        null        generate a runtime error
                        boolean     create a new Boolean object whose default
                                    value is the value of the boolean.
                        number      Create a new Number object whose default
                                    value is the value of the number.
                        string      Create a new String object whose default
                                    value is the value of the string.
                        object      Return the input argument (no conversion).

    Author:             christine@netscape.com
    Date:               17 july 1997
*/

    var SECTION = "15.2.1.2";
    var VERSION = "ECMA_1";
    startTest();
    var TITLE   = "Object()";

    writeHeaderToLog( SECTION + " "+ TITLE);

    var testcases = getTestCases();
    test();

function getTestCases() {
    var array = new Array();
    var item = 0;

    var MYOB = Object();

    array[item++] = new TestCase( SECTION, "var MYOB = Object(); MYOB.valueOf()",    MYOB,      MYOB.valueOf()      );
    array[item++] = new TestCase( SECTION, "typeof Object()",       "object",               typeof (Object(null)) );
    array[item++] = new TestCase( SECTION, "var MYOB = Object(); MYOB.toString()",    "[object Object]",       eval("var MYOB = Object(); MYOB.toString()") );

    return ( array );
}

function test() {
    for ( tc = 0; tc < testcases.length; tc++ ) {
        testcases[tc].passed = writeTestCaseResult(
                    testcases[tc].expect,
                    testcases[tc].actual,
                    testcases[tc].description +" = "+
                    testcases[tc].actual );

        testcases[tc].reason +=
                    ( testcases[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcases );
}
