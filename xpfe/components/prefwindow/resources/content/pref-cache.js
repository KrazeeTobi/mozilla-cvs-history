/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 */

const nsIFilePicker       = Components.interfaces.nsIFilePicker;
const nsILocalFile        = Components.interfaces.nsILocalFile;
const nsIProperties       = Components.interfaces.nsIProperties;
const kCacheParentDirPref = "browser.cache.disk.parent_directory";

var gFolderField;

function Startup()
{
  var prefWindow = parent.hPrefWindow;
  gFolderField = document.getElementById("browserCacheDiskCacheFolder");

  var dir = prefWindow.getPref("localfile", kCacheParentDirPref);
  if (dir == "!/!ERROR_UNDEFINED_PREF!/!")
  {
    try
    {
      // no disk cache folder found; default to profile directory
      var dirSvc = Components.classes["@mozilla.org/file/directory_service;1"]
                             .getService(nsIProperties);
      dir = dirSvc.get("ProfD", nsILocalFile);

      // now remember the new assumption
      prefWindow.setPref("localfile", kCacheParentDirPref, dir);
    }
    catch (ex)
    {
      dir = null;
    }
  }

  // if both pref and dir svc fail leave this field blank else show directory
  if (dir)
    gFolderField.value = (/Mac/.test(navigator.platform)) ? dir.leafName : dir.path;

  document.getElementById("chooseDiskCacheFolder").disabled =
    prefWindow.getPrefIsLocked(kCacheParentDirPref);
}


function prefCacheSelectFolder()
{
  var fp = Components.classes["@mozilla.org/filepicker;1"]
                     .createInstance(nsIFilePicker);
  var prefWindow = parent.hPrefWindow;
  var prefutilitiesBundle = document.getElementById("bundle_prefutilities");
  var title = prefutilitiesBundle.getString("cachefolder");

  fp.init(window, title, nsIFilePicker.modeGetFolder);

  var initialDir = prefWindow.getPref("localfile", kCacheParentDirPref);

  // Pref should always be set but just in case...
  if (initialDir != "!/!ERROR_UNDEFINED_PREF!/!")
    fp.displayDirectory = initialDir;

  fp.appendFilters(nsIFilePicker.filterAll);
  var ret = fp.show();

  if (ret == nsIFilePicker.returnOK) {
    var localFile = fp.file.QueryInterface(nsILocalFile);
    prefWindow.setPref("localfile", kCacheParentDirPref, localFile);
    gFolderField.value = (/Mac/.test(navigator.platform)) ? fp.file.leafName : fp.file.path;
  }
}

function prefClearCache(aType)
{
    var classID = Components.classes["@mozilla.org/network/cache-service;1"];
    var cacheService = classID.getService(Components.interfaces.nsICacheService);
    cacheService.evictEntries(aType);
}

function prefClearDiskAndMemCache()
{
    prefClearCache(Components.interfaces.nsICache.STORE_ON_DISK);
    prefClearCache(Components.interfaces.nsICache.STORE_IN_MEMORY);
}
