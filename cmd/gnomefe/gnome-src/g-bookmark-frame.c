/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
/*
  g-bookmark-frame.c -- bookmark windows.
  Created: Chris Toshok <toshok@hungry.com>, 13-Apr-98.
*/

#include "xp_mem.h"
#include "structs.h"
#include "ntypes.h"
#include "g-commands.h"
#include "g-bookmark-frame.h"
#include "g-bookmark-view.h"

static void
callback(GtkWidget *widget,
	 gpointer client_data)
{
  /* blah */
}

static GnomeUIInfo file_submenu[] = {
  { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo edit_submenu[] = {
  { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo view_submenu[] = {
  { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo window_submenu[] = {
  { GNOME_APP_UI_ITEM, "Navigation Center", NULL, callback, NULL, NULL },

  { GNOME_APP_UI_ITEM, "Navigator", NULL, moz_open_browser, NULL, NULL },

#ifdef EDITOR
  { GNOME_APP_UI_ITEM, "Composer", NULL, callback, NULL, NULL },
#endif
  { GNOME_APP_UI_SEPARATOR },

  { GNOME_APP_UI_ITEM, "Bookmarks", NULL, moz_open_bookmarks, NULL, NULL },

  { GNOME_APP_UI_ITEM, "History", NULL, moz_open_history, NULL, NULL },

  { GNOME_APP_UI_ITEM, "View Security", NULL, callback, NULL, NULL },

  { GNOME_APP_UI_SEPARATOR },
  
  { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo help_submenu[] = {
  { GNOME_APP_UI_HELP, "HelpStuff", NULL, "GnuZilla" },
  { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo menubar_info[] = {
  { GNOME_APP_UI_SUBTREE, "File", NULL, file_submenu },
  { GNOME_APP_UI_SUBTREE, "Edit", NULL, edit_submenu },
  { GNOME_APP_UI_SUBTREE, "View", NULL, view_submenu },
  { GNOME_APP_UI_SUBTREE, "Window", NULL, window_submenu },
  { GNOME_APP_UI_SUBTREE, "Help", NULL, help_submenu },
  { GNOME_APP_UI_ENDOFINFO }
};

void
moz_bookmark_frame_init(MozBookmarkFrame *frame)
{
  /* call our superclass's init method first. */
  moz_frame_init(MOZ_FRAME(frame),
                 menubar_info,
                 NULL);

  /* then do our stuff */
  moz_tagged_set_type(MOZ_TAGGED(frame),
                      MOZ_TAG_BOOKMARK_FRAME);
}

void
moz_bookmark_frame_deinit(MozBookmarkFrame *frame)
{
  printf("moz_bookmark_frame_deinit (empty)\n");
}

/* our one bookmark frame. */
static MozBookmarkFrame* singleton = NULL;

MozBookmarkFrame*
moz_bookmark_frame_create()
{
  if (!singleton)
    {
      MozBookmarkView *view;
      
      singleton = XP_NEW_ZAP(MozBookmarkFrame);

      moz_bookmark_frame_init(singleton);
      
      MOZ_FRAME(singleton)->context->type = MWContextBookmarks;
      
      gtk_widget_realize(MOZ_COMPONENT(singleton)->base_widget);
      
      view = moz_bookmark_view_create(MOZ_FRAME(singleton), MOZ_FRAME(singleton)->context);
      
      MOZ_FRAME(singleton)->top_view = MOZ_VIEW(view);
      
      gtk_widget_show(MOZ_COMPONENT(view)->base_widget);
      
      moz_frame_set_viewarea(MOZ_FRAME(singleton),
                             MOZ_COMPONENT(view)->base_widget);
      
      gtk_widget_set_usize(MOZ_COMPONENT(singleton)->base_widget,
                           300, 400); /* XXX save off the default bookmark window size. */
    }

  return singleton;
}
  
