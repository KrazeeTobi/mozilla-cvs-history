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
 * The Original Code is Download Manager Test Code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Mike Connor <mconnor@mozilla.com> (Original Author)
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

// This tests the cleanup code to make sure we properly remove old downloads.rdf files
// when clearing data.

function cleanup()
{
  // removing rdf
  var rdfFile = dirSvc.get("DLoads", Ci.nsIFile);
  if (rdfFile.exists()) rdfFile.remove(true);
  
  // removing database
  var dbFile = dirSvc.get("ProfD", Ci.nsIFile);
  dbFile.append("downloads.sqlite");
  if (dbFile.exists())
    try { dbFile.remove(true); } catch(e) { /* stupid windows box */ }
}

cleanup();

importDownloadsFile("bug_381535_downloads.rdf");
importDownloadsFile("bug_409179_downloads.sqlite");

function run_test()
{
  var rdfFile = dirSvc.get("DLoads", Ci.nsIFile);
  do_check_true(rdfFile.exists());

  var dm = Cc["@mozilla.org/download-manager;1"].
           getService(Ci.nsIDownloadManager);

  dm.cleanUp();
  
  do_check_false(rdfFile.exists());

  cleanup();
}
