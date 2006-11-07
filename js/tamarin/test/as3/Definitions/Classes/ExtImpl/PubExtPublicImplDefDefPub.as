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

var SECTION = "Definitions";       // provide a document reference (ie, ECMA section)
var VERSION = "Clean AS2";  // Version of JavaScript or ECMA
var TITLE   = "Extend Public Class Implement Default interface";       // Provide ECMA section title or a description
var BUGNUMBER = "";

startTest();                // leave this alone


import PublicClass.*;

var PUBEXTDYNIMPLDEFDEFPUB = new PubExtPublicImplDefDefPub();

// *******************************************
// define default method from interface as a
// public method in the sub class
// *******************************************

// Should give an error

AddTestCase( "*** default(<empty>) method implemented interface ***", 1, 1 );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.testGetBoolean(false)", false, PUBEXTDYNIMPLDEFDEFPUB.testGetBoolean(false) );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.testGetBoolean(true)", true, PUBEXTDYNIMPLDEFDEFPUB.testGetBoolean(true) );

// *******************************************
// define public method from interface as a
// public method in the sub class
// *******************************************

AddTestCase( "*** public method implemented interface ***", 1, 1 );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.testGetPubBoolean(false)", false, PUBEXTDYNIMPLDEFDEFPUB.testGetPubBoolean(false) );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.testGetPubBoolean(true)", true, PUBEXTDYNIMPLDEFDEFPUB.testGetPubBoolean(true) );

// *******************************************
// define default method from interface 2 as a
// public method in the sub class
// *******************************************

//Should give an error

AddTestCase( "*** default(<empty>) method implemented interface 2 ***", 1, 1 );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.testGetSetNumber(420)", 420, PUBEXTDYNIMPLDEFDEFPUB.testGetSetNumber(420) );

// *******************************************
// define public method from interface 2 as a
// public method in the sub class
// *******************************************

AddTestCase( "*** public method implemented interface 2 ***", 1, 1 );
AddTestCase( "PUBEXTDYNIMPLDEFDEFPUB.setPubNumber(14), PUBEXTDYNIMPLDEFDEFPUB.iGetPubNumber()", 14, (PUBEXTDYNIMPLDEFDEFPUB.setPubNumber(14), PUBEXTDYNIMPLDEFDEFPUB.iGetPubNumber()) );

