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
    var SECTION = "JavaObject to string";
    var VERSION = "1_4";
    var TITLE   = "LiveConnect 3.0 JavaScript to Java Data Type Conversion " +
                    SECTION;
    startTest();

    var dt = new DT();

    var a = new Array();
    var i = 0;

    // Call the unwrapped object's toString() method, and return the result as
    // a new java.lang.String.

    var newValue = Math.random();
    var newString = String( newValue );

    a[i++] = new TestObject (
         "dt.PUB_STRING_REPRESENTATION = \""+newString+"\";" +
         "dt.setStringObject( dt )",
         "dt.PUB_STRING +''",
         "dt.getStringObject() +''",
         "typeof dt.getStringObject()",
         newString,
         "string" );

    a[i++] = new TestObject (
         "dt.PUB_DOUBLE = java.lang.Double.valueOf(\"" + newString +"\")",
         "dt.PUB_DOUBLE",
         "dt.getDouble()",
         "typeof dt.getDouble()",
         newValue,
         "number" );

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
    this.javaTypeValue = typeof this.javaFieldValue;

    this.jsValue   = jsValue;
    this.jsType      = jsType;
}
