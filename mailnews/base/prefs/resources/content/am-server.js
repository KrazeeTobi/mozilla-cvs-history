/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 */

var stringBundle;

function onInit() {
    stringBundle = srGetStrBundle("chrome://messenger/locale/messenger.properties");
    initServerType();
}

function onPreInit(account, accountValues)
{
    var type = parent.getAccountValue(account, accountValues, "server", "type");
    
    hideShowControls(type);
}


function initServerType() {
  var serverType = document.getElementById("server.type").value;
  
  var verboseName;

  var propertyName = "serverType-" + serverType;

  verboseName = stringBundle.GetStringFromName(propertyName);

  var hostname = document.getElementById("server.hostName").value;
  var username = document.getElementById("server.username").value;

  setDivText("servertype.verbose", verboseName);
  setDivText("servername.verbose", hostname);
  setDivText("username.verbose", username);

}

function hideShowControls(serverType)
{
    var controls = document.getElementsByAttribute("wsm_persist", "true");
    var len = controls.length;
    for (var i=0; i<len; i++) {
        var control = controls[i];
        var controlName = control.name;
        if (!controlName) continue;
        var controlNameSplit = controlName.split(".");
        
        if (controlNameSplit.length < 2) continue;
        var controlType = controlNameSplit[0];

        // skip generic server/identity things
        if (controlType == "server" ||
            controlType == "identity") continue;

        // we only deal with controls in <html:div>s
        var div = getEnclosingDiv(control);
        
        if (!div) continue;

        // hide unsupported server type
        if (control.type.toLowerCase() != "hidden") {
            if (controlType == serverType)
                div.removeAttribute("hidden")
            else
                div.setAttribute("hidden", "true");
        }
    }

    var serverPrefContainer = document.getElementById("serverPrefContainer");
    if (serverPrefContainer)
        serverPrefContainer.removeAttribute("hidden");
}


function setDivText(divname, value) {
    var div = document.getElementById(divname);
    if (!div) return;
    div.setAttribute("value", value);
}


function openImapAdvanced()
{
    dump("openImapAdvanced()\n");
    var imapServer = getImapServer();
    dump("Opening dialog..\n");
    window.openDialog("chrome://messenger/content/am-imap-advanced.xul",
                      "_blank",
                      "chrome,modal", imapServer);

    saveServerLocally(imapServer);
}

function getImapServer() {
    var imapServer = new Array;

    // boolean prefs, need to do special convertion    
    imapServer.dualUseFolders =
        (document.getElementById("imap.dualUseFolders").value == "true" ?
         true : false);
    imapServer.usingSubscription =
        (document.getElementById("imap.usingSubscription").value == "true" ? true : false);

    // string prefs
    imapServer.personalNamespace = document.getElementById("imap.personalNamespace").value;
    imapServer.publicNamespace = document.getElementById("imap.publicNamespace").value;
    imapServer.serverDirectory = document.getElementById("imap.serverDirectory").value;
    imapServer.otherUsersNamespace = document.getElementById("imap.otherUsersNamespace").value;

    // boolean prefs, need to do special convertion    
    imapServer.overrideNamespaces =
        (document.getElementById("imap.overrideNamespaces").value == "true" ? true : false);
    return imapServer;
}

function saveServerLocally(imapServer)
{
    dump("Saving values in " + imapServer + ":\n");
    for (var i in imapServer) {
        dump("imapServer." + i + " = " + imapServer[i] + "\n");
    }
    // boolean prefs, JS does the conversion for us
    document.getElementById("imap.dualUseFolders").value = imapServer.dualUseFolders;
    document.getElementById("imap.usingSubscription").value = imapServer.usingSubscription;

    // string prefs
    document.getElementById("imap.personalNamespace").value = imapServer.personalNamespace;
    document.getElementById("imap.publicNamespace").value = imapServer.publicNamespace;
    document.getElementById("imap.serverDirectory").value = imapServer.serverDirectory;
    document.getElementById("imap.otherUsersNamespace").value = imapServer.otherUsersNamespace;

    // boolean prefs, JS does the conversion for us
    document.getElementById("imap.overrideNamespaces").value = imapServer.overrideNamespaces;

}

function getEnclosingDiv(startNode) {

    var parent = startNode.parentNode;
    var div;
    
    while (parent && parent != document) {

        if (parent.tagName.toLowerCase() == "div" ||
            parent.tagName.toLowerCase() == "html" ||
            parent.tagName.toLowerCase() == "box") {
            var isContainer =
                (parent.getAttribute("iscontrolcontainer") == "true");
            
            // remember the FIRST div we encounter, or the first
            // controlcontainer
            if (!div || isContainer)
                div=parent;
            
            // break out with a controlcontainer
            if (isContainer)
                break;
        }
        
        parent = parent.parentNode;
    }
    
    return div;
}
