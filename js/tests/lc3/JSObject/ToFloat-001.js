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
/* -*- Mode: java; tab-width: 8 -*-
 * Copyright � 1997, 1998 Netscape Communications Corporation,
 * All Rights Reserved.
 */

/**
 *  JavaScript to Java type conversion.
 *
 *  This test passes JavaScript number values to several Java methods
 *  that expect arguments of various types, and verifies that the value is
 *  converted to the correct value and type.
 *
 *  This tests instance methods, and not static methods.
 *
 *  Running these tests successfully requires you to have
 *  com.netscape.javascript.qa.liveconnect.DataTypeClass on your classpath.
 *
 *  Specification:  Method Overloading Proposal for Liveconnect 3.0
 *
 *  @author: christine@netscape.com
 *
 */
    var SECTION = "JavaScript Object to java.lang.String";
    var VERSION = "1_4";
    var TITLE   = "LiveConnect 3.0 JavaScript to Java Data Type Conversion " +
                    SECTION;
    var BUGNUMBER = "335899";
    startTest();

    var dt = new DT();

    var a = new Array();
    var i = 0;

    // 3.3.6.4 Other JavaScript Objects
    // Passing a JavaScript object to a java method that that expects a float
    // should:
    // 1. Apply the ToPrimitive operator (ECMA 9.3) to the JavaScript object
    // with hint Number
    // 2. Convert Result(1) to Java numeric type using the rules in 3.3.3.

    var bool = new Boolean(true);

    a[i++] = new TestObject(
        "dt.setFloat( bool )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        1,
        "number");

    bool = new Boolean(false);

    a[i++] = new TestObject(
        "dt.setFloat( bool )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        0,
        "number");

    var number = new Number(0);

    a[i++] = new TestObject(
        "dt.setFloat( number )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        0,
        "number");

    nan = new Number(NaN);

    a[i++] = new TestObject(
        "dt.setFloat( nan )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    infinity = new Number(Infinity);

    a[i++] = new TestObject(
        "dt.setFloat( infinity )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        Infinity,
        "number");

    var neg_infinity = new Number(-Infinity);

    a[i++] = new TestObject(
        "dt.setFloat( new Number(neg_infinity))",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        -Infinity,
        "number");

    var string  = new String("JavaScript String Value");

    a[i++] = new TestObject(
        "dt.setFloat(string)",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    var string  = new String("1234567");

    a[i++] = new TestObject(
        "dt.setFloat(string)",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        1234567,
        "number");


    var string  = new String("123456");

    a[i++] = new TestObject(
        "dt.setFloat(string)",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        123456,
        "number");

    var myobject = new MyObject( "5555555" );

    a[i++] = new TestObject(
        "dt.setFloat( myobject )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        5555555,
        "number");

    myobject = new MyOtherObject( "5551212");

    a[i++] = new TestObject(
        "dt.setFloat( myobject )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        5551212,
        "number");

    myobject = new AnotherObject( "6060842");

    a[i++] = new TestObject(
        "dt.setFloat( myobject )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        6060842,
        "number");

    var object = new Object();

    a[i++] = new TestObject(
        "dt.setFloat( object )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    var array = new Array(1,2,3)

    a[i++] = new TestObject(
        "dt.setFloat(array)",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    a[i++] = new TestObject(
        "dt.setFloat( MyObject )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    a[i++] = new TestObject(
        "dt.setFloat( this )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    a[i++] = new TestObject(
        "dt.setFloat( Math )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    a[i++] = new TestObject(
        "dt.setFloat( Function )",
        "dt.PUB_FLOAT",
        "dt.getFloat()",
        "typeof dt.getFloat()",
        NaN,
        "number");

    for ( i = 0; i < a.length; i++ ) {
        testcases[testcases.length] = new TestCase(
            a[i].description +"; "+ a[i].javaFieldName,
            a[i].jsValue,
            a[i].javaFieldValue );

        testcases[testcases.length] = new TestCase(
            a[i].description +"; " + a[i].javaMethodName,
            a[i].jsValue,
            a[i].javaMethodValue );

        testcases[testcases.length] = new TestCase(
            a[i].javaTypeName,
            a[i].jsType,
            a[i].javaTypeValue );
    }

    test();

function MyObject( stringValue ) {
    this.stringValue = String(stringValue);
    this.toString = new Function( "return this.stringValue" );
}

function MyOtherObject( value ) {
    this.toString = null;
    this.value = value;
    this.valueOf = new Function( "return this.value" );
}

function AnotherObject( value ) {
    this.toString = new Function( "return new Number(666)" );
    this.value = value;
    this.valueOf = new Function( "return this.value" );
}

function TestObject( description, javaField, javaMethod, javaType,
    jsValue, jsType )
{
    eval (description );

    this.description = description;
    this.javaFieldName = javaField;
    this.javaFieldValue = eval( javaField );
    this.javaMethodName = javaMethod;
    this.javaMethodValue = eval( javaMethod );
    this.javaTypeName = javaType,
    this.javaTypeValue = eval( javaType );

    this.jsValue   = jsValue;
    this.jsType      = jsType;
}
