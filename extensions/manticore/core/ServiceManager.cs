/* -*- Mode: C#; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is Manticore.
 * 
 * The Initial Developer of the Original Code is
 * Silverstone Interactive. Portions created by Silverstone Interactive are
 * Copyright (C) 2001 Silverstone Interactive. 
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
 *  Ben Goodger <ben@netscape.com> (Original Author)
 *
 */

namespace Silverstone.Manticore.Core
{

  /// <summary>
  /// Access point to application-global services. 
  /// </summary>
  public class ServiceManager
  {
    private static Silverstone.Manticore.Core.Preferences mPreferences = null;
    public static Silverstone.Manticore.Core.Preferences Preferences 
    {
      get 
      {
        // Initialize default and user preferences
        if (mPreferences == null) 
        {
          mPreferences = new Preferences();
          mPreferences.InitializeDefaults();
          mPreferences.LoadUserPreferences();
        }
        return mPreferences;
      }
    }

    private static Silverstone.Manticore.Bookmarks.Bookmarks mBookmarks = null;
    public static Silverstone.Manticore.Bookmarks.Bookmarks Bookmarks
    {
      get 
      {
        // Start the Bookmarks Service if it has not already been initialized
        if (mBookmarks == null) 
        {
          mBookmarks = new Silverstone.Manticore.Bookmarks.Bookmarks();
          mBookmarks.LoadBookmarks();
        }
        return mBookmarks;
      }
    }
  }
}