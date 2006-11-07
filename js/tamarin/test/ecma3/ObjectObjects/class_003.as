/* ***** BEGIN LICENSE BLOCK ***** 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1 

The contents of this file are subject to the Mozilla Public License Version 1.1 (the 
"License"); you may not use this file except in compliance with the License. You may obtain 
a copy of the License at http://www.mozilla.org/MPL/ 

Software distributed under the License is distributed on an "AS IS" basis, WITHOUT 
WARRANTY OF ANY KIND, either express or implied. See the License for the specific 
language governing rights and limitations under the License. 

The Original Code is [Open Source Virtual Machine.] 

The Initial Developer of the Original Code is Adobe System Incorporated.  Portions created 
by the Initial Developer are Copyright (C)[ 2005-2006 ] Adobe Systems Incorporated. All Rights 
Reserved. 

Contributor(s): Adobe AS3 Team

Alternatively, the contents of this file may be used under the terms of either the GNU 
General Public License Version 2 or later (the "GPL"), or the GNU Lesser General Public 
License Version 2.1 or later (the "LGPL"), in which case the provisions of the GPL or the 
LGPL are applicable instead of those above. If you wish to allow use of your version of this 
file only under the terms of either the GPL or the LGPL, and not to allow others to use your 
version of this file under the terms of the MPL, indicate your decision by deleting provisions 
above and replace them with the notice and other provisions required by the GPL or the 
LGPL. If you do not delete the provisions above, a recipient may use your version of this file 
under the terms of any one of the MPL, the GPL or the LGPL. 

 ***** END LICENSE BLOCK ***** */
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
* SUMMARY: Testing the [[Class]] property of native error types.
* See ECMA-262 Edition 3, Section 8.6.2 for the [[Class]] property.
*
* Same as class-001.js - but testing only the native error types here.
* See ECMA-262 Edition 3, Section 15.11.6 for a list of these types.
*
* ECMA expects the [[Class]] property to equal 'Error' in each case.
* See ECMA-262 Edition 3, Sections 15.11.1.1 and 15.11.7.2 for this.
* See http://bugzilla.mozilla.org/show_bug.cgi?id=56868
*
* The getJSClass() function we use is in a utility file, e.g. "shell.js"
*/
//-------------------------------------------------------------------------------------------------
var SECTION = "class_003";
var VERSION = "";
var TITLE   = "Testing the internal [[Class]] property of native error types";
var bug = "56868";

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

	/*
	 * We set the expect variable each time only for readability.
	 * We expect 'Error' every time; see discussion above -
	 */
	status = 'new Error()';
	actual = getJSClass(new Error());
	expect = 'Error';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new EvalError()';
	actual = getJSClass(new EvalError());
	expect = 'EvalError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new RangeError()';
	actual = getJSClass(new RangeError());
	expect = 'RangeError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new ReferenceError()';
	actual = getJSClass(new ReferenceError());
	expect = 'ReferenceError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new SyntaxError()';
	actual = getJSClass(new SyntaxError());
	expect = 'SyntaxError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new TypeError()';
	actual = getJSClass(new TypeError());
	expect = 'TypeError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	status = 'new URIError()';
	actual = getJSClass(new URIError());
	expect = 'URIError';
	array[item++] = new TestCase(SECTION, status, expect, actual);

	return array;
}
