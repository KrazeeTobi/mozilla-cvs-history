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
        File Name:      class-001.js
        Description:


        @author     christine@netscape.com
        @version    1.00
*/
    var SECTION = "LiveConnect Classes";
    var VERSION = "1_3";
    var TITLE   = "JavaClass objects";

    var testcases = new Array();

    startTest();
    writeHeaderToLog( SECTION + " "+ TITLE);

    //  All packages are of the type "function", since they implement [[Construct]]
    // and [[Call]]

    var E_TYPE = "function";

    //  The JavaScript [[Class]] property for all Classes is "[object JavaClass]"
    var E_JSCLASS = "[object JavaClass]";

    //  Create arrays of actual results (java_array) and
    //  expected results (test_array).

    var java_array = new Array();
    var test_array = new Array();

    var i = 0;

    java_array[i] = new JavaValue(  java.awt.Image  );
    test_array[i] = new TestValue(  "java.awt.Image" );
    i++;

    java_array[i] = new JavaValue(  java.beans.Beans  );
    test_array[i] = new TestValue(  "java.beans.Beans" );
    i++;

    java_array[i] = new JavaValue(  java.io.File  );
    test_array[i] = new TestValue(  "java.io.File" );
    i++;

    java_array[i] = new JavaValue(  java.lang.String  );
    test_array[i] = new TestValue(  "java.lang.String" );
    i++;

    java_array[i] = new JavaValue(  java.math.BigDecimal  );
    test_array[i] = new TestValue(  "java.math.BigDecimal" );
    i++;

    java_array[i] = new JavaValue(  java.net.URL  );
    test_array[i] = new TestValue(  "java.net.URL" );
    i++;

    java_array[i] = new JavaValue(  java.rmi.RMISecurityManager  );
    test_array[i] = new TestValue(  "java.rmi.RMISecurityManager" );
    i++;

    java_array[i] = new JavaValue(  java.text.DateFormat  );
    test_array[i] = new TestValue(  "java.text.DateFormat" );
    i++;

    java_array[i] = new JavaValue(  java.util.Vector  );
    test_array[i] = new TestValue(  "java.util.Vector" );
    i++;
/*
    works for rhino only
    java_array[i] = new JavaValue(  Packages.com.netscape.javascript.Context  );
    test_array[i] = new TestValue(  "Packages.com.netscape.javascript.Context" );
    i++;
*/
    for ( i = 0; i < java_array.length; i++ ) {
        CompareValues( java_array[i], test_array[i] );
    }

    test();
function CompareValues( javaval, testval ) {
    //  Check type, which should be E_TYPE
    testcases[testcases.length] = new TestCase( SECTION,
                                                "typeof (" + testval.description +")",
                                                testval.type,
                                                javaval.type );
/*
    //  Check JavaScript class, which should be E_JSCLASS
    testcases[testcases.length] = new TestCase( SECTION,
                                                "(" + testval.description +").getJSClass()",
                                                testval.jsclass,
                                                javaval.jsclass );
*/
    // Check the class's name, which should be the description, minus the "Package." part.
    testcases[testcases.length] = new TestCase( SECTION,
                                                "(" + testval.description +") +''",
                                                testval.classname,
                                                javaval.classname );
}
function JavaValue( value ) {
// this suceeds in LC1, but fails in LC2
// Object.prototype.toString will show its JavaScript wrapper object.
//    value.__proto__.getJSClass = Object.prototype.toString;
//    this.jsclass = value.getJSClass();
    this.classname = value +"";
    this.type   = typeof value;
    return this;
}
function TestValue( description, value ) {
    this.description = description;
    this.type =  E_TYPE;
    this.jsclass = E_JSCLASS;

    this.classname = "[JavaClass " +
                     (  ( description.substring(0,9) == "Packages." )
                        ? description.substring(9,description.length)
                        : description
                     ) + "]"


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
