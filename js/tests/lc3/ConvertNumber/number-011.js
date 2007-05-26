/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

gTestfile = 'number-011.js';

/**
 *  Preferred Argument Conversion.
 *
 *  Use the explicit method invocation syntax to override the preferred
 *  argument conversion.
 *
 */
var SECTION = "Preferred argument conversion:  undefined";
var VERSION = "1_4";
var TITLE   = "LiveConnect 3.0 JavaScript to Java Data Type Conversion " +
  SECTION;
startTest();

var TEST_CLASS = new
  Packages.com.netscape.javascript.qa.lc3.number.Number_001;

new TestCase(
  "TEST_CLASS[\"ambiguous(java.lang.Object)\"](1)",
  "OBJECT",
  TEST_CLASS["ambiguous(java.lang.Object)"](1) +'');


new TestCase(
  "TEST_CLASS[\"ambiguous(java.lang.String)\"](1)",
  "STRING",
  TEST_CLASS["ambiguous(java.lang.String)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(byte)\"](1)",
  "BYTE",
  TEST_CLASS["ambiguous(byte)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(char)\"](1)",
  "CHAR",
  TEST_CLASS["ambiguous(char)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(short)\"](1)",
  "SHORT",
  TEST_CLASS["ambiguous(short)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(int)\"](1)",
  "INT",
  TEST_CLASS["ambiguous(int)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(long)\"](1)",
  "LONG",
  TEST_CLASS["ambiguous(long)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(float)\"](1)",
  "FLOAT",
  TEST_CLASS["ambiguous(float)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(java.lang.Double)\"](1)",
  "DOUBLE_OBJECT",
  TEST_CLASS["ambiguous(java.lang.Double)"](1) +'');

new TestCase(
  "TEST_CLASS[\"ambiguous(double)\"](1)",
  "DOUBLE",
  TEST_CLASS["ambiguous(double)"](1) +'');

test();
