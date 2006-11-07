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

START("11.4.1 - Addition Operator");

employeeData = <name>Fred</name> + <age>28</age> + <hobby>skiing</hobby>;
TEST(1, "xml", typeof(employeeData));
correct = <><name>Fred</name><age>28</age><hobby>skiing</hobby></>;
TEST(2, correct, employeeData);    

order = <order>
        <item>
            <description>Big Screen Television</description>
        </item>
        <item>
            <description>DVD Player</description>
        </item>
        <item>
            <description>CD Player</description>
        </item>
        <item>
            <description>8-Track Player</description>
        </item>
        </order>;

correct =
<item><description>Big Screen Television</description></item> +
<item><description>CD Player</description></item> + 
<item><description>8-Track Player</description></item>;

myItems = order.item[0] + order.item[2] + order.item[3];
TEST(3, "xml", typeof(myItems));
TEST(4, correct, myItems);

correct =
<item><description>Big Screen Television</description></item> +
<item><description>DVD Player</description></item> + 
<item><description>CD Player</description></item> + 
<item><description>8-Track Player</description></item> +
<item><description>New Item</description></item>;

newItems = order.item + <item><description>New Item</description></item>;
TEST(5, "xml", typeof(newItems));
TEST(6, correct, newItems);

order = 
<order>
    <item>
        <description>Big Screen Television</description>
        <price>1299.99</price>
    </item>
    <item>
        <description>DVD Player</description>
        <price>399.99</price>
    </item>
    <item>
        <description>CD Player</description>
        <price>199.99</price>
    </item>
    <item>
        <description>8-Track Player</description>
        <price>69.99</price>
    </item>
</order>;


totalPrice = +order.item[0].price + +order.item[1].price;
TEST(7, "number", typeof(totalPrice));
TEST(8, 1699.98, totalPrice);

totalPrice = +order.item[1].price + +order.item[3].price;
TEST(9, 469.98, totalPrice);


order = 
<order>
    <customer>
        <address>
            <street>123 Foobar Ave.</street>
            <city>Bellevue</city>
            <state>WA</state>
            <zip>98008</zip>
        </address>
     </customer>
</order>;

streetCity = "" + order.customer.address.street + order.customer.address.city;
TEST(10, "string", typeof(streetCity));
TEST(11, "123 Foobar Ave.Bellevue", streetCity);


statezip = String(order.customer.address.state) + order.customer.address.zip;
TEST(12, "string", typeof(statezip));
TEST(13, "WA98008", statezip);

// XML + XML

var x1, y1, z1;

x1 = new XML("<a><b><c>A</c><d>B</d></b></a>");
y1 = new XML("<a><b><c>C</c><d>D</d></b></a>");
z1 = new XMLList("<a><b><c>A</c><d>B</d></b></a><a><b><c>C</c><d>D</d></b></a>");

AddTestCase( "XML     + XML:     ", true, ((x1+y1)==z1) );


// XML + XMLList

x1 = new XML("<a><b><c>A</c><d>B</d></b></a>");
y1 = new XMLList("<e>C</e><e>D</e><e>E</e>");
z1 = new XMLList("<a><b><c>A</c><d>B</d></b></a><e>C</e><e>D</e><e>E</e>");

AddTestCase( "XML     + XMLList: ", true, ((x1+y1)==z1) );


// XMLList + XML

x1 = new XMLList("<e>C</e><e>D</e><e>E</e>");
y1 = new XML("<a><b><c>A</c><d>B</d></b></a>");
z1 = new XMLList("<e>C</e><e>D</e><e>E</e><a><b><c>A</c><d>B</d></b></a>");

AddTestCase( "XMLList + XML:     ", true, ((x1+y1)==z1) );


// XMLList + XMLList

x1 = new XMLList("<a>A</a><a>B</a><a>C</a>");
y1 = new XMLList("<a>D</a><a>E</a><a>F</a>");
z1 = new XMLList("<a>A</a><a>B</a><a>C</a><a>D</a><a>E</a><a>F</a>");

AddTestCase( "XMLList + XMLList: ", true, ((x1+y1)==z1) );

END();
