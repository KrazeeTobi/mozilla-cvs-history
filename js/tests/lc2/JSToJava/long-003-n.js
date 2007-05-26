/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

gTestfile = 'long-003-n.js';

/**
   Template for LiveConnect Tests

   File Name:      number-001.js
   Description:

   This test fails in lc3, but will succeed if the underlying version
   of liveconnect only supports LC2.

   @author     christine@netscape.com
   @version    1.00
*/
var SECTION = "LiveConnect";
var VERSION = "1_3";
var TITLE   = "LiveConnect JavaScript to Java Data Type Conversion";

startTest();
writeHeaderToLog( SECTION + " "+ TITLE);

// typeof all resulting objects is "object";
var E_TYPE = "object";

// JS class of all resulting objects is "JavaObject";
var E_JSCLASS = "[object JavaObject]";

var a = new Array();
var i = 0;

a[i++] = new TestObject( "java.lang.Long.toString(NaN)",
			 java.lang.Long.toString(NaN), "0" );

for ( var i = 0; i < a.length; i++ ) {

  // check typeof
  new TestCase(
    SECTION,
    "typeof (" + a[i].description +")",
    a[i].type,
    typeof a[i].javavalue );
/*
// check the js class
new TestCase(
SECTION,
"("+ a[i].description +").getJSClass()",
E_JSCLASS,
a[i].jsclass );
*/
  // check the number value of the object
  new TestCase(
    SECTION,
    "String(" + a[i].description +")",
    a[i].jsvalue,
    String( a[i].javavalue ) );
}

test();

function TestObject( description, javavalue, jsvalue ) {
  this.description = description;
  this.javavalue = javavalue;
  this.jsvalue = jsvalue;
  this.type = E_TYPE;
//  LC2 does not support the __proto__ property in Java objects.
//    this.javavalue.__proto__.getJSClass = Object.prototype.toString;
//    this.jsclass = this.javavalue.getJSClass();
  return this;
}
