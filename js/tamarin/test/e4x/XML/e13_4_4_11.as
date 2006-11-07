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

START("13.4.4.11 - XML copy()");

//TEST(1, true, XML.prototype.hasOwnProperty("copy"));

emps =
<employees>
    <employee id="0"><name>Jim</name><age>25</age></employee>
    <employee id="1"><name>Joe</name><age>20</age></employee>
</employees>;

correct =
<employee id="0"><name>Jim</name><age>25</age></employee>;

x1 = emps.employee[0].copy();

TEST(2, undefined, x1.parent());
TEST(3, correct, x1);
 
// Make sure we're getting a copy, not a ref to orig.
emps =
<employees>
    <employee id="0"><fname>Jim</fname><age>25</age></employee>
    <employee id="1"><fname>Joe</fname><age>20</age></employee>
</employees>;

correct =
<employee id="0"><fname>Jim</fname><age>25</age></employee>

empCopy = emps.employee[0].copy();

emps.employee[0].fname[0] = "Sally";

TEST(4, correct, empCopy);

// Try copying whole XML twice
emps =
<employees>
    <employee id="0"><name>Jim</name><age>25</age></employee>
    <employee id="1"><name>Joe</name><age>20</age></employee>
</employees>;

empCopy = emps.copy();
x1 = empCopy.copy();

TEST(5, x1, emps);

XML.prettyPrinting = false;
var xmlDoc = "<MLB><Team>Giants</Team><City>San Francisco</City><Team>Padres</Team><City>San Diego</City></MLB>";

AddTestCase( "MYXML = new XML(xmlDoc), XMLCOPY = MYXML.copy()", xmlDoc, 
             (MYXML = new XML(xmlDoc), XMLCOPY = MYXML.copy(), XMLCOPY.toString()) );
AddTestCase( "MYXML = new XML(null), XMLCOPY = MYXML.copy()", "", 
             (MYXML = new XML(null), XMLCOPY = MYXML.copy(), XMLCOPY.toString()) );
AddTestCase( "MYXML = new XML(undefined), XMLCOPY = MYXML.copy()", MYXML.toString(), 
             (MYXML = new XML(undefined), XMLCOPY = MYXML.copy(), XMLCOPY.toString()) );
AddTestCase( "MYXML = new XML(), XMLCOPY = MYXML.copy()", MYXML.toString(), 
             (MYXML = new XML(), XMLCOPY = MYXML.copy(), XMLCOPY.toString()) );
AddTestCase( "MYXML = new XML(xmlDoc), XMLCOPY = MYXML.Team.copy()", "<Team>Giants</Team>" + NL() + "<Team>Padres</Team>", 
             (MYXML = new XML(xmlDoc), XMLCOPY = MYXML.Team.copy(), XMLCOPY.toString()) );

// Make sure it's a copy
var MYXML = new XML(xmlDoc);
var MYXML2 = MYXML.copy();
AddTestCase ("MYXML == MYXML.copy()", true, (MYXML == MYXML.copy()));
MYXML2.foo = "bar";
AddTestCase ("MYXML == MYXML2 where MYXML2 is a copy that has been changed", false, (MYXML == MYXML2));

END();
