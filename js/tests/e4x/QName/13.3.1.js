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
 *   Igor Bukanov
 *   Ethan Hugg
 *   Milen Nankov
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

START("13.3.1 - QName Constructor as a Function");

q = QName("foobar");
p = new QName("foobar");
TEST(1, typeof(p), typeof(q));
TEST(2, p.localName, q.localName);
TEST(3, p.uri, q.uri);

q = QName("http://foobar/", "foobar");
p = new QName("http://foobar/", "foobar");
TEST(4, typeof(p), typeof(q));
TEST(5, p.localName, q.localName);
TEST(6, p.uri, q.uri);

p1 = QName(q);
p2 = new QName(q);
TEST(7, typeof(p2), typeof(p1));
TEST(8, p2.localName, p1.localName);
TEST(9, p2.uri, p1.uri);

n = new Namespace("http://foobar/");
q = QName(n, "foobar");
p = QName(n, "foobar");
TEST(10, typeof(p), typeof(q));
TEST(11, p.localName, q.localName);
TEST(12, p.uri, q.uri);

p = QName(q);
TEST(13, p, q);

END();