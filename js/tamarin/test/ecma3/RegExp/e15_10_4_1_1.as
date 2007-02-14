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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   pschwartau@netscape.com
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
/*
 * Date: 26 November 2000
 *
 *
 *SUMMARY: Passing a RegExp object to a RegExp() constructor.
 *This test arose from Bugzilla bug 61266. The ECMA3 section is:
 *
 *  15.10.4.1 new RegExp(pattern, flags)
 *
 *  If pattern is an object R whose [[Class]] property is "RegExp" and
 *  flags is undefined, then let P be the pattern used to construct R
 *  and let F be the flags used to construct R. If pattern is an object R
 *  whose [[Class]] property is "RegExp" and flags is not undefined,
 *  then throw a TypeError exception. Otherwise, let P be the empty string
 *  if pattern is undefined and ToString(pattern) otherwise, and let F be
 *  the empty string if flags is undefined and ToString(flags) otherwise.
 *
 *
 *The current test will check the first scenario outlined above:
 *
 *   "pattern" is itself a RegExp object R
 *   "flags"  is undefined
 *
 * We check that a new RegExp object obj2 defined from these parameters
 * is morally the same as the original RegExp object obj1. Of course, they
 * can't be equal as objects - so we check their enumerable properties...
 *
 * In this test, the initial RegExp object obj1 will not include a flag. The flags
 * parameter for obj2  will be undefined in the sense of not being provided.
 */
//-------------------------------------------------------------------------------------------------

var SECTION = "e15_10_4_1_1";
var VERSION = "";
var TITLE   = "Passing a RegExp object to a RegExp() constructor";
var bug = "61266";

startTest();
writeHeaderToLog(SECTION + " " + TITLE);
var testcases = getTestCases();
test();

function getTestCases() {
	var array = new Array();
	var item = 0;

	var statprefix = 'Applying RegExp() twice to pattern ';
	var statsuffix =  '; testing property ';
	var singlequote = "'";
	var i = -1; var s = '';
	var obj1 = null; var obj2 = null;
	var status = ''; var actual = ''; var expect = ''; var msg = '';
	var patterns = new Array();


	// various regular expressions to try -
	patterns[0] = '';
	patterns[1] = 'abc';
	patterns[2] = '(.*)(3-1)\s\w';
	patterns[3] = '(.*)(...)\\s\\w';
	patterns[4] = '[^A-Za-z0-9_]';
	patterns[5] = '[^\f\n\r\t\v](123.5)([4 - 8]$)';


	for (i in patterns)
	{
	  s = patterns[i];
	  status = getStatus(s);
	  obj1 = new RegExp(s);
	  obj2 = new RegExp(obj1);

	  msg  = status + quote("dotall");
	  actual = obj2.dotall;
	  expect = obj1.dotall;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);

	  msg  = status + quote("extended");
	  actual = obj2.extended;
	  expect = obj1.extended;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);

	  msg  = status + quote("ignoreCase");
	  actual = obj2.ignoreCase;
	  expect = obj1.ignoreCase;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);

	  msg  = status + quote("lastIndex");
	  actual = obj2.lastIndex;
	  expect = obj1.lastIndex;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);

	  msg  = status + quote("multiline");
	  actual = obj2.multiline;
	  expect = obj1.multiline;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);

	  msg  = status + quote("source");
	  actual = obj2.source;
	  expect = obj1.source;
	  array[item++] = new TestCase(SECTION, msg, expect, actual);
	}


	function getStatus(regexp)
	{
	  return (statprefix  +  quote(regexp) +  statsuffix);
	}


	function quote(text)
	{
	  return (singlequote  +  text  + singlequote);
	}

	return array;
}
