/*
  g-frame.c -- frames.
  Created: Chris Toshok <toshok@hungry.com>, 9-Apr-98.
*/

#include "xp.h"
#include "g-frame.h"
#include "moz-statusbar.h"
#include "gnomefe.h"

void
moz_frame_init(MozFrame *frame,
	       GnomeUIInfo *menubar_info,
	       GnomeUIInfo *toolbar_info)
{
  GtkWidget *window;
  MWContext *context_for_frame;

  /* call our superclass's init */
  moz_component_init(MOZ_COMPONENT(frame));

  /* then do our stuff. */
  moz_tagged_set_type(MOZ_TAGGED(frame),
		      MOZ_TAG_FRAME);

  window = gnome_app_new("gnuzilla", "GnuZilla");

  if (menubar_info)
    gnome_app_create_menus_with_data(GNOME_APP(window),
				     menubar_info,
				     frame);

  if (toolbar_info)
    gnome_app_create_toolbar_with_data(GNOME_APP(window),
				       toolbar_info,
				       frame);
  
  gtk_widget_realize(window);

  frame->vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(frame->vbox);

  frame->statusbar = moz_statusbar_new();
  gtk_widget_show(frame->statusbar);

  gnome_app_set_contents(GNOME_APP(window),
			 frame->vbox);

  moz_component_set_basewidget(MOZ_COMPONENT(frame),
			       window);

  /* now that we've done the widget level stuff, create
     our new MWContext */
  context_for_frame = GNOMEFE_CreateMWContext();

  XP_AddContextToList (context_for_frame);
  /* if (has html display) */
  SHIST_InitSession (context_for_frame);

  context_for_frame->fe.data->frame_or_view = frame;
  frame->context = context_for_frame;
}

void
moz_frame_deinit(MozFrame *frame)
{
  /* do our stuff first */

  /* call our superclass's deinit */
  moz_component_deinit(MOZ_COMPONENT(frame));
}

void
moz_frame_show(MozFrame *frame)
{
  moz_component_show(MOZ_COMPONENT(frame));

  /* now do frame specific stuff */
}

void
moz_frame_raise(MozFrame *frame)
{
  /* XXXX do whatever needs doing to raise a window. */
}

MozView*
moz_frame_get_top_view(MozFrame *frame)
{
  return frame->top_view;
}

MozView*
moz_frame_get_focus_view(MozFrame *frame)
{
  return frame->focus_view;
}

MWContext*
moz_frame_get_context(MozFrame *frame)
{
  return frame->context;
}

void
moz_frame_set_title(MozFrame *frame,
		    char *title)
{
  char buf[1024];
  char buf2[1024];

  PR_snprintf(buf, 1024, "Gnuzilla : %s", title);

  PR_snprintf(buf2, 1024, buf, XP_AppCodeName);

  printf ("moz_frame_set_title\n");
  gtk_window_set_title(GTK_WINDOW(moz_component_get_basewidget(MOZ_COMPONENT(frame))),
		       buf2);
}

void
moz_frame_set_status(MozFrame *frame,
		     char *status)
{
  moz_statusbar_set_label(MOZ_STATUSBAR(frame->statusbar), status);
}

void
moz_frame_set_percent(MozFrame *frame,
		      int32 percent)
{
  moz_statusbar_set_percentage(MOZ_STATUSBAR(frame->statusbar), percent);
}

void
moz_frame_set_viewarea(MozFrame *frame,
		       GtkWidget *viewarea)
{
  frame->viewarea = viewarea;

  gtk_box_pack_start(GTK_BOX(frame->vbox), frame->viewarea, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(frame->vbox), frame->statusbar, FALSE, FALSE, 0);
}
