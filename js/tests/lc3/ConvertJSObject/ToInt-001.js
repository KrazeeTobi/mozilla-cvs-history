/* The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is Mozilla Communicator client code, released March
 * 31, 1998.
 * 
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation. Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation. All Rights Reserved.
 * 
 */
/**
 *  Preferred Argument Conversion.
 *
 *  Passing a JavaScript boolean to a Java method should prefer to call
 *  a Java method of the same name that expects a Java boolean.
 *
 */
    var SECTION = "Preferred argument conversion:  JavaScript Object to int";
    var VERSION = "1_4";
    var TITLE   = "LiveConnect 3.0 JavaScript to Java Data Type Conversion " +
                    SECTION;
    startTest();

    var TEST_CLASS = new Packages.com.netscape.javascript.qa.lc3.jsobject.JSObject_007;

    testcases[testcases.length] = new TestCase(
        "TEST_CLASS.ambiguous( new String() ) +''",
        "INT",
        TEST_CLASS.ambiguous(new String()) +'' );

    testcases[testcases.length] = new TestCase(
        "TEST_CLASS.ambiguous( new Boolean() ) +''",
        "INT",
        TEST_CLASS.ambiguous( new Boolean() )+'' );

    testcases[testcases.length] = new TestCase(
        "TEST_CLASS.ambiguous( new Number() ) +''",
        "INT",
        TEST_CLASS.ambiguous( new Number() )+'' );

    testcases[testcases.length] = new TestCase(
        "TEST_CLASS.ambiguous( new Date(0) ) +''",
        "INT",
        TEST_CLASS.ambiguous( new Date(0) )+'' );

    testcases[testcases.length] = new TestCase(
        "TEST_CLASS.ambiguous( new Array() ) +''",
        "INT",
        TEST_CLASS.ambiguous( new Array() )+'' );

    test();


