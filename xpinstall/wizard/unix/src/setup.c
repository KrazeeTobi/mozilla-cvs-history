/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is the Mozilla browser.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Stuart Parmenter <pavlov@netscape.com>
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include "logo.xpm"

enum {
  LICENSE,
  DESTINATION,
  COMPONENTS
};

static GtkWidget *nextButton;
static GtkWidget *prevButton;

static void next_cb (GtkWidget *, gpointer);
static void prev_cb (GtkWidget *, gpointer);

static void
prev_cb (GtkWidget * widget, gpointer data)
{
  GtkWidget *nb = GTK_WIDGET(data);

  gtk_notebook_prev_page(GTK_NOTEBOOK(nb));
  gtk_label_set(GTK_LABEL(GTK_BIN (nextButton)->child), "Next");
  /* are we on the first page? */
  if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nb)) == 0)
    gtk_widget_set_sensitive(prevButton, FALSE);
    
}

static void
next_cb (GtkWidget * widget, gpointer data)
{
  GtkWidget *nb = GTK_WIDGET(data);

  gtk_notebook_next_page(GTK_NOTEBOOK(nb));

  gtk_widget_set_sensitive(prevButton, TRUE);

  switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(nb)))
  {
  case COMPONENTS:
    gtk_label_set(GTK_LABEL(GTK_BIN (nextButton)->child), "Finish");
    break;
  }
}



GtkWidget *create_license_page()
{
  GtkWidget *text;
  char *foo = "hear mozilla roar!";
  text = gtk_text_new(NULL, NULL);
  gtk_widget_show(text);
  gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, foo, strlen(foo));
  return text;
}

GtkWidget *create_folder_page()
{
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *button;

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);

  frame = gtk_frame_new("");
  gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
  gtk_widget_show(frame);

  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  frame = gtk_frame_new("Installation Location");
  gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
  gtk_widget_show(frame);
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(frame), hbox);

  label = gtk_label_new("in folder /foo");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

  button = gtk_button_new_with_label("Select folder...");
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *create_components_page()
{
  GtkWidget *vbox;
  GtkWidget *clist;
  GtkWidget *label;
  GtkWidget *frame;

  gchar foo[256];
  gchar *text[2];
  text[0] = "x";
  text[1] = foo;

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);

  clist =  gtk_clist_new(2);
  gtk_widget_show(clist);

  label = gtk_label_new("Please select the components you wish to install:");
  gtk_widget_show(label);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), clist, FALSE, FALSE, 0);

  gtk_clist_freeze(GTK_CLIST(clist));
  sprintf(text[1], "mozilla");
  gtk_clist_append(GTK_CLIST(clist), text);
  sprintf(text[1], "blah blah");
  gtk_clist_append(GTK_CLIST(clist), text);
  sprintf(text[1], "blah blah blah");
  gtk_clist_append(GTK_CLIST(clist), text);
  gtk_clist_thaw(GTK_CLIST(clist));


  frame = gtk_frame_new("Description");
  gtk_widget_show(frame);
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
  label = gtk_label_new("apprunner, etc.");
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(frame), label);

  return vbox;
}

void create_main_window()
{
  GtkWidget *win;
  GtkWidget *hbox, *vbox;
  GtkWidget *pixmap;
  GdkPixmap *gpix;
  GdkBitmap *gmask;
  GtkWidget *notebook;
  GtkWidget *label;
  GtkWidget *bbox;

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize(win, 543, 319);
  gtk_container_set_border_width(GTK_CONTAINER(win), 5);
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_widget_show(vbox);

  gtk_container_add(GTK_CONTAINER(win), vbox);

  gpix = gdk_pixmap_create_from_xpm_d(NULL /* GdkWindow  *window */ ,
                                      &gmask,
                                      NULL,
                                      logo_xpm);


  pixmap = gtk_pixmap_new(gpix, gmask);
  gtk_misc_set_alignment(GTK_MISC(pixmap), 0.5, 0.0);
  gtk_widget_show(pixmap);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(vbox), hbox);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap, FALSE, FALSE, 0);

  notebook = gtk_notebook_new();
  gtk_widget_show(notebook);
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_show_border(GTK_NOTEBOOK (notebook), FALSE);

  /* add notebook pages */

  label = gtk_label_new ("welcome");
  gtk_notebook_append_page(GTK_NOTEBOOK (notebook),
                           create_license_page (),
                           label);

  label = gtk_label_new("select destination");
  gtk_notebook_append_page(GTK_NOTEBOOK (notebook),
                           create_folder_page (),
                           label);

  label = gtk_label_new("select components");
  gtk_notebook_append_page(GTK_NOTEBOOK (notebook),
                           create_components_page (),
                           label);

  gtk_box_pack_start(GTK_BOX(hbox), notebook, TRUE, TRUE, 0);


  /* create navigation buttons */
  prevButton = gtk_button_new_with_label("Previous");
  gtk_signal_connect(GTK_OBJECT (prevButton),
                     "clicked",
                     (GtkSignalFunc) prev_cb,
                     notebook);
  gtk_widget_set_sensitive (prevButton, FALSE);
  gtk_widget_show(prevButton);

  nextButton = gtk_button_new_with_label("Next");
  gtk_signal_connect(GTK_OBJECT (nextButton),
                     "clicked",
                     (GtkSignalFunc) next_cb,
                     notebook);
  gtk_widget_show(nextButton);

  bbox = gtk_hbutton_box_new();
  gtk_widget_show(bbox);
  gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(bbox), prevButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(bbox), nextButton, FALSE, FALSE, 0);


  gtk_widget_show(win);
}
