/* 
  The contents of this file are subject to the Netscape Public
  License Version 1.1 (the "License"); you may not use this file
  except in compliance with the License. You may obtain a copy of
  the License at http://www.mozilla.org/NPL/
  
  Software distributed under the License is distributed on an "AS
  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
  implied. See the License for the specific language governing
  rights and limitations under the License.
  
  The Original Code is Mozilla Communicator client code, released
  March 31, 1998.
  
  The Initial Developer of the Original Code is David Hyatt. 
  Portions created by David Hyatt are
  Copyright (C) 2002 David Hyatt. All
  Rights Reserved.
  
  Contributor(s): 
    David Hyatt (hyatt@apple.com)

*/

function buildDialog()
{
  var toolbar = window.opener.document.getElementById("nav-bar");
  var cloneToolbarBox = document.getElementById("cloned-bar-container");
  var newToolbar = toolbar.cloneNode(true);
  cloneToolbarBox.appendChild(newToolbar);
}
