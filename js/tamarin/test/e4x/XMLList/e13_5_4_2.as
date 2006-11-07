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
/* -*- Mode: java; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * The Original Code is Rhino code, released
 * May 6, 1999.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1997-2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Igor Bukanov
 * Ethan Hugg
 * Milen Nankov
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

START("13.5.4.2 - XMLList attribute()");

//TEST(1, true, XMLList.prototype.hasOwnProperty("attribute"));

// Test with list of size 0
emps = new XMLList();
TEST(2, "xml", typeof(emps.attribute("id")));
TEST_XML(3, "", emps.attribute("id"));

// Test with list of size 1
emps += <employee id="0"><name>Jim</name><age>25</age></employee>;

TEST(4, "xml", typeof(emps.attribute("id")));
TEST_XML(5, 0, emps.attribute("id"));

// Test with list of size > 1
emps += <employee id="1"><name>Joe</name><age>20</age></employee>;
TEST(6, "xml", typeof(emps.attribute("id")));

correct = new XMLList();
correct += new XML("0");
correct += new XML("1");
TEST(7, correct, emps.attribute("id"));

// Test one that doesn't exist - should return empty XMLList
TEST(8, "xml", typeof(emps.attribute("foobar")));

// Test args of null and undefined
try {
  emps.attribute(null);
  SHOULD_THROW(9);
} catch (ex1) {
  TEST(9, "TypeError", ex1.name);
}

try {
  emps.attribute(undefined);
  SHOULD_THROW(10);
} catch (ex2) {
  TEST(10, "TypeError", ex2.name);
}

var xmlDoc = "<TEAM foo = 'bar' two='second'>Giants</TEAM><CITY two = 'third'>Giants</CITY>";

// verify that attribute correct finds one attribute node
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('foo') instanceof XMLList", true, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('foo') instanceof XMLList ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('foo') instanceof XML", false, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('foo') instanceof XML ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('foo').length()", 1, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('foo').length() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('foo').toString()", "bar", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('foo').toString() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('foo')[0].nodeKind()", "attribute", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('foo')[0].nodeKind() ));

// verify that attribute correct finds attribute nodes across children
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('two') instanceof XMLList", true, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('two') instanceof XMLList ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('two') instanceof XML", false, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('two') instanceof XML ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('two').length()", 2, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('two').length() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('two').toString()", "secondthird", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('two').toString() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('two')[0].nodeKind()", "attribute", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('two')[0].nodeKind() ));

// verify that attribute doesn't find non-existent names
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST') instanceof XMLList", true, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST') instanceof XMLList ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST') instanceof XML", false, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST') instanceof XML ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST').length()", 0, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST').length() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST').toString()", "", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('DOESNOTEXIST').toString() ));
             
// verify that attribute doesn't find child node names             
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM') instanceof XMLList", true, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM') instanceof XMLList ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM') instanceof XML", false, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM') instanceof XML ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM').toString()", "", 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM').toString() ));
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM').length()", 0, 
             (MYXML = new XMLList(xmlDoc), MYXML.attribute('TEAM').length() ));

END();
