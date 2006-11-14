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

START("11.6.3 - Compound Assignment");

// Insert employee 3 and 4 after the first employee
e = 
<employees>
    <employee id="1">
        <name>Joe</name>
        <age>20</age>
    </employee>
    <employee id="2">
        <name>Sue</name>
        <age>30</age>
    </employee>
</employees>;

correct = 
<employees>
    <employee id="1">
        <name>Joe</name>
        <age>20</age>
    </employee>
    <employee id="3">
        <name>Fred</name>
    </employee>
    <employee id="4">
        <name>Carol</name>
    </employee>
    <employee id="2">
        <name>Sue</name>
        <age>30</age>
    </employee>
</employees>;
    
e.employee[0] += <employee id="3"><name>Fred</name></employee> +
    <employee id="4"><name>Carol</name></employee>;
    
TEST(1, correct, e);

// Append employees 3 and 4 to the end of the employee list
e = 
<employees>
    <employee id="1">
        <name>Joe</name>
        <age>20</age>
    </employee>
    <employee id="2">
        <name>Sue</name>
        <age>30</age>
    </employee>
</employees>;

correct = 
<employees>
    <employee id="1">
        <name>Joe</name>
        <age>20</age>
    </employee>
    <employee id="2">
        <name>Sue</name>
        <age>30</age>
    </employee>
    <employee id="3">
        <name>Fred</name>
    </employee>
    <employee id="4">
        <name>Carol</name>
    </employee>
</employees>;

e.employee[1] += <employee id="3"><name>Fred</name></employee> +
    <employee id="4"><name>Carol</name></employee>;
TEST(2, correct, e);
       
// XML +=

var x1 = new XML("<a><b><c>A0</c><d>A1</d></b><b><c>B0</c><d>B1</d></b><b><c>C0</c><d>C1</d></b></a>");

x1.b[1] += new XML("<b><c>D0</c><d>D1</d></b>");

var y1 = new XML("<a><b><c>A0</c><d>A1</d></b><b><c>B0</c><d>B1</d></b><b><c>D0</c><d>D1</d></b><b><c>C0</c><d>C1</d></b></a>");

AddTestCase( "XML +=     :", true, (x1==y1) );


// XMLList +=

x1 = new XMLList("<a><b>A0</b><c>A1</c></a><a><b>B0</b><c>B1</c></a><a><b>C0</b><c>C1</c></a>");

x1 += new XML("<a><b>D0</b><c>D1</c></a>");

y1 = new XMLList("<a><b>A0</b><c>A1</c></a><a><b>B0</b><c>B1</c></a><a><b>C0</b><c>C1</c></a><a><b>D0</b><c>D1</c></a>");

AddTestCase( "XMLList += :", true, (x1==y1) );


// XMLList +=, last item in XMLList is XML object with non-null parent

x1 = new XML("<a><b><c>A0</c><d>A1</d></b><b><c>B0</c><d>B1</d></b><b><c>C0</c><d>C1</d></b></a>");

x1 += new XMLList("<b><c>D0</c></b><b><c>E0</c></b>");

y1 = new XMLList("<a><b><c>A0</c><d>A1</d></b><b><c>B0</c><d>B1</d></b><b><c>C0</c><d>C1</d></b></a><b><c>D0</c></b><b><c>E0</c></b>");

AddTestCase( "XMLList += :", true, (x1==y1) );

END();
