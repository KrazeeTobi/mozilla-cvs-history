/* -*- Mode: Java; tab-width: 2; c-basic-offset: 2; -*-
 * 
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 *
 * Contributor(s):
 *   Ben "Count XULula" Goodger <rgoodger@ihug.co.nz>
 */

/** class WizardHandlerSet( WidgetStateManager sMgr, WizardManager wMgr ) ;
 *  purpose: class for managing wizard navigation functions
 *  in:  WidgetStateManager sMgr WSM object created by WizardManager
 *       WizardManager wMgr WM object created by WizardManager
 *  out: nothing.
 **/
function WizardHandlerSet( sMgr, wMgr )
{
  // data mambers
  // wizard buttons
  this.backButton         = document.getElementById("wiz.back.button");
  this.nextButton         = document.getElementById("wiz.next.button");
  this.finishButton       = document.getElementById("wiz.finish.button");
  this.cancelButton       = document.getElementById("wiz.cancel.button");
  // wizard handlers
  this.nextButtonFunc     = null;
  this.backButtonFunc     = null;
  this.cancelButtonFunc   = null;
  this.finishButtonFunc   = null;
  this.pageLoadFunc       = null;
  this.enablingFunc       = null;
    
  this.SM                 = sMgr;
  this.WM                 = wMgr;

  // member functions
  this.SetHandlers        = WHS_SetHandlers;

  // construction (points member functions to right functions)
  this.SetHandlers( this.nextButtonFunc, this.backButtonFunc, this.finishButtonFunc, 
                    this.cancelButtonFunc, this.pageLoadFunc, this.enablingFunc );
}
  
// default handler for next page in page sequence
function DEF_onNext()
{
  var oParent = this.WHANDLER;
  if( oParent.nextButton.getAttribute("disabled") == "true" )
    return;

  // make sure page is valid!
  if (!oParent.SM.PageIsValid())
      return;
  
	oParent.SM.SavePageData( this.currentPageTag, null, null, null );      // persist data
  var nextPageTag = this.wizardMap[this.currentPageTag].next;
	this.LoadPage( nextPageTag, false );     // load the next page
  this.ProgressUpdate( ++this.currentPageNumber );
}
// default handler for previous page in sequence
function DEF_onBack()
{
  var oParent = this.WHANDLER;
	if( oParent.backButton.getAttribute("disabled") == "true" )
 	  return;
	oParent.SM.SavePageData( this.currentPageTag, null, null, null );      // persist data
	previousPageTag = this.wizardMap[this.currentPageTag].previous;
  this.LoadPage( previousPageTag, false ); // load the preivous page
  this.ProgressUpdate( --this.currentPageNumber );
}
// default handler for cancelling wizard
function DEF_onCancel()
{
  if( top.window.opener )
    window.close();
}
// default finish handler
function DEF_onFinish()
{
  var oParent = this.WHANDLER;
  if( !this.wizardMap[this.currentPageTag].finish )
    return;
  oParent.SM.SavePageData( this.currentPageTag, null, null, null );
  dump("WizardButtonHandlerSet Warning:\n");
  dump("===============================\n");
  dump("You must provide implementation for onFinish, or else your data will be lost!\n");
}

// default button enabling ** depends on map, see doc
function DEF_DoEnabling( nextButton, backButton, finishButton )
{
  var oParent = this.WHANDLER;
  // make sure we're on a valid page
	if( !this.currentPageTag ) 
    return;
  // "next" button enabling
  nextTag = this.wizardMap[this.currentPageTag].next;
  if( nextTag && oParent.nextButton.getAttribute("disabled") ) {
    oParent.nextButton.removeAttribute( "disabled" );
  }
  else if( !nextTag && !oParent.nextButton.getAttribute("disabled") ) {
    oParent.nextButton.setAttribute( "disabled", "true" );
  }
  // "finish" button enabling
  finishTag = this.wizardMap[this.currentPageTag].finish;
  if( finishTag && oParent.finishButton.getAttribute("disabled") ) {
    oParent.finishButton.removeAttribute( "disabled" );
  }
  else if( !finishTag && !oParent.finishButton.getAttribute("disabled") ) {
    oParent.finishButton.setAttribute( "disabled", "true" );
  }
  // "back" button enabling
  prevTag = this.wizardMap[this.currentPageTag].previous;
	if( prevTag && oParent.backButton.getAttribute("disabled") ) {
 	  oParent.backButton.removeAttribute("disabled");
  }
	else if( !prevTag && !oParent.backButton.getAttribute("disabled") ) {
   	oParent.backButton.setAttribute("disabled", "true"); 
  }
}

/** void PageLoaded( string tag, string frame_id ) ;
 *  purpose: perform initial button enabling and call Startup routines.
 *  in:  string page tag referring to the file name of the current page
 *       string frame_id optional supply of page frame, if content_frame is not 
 *                       defined
 *  out: nothing.
 **/
function DEF_onPageLoad( tag ) 
{
  var oParent = this.WHANDLER;
	this.currentPageTag = tag;
  if( this.DoButtonEnabling )              // if provided, call user-defined button
    this.DoButtonEnabling();               // enabling function
  if( this.content_frame )
    oParent.SM.SetPageData( tag, false );  // set page data in content frame
  else {
    dump("Widget Data Manager Error:\n"); 
    dump("==========================\n");
    dump("content_frame variable not defined. Please specify one as an argument to Startup();\n");
    return;
  }
}

/** void SetHandlers( string nextFunc, string backFunc, string finishFunc, string cancelFunc,
 *                    string pageLoadFunc, string enablingFunc ) ;
 *  purpose: allow user to assign button handler function names
 *  in:  strings referring to the names of the functions for each button
 *  out: nothing.
 **/
function WHS_SetHandlers( nextFunc, backFunc, finishFunc, cancelFunc, pageLoadFunc, enablingFunc )
{
  // var oParent = this.WHANDLER;
  this.nextButtonFunc   = nextFunc      ;
  this.backButtonFunc   = backFunc      ;
  this.cancelButtonFunc = cancelFunc    ;
  this.finishButtonFunc = finishFunc    ;
  this.pageLoadFunc     = pageLoadFunc  ;
  this.enablingFunc     = enablingFunc  ;
  
  // assign handlers to parent object
  // (handler functions are assigned to parent object) 
  this.WM.onNext             = ( !this.nextButtonFunc   ) ? DEF_onNext     : nextFunc ;
  this.WM.onBack             = ( !this.backButtonFunc   ) ? DEF_onBack     : backFunc ;
  this.WM.onCancel           = ( !this.cancelButtonFunc ) ? DEF_onCancel   : cancelFunc ;
  this.WM.onFinish           = ( !this.finishButtonFunc ) ? DEF_onFinish   : finishFunc ;
  this.WM.onPageLoad         = ( !this.pageLoadFunc     ) ? DEF_onPageLoad : pageLoadFunc ;
  this.WM.DoButtonEnabling   = ( !this.enablingFunc     ) ? DEF_DoEnabling : enablingFunc ;
}
