/*
* The contents of this file are subject to the Netscape Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/NPL/
*
* Software distributed under the License is distributed on an "AS IS"
* basis, WITHOUT WARRANTY OF ANY KIND, either expressed
* or implied. See the License for the specific language governing
* rights and limitations under the License.
*
* The Original Code is mozilla.org code.
*
* The Initial Developer of the Original Code is Netscape
* Communications Corporation.  Portions created by Netscape are
* Copyright (C) 1998 Netscape Communications Corporation. All
* Rights Reserved.
*
* Contributor(s): pschwartau@netscape.com
* Date: 14 Mar 2001
*
* SUMMARY: Testing the internal [[Class]] property of objects
* See ECMA-262 Edition 3 13-Oct-1999, Section 8.6.2
*
* The getJSClass() function we use is in a utility file, e.g. "shell.js".
*
*    Modified:		28th October 2004 (gasingh@macromedia.com)
*    			Removed the occurence of new Function('abc').
*    			This is being changed to function() { abc }.
*
*/
//-------------------------------------------------------------------------------------------------
var SECTION = "class_001";
var VERSION = "";
var TITLE   = "Testing the internal [[Class]] property of objects";
var bug = "(none)";

startTest();
writeHeaderToLog(SECTION + " " + TITLE);
var testcases = getTestCases();
test();

function getTestCases() {
	var array = new Array();
	var item = 0;

	var status = '';
	var actual = '';
	var expect= '';
        var k = new Function();
        
        
	status = 'the global object';
	actual = getJSClass(this);
	expect = 'global';
	array[item++] = new TestCase(SECTION, status, expect, actual);
	status = 'new Object()';
	actual = getJSClass(new Object());
	expect = 'Object';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	// new Function() will be dropped in ecma4, will return undefined
	// new Function() has been replaced by function() {}
	/*status = 'new Function()';
	actual = getJSClass(k)+"";
	expect = 'Function';
	array[item++] = new TestCase(SECTION, status, expect, actual);*/

	status = 'new Array()';
	actual = getJSClass(new Array());
	expect = 'Array';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new String()';
	actual = getJSClass(new String());
	expect = 'String';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new Boolean()';
	actual = getJSClass(new Boolean());
	expect = 'Boolean';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new Number()';
	actual = getJSClass(new Number());
	expect = 'Number';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'Math';
	actual = getJSClass(Math);  // can't use 'new' with the Math object (EMCA3, 15.8)
	expect = 'Math';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new Date()';
	actual = getJSClass(new Date());
	expect = 'Date';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new RegExp()';
	actual = getJSClass(new RegExp());
	expect = 'RegExp';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new Error()';
	actual = getJSClass(new Error());
	expect = 'Error';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	return array;
}
