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
 * The Original Code is JavaScript Engine testing utilities.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Brendan Eich
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
//-----------------------------------------------------------------------------
var bug = 350312;
var summary = 'Accessing wrong stack slot with nested catch/finally';
var actual = '';
var expect = '';


//-----------------------------------------------------------------------------
test();
//-----------------------------------------------------------------------------

function test()
{
  enterFunc ('test');
  printBugNumber (bug);
  printStatus (summary);

  function createPrint(obj)
    {
      return new Function("actual += " + obj + " + ','; " + 
                          "writeLineToLog(" + obj + ");");
    }

  function createThrow(obj)
    {
      return new Function("throw " + obj + "; ");
    }


  function f(a, b, c)
    {
      try {
        a();
      } catch (e if e == null) {
        b();
      } finally {
        c();
      }
    }

  writeLineToLog('test 1');
  expect = 'a,c,';
  actual = '';
  try
  {
    f(createPrint("'a'"), createPrint("'b'"), createPrint("'c'"));
  }
  catch(ex)
  {
    actual += 'caught ' + ex;
  }
  reportCompare(expect, actual, summary + ': 1');

  writeLineToLog('test 2');
  expect = 'c,caught a';
  actual = '';
  try
  {
    f(createThrow("'a'"), createPrint("'b'"), createPrint("'c'"));
  }
  catch(ex)
  {
    actual += 'caught ' + ex;
  }
  reportCompare(expect, actual, summary + ': 2');

  writeLineToLog('test 3');
  expect = 'b,c,';
  actual = '';
  try
  {
    f(createThrow("null"), createPrint("'b'"), createPrint("'c'"));
  }
  catch(ex)
  {
    actual += 'caught ' + ex;
  }
  reportCompare(expect, actual, summary + ': 3');

  writeLineToLog('test 4');
  expect = 'a,c,';
  actual = '';
  try
  {
    f(createPrint("'a'"), createThrow("'b'"), createPrint("'c'"));
  }
  catch(ex)
  {
    actual += 'caught ' + ex;
  }
  reportCompare(expect, actual, summary + ': 4');

  writeLineToLog('test 5');
  expect = 'c,caught b';
  actual = '';
  try
  {
    f(createThrow("null"), createThrow("'b'"), createPrint("'c'"));
  }
  catch(ex)
  {
    actual += 'caught ' + ex;
  }
  reportCompare(expect, actual, summary + ': 5');

  exitFunc ('test');
}
