<?xml version="1.0"?>
<!-- ***** BEGIN LICENSE BLOCK *****
   - Version: MPL 1.1/GPL 2.0/LGPL 2.1
   -
   - The contents of this file are subject to the Mozilla Public License Version
   - 1.1 (the "License"); you may not use this file except in compliance with
   - the License. You may obtain a copy of the License at
   - http://www.mozilla.org/MPL/
   -
   - Software distributed under the License is distributed on an "AS IS" basis,
   - WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
   - for the specific language governing rights and limitations under the
   - License.
   -
   - The Original Code is OEone Calendar Code, released October 31st, 2001.
   -
   - The Initial Developer of the Original Code is
   - OEone Corporation.
   - Portions created by the Initial Developer are Copyright (C) 2001
   - the Initial Developer. All Rights Reserved.
   -
   - Contributor(s): Garth Smedley <garths@oeone.com> 
   -                 Mike Potter <mikep@oeone.com>
   -
   - Alternatively, the contents of this file may be used under the terms of
   - either the GNU General Public License Version 2 or later (the "GPL"), or
   - the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
   - in which case the provisions of the GPL or the LGPL are applicable instead
   - of those above. If you wish to allow use of your version of this file only
   - under the terms of either the GPL or the LGPL, and not to allow others to
   - use your version of this file under the terms of the MPL, indicate your
   - decision by deleting the provisions above and replace them with the notice
   - and other provisions required by the LGPL or the GPL. If you do not delete
   - the provisions above, a recipient may use your version of this file under
   - the terms of any one of the MPL, the GPL or the LGPL.
   -
   - ***** END LICENSE BLOCK ***** -->
   
<?xml-stylesheet href="chrome://global/skin/global.css" type="text/css"?> 

<!-- CSS File with all styles specific to the unifinder -->
<?xml-stylesheet href="chrome://calendar/skin/unifinder/unifinder.css" ?>

<!-- DTD File with all strings specific to the unifinder -->
<!DOCTYPE window 
[
    <!ENTITY % dtd1 SYSTEM "chrome://calendar/locale/global.dtd" > %dtd1;
    <!ENTITY % dtd2 SYSTEM "chrome://calendar/locale/unifinder.dtd" > %dtd2;
	<!ENTITY % dtd3 SYSTEM "chrome://calendar/locale/calendar.dtd" > %dtd3;
]>

<?xul-overlay href="chrome://calendar/content/unifinder/unifinderOverlay.xul"?>

<overlay
   id="unifinder-overlay"
   xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"> 
   
<!-- Javascript includes -->



   <unifinder id="calendar-unifinder-overlay">
   
        <!--
        <unifinder-button id="unifinder-new-category-button" class="unifinder-new-category-button" tooltip="aTooltip" tooltiptext="&unifinder.toolbar.newfolder.tooltip;" oncommand="unifinderNewCategoryCommand()" />
        -->
        <unifinder-button id="unifinder-remove-button" class="unifinder-remove-button" tooltip="aTooltip" tooltiptext="&calendar.tools.delete.tooltip;" label="&calendar.delete.label;" oncommand="unifinderRemoveCommand()" />
        <unifinder-button id="unifinder-modify-button" class="unifinder-modify-button" tooltip="aTooltip" tooltiptext="&calendar.tools.modify.tooltip;" label="&calendar.modify.label;" oncommand="unifinderModifyCommand()" />
 
        <box flex="1">
            <tabbox   class="unifinder-tab-box" unifinder-tab-box="true" orient="vertical" flex="1" >
                <tabs  class="unifinder-tab" orient="horizontal" >
                    <tab label="&calendar.main.tab.label;" />
                    <tab label="&unifinder.tab.search.label;" />
                </tabs>
                
                <spacer class="unifinder-tabs-spacer" />
                
                <tabpanels  flex="1">
                    <vbox id="unifinder-categories-box" flex="1"/>
                    <vbox flex="1">
                        <unifinder-search-controls keypressfunction="unifinderSearchKeyPress( this, event )" />
                        <tree id="unifinder-search-results-tree" flex="1" />
                    </vbox>
                </tabpanels>
            </tabbox>
        </box>
   </unifinder>     

  </overlay>

