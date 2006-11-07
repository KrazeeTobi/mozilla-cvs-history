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
// Revised 10/20/06 - cpeyer@adobe.com - Changed all instances of Number.MAX_VALUE from 1.7976931348623157e+308 to 1.79769313486231e+308 since our implementation only goes to 14 digits.


    var SECTION = "9.3.1-3";
    var VERSION = "ECMA_1";
    startTest();
    var BUGNUMBER="129087";

    var TITLE   = "Number To String, String To Number";

    writeHeaderToLog( SECTION + " "+ TITLE);

    var testcases = getTestCases();
    
    test();
    
function getTestCases() {
    var array = new Array();
    var item = 0;

    // test case from http://scopus.mcom.com/bugsplat/show_bug.cgi?id=312954
    var z = 0;

    array[item++] = new TestCase(
        SECTION,
        "var z = 0; print(1/-z)",
        -Infinity,
        1/-z );

    // test cases from bug http://scopus.mcom.com/bugsplat/show_bug.cgi?id=122882

    array[item++] = new TestCase( SECTION,
        '- -"0x80000000"',
        2147483648,
        - -"0x80000000" );

    array[item++] = new TestCase( SECTION,
        '- -"0x100000000"',
        4294967296,
        - -"0x100000000" );

    array[item++] = new TestCase( SECTION,
        '- "-0x123456789abcde8"',
        81985529216486880,
        - "-0x123456789abcde8" );

    // Convert some large numbers to string


    array[item++] = new TestCase( SECTION,
                                    "1e2000 +''",
                                    "Infinity",
                                    1e2000 +"" );
    
    array[item++] = new TestCase( SECTION,
                                    "1e2000",
                                    Infinity,
                                    1e2000 );
    
    array[item++] = new TestCase( SECTION,
                                    "-1e2000 +''",
                                    "-Infinity",
                                    -1e2000 +"" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"1e2000\"",
                                    -Infinity,
                                    -"1e2000" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"-1e2000\" +''",
                                    "Infinity",
                                    -"-1e2000" +"" );
    
    array[item++] = new TestCase( SECTION,
                                    "1e-2000",
                                    0,
                                    1e-2000 );
    
    array[item++] = new TestCase( SECTION,
                                    "1/1e-2000",
                                    Infinity,
                                    1/1e-2000 );
    
    // convert some strings to large numbers
    
    array[item++] = new TestCase( SECTION,
                                    "1/-1e-2000",
                                    -Infinity,
                                    1/-1e-2000 );
    
    array[item++] = new TestCase( SECTION,
                                        "1/\"1e-2000\"",
                                        Infinity,
                                        1/"1e-2000" );
    
    array[item++] = new TestCase( SECTION,
                                    "1/\"-1e-2000\"",
                                    -Infinity,
                                    1/"-1e-2000" );
    
    array[item++] = new TestCase( SECTION,
                                    "parseFloat(\"1e2000\")",
                                    Infinity,
                                    parseFloat("1e2000") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseFloat(\"1e-2000\")",
                                    0,
                                    parseFloat("1e-2000") );
    
    array[item++] = new TestCase( SECTION,
                                    "1.7976931348623157E+308",
                                    1.79769313486231e+308,
                                    1.7976931348623157E+308 );
    
    array[item++] = new TestCase( SECTION,
                                    "1.7976931348623158e+308",
                                    1.79769313486231e+308,
                                    1.7976931348623158e+308 );
    
    array[item++] = new TestCase( SECTION,
                                    "1.7976931348623159e+308",
                                    Infinity,
                                    1.7976931348623159e+308 );
    
    s =
    "17976931348623158079372897140530341507993413271003782693617377898044496829276475094664901797758720709633028641669288791094655554785194040263065748867150582068";
    
    array[item++] = new TestCase( SECTION,
                                    "s = " + s +"; s +="+
    "\"190890200070838367627385484581771153176447573027006985557136695962284291481986083493647529271907416844436551070434271155969950809304288017790417449779\""+
    
    +"; s",
    "17976931348623158079372897140530341507993413271003782693617377898044496829276475094664901797758720709633028641669288791094655554785194040263065748867150582068190890200070838367627385484581771153176447573027006985557136695962284291481986083493647529271907416844436551070434271155969950809304288017790417449779",
    s +=
    "190890200070838367627385484581771153176447573027006985557136695962284291481986083493647529271907416844436551070434271155969950809304288017790417449779"
    );
    
    s1 = s+1;
    
    array[item++] = new TestCase( SECTION,
    "s1 = s+1; s1",
    "179769313486231580793728971405303415079934132710037826936173778980444968292764750946649017977587207096330286416692887910946555547851940402630657488671505820681908902000708383676273854845817711531764475730270069855571366959622842914819860834936475292719074168444365510704342711559699508093042880177904174497791",
    s1 );
    
    array[item++] = new TestCase( SECTION,
    "-s1 == -1.79769313486231e+308 || -s1 == -Infinity",
    true,
    -s1 == -1.79769313486231e+308 || -s1 == -Infinity );
    
    s2 = s + 2;
    
    array[item++] = new TestCase( SECTION,
    "s2 = s+2; s2",
    "179769313486231580793728971405303415079934132710037826936173778980444968292764750946649017977587207096330286416692887910946555547851940402630657488671505820681908902000708383676273854845817711531764475730270069855571366959622842914819860834936475292719074168444365510704342711559699508093042880177904174497792",
    s2 );
    
    // ***** This answer is preferred but -1.79769313486231e+308 is also acceptable here *****
    array[item++] = new TestCase( SECTION,
    "-s2 == -Infinity || -s2 == -1.79769313486231e+308 ",
    true,
    -s2 == -Infinity || -s2 == -1.79769313486231e+308 );
    
    s3 = s+3;
    
    array[item++] = new TestCase( SECTION,
    "s3 = s+3; s3",
    "179769313486231580793728971405303415079934132710037826936173778980444968292764750946649017977587207096330286416692887910946555547851940402630657488671505820681908902000708383676273854845817711531764475730270069855571366959622842914819860834936475292719074168444365510704342711559699508093042880177904174497793",
    s3 );
    
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000080",
                                    1152921504606847000,
                                    0x1000000000000080 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000081",
                                    1152921504606847200,
                                    0x1000000000000081 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000100",
                                    1152921504606847200,
                                    0x1000000000000100 );
    array[item++] = new TestCase( SECTION,
                                    "0x100000000000017f",
                                    1152921504606847200,
                                    0x100000000000017f );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000180",
                                    1152921504606847400,
                                    0x1000000000000180 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000181",
                                    1152921504606847400,
                                    0x1000000000000181 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x10000000000001f0",
                                    1152921504606847400,
                                    0x10000000000001f0 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000200",
                                    1152921504606847400,
                                    0x1000000000000200 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x100000000000027f",
                                    1152921504606847400,
                                    0x100000000000027f );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000280",
                                    1152921504606847400,
                                    0x1000000000000280 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000281",
                                    1152921504606847700,
                                    0x1000000000000281 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x10000000000002ff",
                                    1152921504606847700,
                                    0x10000000000002ff );
    
    array[item++] = new TestCase( SECTION,
                                    "0x1000000000000300",
                                    1152921504606847700,
                                    0x1000000000000300 );
    
    array[item++] = new TestCase( SECTION,
                                    "0x10000000000000000",
                                    18446744073709552000,
                                    0x10000000000000000 );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"000000100000000100100011010001010110011110001001101010111100\",2)",
    9027215253084860,
    parseInt("000000100000000100100011010001010110011110001001101010111100",2) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"000000100000000100100011010001010110011110001001101010111101\",2)",
    9027215253084860,
    parseInt("000000100000000100100011010001010110011110001001101010111101",2) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"000000100000000100100011010001010110011110001001101010111111\",2)",
    9027215253084864,
    parseInt("000000100000000100100011010001010110011110001001101010111111",2) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"0000001000000001001000110100010101100111100010011010101111010\",2)",
    18054430506169720,
    parseInt("0000001000000001001000110100010101100111100010011010101111010",2));
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"0000001000000001001000110100010101100111100010011010101111011\",2)",
    18054430506169724,
    parseInt("0000001000000001001000110100010101100111100010011010101111011",2) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"0000001000000001001000110100010101100111100010011010101111100\",2)",
    18054430506169724,
    parseInt("0000001000000001001000110100010101100111100010011010101111100",2));
    
    array[item++] = new TestCase( SECTION,
    "parseInt(\"0000001000000001001000110100010101100111100010011010101111110\",2)",
    18054430506169728,
    parseInt("0000001000000001001000110100010101100111100010011010101111110",2));
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"yz\",35)",
                                    34,
                                    parseInt("yz",35) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"yz\",36)",
                                    1259,
                                    parseInt("yz",36) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"yz\",37)",
                                    NaN,
                                    parseInt("yz",37) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"+77\")",
                                    77,
                                    parseInt("+77") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"-77\",9)",
                                    -70,
                                    parseInt("-77",9) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"\\u20001234\\u2000\")",
                                    1234,
                                    parseInt("\u20001234\u2000") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"123456789012345678\")",
                                    123456789012345700,
                                    parseInt("123456789012345678") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"9\",8)",
                                    NaN,
                                    parseInt("9",8) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"1e2\")",
                                    1,
                                    parseInt("1e2") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"1.9999999999999999999\")",
                                    1,
                                    parseInt("1.9999999999999999999") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0x10\")",
                                    16,
                                    parseInt("0x10") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0x10\",10)",
                                    0,
                                    parseInt("0x10",10) );
    
    /*array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0022\")",
                                    18,
                                    parseInt("0022") );
    */
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0022\",10)",
                                    22,
                                    parseInt("0022",10) );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0x1000000000000080\")",
                                    1152921504606847000,
                                    parseInt("0x1000000000000080") );
    
    array[item++] = new TestCase( SECTION,
                                    "parseInt(\"0x1000000000000081\")",
                                    1152921504606847200,
                                    parseInt("0x1000000000000081") );
    
    s =
    "0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    
    array[item++] = new TestCase( SECTION, "s = "+
    "\"0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\";"+
    "s",
    "0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s );
    
    
    array[item++] = new TestCase( SECTION, "s +="+
    "\"0000000000000000000000000000000000000\"; s",
    "0xFFFFFFFFFFFFF800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s += "0000000000000000000000000000000000000" );
    
    array[item++] = new TestCase( SECTION, "-s",
    -1.79769313486231e+308,
    -s );
    
    s =
    "0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    
    array[item++] = new TestCase( SECTION, "s ="+
    "\"0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\";"+
    "s",
    "0xFFFFFFFFFFFFF80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s );
    
    array[item++] = new TestCase( SECTION,
    "s += \"0000000000000000000000000000000000001\"",
    "0xFFFFFFFFFFFFF800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001",
    s += "0000000000000000000000000000000000001" );
    
    array[item++] = new TestCase( SECTION,
    "-s",
    -1.79769313486231e+308,
    -s );
    
    s =
    "0xFFFFFFFFFFFFFC0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    
    array[item++] = new TestCase( SECTION,
    "s ="+
    "\"0xFFFFFFFFFFFFFC0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\";"+
    "s",
    "0xFFFFFFFFFFFFFC0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s );
    
    
    array[item++] = new TestCase( SECTION,
    "s += \"0000000000000000000000000000000000000\"",
    "0xFFFFFFFFFFFFFC00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s += "0000000000000000000000000000000000000");
    
    
    array[item++] = new TestCase( SECTION,
    "-s",
    -Infinity,
    -s );
    
    s =
    "0xFFFFFFFFFFFFFB0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    
    array[item++] = new TestCase( SECTION,
    "s = "+
    "\"0xFFFFFFFFFFFFFB0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\";s",
    "0xFFFFFFFFFFFFFB0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    s);
    
    array[item++] = new TestCase( SECTION,
    "s += \"0000000000000000000000000000000000001\"",
    "0xFFFFFFFFFFFFFB00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001",
    s += "0000000000000000000000000000000000001" );
    
    array[item++] = new TestCase( SECTION,
    "-s",
    -1.79769313486231e+308,
    -s );
    
    array[item++] = new TestCase( SECTION,
    "s += \"0\"",
    "0xFFFFFFFFFFFFFB000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010",
    s += "0" );
    
    array[item++] = new TestCase( SECTION,
    "-s",
    -Infinity,
    -s );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(s)",
    Infinity,
    parseInt(s) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(s,32)",
    0,
    parseInt(s,32) );
    
    array[item++] = new TestCase( SECTION,
    "parseInt(s,36)",
    Infinity,
    parseInt(s,36) );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"\"",
                                    0,
                                    -"" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\" \"",
                                    0,
                                    -" " );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"999\"",
                                    -999,
                                    -"999" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\" 999\"",
                                    -999,
                                    -" 999" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"\\t999\"",
                                    -999,
                                    -"\t999" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"013  \"",
                                    -13,
                                    -"013  " );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"999\\t\"",
                                    -999,
                                    -"999\t" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"-Infinity\"",
                                    Infinity,
                                    -"-Infinity" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"-infinity\"",
                                    NaN,
                                    -"-infinity" );
    
    
    array[item++] = new TestCase( SECTION,
                                    "-\"+Infinity\"",
                                    -Infinity,
                                    -"+Infinity" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"+Infiniti\"",
                                    NaN,
                                    -"+Infiniti" );
    
    array[item++] = new TestCase( SECTION,
                                    "- -\"0x80000000\"",
                                    2147483648,
                                    - -"0x80000000" );
    
    array[item++] = new TestCase( SECTION,
                                    "- -\"0x100000000\"",
                                    4294967296,
                                    - -"0x100000000" );
    
    array[item++] = new TestCase( SECTION,
                                    "- \"-0x123456789abcde8\"",
                                    81985529216486880,
                                    - "-0x123456789abcde8" );
    
    // the following two tests are not strictly ECMA 1.0
    
    array[item++] = new TestCase( SECTION,
                                    "-\"\\u20001234\\u2001\"",
                                    -1234,
                                    -"\u20001234\u2001" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"\\u20001234\\0\"",
                                    -12340,
                                    -"\u20001234\0" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"0x10\"",
                                    -16,
                                    -"0x10" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"+\"",
                                    NaN,
                                    -"+" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"-\"",
                                    NaN,
                                    -"-" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"-0-\"",
                                    NaN,
                                    -"-0-" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"1e-\"",
                                    NaN,
                                    -"1e-" );
    
    array[item++] = new TestCase( SECTION,
                                    "-\"1e-1\"",
                                    -0.1,
                                    -"1e-1" );
    return array;
}
