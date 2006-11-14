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

START("13.5.4.12 - XMLList hasComplexContent()");

//TEST(1, true, XMLList.prototype.hasOwnProperty("hasComplexContent"));

// One element should be same as XML case
x1 = 
<>
<alpha attr1="value1">
    <bravo>one</bravo>
    <charlie>
        two
        three
        <echo>four</echo>
    </charlie>
    <delta />
    <foxtrot attr2="value2">five</foxtrot>
    <golf attr3="value3" />
    <hotel>
        six
        seven
    </hotel>
    <india><juliet /></india>
</alpha>;
</>;

TEST(2, true, x1.hasComplexContent());
TEST(3, false, x1.bravo.hasComplexContent());
TEST(4, true, x1.charlie.hasComplexContent());
TEST(5, false, x1.delta.hasComplexContent());
TEST(6, false, x1.foxtrot.hasComplexContent());
TEST(7, false, x1.golf.hasComplexContent());
TEST(8, false, x1.hotel.hasComplexContent());
TEST(9, false, x1.@attr1.hasComplexContent());
TEST(10, false, x1.bravo.child(0).hasComplexContent());
TEST(11, true, x1.india.hasComplexContent());

// More than one element is complex if one or more things in the list are elements.
x1 = 
<>
<alpha>one</alpha>
<bravo>two</bravo>
</>;

TEST(12, true, x1.hasComplexContent());

x1 =
<root>
    one
    <alpha>one</alpha>
</root>;

TEST(13, true, x1.*.hasComplexContent());

x1 =
<root attr1="value1" attr2="value2">
    one
</root>;

TEST(14, false, x1.@*.hasComplexContent());

var xmlDoc = "<employee id='1'><firstname>John</firstname><lastname>Walton</lastname><age dob='1/2/1978'>25</age></employee>"

// propertyName as a string
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()", true, 
             (MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()));

xmlDoc = "<firstname>John</firstname>"  
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()", false, 
	  (MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()));

XML.ignoreComments = false;
xmlDoc = "<XML><!-- firstname --></XML>"  
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()", false, 
	  (MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()));

XML.ignoreProcessingInstructions = false;
xmlDoc = "<XML><?xml-stylesheet href=\"classic.xsl\" type=\"text/xml\"?></XML>"  
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()", false, 
	  (MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()));

XML.ignoreComments = false;
xmlDoc = "<XML>foo</XML>"  
AddTestCase( "MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()", false, 
	  (MYXML = new XMLList(xmlDoc), MYXML.hasComplexContent()));
	  
END();
