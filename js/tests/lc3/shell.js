/* The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is Mozilla Communicator client code, released March
 * 31, 1998.
 * 
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation. Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation. All Rights Reserved.
 * 
 */
var completed = false;
var testcases;
var tc;
var DT;

TITLE   = "";
SECTION = "";
VERSION = "";
BUGNUMBER="";

var GLOBAL= "[object global]";
var TT = "";
var TT_ = "";
var BR = "";
var NBSP = " ";
var CR = "\n";
var FONT = "";
var FONT_ = "";
var FONT_RED = "";
var FONT_GREEN = "";
var B = "";
var B_ = ""
var H2 = "";
var H2_ = "";
var HR = "";

var PASSED = " PASSED!"
var FAILED = " FAILED! expected: ";

function test() {
    for ( tc=0; tc < testcases.length; tc++ ) {
        testcases[tc].passed = writeTestCaseResult(
                            testcases[tc].expect,
                            testcases[tc].actual,
                            testcases[tc].description +" = "+
                            testcases[tc].actual );

        testcases[tc].reason += ( testcases[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcases );
}

function TestCase( d, e, a ) {
    this.name        = SECTION;
    this.description = d;
    this.expect      = e;
    this.actual      = a;
    this.passed      = true;
    this.reason      = "";
    this.bugnumber   = BUGNUMBER;

    this.passed = getTestCaseResult( this.expect, this.actual );
}
function startTest() {
    testcases = new Array();
    tc = 0;

    //  JavaScript 1.3 is supposed to be compliant ecma version 1.0
    if ( VERSION == "JS1_4" ) {
        version( 140 );
    }
    if ( VERSION == "ECMA_1" ) {
        version ( "130" );
    }
    if ( VERSION == "JS1_3" ) {
        version ( "130" );
    }
    if ( VERSION == "JS1_2" ) {
        version ( "120" );
    }
    if ( VERSION  == "JS1_1" ) {
        version ( "110" );
    }

    writeHeaderToLog( SECTION + " "+ TITLE);

    // verify that DataTypeClass is on the CLASSPATH

    DT = Packages.com.netscape.javascript.qa.liveconnect.DataTypeClass;

    if ( typeof DT != "function" ) {
        throw "Test Exception:  "+
        "com.netscape.javascript.qa.liveconnect.DataTypeClass "+
        "is not on the CLASSPATH";
    }
}

function getTestCaseResult( expect, actual ) {
    //  because ( NaN == NaN ) always returns false, need to do
    //  a special compare to see if we got the right result.
        if ( actual != actual ) {
            if ( typeof actual == "object" ) {
                actual = "NaN object";
            } else {
                actual = "NaN number";
            }
        }
        if ( expect != expect ) {
            if ( typeof expect == "object" ) {
                expect = "NaN object";
            } else {
                expect = "NaN number";
            }
        }

        var passed = ( expect == actual ) ? true : false;

    //  if both objects are numbers
    // need to replace w/ IEEE standard for rounding
        if (    !passed
                && typeof(actual) == "number"
                && typeof(expect) == "number"
            ) {
                if ( Math.abs(actual-expect) < 0.0000001 ) {
                    passed = true;
                }
        }

    //  verify type is the same
        if ( typeof(expect) != typeof(actual) ) {
            passed = false;
        }

        return passed;
}
function writeTestCaseResult( expect, actual, string ) {
        var passed = getTestCaseResult( expect, actual );
        writeFormattedResult( expect, actual, string, passed );
        return passed;
}
function writeFormattedResult( expect, actual, string, passed ) {
        var s = TT + string ;

        for ( k = 0;
              k <  (60 - string.length >= 0 ? 60 - string.length : 5) ;
              k++ ) {
//              s += NBSP;
        }

        s += B ;
        s += ( passed ) ? FONT_GREEN + NBSP + PASSED : FONT_RED + NBSP + FAILED + expect + TT_ ;

        writeLineToLog( s + FONT_ + B_ + TT_ );

        return passed;
}

function writeLineToLog( string ) {
    print( string + BR + CR );
}
function writeHeaderToLog( string ) {
    print( H2 + string + H2_ );
}
function stopTest() {
    var gc;

    if ( gc != undefined ) {
        gc();
    }
    print( HR );
}
function getFailedCases() {
  for ( var i = 0; i < testcases.length; i++ ) {
     if ( ! testcases[i].passed ) {
        print( testcases[i].description +" = " +testcases[i].actual +" expected: "+ testcases[i].expect );
     }
  }
}
