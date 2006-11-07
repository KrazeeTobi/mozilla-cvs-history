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
package{

var SECTION = "Directives";       // provide a document reference (ie, Actionscript section)
var VERSION = "AS 3.0";        // Version of ECMAScript or ActionScript
var TITLE   = "QNS Property Access";       // Provide ECMA section title or a description
//var BUGNUMBER = "";

startTest();                // leave this alone


public namespace T1;

class nsTest
{
	T1 var o = new Object();
	T1 function get x () { return "T1 ns";}
	public function get x () {return "public ns";}
	private function get z () {return "private var";}

	function nsTest()
	{
		T1::o.y = "hello y";
	}



}

var myTest = new nsTest;

var testResult:String = new String();

try {
    testResult = myTest.x;
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( ". Lookup", "public ns", testResult);
}

try {
    testResult = myTest['x'];
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "[] Lookup", "public ns", testResult);
}

try {
    testResult = myTest.public::x;
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "explicit public ns Lookup", "public ns", testResult);
}

// bug 151552
//Added referenceError() so that the regress folder will not be different for release avmplus
//and release debugger avmplus should be modified after bug 151552 is fixed
try {
    testResult = myTest.public::['x'];
} catch (e:ReferenceError) {

    testResult = e;
} finally {
    AddTestCase( ".explicit public ns [] Lookup", "public ns", referenceError(testResult));
}

try {
    testResult = myTest.T1::x;
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( ". ns lookup", "T1 ns", testResult);
}

try {
    testResult = myTest.T1::['x'];
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "[] ns lookup", "T1 ns", testResult);
}

try {
    testResult = myTest.T1::o.y;
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "subobject lookup using .", "hello y", testResult);
}

try {
    testResult = myTest.T1::['o']['y'];
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "subobject lookup using []", "hello y", testResult);
}

try {
    testResult = myTest.T1::['o'].y;
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "subobject lookup using []. combo", "hello y", testResult);
}

try {
    testResult = myTest.T1::o['y'];
} catch (e) {
    testResult = e;
} finally {
    AddTestCase( "subobject lookup using obj[] combo", "hello y", testResult);
}

test();       // leave this alone.  this executes the test cases and
              // displays results.
}
