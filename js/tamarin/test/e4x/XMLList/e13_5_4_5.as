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

START("13.5.4.5 - XMLList children()");

//TEST(1, true, XMLList.prototype.hasOwnProperty("children"));

// Test with XMLList of size 0
x1 = new XMLList()
TEST(2, "xml", typeof(x1.children()));
TEST_XML(3, "", x1.children());

// Test with XMLList of size 1
x1 += <alpha>one<bravo>two</bravo></alpha>;    
TEST(4, "xml", typeof(x1.children()));

correct = <>one<bravo>two</bravo></>;
TEST(5, correct, x1.children());

// Test with XMLList of size > 1
x1 += <charlie><bravo>three</bravo></charlie>;
TEST(6, "xml", typeof(x1.children()));

correct = <>one<bravo>two</bravo><bravo>three</bravo></>;
TEST(7, correct, x1.children());

// Test no children
x1 = new XMLList();
x1 += <alpha></alpha>;
x1 += <bravo></bravo>;
TEST(8, "xml", typeof(x1.children()));
TEST_XML(9, "", x1.children());

//get all grandchildren of the order that have the name price

order = 
<order>
    <customer>
        <name>John Smith</name>
    </customer>
    <item id="1">
        <description>Big Screen Television</description>
        <price>1299.99</price>
    </item>
    <item id="2">
        <description>DVD Player</description>
        <price>399.99</price>
    </item>
</order>;

correct= <price>1299.99</price> + <price>399.99</price>;

TEST(10, correct, order.children().price);

var xmlDoc = "<employee id=\"1\"><firstname>John</firstname><lastname>Walton</lastname><age>25</age></employee><employee id=\"2\"><firstname>Sue</firstname><lastname>Day</lastname><age>32</age></employee>"

// propertyName as a string
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.children()", 
		"<firstname>John</firstname>" + NL() + "<lastname>Walton</lastname>" + NL() + "<age>25</age>" + NL() + "<firstname>Sue</firstname>" + NL() + "<lastname>Day</lastname>" + NL() + "<age>32</age>", 
        (MYXML = new XMLList(xmlDoc), MYXML.children().toString()));

END();
