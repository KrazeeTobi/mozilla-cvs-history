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

START("13.4.4.23 - XML getNamespace()");

//TEST(1, true, XML.prototype.hasOwnProperty("getNamespace"));

// Prefix case
x1 =
<foo:alpha xmlns:foo="http://foo/" xmlns:bar="http://bar/">
    <foo:bravo>one</foo:bravo>
</foo:alpha>;

//y = x1.getNamespace();
y1 = myGetNamespace(x1);
TEST(2, "object", typeof(y1));
TEST(3, Namespace("http://foo/"), y1);

//y = x1.getNamespace("bar");
y1 = myGetNamespace(x1, "bar");
TEST(4, "object", typeof(y1));
TEST(5, Namespace("http://bar/"), y1);

// No Prefix Case
x1 =
<alpha xmlns="http://foobar/">
    <bravo>one</bravo>
</alpha>;

//y = x1.getNamespace();
y1 = myGetNamespace(x1);
TEST(6, "object", typeof(y1));
TEST(7, Namespace("http://foobar/"), y1);

// No Namespace case
x1 =
<alpha>
    <bravo>one</bravo>
</alpha>;
//TEST(8, Namespace(""), x1.getNamespace());
TEST(8, Namespace(""), myGetNamespace(x1));

// Namespaces of attributes
x1 =
<alpha xmlns="http://foo/">
    <ns:bravo xmlns:ns="http://bar" name="two" ns:value="three">one</ns:bravo>
</alpha>;

var ns = new Namespace("http://bar");
//y1 = x1.ns::bravo.@name.getNamespace();
y1 = myGetNamespace(x1.ns::bravo.@name);
TEST(9, Namespace(""), y1);

//y1 = x1.ns::bravo.@ns::value.getNamespace();
y1 = myGetNamespace(x1.ns::bravo.@ns::value);
TEST(10, ns.toString(), y1.toString());

var xmlDoc = "<?xml version=\"1.0\"?><xsl:stylesheet xmlns:xsl=\"http://www.w3.org/TR/xsl\"><b><c xmlns:foo=\"http://www.foo.org/\">hi</c></b></xsl:stylesheet>";
var ns1 = Namespace ("xsl", "http://www.w3.org/TR/xsl");
var ns2 = Namespace ("foo", "http://www.foo.org");


AddTestCase( "MYXML = new XML(xmlDoc), MYXML.getNamespace()", 
	"http://www.w3.org/TR/xsl", 
	(MYXML = new XML(xmlDoc), myGetNamespace(MYXML).toString()));

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.getNamespace()", 
	new Namespace("http://www.w3.org/TR/xsl"), 
	(MYXML = new XML(xmlDoc), myGetNamespace(MYXML)));

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.getNamespace('stylesheet')", 
	new Namespace ("http://www.w3.org/TR/xsl"), 
	(MYXML = new XML(xmlDoc), myGetNamespace(MYXML, 'xsl')));

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.getNamespace('xsl')", 
	"http://www.w3.org/TR/xsl", (MYXML = new XML(xmlDoc), 
	myGetNamespace(MYXML, 'xsl').toString()));

AddTestCase( "MYXML = new XML(xmlDoc), MYXML.getNamespace('foo')", 
	undefined, (MYXML = new XML(xmlDoc), 
	myGetNamespace(MYXML, 'foo')));


AddTestCase( "MYXML = new XML(xmlDoc), MYXML.b.c.getNamespace('foo')", 
	"http://www.foo.org/", (MYXML = new XML(xmlDoc), 
	myGetNamespace(MYXML.b.c, 'foo').toString()));
	
	
x1 =
<><foo:alpha xmlns:foo="http://foo/" xmlns:bar="http://bar/">
<foo:bravo>one</foo:bravo>
</foo:alpha><a>b</a></>;

try {
	ns = x1.namespace();
	result = ns;
} catch (e1) {
	result = typeError(e1.toString());
}

AddTestCase("Calling namespace on list with two items", "TypeError: Error #1086", result);

END();
