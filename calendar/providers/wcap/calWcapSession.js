/* -*- Mode: javascript; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems, Inc.
 * Portions created by Sun Microsystems are Copyright (C) 2006 Sun
 * Microsystems, Inc. All Rights Reserved.
 *
 * Original Author: Daniel Boelzle (daniel.boelzle@sun.com)
 *
 * Contributor(s):
 *
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

// globals:
var g_serverTimeDiffs = {};
var g_allSupportedTimezones = {};

function calWcapSession() {
    this.wrappedJSObject = this;
    this.m_observers = [];
    this.m_calIdToCalendar = {};
    this.m_asyncQueue = new AsyncQueue();
    this.m_calPropsTimer = new Timer();
    
    // init queued calls:
    this.getFreeBusyTimes = makeQueuedCall(
        this.asyncQueue, this, this.getFreeBusyTimes_queued);
    this.searchForCalendars = makeQueuedCall(
        this.asyncQueue, this, this.searchForCalendars_queued);
}

calWcapSession.prototype = {
    m_asyncQueue: null,
    get asyncQueue() { return this.m_asyncQueue; },
    
    m_ifaces: [ Components.interfaces.calIWcapSession,
                Components.interfaces.calICalendarManagerObserver,
                Components.interfaces.nsIInterfaceRequestor,
                Components.interfaces.nsIClassInfo,
                Components.interfaces.nsISupports ],
    
    // nsISupports:
    QueryInterface:
    function( iid )
    {
        for each ( var iface in this.m_ifaces ) {
            if (iid.equals( iface ))
                return this;
        }
        throw Components.results.NS_ERROR_NO_INTERFACE;
    },
    
    // nsIClassInfo:
    getInterfaces:
    function( count )
    {
        count.value = this.m_ifaces.length;
        return this.m_ifaces;
    },
    get classDescription() {
        return calWcapCalendarModule.WcapSessionInfo.classDescription;
    },
    get contractID() {
        return calWcapCalendarModule.WcapSessionInfo.contractID;
    },
    get classID() {
        return calWcapCalendarModule.WcapSessionInfo.classID;
    },
    getHelperForLanguage: function( language ) { return null; },
    implementationLanguage:
    Components.interfaces.nsIProgrammingLanguage.JAVASCRIPT,
    flags: 0,
    
    // nsIInterfaceRequestor:
    getInterface:
    function( iid, instance )
    {
        if (iid.equals(Components.interfaces.nsIAuthPrompt)) {
            // use the window watcher service to get a nsIAuthPrompt impl
            return getWindowWatcher().getNewAuthPrompter(null);
        }
        else if (iid.equals(Components.interfaces.nsIPrompt)) {
            // use the window watcher service to get a nsIPrompt impl
            return getWindowWatcher().getNewPrompter(null);
        }
        Components.returnCode = Components.results.NS_ERROR_NO_INTERFACE;
        return null;
    },
    
    toString:
    function( msg )
    {
        var str = (this.uri ? this.uri.spec : "no uri");
        if (this.m_userId != null) {
            str += (", userId=" + this.userId);
        }
        if (this.m_sessionId == null) {
            str += (getIoService().offline ? ", offline" : ", not logged in");
        }
        return str;
    },
    log:
    function( msg, context )
    {
        return logMessage( context ? context : this.toString(), msg );
    },
    logError:
    function( err, context )
    {
        var msg = errorToString(err);
        Components.utils.reportError( this.log("error: " + msg, context) );
        return msg;
    },
    notifyError:
    function( err )
    {
        debugger;
        var msg = this.logError(err);
        this.notifyObservers(
            "onError",
            err instanceof Components.interfaces.nsIException
            ? [err.result, err.message] : [-1, msg] );
    },
    
    m_observers: null,
    notifyObservers:
    function( func, args )
    {
        this.m_observers.forEach(
            function( obj ) {
                try {
                    obj[func].apply( obj, args );
                }
                catch (exc) {
                    // don't call notifyError() here:
                    Components.utils.reportError( exc );
                }
            } );
    },
    
    addObserver:
    function( observer )
    {
        if (this.m_observers.indexOf( observer ) == -1) {
            this.m_observers.push( observer );
        }
    },
    
    removeObserver:
    function( observer )
    {
        this.m_observers = this.m_observers.filter(
            function(x) { return x != observer; } );
    },
    
    getSupportedTimezones:
    function( bRefresh )
    {
        var url = this.getCommandUrl("get_all_timezones"); // + logged in
        var key = this.sessionUri.hostPort;
        if (bRefresh || !g_allSupportedTimezones[key]) {   
            url += "&fmt-out=text%2Fcalendar";
            var icalRootComp = this.issueSyncRequest( url, stringToIcal );
            var tzids = [];
            var this_ = this;
            forEachIcalComponent(
                icalRootComp, "VTIMEZONE",
                function( subComp ) {
                    try {
                        var tzCal = getIcsService().createIcalComponent(
                            "VCALENDAR" );
                        subComp = subComp.clone();
                        tzCal.addSubcomponent( subComp );
                        getIcsService().addTimezone( tzCal, "", "" );
                        tzids.push( subComp.getFirstProperty("TZID").value );
                    }
                    catch (exc) { // ignore errors:
                        this_.logError( exc );
                    }
                } );
            g_allSupportedTimezones[key] = tzids;
        }
        return g_allSupportedTimezones[key];
    },
    
    getServerTime:
    function( localTime )
    {
        var ret = (localTime ? localTime.clone() : getTime());
        ret.addDuration( this.getServerTimeDiff() );
        return ret;
    },
    
    getServerTimeDiff:
    function( bRefresh )
    {
        var url = this.getCommandUrl("gettime"); // + logged in
        var key = this.sessionUri.hostPort;
        if (bRefresh || !g_serverTimeDiffs[key]) {
            url += "&fmt-out=text%2Fcalendar";
            // xxx todo: think about
            // assure that locally calculated server time is smaller than the
            // current (real) server time:
            var icalRootComp = this.issueSyncRequest( url, stringToIcal );
            var localTime = getTime();
            var serverTime = getDatetimeFromIcalProp(
                icalRootComp.getFirstProperty( "X-NSCP-WCAPTIME" ) );
            var diff = serverTime.subtractDate( localTime );
            this.log( "server time diff is: " + diff );
            g_serverTimeDiffs[key] = diff;
        }
        return g_serverTimeDiffs[key];
    },
    
    isSupportedTimezone:
    function( tzid )
    {
        var tzids = this.getSupportedTimezones();
        for each ( var id in tzids ) {
            if (id == tzid)
                return true;
        }
        return false;
    },
    
    m_sessionId: null,
    m_bNoLoginsAnymore: false,
    m_calPropsTimer: null,
    
    getSessionId:
    function( timedOutSessionId )
    {
        if (getIoService().offline) {
            this.log( "in offline mode." );
            throw new Components.Exception(
                "The requested action could not be completed while the " +
                "networking library is in the offline state.",
                NS_ERROR_OFFLINE );
        }
        if (this.m_bNoLoginsAnymore) {
            this.log( "login has failed, no logins anymore for this user." );
            throw new Components.Exception(
                "Login failed. Invalid session ID.",
                Components.interfaces.calIWcapErrors.WCAP_LOGIN_FAILED );
        }
        
        if (!this.m_sessionId || this.m_sessionId == timedOutSessionId) {
            
            try {
                this.m_sessionId = null;
                if (timedOutSessionId) {
                    this.log( "prompting to reconnect.", "session timeout" );
                    var prompt = getWindowWatcher().getNewPrompter(null);
                    var bundle = getWcapBundle();
                    if (!prompt.confirm(
                            bundle.GetStringFromName(
                                "reconnectConfirmation.label"),
                            bundle.formatStringFromName(
                                "reconnectConfirmation.text",
                                [this.uri.hostPort], 1 ) )) {
                        this.log( "reconnect cancelled." );
                        throw new Components.Exception(
                            "Login failed. Invalid session ID.",
                            Components.interfaces.
                            calIWcapErrors.WCAP_LOGIN_FAILED );
                    }
                }
                
                var sessionUri = this.uri.clone();
                sessionUri.userPass = "";
                if (!sessionUri.schemeIs("https") &&
                    sessionUri.port == -1 /* no port specified */)
                {
                    // silently probe for https support:
                    try { // enforce https:
                        sessionUri.scheme = "https";
                        this.getSessionId_(sessionUri);
                    }
                    catch (exc) {
                        // restore scheme:
                        sessionUri.scheme = this.uri.scheme;
                        if (testResultCode(exc, Components.interfaces.
                                           calIWcapErrors.WCAP_LOGIN_FAILED))
                            throw exc; // forward login failures
                        // but ignore connection errors
                    }
                }
                if (!this.m_sessionId)
                    this.getSessionId_(sessionUri);
            }
            catch (exc) {
                this.m_bNoLoginsAnymore = true;
                this.logError( exc );
                this.log( "no logins anymore." );
                throw exc;
            }
            
            this.assureInstalledLogoutObservers();
            
            this.getServerTimeDiff(true /* refresh */);
            
            if (!timedOutSessionId) // timezones don't change that frequently
                this.getSupportedTimezones(true /* refresh */);
            
            // reset calprops of all subscribed calendars before:
            this.getSubscribedCalendars({}).forEach(
                function(cal) { cal.resetCalProps(); } );
            
            this.m_calPropsTimer.initWithCallback(
                { // nsITimerCallback:
                    m_pos: 0,
                    m_session: this,
                    notify: function(timer_) {
                        var cals = this.m_session.getSubscribedCalendars({});
                        var c = (cals.length - this.m_pos);
                        if (c > 0) {
                            if (c > 5)
                                c = 5;
                            while (c--)
                                cals[this.m_pos++].getCalProps_(true /*async*/);
                        }
                        if (this.m_pos >= cals.length)
                            timer_.cancel();
                    }
                },
                15 * 1000,
                Components.interfaces.nsITimer.TYPE_REPEATING_SLACK );
        }
        
        if (!this.m_sessionId) {
            throw new Components.Exception(
                "Login failed. Invalid session ID.",
                Components.interfaces.calIWcapErrors.WCAP_LOGIN_FAILED );
        }
        return this.m_sessionId;
    },
    
    assureSecureLogin:
    function( sessionUri )
    {
        if (!sessionUri.schemeIs("https") &&
            !confirmInsecureLogin(sessionUri)) {
            this.log( "user rejected insecure login on " + sessionUri.spec );
            throw new Components.Exception(
                "Login failed. Invalid session ID.",
                Components.interfaces.calIWcapErrors.WCAP_LOGIN_FAILED );
        }
    },
    
    getSessionId_:
    function( sessionUri )
    {
        // probe whether server is accessible:
        var loginText = this.getServerInfo(sessionUri);
        
        this.log( "attempting to get a session id for " + sessionUri.spec );
        var outUser = { value: this.userId };
        var outPW = { value: null };
        
        var passwordManager =
            Components.classes["@mozilla.org/passwordmanager;1"]
            .getService(Components.interfaces.nsIPasswordManager);
        
        // xxx todo: pw host names must not have a trailing slash
        var pwHost = sessionUri.spec;
        if (pwHost[pwHost.length - 1] == '/')
            pwHost = pwHost.substr(0, pwHost.length - 1);
        this.log( "looking in pw db for: " + pwHost );
        try {
            var enumerator = passwordManager.enumerator;
            while (enumerator.hasMoreElements()) {
                var pwEntry = enumerator.getNext().QueryInterface(
                    Components.interfaces.nsIPassword );
                if (LOG_LEVEL > 1) {
                    this.log( "pw entry:\n\thost=" + pwEntry.host +
                              "\n\tuser=" + pwEntry.user );
                }
                if (pwEntry.host == pwHost) {
                    // found an entry matching URI:
                    outUser.value = pwEntry.user;
                    outPW.value = pwEntry.password;
                    break;
                }
            }
        }
        catch (exc) { // just log error
            this.logError(exc, "password manager lookup");
        }
        
        if (outPW.value) {
            this.log( "password entry found for host " + pwHost +
                      "\nuser is " + outUser.value );
            this.assureSecureLogin(sessionUri);
            this.login_( sessionUri, outUser.value, outPW.value );
        }
        else
            this.log( "no password entry found for " + pwHost );
        
        if (!this.m_sessionId) {
            if (outPW.value) {
                // login failed before, so try to remove from pw db:
                try {
                    passwordManager.removeUser( pwHost, outUser.value );
                    this.log( "removed from pw db: " + pwHost );
                }
                catch (exc) {
                    this.logError( "error removing from pw db: " + exc );
                }
            }
            else // if not already checked in pw manager run
                this.assureSecureLogin(sessionUri);
            
            var savePW = { value: false };
            while (!this.m_sessionId) {
                this.log( "prompting for user/pw..." );
                var prompt = getWindowWatcher().getNewPrompter(null);
                if (prompt.promptUsernameAndPassword(
                        getWcapBundle().GetStringFromName(
                            "loginDialog.label"),
                        loginText, outUser, outPW,
                        getWcapBundle().GetStringFromName(
                            "loginDialog.check.text"),
                        savePW ))
                {
                    if (this.login_( sessionUri, outUser.value, outPW.value ))
                        break;
                }
                else {
                    this.log( "login prompt cancelled." );
                    throw new Components.Exception(
                        "Login failed. Invalid session ID.",
                        Components.interfaces.calIWcapErrors.WCAP_LOGIN_FAILED);
                }
            }
            if (savePW.value) {
                try { // save pw under session uri:
                    passwordManager.addUser( pwHost,
                                             outUser.value, outPW.value );
                    this.log( "added to pw db: " + pwHost );
                }
                catch (exc) {
                    this.logError( "error adding pw to db: " + exc );
                }
            }
        }
        return this.m_sessionId;
    },
    
    login_:
    function( sessionUri, user, pw )
    {
        var str;
        var icalRootComp = null;
        try {
            // currently, xml parsing at an early stage during process startup
            // does not work reliably, so use libical parsing for now:
            str = issueSyncRequest(
                sessionUri.spec + "login.wcap?fmt-out=text%2Fcalendar&user=" +
                encodeURIComponent(user) +
                "&password=" + encodeURIComponent(pw),
                null /* receiverFunc */, false /* no logging */ );
            if (str.indexOf("BEGIN:VCALENDAR") < 0)
                throw new Components.Exception("no ical data returned!");
            icalRootComp = getIcsService().parseICS( str );
            checkWcapIcalErrno( icalRootComp );
        }
        catch (exc) {
            if (testResultCode(exc, Components.interfaces.
                               calIWcapErrors.WCAP_LOGIN_FAILED)) {
                this.logError( exc ); // log wrong pw
                return false;
            }
            if (!isNaN(exc) && getErrorModule(exc) == NS_ERROR_MODULE_NETWORK) {
                // server seems unavailable:
                throw new Components.Exception(
                    getWcapBundle().formatStringFromName(
                        "accessingServerFailedError.text",
                        [sessionUri.hostPort], 1 ),
                    exc );
            }
            throw exc;
        }
        var prop = icalRootComp.getFirstProperty("X-NSCP-WCAP-SESSION-ID");
        if (!prop) {
            throw new Components.Exception(
                "missing X-NSCP-WCAP-SESSION-ID in\n" + str );
        }
        this.m_sessionId = prop.value;        
        this.m_userId = user;
        this.m_sessionUri = sessionUri;
        this.log( "login succeeded, setting sessionUri to " +
                  this.sessionUri.spec );
        return true;
    },
    
    getServerInfo:
    function( uri )
    {
        var str;
        var icalRootComp = null;
        try {
            // currently, xml parsing at an early stage during process startup
            // does not work reliably, so use libical:
            str = issueSyncRequest(
                uri.spec + "version.wcap?fmt-out=text%2Fcalendar" );
            if (str.indexOf("BEGIN:VCALENDAR") < 0)
                throw new Components.Exception("no ical data returned!");
            icalRootComp = getIcsService().parseICS( str );
        }
        catch (exc) {
            this.log( errorToString(exc) ); // soft error; request denied etc.
            throw new Components.Exception(
                getWcapBundle().formatStringFromName(
                    "accessingServerFailedError.text", [uri.hostPort], 1 ),
                isNaN(exc) ? Components.results.NS_ERROR_FAILURE : exc );
        }
        
        var prop = icalRootComp.getFirstProperty( "X-NSCP-WCAPVERSION" );
        if (!prop) {
            throw new Components.Exception(
                "missing X-NSCP-WCAPVERSION in\n" + str );
        }
        var wcapVersion = parseInt(prop.value);
        if (wcapVersion < 3) {
            var strVers = prop.value;
            var vars = [uri.hostPort];
            prop = icalRootComp.getFirstProperty( "PRODID" );
            vars.push( prop ? prop.value : "<unknown>" );
            prop = icalRootComp.getFirstProperty( "X-NSCP-SERVERVERSION" );
            vars.push( prop ? prop.value : "<unknown>" );
            vars.push( strVers );
            
            var prompt = getWindowWatcher().getNewPrompter(null);
            var bundle = getWcapBundle();
            var labelText = bundle.GetStringFromName(
                "insufficientWcapVersionConfirmation.label");
            if (!prompt.confirm( labelText,
                                 bundle.formatStringFromName(
                                     "insufficientWcapVersionConfirmation.text",
                                     vars, vars.length ) )) {
                throw new Components.Exception(labelText);
            }
        }
        return getWcapBundle().formatStringFromName( "loginDialog.text",
                                                     [uri.hostPort], 1 );
    },
    
    getCommandUrl:
    function( wcapCommand )
    {
        if (!this.uri)
            throw new Components.Exception("no URI!");
        // ensure established session, so sessionUri and userId are set;
        // (calId defaults to userId) if not set:
        this.getSessionId();
        return (this.sessionUri.spec +
                wcapCommand + ".wcap?appid=mozilla-calendar");
    },
    
    issueRequest:
    function( url, issueFunc, dataConvFunc, receiverFunc )
    {
        var sessionId = this.getSessionId();
        
        var this_ = this;
        issueFunc(
            url + ("&id=" + encodeURIComponent(sessionId)),
            function( data ) {
                var wcapResponse = new WcapResponse();
                try {
                    try {
                        wcapResponse.data = dataConvFunc( data );
                    }
                    catch (exc) {
                        if (testResultCode(
                                exc, Components.interfaces.
                                calIWcapErrors.WCAP_LOGIN_FAILED)) /* timeout */
                        {
                            // getting a new session will throw any exception in
                            // this block, thus it is notified into receiverFunc
                            this_.getSessionId(
                                sessionId /* (old) timed-out session */ );
                            // try again:
                            this_.issueRequest(
                                url, issueFunc, dataConvFunc, receiverFunc );
                            return;
                        }
                        throw exc; // rethrow
                    }
                }
                catch (exc) {
                    // setting the request's exception will rethrow exception
                    // when request's data is retrieved.
                    wcapResponse.exception = exc;
                }
                receiverFunc( wcapResponse );
            } );
    },
    
    issueAsyncRequest:
    function( url, dataConvFunc, receiverFunc )
    {
        this.issueRequest(
            url, issueAsyncRequest, dataConvFunc, receiverFunc );
    },
    
    issueSyncRequest:
    function( url, dataConvFunc, receiverFunc )
    {
        var ret = null;
        this.issueRequest(
            url, issueSyncRequest,
            dataConvFunc,
            function( wcapResponse ) {
                if (receiverFunc) {
                    receiverFunc( wcapResponse );
                }
                ret = wcapResponse.data; // may throw
            } );
        return ret;
    },
    
    // calIWcapSession:
    
    m_uri: null,
    m_sessionUri: null,
    get uri() { return this.m_uri; },
    get sessionUri() { return this.m_sessionUri; },
    set uri( thatUri )
    {
        if (this.m_uri == null || thatUri == null ||
            !this.m_uri.equals(thatUri))
        {
            this.logout();
            this.m_uri = null;
            if (thatUri != null) {
                // sensible default for user id login:
                var username = decodeURIComponent( thatUri.username );
                if (username != "") {
                    // xxx todo: might vanish soon...
                    // patching the default calId via url:
                    this.m_defaultCalId = username;
                    var nColon = username.indexOf(':');
                    this.m_userId = (nColon >= 0
                                     ? username.substr(0, nColon) : username);
                }
                this.m_uri = thatUri.clone();
                this.log( "setting uri to " + this.uri.spec );
            }
        }
    },
    
    m_userId: null,
    get userId() { return this.m_userId; },
    
    m_defaultCalId: null,
    get defaultCalId() {
        if (this.m_defaultCalId)
            return this.m_defaultCalId;
        var list = this.getUserPreferences("X-NSCP-WCAP-PREF-icsCalendar", {});
        return ((list && list.length > 0) ? list[0] : this.userId);
    },
    
    assureLoggedIn:
    function()
    {
        if (!this.isLoggedIn) {
            throw new Components.Exception(
                "Not logged in yet.",
                Components.results.NS_ERROR_NOT_AVAILABLE);
        }
    },
    
    get isLoggedIn() { return this.m_sessionId != null; },
    
    login:
    function()
    {
        this.log("login");
        this.getSessionId(); // does *not* assure that ticket is valid
    },
    
    logout:
    function()
    {
        this.log("logout");
        this.m_calPropsTimer.cancel(); // stop any timed calprops getters
        this.asyncQueue.reset(); // stop any queued actions
        
        if (this.m_sessionId) {
            this.log( "attempting to log out..." );
            // although io service's offline flag is already
            // set BEFORE notification (about to go offline, nsIOService.cpp).
            // WTF.
            var url = (this.sessionUri.spec +
                       "logout.wcap?fmt-out=text%2Fxml&id=" +
                       encodeURIComponent(this.m_sessionId));
            try {
                checkWcapXmlErrno( issueSyncXMLRequest(url),
                                   -1 /* logout successfull */ );
                this.log( "logout succeeded." );
            }
            catch (exc) {
                this.log(exc, "logout failed.");
                Components.utils.reportError( exc );
            }
            this.m_sessionId = null;
        }
        
        this.m_sessionUri = null;
        this.m_userId = null;
        this.m_userPrefs = null; // reread prefs
        this.m_defaultCalId = null;
        this.m_bNoLoginsAnymore = false;
    },
    
    refresh:
    function()
    {
        if (this.m_bNoLoginsAnymore)
            this.logout(); // reset this session
        // else the next call will refresh any timed out ticket...
    },
    
    getWcapErrorString:
    function( rc )
    {
        return wcapErrorToString(rc);
    },
    
    m_defaultCalendar: null,
    get defaultCalendar() {
        if (!this.m_defaultCalendar) {
            this.m_defaultCalendar = createWcapCalendar(
                null /* calId: null indicates default calendar */, this);
        }
        return this.m_defaultCalendar;
    },
    
    m_calIdToCalendar: {},
    getCalendarByCalId_:
    function( calId, bCreate )
    {
        var ret = null;
        if (!calId || (LOG_LEVEL == 42 && // xxx todo: temp tbe hack
                       this.defaultCalId == calId)) {
            ret = this.defaultCalendar;
        }
        else {
            var key = encodeURIComponent(calId);
            ret = this.m_calIdToCalendar[key];
            if (!ret && bCreate) {
                ret = createWcapCalendar(calId, this);
                this.m_calIdToCalendar[key] = ret;
            }
        }
        return ret;
    },
    getCalendarByCalId:
    function( calId )
    {
        return this.getCalendarByCalId_(calId, true/*bCreate*/);
    },
    
    getCalendars:
    function( out_count, bGetOwnedCals )
    {
        var list = this.getUserPreferences(
            bGetOwnedCals ? "X-NSCP-WCAP-PREF-icsCalendarOwned"
                          : "X-NSCP-WCAP-PREF-icsSubscribed", {} );
        var ret = [];
        for each( var item in list ) {
            var ar = item.split(',');
            // ',', '$' are not encoded. ',' can be handled here. WTF.
            for each ( a in ar ) {
                var dollar = a.indexOf('$');
                if (dollar >= 0) {
                    ret.push(
                        this.getCalendarByCalId( a.substring(0, dollar) ) );
                }
            }
        }
        out_count.value = ret.length;
        return ret;
    },
    getOwnedCalendars:
    function( out_count )
    {
        return this.getCalendars( out_count, true );
    },
    getSubscribedCalendars:
    function( out_count )
    {
        return this.getCalendars( out_count, false );
    },
    
    createCalendar:
    function( calId, name )
    {
        try {
//             this.assureLoggedIn();
            var url = this.getCommandUrl("createcalendar");
            url += "&allowdoublebook=1&set_calprops=1&subscribe=1";
            url += ("&calid=" + encodeURIComponent(calId));
            // xxx todo: name undocumented!
            url += ("&name=" + encodeURIComponent(name));
            // xxx todo: what about categories param???
            this.issueSyncRequest(url + "&fmt-out=text%2Fxml", stringToXml);
            this.m_userPrefs = null; // reread prefs
            return this.getCalendarByCalId(this.userId + ":" + calId);
        }
        catch (exc) {
            this.logError( exc );
            throw exc;
        }
    },
    
    deleteCalendar:
    function( cal )
    {
        try {
//             this.assureLoggedIn();
            // xxx todo:
            cal.assureAccess(Components.interfaces.calIWcapCalendar.AC_FULL);
            var calId = cal.calId;
            var url = this.getCommandUrl("deletecalendar");
            url += ("&calid=" + encodeURIComponent(calId));
            url += "&unsubscribe=1&fmt-out=text%2Fxml";
            this.issueSyncRequest(url, stringToXml);
            this.m_userPrefs = null; // reread prefs
            // xxx todo: delete here?
            this.m_calIdToCalendar[encodeURIComponent(calId)] = null;
            cal.readOnly = true;
        }
        catch (exc) {
            this.logError(exc);
            throw exc;
        }
    },
    
    modifyCalendarSubscriptions:
    function( cals, bSubscribe )
    {
        try {
//             this.assureLoggedIn();
            var url = this.getCommandUrl(
                bSubscribe ? "subscribe_calendars" : "unsubscribe_calendars" );
            var calId = "";
            for each ( var cal in cals ) {
                if (calId.length > 0)
                    calId += ";";
                calId += encodeURIComponent(cal.calId);
            }
            url += ("&calid=" + calId);
            this.issueSyncRequest(url + "&fmt-out=text%2Fxml", stringToXml);
            this.m_userPrefs = null; // reread prefs
            for each ( cal in cals ) {
                if (bSubscribe) {
                    this.m_calIdToCalendar[
                        encodeURIComponent(cal.calId)] = cal;
                }
                else { // remove from cached instances
                    // xxx todo: delete here?
                    this.m_calIdToCalendar[
                        encodeURIComponent(cal.calId)] = null;
                }
            }
        }
        catch (exc) {
            this.logError( exc );
            throw exc;
        }
    },
    
    subscribeToCalendars:
    function( count, cals )
    {
        this.modifyCalendarSubscriptions(cals, true/*bSubscribe*/);
    },
    
    unsubscribeFromCalendars:
    function( count, cals )
    {
        this.modifyCalendarSubscriptions(cals, false/*!bSubscribe*/);
    },
    
    m_userPrefs: null,
    getUserPreferences:
    function( prefName, out_count )
    {
        try {
            if (!this.m_userPrefs) {
//                 this.assureLoggedIn();
                var url = this.getCommandUrl( "get_userprefs" );
                url += ("&userid=" + encodeURIComponent(this.userId));
                this.m_userPrefs = this.issueSyncRequest(
                    url + "&fmt-out=text%2Fxml", stringToXml );
            }
            var ret = [];
            var nodeList = this.m_userPrefs.getElementsByTagName(prefName);
            for ( var i = 0; i < nodeList.length; ++i ) {
                var node = nodeList.item(i);
                ret.push( trimString(node.textContent) );
            }
            out_count.value = ret.length;
            return ret;
        }
        catch (exc) {
            this.logError( exc );
            throw exc;
        }
    },
    
    get defaultAlarmStart() {
        var alarmStart = null;
        var ar = this.getUserPreferences(
            "X-NSCP-WCAP-PREF-ceDefaultAlarmStart", {});
        if (ar.length > 0 && ar[0].length > 0) {
            // workarounding cs duration bug, missing "T":
            var dur = ar[0].replace(/(^P)(\d+[HMS]$)/, "$1T$2");
            alarmStart = new CalDuration();
            alarmStart.icalString = dur;
            alarmStart.isNegative = !alarmStart.isNegative;
        }
        return alarmStart;
    },
    
    getDefaultAlarmEmails:
    function( out_count )
    {
        var ret = [];
        var ar = this.getUserPreferences(
            "X-NSCP-WCAP-PREF-ceDefaultAlarmEmail", {});
        if (ar.length > 0 && ar[0].length > 0) {
            for each ( var i in ar ) {
                ret = ret.concat( i.split(/[;,]/).map(trimString) );
            }
        }
        out_count.value = ret.length;
        return ret;
    },
    
    getFreeBusyTimes_resp:
    function( wcapResponse, calId, listener, requestId )
    {
        try {
            var xml = wcapResponse.data; // first statement, may throw
            if (listener != null) {
                var ret = [];
                var nodeList = xml.getElementsByTagName("FB");
                for ( var i = 0; i < nodeList.length; ++i ) {
                    var node = nodeList.item(i);
                    var str = node.textContent;
                    var slash = str.indexOf( '/' );
                    var start = getDatetimeFromIcalString(str.substr(0, slash));
                    var end = getDatetimeFromIcalString(str.substr(slash + 1));
                    var entry = {
                        isBusyEntry:
                        (node.attributes.getNamedItem("FBTYPE").nodeValue
                         == "BUSY"),
                        dtRangeStart: start,
                        dtRangeEnd: end
                    };
                    ret.push( entry );
                }
                listener.onGetFreeBusyTimes(
                    Components.results.NS_OK,
                    requestId, calId, ret.length, ret );
            }
            if (LOG_LEVEL > 0) {
                this.log( "calId=" + calId + ", " +
                          getWcapRequestStatusString(xml),
                          "getFreeBusyTimes_resp()" );
            }
        }
        catch (exc) {
            const calIWcapErrors = Components.interfaces.calIWcapErrors;
            switch (getResultCode(exc)) {
            case calIWcapErrors.WCAP_NO_ERRNO: // workaround
            case calIWcapErrors.WCAP_ACCESS_DENIED_TO_CALENDAR:
            case calIWcapErrors.WCAP_CALENDAR_DOES_NOT_EXIST:
                this.log("ignored error: " + errorToString(exc),
                         "getFreeBusyTimes_resp()");
                break;
            default:
                this.notifyError( exc );
                break;
            }
            if (listener != null)
                listener.onGetFreeBusyTimes( exc, requestId, calId, 0, [] );
        }
    },
    
    getFreeBusyTimes_queued:
    function( calId, rangeStart, rangeEnd, bBusyOnly, listener, b, requestId )
    {
        try {
            // assure DATETIMEs:
            if (rangeStart != null && rangeStart.isDate) {
                rangeStart = rangeStart.clone();
                rangeStart.isDate = false;
            }
            if (rangeEnd != null && rangeEnd.isDate) {
                rangeEnd = rangeEnd.clone();
                rangeEnd.isDate = false;
            }
            var zRangeStart = getIcalUTC(rangeStart);
            var zRangeEnd = getIcalUTC(rangeEnd);
            this.log( "\n\trangeStart=" + zRangeStart +
                      ",\n\trangeEnd=" + zRangeEnd,
                      "getFreeBusyTimes()" );
            
            var url = this.getCommandUrl( "get_freebusy" );
            url += ("&calid=" + encodeURIComponent(calId));
            url += ("&busyonly=" + (bBusyOnly ? "1" : "0"));
            url += ("&dtstart=" + zRangeStart);
            url += ("&dtend=" + zRangeEnd);
            url += "&fmt-out=text%2Fxml";
            
            var this_ = this;
            function resp( wcapResponse ) {
                this_.getFreeBusyTimes_resp(
                    wcapResponse, calId, listener, requestId );
            }
            this.issueAsyncRequest( url, stringToXml, resp );
        }
        catch (exc) {
            this.notifyError( exc );
            if (listener != null)
                listener.onGetFreeBusyTimes( exc, requestId, calId, 0, [] );
        }
    },
    
    searchForCalendars_resp:
    function( wcapResponse, listener, requestId )
    {
        try {
            var xml = wcapResponse.data; // first statement, may throw
            if (listener) {
                var ret = [];
                var nodeList = xml.getElementsByTagName("iCal");
                for ( var i = 0; i < nodeList.length; ++i ) {
                    var node = nodeList.item(i);
                    try {
                        checkWcapXmlErrno(node);
                        var ar = filterCalProps(
                            "X-NSCP-CALPROPS-RELATIVE-CALID", node);
                        if (ar.length > 0) {
                            var calId = ar[0];
                            // take existing one from subscribed list;
                            // xxx todo assuming there is an existing instance
                            // for every subscribed calendar at this point!
                            var cal = this.getCalendarByCalId_(
                                calId, false/*bCreate*/);
                            if (!cal)
                                cal = createWcapCalendar(calId, this, node);
                            ret.push(cal);
                        }
                    }
                    catch (exc) {
                        const calIWcapErrors = Components.interfaces
                                               .calIWcapErrors;
                        switch (getResultCode(exc)) {
                        case calIWcapErrors.WCAP_NO_ERRNO: // workaround
                        case calIWcapErrors.WCAP_ACCESS_DENIED_TO_CALENDAR:
                            this.log("ignored error: " + errorToString(exc),
                                     "searchForCalendars_resp()");
                            break;
                        default:
                            this.notifyError(exc);
                            break;
                        }
                    }
                }
                this.log("number of found calendars: " + ret.length);
                listener.onSearchForCalendarsResults(
                    Components.results.NS_OK, requestId, ret.length, ret);
            }
            if (LOG_LEVEL > 0)
                this.log("search done.");
        }
        catch (exc) {
            this.notifyError(exc);
            if (listener)
                listener.onSearchForCalendarsResults(exc, requestId, 0, []);
        }
    },
    
    searchForCalendars_queued:
    function( searchString, searchOptions, listener, requestId )
    {
        try {
            var url = this.getCommandUrl("search_calprops");
            url += ("&search-string=" + encodeURIComponent(searchString));
            url += ("&searchOpts=" + (searchOptions & 3).toString(10));
            const calIWcapSession = Components.interfaces.calIWcapSession;
            if (searchOptions & calIWcapSession.SEARCH_INCLUDE_CALID)
                url += "&calid=1";
            if (searchOptions & calIWcapSession.SEARCH_INCLUDE_NAME)
                url += "&name=1";
            if (searchOptions & calIWcapSession.SEARCH_INCLUDE_OWNER)
                url += "&primaryOwner=1";
            url += "&fmt-out=text%2Fxml";
            
            var this_ = this;
            this.issueAsyncRequest(
                url,
                // string to xml converter func without WCAP errno check:
                function(data) {
                    if (!data || data == "") { // assuming time-out
                        throw new Components.Exception(
                            "Login failed. Invalid session ID.",
                            Components.interfaces.calIWcapErrors
                            .WCAP_LOGIN_FAILED );
                    }
                    return getDomParser().parseFromString(data, "text/xml");
                },
                // response func:
                function(wcapResponse) {
                    this_.searchForCalendars_resp(
                        wcapResponse, listener, requestId);
                } );
        }
        catch (exc) {
            this.notifyError(exc);
            if (listener)
                listener.onSearchForCalendarsResults(exc, requestId, 0, []);
        }
    },
    
    m_bInstalledLogoutObservers: false,
    assureInstalledLogoutObservers:
    function()
    {
        // don't do this in ctor, calendar manager calls back to all
        // registered calendars!
        if (!this.m_bInstalledLogoutObservers) {
            this.m_bInstalledLogoutObservers = true;
            // listen for shutdown, being logged out:
            // network:offline-about-to-go-offline will be fired for
            // XPCOM shutdown, too.
            // xxx todo: alternatively, add shutdown notifications to
            //           cal manager
            // xxx todo: how to simplify this for multiple topics?
            var observerService =
                Components.classes["@mozilla.org/observer-service;1"]
                .getService(Components.interfaces.nsIObserverService);
            observerService.addObserver(
                this, "quit-application",
                false /* don't hold weakly: xxx todo */);
            observerService.addObserver(
                this, "network:offline-about-to-go-offline",
                false /* don't hold weakly: xxx todo */);
            getCalendarManager().addObserver(this);
        }
    },
    
    // nsIObserver:
    observe:
    function( subject, topic, data )
    {
        this.log( "observing: " + topic + ", data: " + data );
        if (topic == "network:offline-about-to-go-offline") {
            this.logout();
        }
        else if (topic == "quit-application") {
            this.logout();
            // xxx todo: valid upon notification?
            getCalendarManager().removeObserver(this);
            var observerService =
                Components.classes["@mozilla.org/observer-service;1"]
                .getService(Components.interfaces.nsIObserverService);
            observerService.removeObserver(
                this, "quit-application" );
            observerService.removeObserver(
                this, "network:offline-about-to-go-offline" );
        }
    },
    
    // calICalendarManagerObserver:
    
    // called after the calendar is registered
    onCalendarRegistered:
    function( cal )
    {
    },
    
    // called before the unregister actually takes place
    onCalendarUnregistering:
    function( cal )
    {
        try {
            cal = cal.QueryInterface(Components.interfaces.calIWcapCalendar);
        }
        catch (exc) {
            cal = null;
        }
        if (cal && cal.session.wrappedJSObject == this &&
            cal.calId == this.defaultCalId) {
            // calendar is to be deleted, so logout before:
            this.logout();
        }
    },
    
    // called before the delete actually takes place
    onCalendarDeleting:
    function( cal )
    {
    },
    
    // called after the pref is set
    onCalendarPrefSet:
    function( cal, name, value )
    {
    },
    
    // called before the pref is deleted
    onCalendarPrefDeleting:
    function( cal, name )
    {
    }
};

var g_confirmedHttpLogins = null;
function confirmInsecureLogin( uri )
{
    if (g_confirmedHttpLogins == null) {
        g_confirmedHttpLogins = {};
        var confirmedHttpLogins = getPref(
            "calendar.wcap.confirmed_http_logins", "");
        var tuples = confirmedHttpLogins.split(',');
        for each ( var tuple in tuples ) {
            var ar = tuple.split(':');
            g_confirmedHttpLogins[ar[0]] = ar[1];
        }
    }
    
    var bConfirmed = false;
    
    var host = uri.hostPort;
    var encodedHost = encodeURIComponent(host);
    var confirmedEntry = g_confirmedHttpLogins[encodedHost];
    if (confirmedEntry) {
        bConfirmed = (confirmedEntry == "1" ? true : false);
    }
    else {
        var prompt = getWindowWatcher().getNewPrompter(null);
        var bundle = getWcapBundle();
        var dontAskAgain = { value: false };
        var bConfirmed = prompt.confirmCheck(
            bundle.GetStringFromName("noHttpsConfirmation.label"),
            bundle.formatStringFromName("noHttpsConfirmation.text", [host], 1),
            bundle.GetStringFromName("noHttpsConfirmation.check.text"),
            dontAskAgain );
        
        if (dontAskAgain.value) {
            // save decision for all running calendars and
            // all future confirmations:
            var confirmedHttpLogins = getPref(
                "calendar.wcap.confirmed_http_logins", "");
            if (confirmedHttpLogins.length > 0)
                confirmedHttpLogins += ",";
            confirmedEntry = (bConfirmed ? "1" : "0");
            confirmedHttpLogins += (encodedHost + ":" + confirmedEntry);
            setPref("calendar.wcap.confirmed_http_logins", confirmedHttpLogins);
            g_confirmedHttpLogins[encodedHost] = confirmedEntry;
        }
    }
    
    logMessage("confirmInsecureLogin(" + host + ")", "returned: " + bConfirmed);
    return bConfirmed;
}

