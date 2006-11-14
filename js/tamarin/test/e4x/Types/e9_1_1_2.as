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

START('9.1.1.2 - XML [[Put]]');


// .
var x1 = 
<alpha attr1="value1" attr2="value2">
    <bravo>
        one
        <charlie>two</charlie>
    </bravo>
</alpha>;

var correct =
<charlie>new</charlie>;

x1.bravo.charlie = "new"
TEST(1, correct, x1.bravo.charlie)
x1.bravo = <delta>three</delta>
TEST(2, "three", x1.delta.toString())

// .@
x1 = <alpha attr1="value1" attr2="value2"><bravo>one<charlie>two</charlie></bravo></alpha>
x1.@attr1 = "newValue"
TEST_XML(3, "newValue", x1.@attr1)

x1 = <alpha attr1="value1" name="value2"><bravo>one<charlie>two</charlie></bravo></alpha>
x1.@name = "foo";
TEST_XML(4, "foo", x1.@name)

var a = <a><b><c/></b></a>;

try {
	a.b[0] = a;
	result = a;
} catch (e1) {
	result = typeError(e1.toString());
}


// This might fail in tomorrow's build. 11/02/05
a = <a><b/>some junk<c/>some junk<d/>some junk</a>;
correct = <a><b/>some junk<c/>some junk<d/>some junk<newnode>with some text</newnode></a>;

AddTestCase("blah", correct, (a.*::foo = <newnode>with some text</newnode>, a));


END();
