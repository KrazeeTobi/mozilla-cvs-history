/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License. 
 *
 * The Original Code is The JavaScript Debugger
 * 
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation
 * Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the MPL or the GPL.
 *
 * Contributor(s):
 *  Robert Ginda, <rginda@netscape.com>, original author
 *
 */

const PREF_RELOAD = true;
const PREF_WRITETHROUGH = true;

function PrefManager (branchName)
{
    var prefManager = this;
    
    function pm_observe (prefService, topic, prefName)
    {
        var r = prefManager.prefRecords[prefName];
        if (!r)
            return;
        
        var oldValue = (r.realValue != null) ? r.realValue : r.defaultValue;
        r.realValue = prefManager.getPref(prefName, PREF_RELOAD);
        prefManager.onPrefChanged(prefName, r.realValue, oldValue);
    };

    const PREF_CTRID = "@mozilla.org/preferences-service;1";
    const nsIPrefService = Components.interfaces.nsIPrefService;
    const nsIPrefBranch = Components.interfaces.nsIPrefBranch;
    const nsIPrefBranchInternal = Components.interfaces.nsIPrefBranchInternal;

    this.prefService =
        Components.classes[PREF_CTRID].getService(nsIPrefService);
    this.prefBranch = this.prefService.getBranch(branchName);
    this.defaultValues = new Object();
    this.prefs = new Object();
    this.prefNames = new Array();
    this.prefRecords = new Object();
    this.observer = { observe: pm_observe };

    this.prefBranchInternal =
        this.prefBranch.QueryInterface(nsIPrefBranchInternal);
    this.prefBranchInternal.addObserver("", this.observer, false);

    this.valid = true;
}

PrefManager.prototype.destroy =
function pm_destroy()
{
    if (this.valid)
    {
        this.prefBranchInternal.removeObserver("", this.observer);
        this.valid = false;
    }
}

PrefManager.prototype.getBranch =
function pm_getbranch(suffix)
{
    return this.prefService.getBranch(this.prefBranch.root + suffix);
}

PrefManager.prototype.getBranchManager =
function pm_getbranchmgr(suffix)
{
    return new PrefManager(this.prefBranch.root + suffix);
}

PrefManager.prototype.onPrefChanged =
function pm_changed(prefName, prefManager, topic)
{
    /* clients can override this to hear about pref changes */
}

PrefManager.prototype.listPrefs =
function pm_listprefs (prefix)
{
    var list = new Array();
    var names = this.prefNames;
    for (var i = 0; i < names.length; ++i)
    {
        if (!prefix || names[i].indexOf(prefix) == 0)
            list.push (names[i]);
    }

    return list;
}

PrefManager.prototype.readPrefs =
function pm_readprefs ()
{
    const nsIPrefBranch = Components.interfaces.nsIPrefBranch;

    var list = this.prefBranch.getChildList("", {});
    for (var i = 0; i < list.length; ++i)
    {
        if (!(list[i] in this))
        {
            var type = this.prefBranch.getPrefType (list[i]);
            var defaultValue;
            
            switch (type)
            {
                case nsIPrefBranch.PREF_INT:
                    defaultValue = 0;
                    break;
                
                case nsIPrefBranch.PREF_BOOL:
                    defaultValue = false;
                    break;

                default:
                    defaultValue = "";
            }
            
            this.addPref(list[i], defaultValue);
        }
    }
}

PrefManager.prototype.isKnownPref =
function pm_ispref(prefName)
{
    return (prefName in this.prefRecords);
}

PrefManager.prototype.addPrefs =
function pm_addprefs(prefSpecs)
{
    for (var i = 0; i < prefSpecs.length; ++i)
    {
        this.addPref(prefSpecs[i][0], prefSpecs[i][1],
                     2 in prefSpecs[i] ? prefSpecs[i][2] : null);
    }
}

PrefManager.prototype.addDeferredPrefs =
function pm_addprefsd(targetManager, writeThrough)
{
    function deferGet(prefName)
    {
        return targetManager.getPref(prefName);
    };

    function deferSet(prefName, value)
    {
        return targetManager.setPref(prefName, value);
    };

    var setter = null;
    
    if (writeThrough)
        setter = deferSet;
    
    var prefs = targetManager.prefs;
    
    for (var i = 0; i < prefs.length; ++i)
        this.addPref(prefs[i], deferGet, setter);
}

PrefManager.prototype.updateArrayPref =
function pm_arrayupdate(prefName)
{
    var record = this.prefRecords[prefName];
    if (!ASSERT(record, "Unknown pref: " + prefName))
        return;

    if (record.realValue == null)
        record.realValue = record.defaultValue;
    
    if (!ASSERT(record.realValue instanceof Array, "Pref is not an array"))
        return;

    this.prefBranch.setCharPref(prefName, this.arrayToString(record.realValue));
    this.prefService.savePrefFile(null);
}

PrefManager.prototype.stringToArray =
function pm_s2a(string)
{
    if (string.search(/\S/) == -1)
        return [];
    
    var ary = string.split(/\s*;\s*/);
    for (var i = 0; i < ary.length; ++i)
        ary[i] = unescape(ary[i]);

    return ary;
}

PrefManager.prototype.arrayToString =
function pm_a2s(ary)
{
    var escapedAry = new Array()
    for (var i = 0; i < ary.length; ++i)
        escapedAry[i] = escape(ary[i]);

    return escapedAry.join("; ");
}

PrefManager.prototype.getPref =
function pm_getpref(prefName, reload)
{
    var prefManager = this;
    
    function updateArrayPref() { prefManager.updateArrayPref(prefName); };
    
    var record = this.prefRecords[prefName];
    if (!ASSERT(record, "Unknown pref: " + prefName))
        return null;
    
    var defaultValue;

    if (typeof record.defaultValue == "function")
    {
        // deferred pref, call the getter, and don't cache the result.
        defaultValue = record.defaultValue(prefName);
    }
    else
    {
        if (!reload && record.realValue != null)
            return record.realValue;

        defaultValue = record.defaultValue;
    }
    
    var realValue = null;

    try
    {
        if (typeof defaultValue == "boolean")
        {
            realValue = this.prefBranch.getBoolPref(prefName);
        }
        else if (typeof defaultValue == "number")
        {
            realValue = this.prefBranch.getIntPref(prefName);
        }
        else if (defaultValue instanceof Array)
        {
            realValue = this.prefBranch.getCharPref(prefName);
            realValue = this.stringToArray(realValue);
            realValue.update = updateArrayPref;
        }
        else if (typeof defaultValue == "string" ||
                 defaultValue == null)
        {
            realValue = this.prefBranch.getCharPref(prefName);
        }
    }
    catch (ex)
    {
        // if the pref doesn't exist, ignore the exception.
    }

    if (realValue == null)
        return defaultValue;

    record.realValue = realValue;
    return realValue;
}

PrefManager.prototype.setPref =
function pm_setpref(prefName, value)
{
    var prefManager = this;
    
    function updateArrayPref() { prefManager.updateArrayPref(prefName); };

    var record = this.prefRecords[prefName];
    if (!ASSERT(record, "Unknown pref: " + prefName))
        return null;
    
    if ((record.realValue == null && value == record.defaultValue) ||
        record.realValue == value)
    {
        // no realvalue, and value is the same as default value ... OR ...
        // no change at all.  just bail.
        return record.realValue;
    }

    if (value == record.defaultValue)
    {
        this.clearPref(prefName);
        return value;
    }
    
    var defaultValue = record.defaultValue;

    if (typeof defaultValue == "function")
        defaultValue = defaultValue(prefName);
    
    if (typeof defaultValue == "boolean")
    {
        this.prefBranch.setBoolPref(prefName, value);
    }
    else if (typeof defaultValue == "number")
    {
        this.prefBranch.setIntPref(prefName, value);
    }
    else if (defaultValue instanceof Array)
    {
        var str = this.arrayToString(value);
        this.prefBranch.setCharPref(prefName, str);
        value.update = updateArrayPref;
    }
    else
    {
        this.prefBranch.setCharPref(prefName, value);
    }
    
    this.prefService.savePrefFile(null);

    record.realValue = value;
    
    return value;
}

PrefManager.prototype.clearPref =
function pm_reset(prefName)
{
    this.prefRecords[prefName].realValue = null;
    this.prefBranch.clearUserPref(prefName);
    this.prefService.savePrefFile(null);
}

PrefManager.prototype.addPref =
function pm_addpref(prefName, defaultValue, setter)
{
    var prefManager = this;

    function updateArrayPref() { prefManager.updateArrayPref(prefName); };
    function prefGetter() { return prefManager.getPref(prefName); };
    function prefSetter(value) { return prefManager.setPref(prefName, value); };

    if (!ASSERT(!(prefName in this.defaultValues),
                "Preference already exists: " + prefName))
    {
        return;
    }
    
    if (!setter)
        setter = prefSetter;
    
    if (defaultValue instanceof Array)
        defaultValue.update = updateArrayPref;

    this.prefRecords[prefName] = {defaultValue: defaultValue, realValue: null};

    this.prefNames.push(prefName);
    this.prefNames.sort();

    this.prefs.__defineGetter__(prefName, prefGetter);
    this.prefs.__defineSetter__(prefName, setter);
}
