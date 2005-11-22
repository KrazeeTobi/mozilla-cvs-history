/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Marcio S. Galli - mgalli@geckonnection.com
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

const nsIWebNavigation = Components.interfaces.nsIWebNavigation;
const nsIWebProgressListener = Components.interfaces.nsIWebProgressListener;

var gURLBar = null;
var gBrowserStatusHandler;
var gSelectedTab=null;
var gFullScreen=false;
var gRSSTag="minimo";
var gGlobalHistory = null;
var gURIFixup = null;
var gShowingMenuPopup=null;
var gFocusedElementHREFContextMenu=null;

var gPref = null;                    // so far snav toggles on / off via direct access to pref.
                                     // See bugzilla.mozilla.org/show_bug.cgi?id=311287#c1

var gSNAV=-1; 

function nsBrowserStatusHandler()
{
}

nsBrowserStatusHandler.prototype = 
{
  QueryInterface : function(aIID)
  {
    if (aIID.equals(Components.interfaces.nsIWebProgressListener) ||
        aIID.equals(Components.interfaces.nsISupportsWeakReference) ||
        aIID.equals(Components.interfaces.nsISupports))
    {
      return this;
    }
    throw Components.results.NS_NOINTERFACE;
  },

  init : function()
  {
    this.urlBar           = document.getElementById("urlbar");
    this.stopreloadButton = document.getElementById("reload-stop-button");
    this.progressBGPosition = 0;  /* To be removed, fix in onProgressChange ... */ 
  },

  destroy : function()
  {
    this.urlBar = null;
    this.stopreloadButton = null;
    this.progressBGPosition = null;  /* To be removed, fix in onProgressChange ... */ 
    
  },

  onStateChange : function(aWebProgress, aRequest, aStateFlags, aStatus)
  {
    var refBrowser=null;
    var tabItem=null;
    
    if (aStateFlags & nsIWebProgressListener.STATE_IS_NETWORK)
    {

      if (aStateFlags & nsIWebProgressListener.STATE_START)
      {
      
        
		if(aRequest && aWebProgress.DOMWindow == content) {
			this.startDocumentLoad(aRequest);
		}
		
        this.stopreloadButton.className = "stop-button";
        this.stopreloadButton.command = "cmd_BrowserStop";
        
        return;
      }
      
      if (aStateFlags & nsIWebProgressListener.STATE_STOP)
      {

      
        /* To be fixed. We dont want to directly access sytle from here */
        document.styleSheets[1].cssRules[0].style.backgroundPosition="1000px 100%";

        this.stopreloadButton.className = "reload-button";
        this.stopreloadButton.command= "cmd_BrowserReload";
        


        return;
      }
      return;
    }

    if (aStateFlags & nsIWebProgressListener.STATE_IS_DOCUMENT)
    { 
      if (aStateFlags & nsIWebProgressListener.STATE_START)
      {
        return;
      }
      
      if (aStateFlags & nsIWebProgressListener.STATE_STOP)
      {
        return;
      }
      return;
    }
  },
  onProgressChange : function(aWebProgress, aRequest, aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress)
  {
    var percentage = parseInt((aCurTotalProgress * 100) / aMaxTotalProgress);
    document.styleSheets[1].cssRules[0].style.backgroundPosition=percentage+"px 100%";
  },
  onLocationChange : function(aWebProgress, aRequest, aLocation)
  {
     /* Ideally we dont want to check this here.
     Better to have some other protocol view-rss in the chrome */

     const rssmask = "chrome://minimo/content/rssview/rssload.xhtml?url=";
     const sbmask = "chrome://minimo/content/rssview/rssload.xhtml?url=http://del.icio.us/rss/tag/";

     if(aLocation.spec.substr(0, rssmask .length) == rssmask ) {

	     if(aLocation.spec.substr(0, sbmask .length) == sbmask ) {
	        /* We trap the URL */ 
	        this.urlBar.value="sb:"+gRSSTag; 
  
		} else {

	        /* We trap the URL */ 
	        this.urlBar.value="rss:"+gRSSTag; 

		}

     } else {
      domWindow = aWebProgress.DOMWindow;
      // Update urlbar only if there was a load on the root docshell
      if (domWindow == domWindow.top) {
        this.urlBar.value = aLocation.spec;
      }
    }

    BrowserUpdateFeeds();
},

  onStatusChange : function(aWebProgress, aRequest, aStatus, aMessage)
  {
  },
  startDocumentLoad : function(aRequest)
  {
    getBrowser().mCurrentBrowser.feeds = null;
  },
  onSecurityChange : function(aWebProgress, aRequest, aState)
  {
    /* Color is temporary. We shall dynamically assign a new class to the element and or to 
    evaluate access from another class rule, the security identity color has to be with the minimo.css */ 

    switch (aState) {
      case nsIWebProgressListener.STATE_IS_SECURE | nsIWebProgressListener.STATE_SECURE_HIGH:
        //this.urlBar.value="level high";
        document.styleSheets[1].cssRules[0].style.backgroundColor="yellow";
        document.getElementById("lock-icon").className="security-notbroken";
        break;	
      case nsIWebProgressListener.STATE_IS_SECURE | nsIWebProgressListener.STATE_SECURE_LOW:
        // this.urlBar.value="level low";
        document.styleSheets[1].cssRules[0].style.backgroundColor="lightyellow";
        document.getElementById("lock-icon").className="security-notbroken";
        break;
      case nsIWebProgressListener.STATE_IS_BROKEN:
        //this.urlBar.value="level broken";
        document.styleSheets[1].cssRules[0].style.backgroundColor="lightred";
        document.getElementById("lock-icon").className="security-broken";
        break;
      case nsIWebProgressListener.STATE_IS_INSECURE:
        default:
        document.styleSheets[1].cssRules[0].style.backgroundColor="white";
        document.getElementById("lock-icon").className="security-na";
        break;
      }   
  }
}

/** 
  * Initial Minimo Startup 
  * 
  **/

/* moved this as global */ 

function MiniNavStartup()
{
  var homepage = "http://www.mozilla.org";

  try {

    gURLBar = document.getElementById("urlbar");
    var currentTab=getBrowser().selectedTab;
    browserInit(currentTab);
    gSelectedTab=currentTab;
    
    var BrowserStatusHandler = new nsBrowserStatusHandler();
    BrowserStatusHandler.init();

    getBrowser().addProgressListener(BrowserStatusHandler, Components.interfaces.nsIWebProgress.NOTIFY_ALL);
  
    // Current build was not able to get it. Taking it from the tab browser element. 
    // var webNavigation=getBrowser().webNavigation;

    var refBrowser=getBrowser().getBrowserForTab(currentTab);
    var webNavigation=refBrowser.webNavigation;
    
    webNavigation.sessionHistory = Components.classes["@mozilla.org/browser/shistory;1"].createInstance(Components.interfaces.nsISHistory);

    getBrowser().docShell.QueryInterface(Components.interfaces.nsIDocShellHistory).useGlobalHistory = true;

    gGlobalHistory = Components.classes["@mozilla.org/browser/global-history;2"]
                               .getService(Components.interfaces.nsIBrowserHistory);
  
    gURIFixup = Components.classes["@mozilla.org/docshell/urifixup;1"]
                          .getService(Components.interfaces.nsIURIFixup);


    try {
      gPref = Components.classes["@mozilla.org/preferences-service;1"]
                         .getService(Components.interfaces.nsIPrefBranch);

      var page = gPref.getCharPref("browser.startup.homepage");

      if (page != null)
      {
        var fixedUpURI = gURIFixup.createFixupURI(page, 2 /*fixup url*/ );
        homepage = fixedUpURI.spec;
      }
    } catch (ignore) {}
      
  } catch (e) {
    alert("Error trying to startup browser.  Please report this as a bug:\n" + e);
  }

  loadURI(homepage);
  
  /*
   * We add event handler to catch the right and left keys on the main_MenuPopup 
   */
  document.addEventListener("keypress",eventHandlerMenu,true);

  /* 
   * Override the title attribute <title /> in this doc with a setter.
   * This is our workaround solution so that when the tabbrowser::updateTitle
   * tries to update this document's title, nothing happens. Bug 311564
   */ 
   
  document.__defineSetter__("title",function(x){}); // Stays with the titled defined by the XUL element. 
  
  
  /*
   * Sync UI zoom level 
   */
 
  syncUIZoom();
  

  /* 
   * Add event clicks to Minimo toolbars and also to the mStrip BOX in the tabbrowser
   */
  getBrowser().mStrip.addEventListener("click",BrowserWithoutSNAV,false);
  document.getElementById("mini-toolbars").addEventListener("click",BrowserWithoutSNAV,false);
  

  getBrowser().addEventListener("DOMLinkAdded", BrowserLinkAdded, false);

}

/*
 * Page's new Link tag handlers. This should be able to be smart about RSS, CSS, and maybe other Minimo stuff?  
 * So far we have this here, so we can experience and try some new stuff. To be tabrowsed.
 */
function BrowserLinkAdded(event) {
// ref http://lxr.mozilla.org/mozilla/source/browser/base/content/browser.js#2070

	/* 
       * Taken from browser.js - yes this should be in tabbrowser
       */

	var erel = event.target.rel;
	var etype = event.target.type;
	var etitle = event.target.title;
	var ehref = event.target.href;

	const alternateRelRegex = /(^|\s)alternate($|\s)/i;
	const rssTitleRegex = /(^|\s)rss($|\s)/i;

	if (!alternateRelRegex.test(erel) || !etype) return;
	
	etype = etype.replace(/^\s+/, "");
	etype = etype.replace(/\s+$/, "");
	etype = etype.replace(/\s*;.*/, "");
	etype = etype.toLowerCase();

	if (etype == "application/rss+xml" || etype == "application/atom+xml" || (etype == "text/xml" || etype == "application/xml" || etype == "application/rdf+xml") && rssTitleRegex.test(etitle))
	{

		const targetDoc = event.target.ownerDocument;

		var browsers = getBrowser().browsers;
		var shellInfo = null;

		for (var i = 0; i < browsers.length; i++) {
			var shell = findChildShell(targetDoc, browsers[i].docShell, null);
			if (shell) shellInfo = { shell: shell, browser: browsers[i] };
		}

		//var shellInfo = this._getContentShell(targetDoc);

		var browserForLink = shellInfo.browser;

		if(!browserForLink) return;

		var feeds = [];
		if (browserForLink.feeds != null) feeds = browserForLink.feeds;
		var wrapper = event.target;
		feeds.push({ href: wrapper.href, type: etype, title: wrapper.title});
		browserForLink.feeds = feeds;

		if (browserForLink == getBrowser() || browserForLink == getBrowser().mCurrentBrowser) {
			var feedButton = document.getElementById("feed-button");
			if (feedButton) {
				feedButton.setAttribute("feeds", "true");
//				feedButton.setAttribute("tooltiptext", gNavigatorBundle.getString("feedHasFeeds"));	
                        document.getElementById("feed-button-menu").setAttribute("onpopupshowing","DoBrowserRSS('"+ehref+"')");
			}
		}
	}
}

function BrowserUpdateFeeds() {
	var feedButton = document.getElementById("feed-button");
	if (!feedButton)
		return;

	var feeds = getBrowser().mCurrentBrowser.feeds;

	if (!feeds || feeds.length == 0) {
		if (feedButton.hasAttribute("feeds")) feedButton.removeAttribute("feeds");
//		feedButton.setAttribute("tooltiptext",  gNavigatorBundle.getString("feedNoFeeds"));
	} else {
		feedButton.setAttribute("feeds", "true");
            document.getElementById("feed-button-menu").setAttribute("onpopupshowing","DoBrowserRSS('"+feeds[0].href+"')");

//		feedButton.setAttribute("tooltiptext", gNavigatorBundle.getString("feedHasFeeds"));
	}
}


function findChildShell(aDocument, aDocShell, aSoughtURI) {
		aDocShell.QueryInterface(Components.interfaces.nsIWebNavigation);
		aDocShell.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
		var doc = aDocShell.getInterface(Components.interfaces.nsIDOMDocument);
		if ((aDocument && doc == aDocument) || 
			(aSoughtURI && aSoughtURI.spec == aDocShell.currentURI.spec))
			return aDocShell;
 
		var node = aDocShell.QueryInterface(Components.interfaces.nsIDocShellTreeNode);
		for (var i = 0; i < node.childCount; ++i) {
			var docShell = node.getChildAt(i);
			docShell = findChildShell(aDocument, docShell, aSoughtURI);
			if (docShell) return docShell;
		}
		return null;
}


function BrowserWithoutSNAV(e) {
 if(gSNAV==1||gSNAV==-1) {
	gSNAV=0;
      gPref.setBoolPref("snav.enabled", false);                                   
 } 
}

function BrowserWithSNAV(e) {
 if(gSNAV==0||gSNAV==-1) {
	gSNAV=1;
      gPref.setBoolPref("snav.enabled", true);                                   
 } 
}

/*
 *  Focus Shortcut Action. This is just a focus action dispatcher based on certain conditions 
 *  defined in the XUL elements. Ideally would be interesting to have this as part of some new
 *  XUL elements that are based on existing XUL elements, or to incorporate, import, this behavior
 *  in the XUL declaration. 
 */
function eventHandlerMenu(e) {

  if( (e.keyCode==39 || e.keyCode==37) && (gShowingMenuPopup) ) {
    BrowserMenuPopupFalse();
    document.getElementById("menu-button").focus(); // forcing state back to the menu. 
  }

  if( e.keyCode==70) /*SoftKey1 or HWKey1*/ {
  	document.getElementById("menu-button").focus();
	e.preventBubble();
	BrowserWithoutSNAV();
	
  } 
  
  if(document.commandDispatcher&&document.commandDispatcher.focusedElement) { 

	  var outnavTarget=document.commandDispatcher.focusedElement.getAttribute("accessrule");
	  if(outnavTarget!="" && (e.keyCode==40||e.keyCode==38) && !gShowingMenuPopup) {
	      e.preventBubble();
	      if(e.keyCode==40) {
	
	        ruleElement=findRuleById(document.getElementById(outnavTarget).getAttribute("accessnextrule"),"accessnextrule");
	      }
	      if(e.keyCode==38) {
	
	        ruleElement=findRuleById(document.getElementById(outnavTarget).getAttribute("accessprevrule"),"accessprevrule"); 
	      }
		  var tempElement=ruleElement.getAttribute("accessfocus");
	      if(tempElement.indexOf("#")>-1) {
	
			if(tempElement=="#tabContainer") { 
				if(getBrowser().tabContainer) {
					getBrowser().selectedTab.focus();

					if(getBrowser().mStrip.collapsed) {				
						getBrowser().contentWindow.focus();
					} 

				}
			} 
			if(tempElement=="#tabContent") { 			
				getBrowser().contentWindow.focus();
			} 
       
		  } else { 
			  document.getElementById(tempElement).focus();
		  }
	  }
  }
}

function findRuleById(outnavTarget,ruleattribute) {
  var ruleElement=document.getElementById(outnavTarget);

  if(document.getElementById(ruleElement.getAttribute("target")).collapsed) {
      return findRuleById(ruleElement.getAttribute(ruleattribute), ruleattribute);
  } else {
	return ruleElement;
  } 
}



/** 
  * Init stuff
  * 
  **/
function browserInit(refTab)
{
  /* 
   * addRule access navigational rule to each tab 
   */

  refTab.setAttribute("accessrule","focus_content");
  
  /*
   * 
   */
  var refBrowser=getBrowser().getBrowserForTab(refTab);
  
  /* New Browser OnFocus SNAV Toggle */
  
  refBrowser.addEventListener("focus", BrowserWithSNAV , true);
  
  
  try {
    refBrowser.markupDocumentViewer.textZoom = .90;
  } catch (e) {
  
  }
  gURLBar = document.getElementById("urlbar");
  
}

function MiniNavShutdown()
{
  if (gBrowserStatusHandler) gBrowserStatusHandler.destroy();
}

function getBrowser()
{
  return document.getElementById("content");
}

function getWebNavigation()
{
  return getBrowser().webNavigation;
}

function loadURI(uri)
{
  getWebNavigation().loadURI(uri, nsIWebNavigation.LOAD_FLAGS_NONE, null, null, null);
}

function BrowserBack()
{
  getWebNavigation().goBack();
}

function BrowserForward()
{
  getWebNavigation().goForward();
}

function BrowserStop()
{
  getWebNavigation().stop(nsIWebNavigation.STOP_ALL);
}

function BrowserReload()
{
  getWebNavigation().reload(nsIWebNavigation.LOAD_FLAGS_NONE);
}

/* 
 * Combine the two following functions in one
 */
function BrowserOpenTab()
{
  try { 
    getBrowser().selectedTab = getBrowser().addTab('about:blank');
    browserInit(getBrowser().selectedTab);
  } catch (e) {
    alert(e);
  }
  //  if (gURLBar) setTimeout(function() { gURLBar.focus(); }, 0);

}

/* 
 * Used by the Context Menu - Open link as Tab 
 */
function BrowserOpenLinkAsTab() 
{

  if(gFocusedElementHREFContextMenu) {
    try { 
      getBrowser().selectedTab = getBrowser().addTab(gFocusedElementHREFContextMenu);
      browserInit(getBrowser().selectedTab);
    } catch (e) {
      alert(e);
    }
  }
}

/**
  * FOR - keyboard acessibility - context menu for tabbed area *** 
  * Launches the popup for the tabbed area / tabbrowser. Make sure to call this function 
  * when the tabbed panel is available. WARNING somehow need to inform which tab was lack clicked 
  * or mouse over.  
  *
  **/
function BrowserLaunchTabbedPopup() {
	var tabMenu = document.getAnonymousElementByAttribute(document.getElementById("content"),"anonid","tabContextMenu");
	tabMenu.showPopup(getBrowser().selectedTab,-1,-1,"popup","bottomleft", "topleft");
}

/**
  * Has to go through some other approach like a XML-based rule system. 
  * Those are constraints conditions and action. 
  **/
  
function BrowserViewOptions() {
  document.getElementById("toolbar-view").collapsed=!document.getElementById("toolbar-view").collapsed;
  if(document.getElementById("toolbar-view").collapsed &&  document.getElementById("command_ViewOptions").getAttribute("checked")=="true") {
	document.getElementById("command_ViewOptions").setAttribute("checked","false");
  }
}

/**
  * Has to go through some other approach like a XML-based rule system. 
  * Those are constraints conditions and action. 
  **/
  
function BrowserViewRSS() {
  document.getElementById("toolbar-rss").collapsed=!document.getElementById("toolbar-rss").collapsed;
  if(document.getElementById("toolbar-rss").collapsed &&  document.getElementById("command_ViewRSS").getAttribute("checked")=="true") {
	document.getElementById("command_ViewRSS").setAttribute("checked","false");
  }
}

/**
  * Has to go through some other approach like a XML-based rule system. 
  * Those are constraints conditions and action. 
  **/
  
function BrowserViewSearch() {
  document.getElementById("toolbar-search").collapsed=!document.getElementById("toolbar-search").collapsed;
  if(document.getElementById("toolbar-search").collapsed &&  document.getElementById("command_ViewSearch").getAttribute("checked")=="true") {
	document.getElementById("command_ViewSearch").setAttribute("checked","false");
  }
}


/**
  * Has to go through some other approach like a XML-based rule system. 
  * Those are constraints conditions and action. 
  **/
  
function BrowserViewFind() {
  document.getElementById("toolbar-find").collapsed=!document.getElementById("toolbar-find").collapsed;
  if(document.getElementById("toolbar-find").collapsed &&  document.getElementById("command_ViewFind").getAttribute("checked")=="true") {
	document.getElementById("command_ViewFind").setAttribute("checked","false");
  }
}

/** 
  * urlbar indentity, style, progress indicator.
  **/ 
function urlbar() {
}


/* Reset the text size */ 
function BrowserResetZoomPlus() {
 	getBrowser().selectedBrowser.markupDocumentViewer.textZoom+= .25;
}

function BrowserResetZoomMinus() {
 	getBrowser().selectedBrowser.markupDocumentViewer.textZoom-= .25;
}


/* Reset the UI text size */ 
function BrowserUIResetZoomPlus() {
    var currentUILevel=gPref.getIntPref("browser.display.zoomui");
    currentUILevel+=3;
    gPref.setIntPref("browser.display.zoomui", currentUILevel);
    syncUIZoom();
    
    /* 
     * YES I know. 
     * I do this because somehow the grid does not expand
     * when the style CSS syncUIzoom kicks in 
     */
    document.getElementById("uizoomminusitem").focus();
    document.getElementById("uizoomplusitem").focus();
}

function BrowserUIResetZoomMinus() {
    var currentUILevel=gPref.getIntPref("browser.display.zoomui");
    currentUILevel-=3;
    gPref.setIntPref("browser.display.zoomui", currentUILevel);
    syncUIZoom();

     /* 
     * YES I know. 
     * I do this because somehow the grid does not expand
     * when the style CSS syncUIzoom kicks in 
     */
    document.getElementById("uizoomplusitem").focus();
    document.getElementById("uizoomminusitem").focus();
}


/* 
  We want to intercept before it shows, 
  to evaluate when the selected content area is a phone number, 
  thus mutate the popup menu to the right make call item 
*/ 



function BrowserPopupShowing () {

  /*
   * Open Link as New Tab  
   */ 
   
  if(document.commandDispatcher.focusedElement && document.commandDispatcher.focusedElement.href) {
	gFocusedElementHREFContextMenu=document.commandDispatcher.focusedElement.href;
	document.getElementById("link_as_new_tab").hidden=false;

	document.getElementById("item-backbutton").hidden=true;
	document.getElementById("item-forwardbutton").hidden=true;
	document.getElementById("item-reloadbutton").hidden=true;

  } else {
	document.getElementById("link_as_new_tab").hidden=true;
	
	document.getElementById("item-backbutton").hidden=false;
	document.getElementById("item-forwardbutton").hidden=false;
	document.getElementById("item-reloadbutton").hidden=false;
	
  }

  var selectedRange=getBrowser().selectedBrowser.contentDocument.getSelection();
 
  /* Enable Copy */
  if(selectedRange.toString()) {

    document.getElementById("item-copy").style.display="block";
  }
  
  /* Enable Paste - Can paste only if the focused element has a value attribute. :) 
    THis may affect XHTML nodes. Users may be able to paste things within XML nodes. 
  */
  if (document.commandDispatcher.focusedElement) {
    if(document.commandDispatcher.focusedElement.nodeName=="INPUT"||document.commandDispatcher.focusedElement.nodeName=="TEXTAREA") {
      if(DoClipCheckPaste()) {
        document.getElementById("item-paste").style.display="block";	
      }
    }
  }
}

/* Toolbar specific code - to be removed from here */ 

function DoBrowserSearch() {
  BrowserViewSearch();
  try { 
    var vQuery=document.getElementById("toolbar-search-tag").value;
    if(vQuery!="") {
	 getBrowser().selectedTab = getBrowser().addTab('http://www.google.com/xhtml?q='+vQuery+'&hl=en&lr=&safe=off&btnG=Search&site=search&mrestrict=xhtml');
   	 browserInit(getBrowser().selectedTab);
    }
  } catch (e) {
   
  }  
}

/* Toolbar specific code - to be removed from here */ 

function DoBrowserRSS(sKey) {

	  if(!sKey) BrowserViewRSS(); // The toolbar is being used. Otherwise it is via the sb: trap protocol. 
	
	  try { 
	  
	    if(sKey) {
	      gRSSTag=sKey;
	    } else if(document.getElementById("toolbar-rss-rsstag").value!="") {
	      gRSSTag=document.getElementById("toolbar-rss-rsstag").value;
	    }

	    getBrowser().selectedTab = getBrowser().addTab('chrome://minimo/content/rssview/rssload.xhtml?url='+gRSSTag);
	    
	    browserInit(getBrowser().selectedTab);
	  } catch (e) {
	   
	  }  
}


/* Toolbar specific code - to be removed from here */ 

function DoBrowserSB(sKey) {

	  if(!sKey) BrowserViewRSS(); // The toolbar is being used. Otherwise it is via the sb: trap protocol. 
	
	  try { 
	    if(sKey) {
	      gRSSTag=sKey;
	    } else if(document.getElementById("toolbar-rss-rsstag").value!="") {
	      gRSSTag=document.getElementById("toolbar-rss-rsstag").value;
	    }

	    getBrowser().selectedTab = getBrowser().addTab('chrome://minimo/content/rssview/rssload.xhtml?url=http://del.icio.us/rss/tag/'+gRSSTag);
	    browserInit(getBrowser().selectedTab);
	  } catch (e) {
	   
	  }  
}

/* Toolbar specific code - to be removed from here */ 


function DoBrowserFind() {
//  BrowserViewFind();
  try { 
    var vQuery=document.getElementById("toolbar-find-tag").value;
    if(vQuery!="") {
	getBrowser().contentWindow.focus();

	/* FIND DOCUMENTATION: 
	 41 const FIND_NORMAL = 0;
	 42 const FIND_TYPEAHEAD = 1;
	 43 const FIND_LINKS = 2;
	http://lxr.mozilla.org/mozilla/source/toolkit/components/typeaheadfind/content/findBar.js
	*/
	getBrowser().fastFind.find(vQuery,0);
    }
  } catch (e) {
   alert(e);
  }  
}

/* Toolbar specific code - to be removed from here */ 

function DoBrowserFindNext() {
  try { 
	getBrowser().fastFind.findNext();
  } catch (e) {
   alert(e);
  }  
}




function DoPanelPreferences() {
  window.openDialog("chrome://minimo/content/preferences/preferences.xul","preferences","modal,centerscreeen,chrome,resizable=no");
  // BrowserReload(); 
  syncUIZoom();
}

/* 
  Testing the SMS and Call Services 
*/
function DoTestSendCall(toCall) {
  var phoneInterface= Components.classes["@mozilla.org/phone/support;1"].createInstance(Components.interfaces.nsIPhoneSupport);
  phoneInterface.makeCall(toCall,"");
}

function DoFullScreen()
{
  gFullScreen = !gFullScreen;
  
  document.getElementById("nav-bar").hidden = gFullScreen;

  // Is this the simpler approach to count tabs? 
  if(getBrowser().mPanelContainer.childNodes.length>1) {
	  getBrowser().setStripVisibilityTo(!gFullScreen);
  } 
  
  window.fullScreen = gFullScreen;  

  document.getElementById("nav-bar-contextual").hidden = !gFullScreen;    
  
}

/* 

 */
function DoClipCopy()
{
  var copytext=getBrowser().selectedBrowser.contentDocument.getSelection().toString();
  var str = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
  if (!str) return false;
  str.data = copytext;
  var trans = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
  if (!trans) return false;
  trans.addDataFlavor("text/unicode");
  trans.setTransferData("text/unicode",str,copytext.length * 2);
  var clipid = Components.interfaces.nsIClipboard;
  var clip = Components.classes["@mozilla.org/widget/clipboard;1"].getService(clipid);
  if (!clip) return false;
  clip.setData(trans,null,clipid.kGlobalClipboard);
}

/* 
 Currently supports text/unicode. 
 */
function DoClipCheckPaste()
{
  var clip = Components.classes["@mozilla.org/widget/clipboard;1"].getService(Components.interfaces.nsIClipboard);
  if (!clip) return false;
  var trans = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
  if (!trans) return false;
  trans.addDataFlavor("text/unicode");
  clip.getData(trans,clip.kGlobalClipboard);
  var str = new Object();
  var strLength = new Object();
  var pastetext = null;
  trans.getTransferData("text/unicode",str,strLength);
  if (str) str = str.value.QueryInterface(Components.interfaces.nsISupportsString);
  if (str) pastetext = str.data.substring(0,strLength.value / 2);
  if(pastetext) {
    return pastetext;
  } else return false;
}

function DoClipPaste()
{

  /* 008 note - value is there in the clipboard, however somehow paste action does not happen. 
   If you evaluate the canpaste you get false. */ 
   
  var disp = document.commandDispatcher;
  var cont = disp.getControllerForCommand("cmd_paste");
  cont.doCommand("cmd_paste");
}

function URLBarEntered()
{
  try
  {
    if (!gURLBar)
      return;

    var url = gURLBar.value;
    if (gURLBar.value == "" || gURLBar.value == null)
      return;

    /* Trap to SB 'protocol' */ 

    if(gURLBar.value.substring(0,3)=="sb:") {
		DoBrowserSB(gURLBar.value.split("sb:")[1]);
		return;
    }

    /* Trap to RSS 'protocol' */ 

    if(gURLBar.value.substring(0,4)=="rss:") {
		DoBrowserRSS(gURLBar.value.split("rss:")[1]);
		return;
    }
    
    /* Other normal cases */ 

    var fixedUpURI = gURIFixup.createFixupURI(url, 2 /*fixup url*/ );
    gGlobalHistory.markPageAsTyped(fixedUpURI);
    
    gURLBar.value = fixedUpURI.spec;
    
    loadURI(fixedUpURI.spec);
    
    content.focus();
  }
  catch(ex) {alert(ex);}


  return true;
}

function URLBarFocusHandler()
{
  gURLBar.showHistoryPopup();
}

var gRotationDirection = true;

function BrowserScreenRotate()
{
  try {
  var deviceSupport = Components.classes["@mozilla.org/device/support;1"].getService(Components.interfaces.nsIDeviceSupport);
  
  deviceSupport.rotateScreen(gRotationDirection);
  gRotationDirection != gRotationDirection;
  }
  catch (ex)
  {
    alert(ex);
  }
}

/* 
 * Main Menu 
 */ 
 
function BrowserMenuPopup() {
    document.getElementById("menu_MainPopup").showPopup(document.getElementById("menu-button"),-1,-1,"popup","bottomleft", "topleft");
}

function BrowserMenuPopupFalse() {
    document.getElementById("menu_MainPopup").hidePopup();
}

function BrowserMenuPopupContextualMenu() {
    document.getElementById("contentAreaContextMenu").hidePopup();
    DoFullScreen();
    BrowserMenuPopup();
}

function MenuPopupShowing() {
    gShowingMenuPopup=true;
    document.getElementById("menu-button").focus();
}

function MenuPopupHidden() {
    gShowingMenuPopup=false;
}




