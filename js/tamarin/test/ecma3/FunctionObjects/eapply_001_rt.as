/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Communicator client code, released March
 * 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation. Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Rob Ginda rginda@netscape.com
 */

trace("STATUS: f.apply crash test.");

trace("BUGNUMBER: 21836");
startTest();

var testcases = getTestCases();

test();

function f()
{
}

function getTestCases() {
    var array = new Array();
    var item = 0;
    var thisError="no error";
    trace("The second argument of f.apply() should be an array:  "+test());

    function doTest():String
    {
        //f.apply(2,2);
    
        try{
            f.apply(2,2);
        }catch(e:Error){
            thisError=(e.toString()).substring(0,22);
        }finally{
    
        }
        return thisError;
    }

    doTest();

    array[item++] = new TestCase(SECTION, "Make sure we don't crash", true, true);

    return array;
}
