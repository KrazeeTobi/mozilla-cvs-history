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

START("13.4.4.40 - valueOf");

//TEST(1, true, XML.prototype.hasOwnProperty("valueOf"));

x1 =
<alpha>
    <bravo>one</bravo>
</alpha>;

TEST(2, x1, x1.valueOf());

// Make sure they're the same and not copies

x1 =
<alpha>
    <bravo>one</bravo>
</alpha>;

y1 = x1.valueOf();

x1.bravo = "two";

TEST(3, x1, y1);

var xmlDoc = "<employees><employee id=\"1\"><firstname>John</firstname><lastname>Walton</lastname><age>25</age></employee></employees>"


// propertyName as a string
var MYXML = new XML(xmlDoc);
AddTestCase( "MYXML = new XML(xmlDoc),MYXML.valueOf() === MYXML", 
	true, 
	(MYXML.valueOf() === MYXML));
MYXML = new XML();
AddTestCase( "MYXML = new XML(),MYXML.valueOf() === MYXML", 
	true, 
	(MYXML.valueOf() === MYXML));
MYXML = new XML('');
AddTestCase( "MYXML = new XML(''),MYXML.valueOf() === MYXML", 
	true, 
	(MYXML.valueOf() === MYXML));
MYXML = new XML('foo');
AddTestCase( "MYXML = new XML('foo'),MYXML.valueOf() === MYXML", 
	true, 
	(MYXML.valueOf() === MYXML));

END();
