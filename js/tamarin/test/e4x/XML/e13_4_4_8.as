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

START("13.4.4.8 - XML children()");

//TEST(1, true, XML.prototype.hasOwnProperty("children"));

emps =
<employees>
    <employee id="0"><name>Jim</name><age>25</age></employee>
    <employee id="1"><name>Joe</name><age>20</age></employee>
</employees>;

correct = new XMLList();
correct += <employee id="0"><name>Jim</name><age>25</age></employee>;
correct += <employee id="1"><name>Joe</name><age>20</age></employee>;

TEST(2, "xml", typeof(emps.children()));
TEST(3, correct, emps.children());

// Get the child elements of the first employee
correct = new XMLList();
correct += <name>Jim</name>,
correct += <age>25</age>;

TEST(4, correct, emps.employee[0].children());

var xmlDoc = "<company><employee id='1'><name>John</name> <city>California</city> </employee> <employee id='2'><name>Mary</name><city>Texas</city></employee></company>";

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.employee[0].children().toString()", "<name>John</name>" + NL() + "<city>California</city>", (MYXML = new XML(xmlDoc), MYXML.employee[0].children().toString()));

// Same results whether or not prettyPrinting is true (XMLList.toString testing)
XML.prettyPrinting = false;
AddTestCase( "MYXML = new XML(xmlDoc), MYXML.employee[0].children().toString()", "<name>John</name>" + NL() + "<city>California</city>", (MYXML = new XML(xmlDoc), MYXML.employee[0].children().toString()));

//!!@This crashes ASC (because of the (id == '1') code
//!!@This does not work in Rhino
//!!@AddTestCase( "MYXML = new XML(xmlDoc), MYXML.employee.(id == '1').children()", "<name>John</name>, <city>California</city>", (MYXML = new XML(xmlDoc), MYXML.employee.(id == '1').children()));

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.employee[1].children().toString()", "<name>Mary</name>" + NL() + "<city>Texas</city>", (MYXML = new XML(xmlDoc), MYXML.employee[1].children().toString()));

END();
