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
*
*
*This test arose from Bugzilla bug 58946.
*The bug was filed when we got the following error (see code below):
*
*                          "ReferenceError: e is not defined"
*
*There was no error if we replaced "return e" in the code below with "print(e)".
*There should be no error with "return e", either  -
*/
//-------------------------------------------------------------------------------------------------
var THE_ERROR = "return_error";

startTest();

var stat = "Testing a return statement inside a catch block inside a function";
printStatus (stat);
var testcases = getTestCases();
test();

function getTestCases()
{
    var array = new Array();
    var item = 0;

    var thisError = throwError();
    
    array[item++] = new TestCase(SECTION, "throwError()", THE_ERROR, thisError);
    
    return array;
}

function throwError()
{
	try
	{
		throw THE_ERROR;
	}
	catch(e)
	{
		return e;
	}
}
