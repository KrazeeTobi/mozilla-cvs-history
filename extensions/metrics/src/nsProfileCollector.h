/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Original Code is the Metrics extension.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#ifndef nsProfileCollector_h_
#define nsProfileCollector_h_

#include "nscore.h"

class nsISupports;
class nsIMetricsEventItem;
class nsIPropertyBag;
class nsAString;

// This file defines the profile collector class, which generates
// a profile event at application startup.  The profile event logs
// various information about the user's software and hardware configuration.

class nsProfileCollector
{
 public:
  // Enables or disables the profile collector.
  // The collector should be shut down with SetEnabled(PR_FALSE)
  // when the metrics service is shut down.
  static nsresult SetEnabled(PRBool enabled);

  // Returns the singleton nsProfileCollector object, or NULL
  // if the profile collector is not enabled.
  static nsProfileCollector *GetInstance() { return sInstance; }

  // Called to log a profile event to the metrics service.
  nsresult LogProfile();

 protected:
  // Instances of this class are only created from SetEnabled().
  nsProfileCollector() { }

 private:
  ~nsProfileCollector() { }

  // These methods each create a particular item and append it to the profile.
  nsresult LogCPU(nsIMetricsEventItem *profile);
  nsresult LogMemory(nsIMetricsEventItem *profile);
  nsresult LogOS(nsIMetricsEventItem *profile);
  nsresult LogInstall(nsIMetricsEventItem *profile);
  nsresult LogExtensions(nsIMetricsEventItem *profile);
  nsresult LogPlugins(nsIMetricsEventItem *profile);
  nsresult LogDisplay(nsIMetricsEventItem *profile);

  class PluginEnumerator;
  class ExtensionEnumerator;

  // the profile collector singleton
  static nsProfileCollector *sInstance;
};

#endif  // nsProfileCollector_h_
