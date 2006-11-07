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

    var SECTION = "15.9.5.15";
    var VERSION = "ECMA_1";
    startTest();
    var TITLE   = "Date.prototype.getUTCHours()";

    writeHeaderToLog( SECTION + " "+ TITLE);

    var testcases = getTestCases();
    test();
    
function getTestCases() {
    var array = new Array();
    var item = 0;    

    var TZ_ADJUST = TZ_DIFF * msPerHour;

    // get the current time
    var now = (new Date()).valueOf();

    // calculate time for year 0
    for ( var time = 0, year = 1969; year >= 0; year-- ) {
        time -= TimeInYear(year);
    }


    // get time for 29 feb 2000
    var UTC_FEB_29_2000 = TIME_2000 + 31*msPerDay + 28*msPerHour;


    // get time for 1 jan 2005
    var UTC_JAN_1_2005 = TIME_2000 + TimeInYear(2000)+TimeInYear(2001)+
    TimeInYear(2002)+TimeInYear(2003)+TimeInYear(2004);

    addTestCase2( now );
    addTestCase( time );
    addTestCase( TIME_1970 );
    addTestCase( TIME_1900 );
    addTestCase( TIME_2000 );
    addTestCase( UTC_FEB_29_2000 );
    addTestCase( UTC_JAN_1_2005 );

    array[item++] = new TestCase( SECTION,
                                    "(new Date(NaN)).getUTCHours()",
                                    NaN,
                                    (new Date(NaN)).getUTCHours() );

    array[item++] = new TestCase( SECTION,
                                    "Date.prototype.getUTCHours.length",
                                    0,
                                    Date.prototype.getUTCHours.length );
    
    function addTestCase( t ) {
        for ( h = 0; h < 24; h+=3 ) {
                t += msPerHour;
                array[item++] = new TestCase( SECTION,
                                        "(new Date("+t+")).getUTCHours()",
                                        true, HourFromTime((t)) ==
                                        (new Date(t)).getUTCHours() );
        }
    }


    //used only for current Date
    function addTestCase2( t ) {
        for ( h = 0; h < 24; h+=3 ) {
                t += msPerHour;
                array[item++] = new TestCase( SECTION,
                                        "(new Date(currentDate)).getUTCHours()",
                                        true, HourFromTime((t)) ==
                                        (new Date(t)).getUTCHours() );
        }
    }

    return array;
}
