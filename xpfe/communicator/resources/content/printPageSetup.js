/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): 
 *   Masaki Katakai <katakai@japan.sun.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Asko Tontti <atontti@cc.hut.fi>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

var dialog;
var printSettings = null;
var stringBundle  = null;
var paperArray;

var gPrintSettingsInterface = Components.interfaces.nsIPrintSettings;
var doDebug            = false;

//---------------------------------------------------
function initDialog()
{
  dialog = new Object;

  dialog.paperList       = document.getElementById("paperList");
  dialog.paperGroup      = document.getElementById("paperGroup");
  
  dialog.orientation     = document.getElementById("orientation");
  dialog.printBGColors   = document.getElementById("printBGColors");
  dialog.printBGImages   = document.getElementById("printBGImages");

  dialog.topInput        = document.getElementById("topInput");
  dialog.bottomInput     = document.getElementById("bottomInput");
  dialog.leftInput       = document.getElementById("leftInput");
  dialog.rightInput      = document.getElementById("rightInput");

  dialog.hLeftInput      = document.getElementById("hLeftInput");
  dialog.hCenterInput    = document.getElementById("hCenterInput");
  dialog.hRightInput     = document.getElementById("hRightInput");

  dialog.fLeftInput      = document.getElementById("fLeftInput");
  dialog.fCenterInput    = document.getElementById("fCenterInput");
  dialog.fRightInput     = document.getElementById("fRightInput");

  dialog.scalingInput    = document.getElementById("scalingInput");

  dialog.enabled         = false;
}

//---------------------------------------------------
function checkValid(elementID)
{
  var editField = document.getElementById( elementID );
  if ( !editField )
    return;
  var stringIn = editField.value;
  if (stringIn && stringIn.length > 0)
  {
    stringIn = stringIn.replace(/[^\.|^0-9]/g,"");
    if (!stringIn) stringIn = "";
    editField.value = stringIn;
  }
}

//---------------------------------------------------
function stripTrailingWhitespace(element)
{
  var stringIn = element.value;
  stringIn = stringIn.replace(/\s+$/,"");
  element.value = stringIn;
}

//---------------------------------------------------
function getDoubleStr( val, dec )
{
  var str = val.toString();
  var inx = str.indexOf(".");
  return str.substring(0, inx+dec+1);
}

//---------------------------------------------------
function listElement( aListElement )
  {
    this.listElement = aListElement;
  }

listElement.prototype =
  {
    clearList:
      function ()
        {
          // remove the menupopup node child of the menulist.
          this.listElement.removeChild( this.listElement.firstChild );
        },

    appendPaperNames: 
      function ( aDataObject ) 
        { 
          var popupNode = document.createElement( "menupopup" ); 
          for (var i=0;i<aDataObject.length;i++)  {
            var paperObj = aDataObject[i];
            var itemNode = document.createElement( "menuitem" );
            try {
              var label = stringBundle.GetStringFromName(paperObj.name)
              itemNode.setAttribute( "label", label );
              itemNode.setAttribute( "value", i );
              popupNode.appendChild( itemNode );
            } catch (e) {}
          }
          this.listElement.appendChild( popupNode ); 
        } 
  };

//---------------------------------------------------
function createPaperArray()
{
  var paperNames   = ["letterSize", "legalSize", "exectiveSize", "a4Size", "a3Size"];
  //var paperNames   = ["&letterRadio.label;", "&legalRadio.label;", "&exectiveRadio.label;", "&a4Radio.label;", "&a3Radio.label;"];
  var paperWidths  = [8.5, 8.5, 7.25, 210.0, 287.0];
  var paperHeights = [11.0, 14.0, 10.5, 297.0, 420.0];
  var paperInches  = [true, true, true, false, false];

  paperArray = new Array();

  for (var i=0;i<paperNames.length;i++) {
    var obj    = new Object();
    obj.name   = paperNames[i];
    obj.width  = paperWidths[i];
    obj.height = paperHeights[i];
    obj.inches = paperInches[i];
    paperArray[i] = obj;
  }
}

//---------------------------------------------------
function createPaperSizeList( selectedInx )
{
  stringBundle = srGetStrBundle("chrome://communicator/locale/printPageSetup.properties");

  var selectElement = new listElement(dialog.paperList);
  selectElement.clearList();

  selectElement.appendPaperNames(paperArray);

  if (selectedInx > -1) {
    selectElement.listElement.selectedIndex = selectedInx;
  }

  //dialog.paperList = selectElement;
}   

//---------------------------------------------------
function loadDialog()
{
  var print_paper_type    = 0;
  var print_paper_unit    = 0;
  var print_paper_width   = 0.0;
  var print_paper_height  = 0.0;
  var print_orientation   = 0;
  var print_margin_top    = 0.5;
  var print_margin_left   = 0.5;
  var print_margin_bottom = 0.5;
  var print_margin_right  = 0.5;

  if (printSettings) {
    print_paper_type   = printSettings.paperSizeType;
    print_paper_unit   = printSettings.paperSizeUnit;
    print_paper_width  = printSettings.paperWidth;
    print_paper_height = printSettings.paperHeight;
    print_orientation  = printSettings.orientation;

    print_margin_top    = printSettings.marginTop;
    print_margin_left   = printSettings.marginLeft;
    print_margin_right  = printSettings.marginRight;
    print_margin_bottom = printSettings.marginBottom;
  }

  if (doDebug) {
    dump("paperSizeType "+print_paper_unit+"\n");
    dump("paperWidth    "+print_paper_width+"\n");
    dump("paperHeight   "+print_paper_height+"\n");
    dump("orientation   "+print_orientation+"\n");

    dump("print_margin_top    "+print_margin_top+"\n");
    dump("print_margin_left   "+print_margin_left+"\n");
    dump("print_margin_right  "+print_margin_right+"\n");
    dump("print_margin_bottom "+print_margin_bottom+"\n");
  }

  createPaperArray();

  var selectedInx = 0;
  for (var i=0;i<paperArray.length;i++) {
    if (print_paper_width == paperArray[i].width && print_paper_height == paperArray[i].height) {
      selectedInx = i;
      break;
    }
  }
  createPaperSizeList(selectedInx);

  dialog.printBGColors.checked = printSettings.printBGColors;
  dialog.printBGImages.checked = printSettings.printBGImages;


  if ( print_orientation == gPrintSettingsInterface.kPortraitOrientation ) {
    dialog.orientation.selectedIndex = 0;

  } else if ( print_orientation == gPrintSettingsInterface.kLandscapeOrientation ) {
    dialog.orientation.selectedIndex = 1;
  }

  dialog.topInput.value    = getDoubleStr(print_margin_top, 1);
  dialog.bottomInput.value = getDoubleStr(print_margin_bottom, 1);
  dialog.leftInput.value   = getDoubleStr(print_margin_left, 1);
  dialog.rightInput.value  = getDoubleStr(print_margin_right, 1);

  dialog.hLeftInput.value   = printSettings.headerStrLeft;
  dialog.hCenterInput.value = printSettings.headerStrCenter;
  dialog.hRightInput.value  = printSettings.headerStrRight;

  dialog.fLeftInput.value   = printSettings.footerStrLeft;
  dialog.fCenterInput.value = printSettings.footerStrCenter;
  dialog.fRightInput.value  = printSettings.footerStrRight;

  dialog.scalingInput.value  = getDoubleStr(printSettings.scaling * 100.0, 3);

}

var param;

//---------------------------------------------------
function onLoad()
{
  // Init dialog.
  initDialog();

  if (window.arguments[0] != null) {
    printSettings = window.arguments[0].QueryInterface(Components.interfaces.nsIPrintSettings);
  } else if (doDebug) {
    alert("window.arguments[0] == null!");
  }

  if (printSettings) {
    loadDialog();
  } else if (doDebug) {
    alert("Could initialize dialog, PrintSettings is null!");
  }
}

//---------------------------------------------------
function onAccept()
{
  var print_paper_type   = gPrintSettingsInterface.kPaperSizeDefined;
  var print_paper_unit   = gPrintSettingsInterface.kPaperSizeInches;
  var print_paper_width  = 8.5;
  var print_paper_height = 11.0;

  if (printSettings) {
    var selectedInx = dialog.paperList.selectedIndex;
    if (paperArray[selectedInx].inches) {
      print_paper_unit = gPrintSettingsInterface.kPaperSizeInches;
    } else {
      print_paper_unit = gPrintSettingsInterface.kPaperSizeMillimeters;
    }
    print_paper_width  = paperArray[selectedInx].width;
    print_paper_height = paperArray[selectedInx].height;


    printSettings.paperSizeType = print_paper_type;
    printSettings.paperSizeUnit = print_paper_unit;
    printSettings.paperWidth    = print_paper_width;
    printSettings.paperHeight   = print_paper_height;

    printSettings.orientation = dialog.orientation.selectedIndex;

    // save these out so they can be picked up by the device spec
    printSettings.marginTop    = dialog.topInput.value;
    printSettings.marginLeft   = dialog.leftInput.value;
    printSettings.marginBottom = dialog.bottomInput.value;
    printSettings.marginRight  = dialog.rightInput.value;


    printSettings.headerStrLeft   = dialog.hLeftInput.value;
    printSettings.headerStrCenter = dialog.hCenterInput.value;
    printSettings.headerStrRight  = dialog.hRightInput.value;

    printSettings.footerStrLeft   = dialog.fLeftInput.value;
    printSettings.footerStrCenter = dialog.fCenterInput.value;
    printSettings.footerStrRight  = dialog.fRightInput.value;

    printSettings.printBGColors = dialog.printBGColors.checked;
    printSettings.printBGImages = dialog.printBGImages.checked;

    var scaling = document.getElementById("scalingInput").value;
    if (scaling < 50.0 || scaling > 100.0) {
      scaling = 100.0;
    }
    scaling /= 100.0;
    printSettings.scaling = scaling;

    if (doDebug) {
      dump("******* Page Setup Accepting ******\n");
      dump("paperSizeType "+print_paper_type+" (should be 1)\n");
      dump("paperSizeUnit "+print_paper_unit+"\n");
      dump("paperWidth    "+print_paper_width+"\n");
      dump("paperHeight   "+print_paper_height+"\n");

      dump("print_margin_top    "+dialog.topInput.value+"\n");
      dump("print_margin_left   "+dialog.leftInput.value+"\n");
      dump("print_margin_right  "+dialog.bottomInput.value+"\n");
      dump("print_margin_bottom "+dialog.rightInput.value+"\n");
    }

  } else {
    dump("************ printSettings: "+printSettings+"\n");
  }

  return true;
}

//---------------------------------------------------
function onCancel()
{
  return true;
}

