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
        File Name:      package-003.js
        Description:

        Set the package name to a JavaScript variable, and attempt to access
        classes relative to the package name.

        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect Packages";
    var VERSION = "1_3";
    var TITLE   = "LiveConnect Packages";

    var testcases = new Array();

    startTest();
    writeHeaderToLog( SECTION + " "+ TITLE);

    //  All packages are of the type "object"
    var E_TYPE = "object";

    //  The JavaScript [[Class]] property for all Packages is "[JavaPackage <packagename>]"
    var E_JSCLASS = "[JavaPackage ";

    //  Create arrays of actual results (java_array) and
    //  expected results (test_array).

    var java_array = new Array();
    var test_array = new Array();

    var i = 0;

    var js = Packages.javax.javascript;

    java_array[i] = new JavaValue(  js  );
    test_array[i] = new TestValue(  "javax.javascript" );
    i++;

    var util = java.util;

    java_array[i] = new JavaValue(  util  );
    test_array[i] = new TestValue(  "java.util" );
    i++;

    for ( i = 0; i < java_array.length; i++ ) {
        CompareValues( java_array[i], test_array[i] );

    }

    var v = new util.Vector();

    test();

function CompareValues( javaval, testval ) {
    //  Check typeof, which should be E_TYPE
    testcases[testcases.length] = new TestCase( SECTION,
                                                "typeof (" + testval.description +")",
                                                testval.type,
                                                javaval.type );

    //  Check JavaScript class, which should be E_JSCLASS + the package name
    testcases[testcases.length] = new TestCase( SECTION,
                                                "(" + testval.description +").getJSClass()",
                                                testval.jsclass,
                                                javaval.jsclass );

    //  Number( package ) is NaN
    testcases[testcases.length] = new TestCase( SECTION,
                                                "Number (" + testval.description +")",
                                                NaN,
                                                Number( javaval.value ) );

    //  String( package ) is string value
    testcases[testcases.length] = new TestCase( SECTION,
                                                "String (" + testval.description +")",
                                                testval.jsclass,
                                                String(javaval.value) );
/*
    //  ( package ).toString() is string value
    testcases[testcases.length] = new TestCase( SECTION,
                                                "(" + testval.description +").toString()",
                                                testval.jsclass,
                                                (javaval.value).toString() );
*/
    //  Boolean( package ) is true
    testcases[testcases.length] = new TestCase( SECTION,
                                                "Boolean (" + testval.description +")",
                                                true,
                                                Boolean( javaval.value ) );
    //  add 0 is name + "0"
    testcases[testcases.length] = new TestCase( SECTION,
                                                "(" + testval.description +") +0",
                                                testval.jsclass +"0",
                                                javaval.value + 0);
}
function JavaValue( value ) {
    this.value  = value;
    this.type   = typeof value;
    this.jsclass = value +""
    return this;
}
function TestValue( description ) {
    this.packagename = (description.substring(0, "Packages.".length) ==
        "Packages.") ? description.substring("Packages.".length, description.length ) :
        description;

    this.description = description;
    this.type =  E_TYPE;
    this.jsclass = E_JSCLASS +  this.packagename +"]";
    return this;
}
function test() {
    for ( tc=0; tc < testcases.length; tc++ ) {
        testcases[tc].passed = writeTestCaseResult(
                            testcases[tc].expect,
                            testcases[tc].actual,
                            testcases[tc].description +" = "+
                            testcases[tc].actual );

        testcases[tc].reason += ( testcases[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcases );
}
