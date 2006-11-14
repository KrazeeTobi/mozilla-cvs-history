/*
* The contents of this file are subject to the Netscape Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/NPL/
*
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
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
* Date: 01 June 2001
*
* SUMMARY: Testing that we don't crash on switch case -1...
* See http://bugzilla.mozilla.org/show_bug.cgi?id=83532
*
*/
//-------------------------------------------------------------------------------------------------
var bug = 83532;
var summary = "Testing that we don't crash on switch case -1";
var sToEval = '';

startTest();

var testcases = getTestCases();

//-------------------------------------------------------------------------------------------------
test();
//-------------------------------------------------------------------------------------------------

function getTestCases()
{
    var array = new Array();
    var item = 0;
    
    doTest();
    
    array[item++] = new TestCase("", "Make sure we don't crash", true, true);
    
    return array;
}

function doTest()
{
  printBugNumber (bug);
  printStatus (summary);

  // Just testing that we don't crash on these -
  sToEval += 'function f () {switch(1) {case -1:}};';
  sToEval += 'function g(){switch(1){case (-1):}};';
  sToEval += 'var h = function() {switch(1) {case -1:}};'
  sToEval += 'f();';
  sToEval += 'g();';
  sToEval += 'h();';
}
