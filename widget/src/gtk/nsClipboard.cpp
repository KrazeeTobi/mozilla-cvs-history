/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsClipboard.h"

#include "nsCOMPtr.h"

#include "nsISupportsArray.h"
#include "nsIClipboardOwner.h"
#include "nsITransferable.h"   // kTextMime

#include "nsIWidget.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsWidgetsCID.h"

#include "nsVoidArray.h"

// XXX: This must go away when nsAutoCString moves out of nsFileSpec.h
#include "nsFileSpec.h" // for nsAutoCString()



// The class statics:
GtkWidget* nsClipboard::sWidget = 0;

NS_IMPL_ADDREF_INHERITED(nsClipboard, nsBaseClipboard)
NS_IMPL_RELEASE_INHERITED(nsClipboard, nsBaseClipboard)


#if defined(DEBUG_akkana) || defined(DEBUG_mcafee) || defined(DEBUG_pavlov)
#define DEBUG_CLIPBOARD
#endif
 
enum {
  TARGET_NONE,
  TARGET_TEXT_PLAIN,
  TARGET_TEXT_XIF,
  TARGET_TEXT_UNICODE,
  TARGET_TEXT_HTML,
  TARGET_AOLMAIL,
  TARGET_IMAGE_PNG,
  TARGET_IMAGE_JPEG,
  TARGET_IMAGE_GIF,
  // compatibility types
  TARGET_NS4_HTML_NETSCAPE,
  TARGET_NS4_HTML,
  TARGET_UNKNOWN,
  TARGET_LAST
};

static GdkAtom sSelTypes[TARGET_LAST];

//-------------------------------------------------------------------------
//
// nsClipboard constructor
//
//-------------------------------------------------------------------------
nsClipboard::nsClipboard() : nsBaseClipboard()
{
#ifdef DEBUG_CLIPBOARD
  g_print("nsClipboard::nsClipboard()\n");
#endif /* DEBUG_CLIPBOARD */

  //NS_INIT_REFCNT();
  mIgnoreEmptyNotification = PR_FALSE;
  mClipboardOwner = nsnull;
  mTransferable   = nsnull;
  mSelectionData.data = nsnull;
  mSelectionData.length = 0;

  // initialize the widget, etc we're binding to
  Init();
}

//-------------------------------------------------------------------------
//
// nsClipboard destructor
//
//-------------------------------------------------------------------------
nsClipboard::~nsClipboard()
{
#ifdef DEBUG_CLIPBOARD
  printf("nsClipboard::~nsClipboard()\n");  
#endif /* DEBUG_CLIPBOARD */

  // Remove all our event handlers:
  if (sWidget &&
      (gdk_selection_owner_get(GDK_SELECTION_PRIMARY) == sWidget->window))
    gtk_selection_remove_all(sWidget);

  // free the selection data, if any
  if (mSelectionData.data != nsnull)
    g_free(mSelectionData.data);

  nsClipboard *cb = (nsClipboard*)gtk_object_get_data(GTK_OBJECT(sWidget), "cb");
  if (cb != nsnull)
  {
    NS_RELEASE(cb);
    gtk_object_remove_data(GTK_OBJECT(sWidget), "cb");
  }

  if (sWidget)
  {
    gtk_widget_destroy(sWidget);
    sWidget = nsnull;
  }
}

/**
 * @param aIID The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. NS_ISUPPORTS_IID)
 * 
*/ 
nsresult nsClipboard::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }

  nsresult rv = NS_NOINTERFACE;

  if (aIID.Equals(nsIClipboard::GetIID())) {
    *aInstancePtr = (void*) ((nsIClipboard*)this);
    NS_ADDREF_THIS();
    return NS_OK;
  }

  return rv;
}


// 
// GTK Weirdness!
// This is here in the hope of being able to call
//  gtk_selection_add_targets(w, GDK_SELECTION_PRIMARY,
//                            targets,
//                            1);
// instead of
//   gtk_selection_add_target(sWidget, 
//                            GDK_SELECTION_PRIMARY,
//                            GDK_SELECTION_TYPE_STRING,
//                            GDK_SELECTION_TYPE_STRING);
// but it turns out that this changes the whole gtk selection model;
// when calling add_targets copy uses selection_clear_event and the
// data structure needs to be filled in in a way that we haven't
// figured out; when using add_target copy uses selection_get and
// the data structure is already filled in as much as it needs to be.
// Some gtk internals wizard will need to solve this mystery before
// we can use add_targets().
//static GtkTargetEntry targets[] = {
//  { "strings n stuff", GDK_SELECTION_TYPE_STRING, GDK_SELECTION_TYPE_STRING }
//};
//

//-------------------------------------------------------------------------
void nsClipboard::Init(void)
{
#ifdef DEBUG_CLIPBOARD
  g_print("nsClipboard::Init\n");
#endif

  sSelTypes[TARGET_NONE]          = GDK_NONE;
  sSelTypes[TARGET_TEXT_PLAIN]    = gdk_atom_intern(kTextMime, FALSE);
  sSelTypes[TARGET_TEXT_XIF]      = gdk_atom_intern(kXIFMime, FALSE);
  sSelTypes[TARGET_TEXT_UNICODE]  = gdk_atom_intern(kUnicodeMime, FALSE);
  sSelTypes[TARGET_TEXT_HTML]     = gdk_atom_intern(kHTMLMime, FALSE);
  sSelTypes[TARGET_AOLMAIL]       = gdk_atom_intern(kAOLMailMime, FALSE);
  sSelTypes[TARGET_IMAGE_PNG]     = gdk_atom_intern(kPNGImageMime, FALSE);
  sSelTypes[TARGET_IMAGE_JPEG]    = gdk_atom_intern(kJPEGImageMime, FALSE);
  sSelTypes[TARGET_IMAGE_GIF]     = gdk_atom_intern(kGIFImageMime, FALSE);
  // compatibility with other apps
  sSelTypes[TARGET_NS4_HTML_NETSCAPE] = gdk_atom_intern("NETSCAPE_HTML", FALSE);
  sSelTypes[TARGET_NS4_HTML]          = gdk_atom_intern("HTML", FALSE);


  // create invisible widget to use for the clipboard
  sWidget = gtk_invisible_new();

  // add the clipboard pointer to the widget so we can get it.
  gtk_object_set_data(GTK_OBJECT(sWidget), "cb", this);

  NS_ADDREF_THIS();

  // Handle selection requests if we called gtk_selection_add_target:
  gtk_signal_connect(GTK_OBJECT(sWidget), "selection_get",
                     GTK_SIGNAL_FUNC(nsClipboard::SelectionGetCB),
                     nsnull);

  // When someone else takes the selection away:
  gtk_signal_connect(GTK_OBJECT(sWidget), "selection_clear_event",
                     GTK_SIGNAL_FUNC(nsClipboard::SelectionClearCB),
                     nsnull);

  // Set up the paste handler:
  gtk_signal_connect(GTK_OBJECT(sWidget), "selection_received",
                     GTK_SIGNAL_FUNC(nsClipboard::SelectionReceivedCB),
                     nsnull);


#if 0
  // Handle selection requests if we called gtk_selection_add_targets:
  gtk_signal_connect(GTK_OBJECT(sWidget), "selection_request_event",
                     GTK_SIGNAL_FUNC(nsClipboard::SelectionRequestCB),
                     nsnull);
  
  // Watch this, experimenting with Gtk :-)
  gtk_signal_connect(GTK_OBJECT(sWidget), "selection_notify_event",
                     GTK_SIGNAL_FUNC(nsClipboard::SelectionNotifyCB),
                     nsnull);
#endif
}


//-------------------------------------------------------------------------
NS_IMETHODIMP nsClipboard::SetNativeClipboardData()
{
  mIgnoreEmptyNotification = PR_TRUE;

#ifdef DEBUG_CLIPBOARD
  printf("  nsClipboard::SetNativeClipboardData()\n");
#endif /* DEBUG_CLIPBOARD */

  // make sure we have a good transferable
  if (nsnull == mTransferable) {
    printf("nsClipboard::SetNativeClipboardData(): no transferable!\n");
    return NS_ERROR_FAILURE;
  }

  // are we already the owner?
  if (gdk_selection_owner_get(GDK_SELECTION_PRIMARY) == sWidget->window)
  {
    // if so, clear all the targets
    gtk_selection_remove_all(sWidget);
  }
#if 0
  else
  {
#endif
    // we arn't already the owner, so we will become it
    gint have_selection = gtk_selection_owner_set(sWidget,
                                                  GDK_SELECTION_PRIMARY,
                                                  GDK_CURRENT_TIME);
    if (have_selection == 0)
      return NS_ERROR_FAILURE;
#if 0
  }
#endif

  nsString *df;
  int i = 0;
  nsVoidArray *dfList;
  // find out what types this data can be
  mTransferable->FlavorsTransferableCanExport(&dfList);

  int cnt = dfList->Count();

  for (i=0;i<cnt;i++)
  {
    df = (nsString *)dfList->ElementAt(i);
    if (nsnull != df) {
      gint format = GetFormat(*df);

      // add these types as selection targets
      RegisterFormat(format);

    }
  }

  mIgnoreEmptyNotification = PR_FALSE;

  return NS_OK;
}

gint nsClipboard::GetFormat(const nsString &aMimeStr)
{
  gint type = TARGET_NONE;

#ifdef DEBUG_CLIPBOARD
  char *foo = aMimeStr.ToNewCString();
  g_print("  nsClipboard::GetFormat(%s)\n", foo);
  delete [] foo;
#endif
  if (aMimeStr.Equals(kTextMime)) {
    type = TARGET_TEXT_PLAIN;
  } else if (aMimeStr.Equals("STRING")) {
    type = TARGET_TEXT_PLAIN;
  } else if (aMimeStr.Equals(kXIFMime)) {
    type = TARGET_TEXT_XIF;
  } else if (aMimeStr.Equals(kUnicodeMime)) {
    type = TARGET_TEXT_UNICODE;
  } else if (aMimeStr.Equals(kHTMLMime)) {
    type = TARGET_TEXT_HTML;
  } else if (aMimeStr.Equals("HTML")) {
    type = TARGET_TEXT_HTML;
  } else if (aMimeStr.Equals("NETSCAPE_HTML")) {
    type = TARGET_TEXT_HTML;
  } else if (aMimeStr.Equals(kAOLMailMime)) {
    type = TARGET_AOLMAIL;
  } else if (aMimeStr.Equals(kPNGImageMime)) {
    type = TARGET_IMAGE_PNG;
  } else if (aMimeStr.Equals(kJPEGImageMime)) {
    type = TARGET_IMAGE_JPEG;
  } else if (aMimeStr.Equals(kGIFImageMime)) {
    type = TARGET_IMAGE_GIF;
  }

#ifdef WE_DO_DND
  else if (aMimeStr.Equals(kDropFilesMime)) {
    format = CF_HDROP;
  } else {
    char * str = aMimeStr.ToNewCString();
    format = ::RegisterClipboardFormat(str);
    delete[] str;
  }
#endif
  return type;
}

void nsClipboard::RegisterFormat(gint format)
{
#ifdef DEBUG_CLIPBOARD
  g_print("  nsClipboard::RegisterFormat(%s)\n", gdk_atom_name(sSelTypes[format]));
#endif

  /* when doing the selection_add_target, each case should have the same last parameter
     which matches the case match */
  switch(format)
  {
  case TARGET_TEXT_PLAIN:
    // text/plain (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    // STRING (what X uses)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             GDK_SELECTION_TYPE_STRING,
                             format);
    break;

  case TARGET_TEXT_XIF:
    // text/xif (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;

  case TARGET_TEXT_UNICODE:
    // text/unicode (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;

  case TARGET_TEXT_HTML:
    // text/html (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    // NETSCAPE_HTML (used in NS4.x)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[TARGET_NS4_HTML_NETSCAPE],
                             format);
    // HTML (used in NS4.x)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[TARGET_NS4_HTML],
                             format);

    break;

  case TARGET_AOLMAIL:
    // text/aolmail (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;
  case TARGET_IMAGE_PNG:
    // image/png (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;
  case TARGET_IMAGE_JPEG:
    // image/jpeg (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;
  case TARGET_IMAGE_GIF:
    // image/gif (default)
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
    break;
  default:
    // if we don't match something above, then just add it like its something we know about...
    gtk_selection_add_target(sWidget, 
                             GDK_SELECTION_PRIMARY,
                             sSelTypes[format],
                             format);
  }
}

PRBool nsClipboard::DoRealConvert(GdkAtom type)
{
#ifdef DEBUG_CLIPBOARD
  g_print("    nsClipboard::DoRealConvert(%i)\n    {\n", type);
#endif
  int e = 0;
  // Set a flag saying that we're blocking waiting for the callback:
  mBlocking = PR_TRUE;

  //
  // ask X what kind of data we can get
  //
#ifdef DEBUG_CLIPBOARD
  g_print("     Doing real conversion of atom type '%s'\n", gdk_atom_name(type));
#endif
  gtk_selection_convert(sWidget,
                        GDK_SELECTION_PRIMARY,
                        type,
                        GDK_CURRENT_TIME);

  // Now we need to wait until the callback comes in ...
  // i is in case we get a runaway (yuck).
#ifdef DEBUG_CLIPBOARD
  printf("      Waiting for the callback... mBlocking = %d\n", mBlocking);
#endif /* DEBUG_CLIPBOARD */
  for (e=0; mBlocking == PR_TRUE && e < 1000; ++e)
  {
    gtk_main_iteration_do(PR_TRUE);
  }

#ifdef DEBUG_CLIPBOARD
  g_print("    }\n");
#endif

  if (mSelectionData.length > 0)
    return PR_TRUE;

  return PR_FALSE;
}

/* return PR_TRUE if we have converted or PR_FALSE if we havn't and need to keep being called */
PRBool nsClipboard::DoConvert(gint format)
{
#ifdef DEBUG_CLIPBOARD
  g_print("  nsClipboard::DoConvert(%s)\n", gdk_atom_name(sSelTypes[format]));
#endif

  /* when doing the selection_add_target, each case should have the same last parameter
     which matches the case match */
  PRBool r = PR_FALSE;

  switch(format)
  {
  case TARGET_TEXT_PLAIN:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    r = DoRealConvert(GDK_SELECTION_TYPE_STRING);
    if (r) return r;
    break;

  case TARGET_TEXT_XIF:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  case TARGET_TEXT_UNICODE:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  case TARGET_TEXT_HTML:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    r = DoRealConvert(sSelTypes[TARGET_NS4_HTML_NETSCAPE]);
    if (r) return r;
    r = DoRealConvert(sSelTypes[TARGET_NS4_HTML]);
    if (r) return r;
    break;

  case TARGET_AOLMAIL:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  case TARGET_IMAGE_PNG:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  case TARGET_IMAGE_JPEG:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  case TARGET_IMAGE_GIF:
    r = DoRealConvert(sSelTypes[format]);
    if (r) return r;
    break;

  default:
    g_print("DoConvert called with bogus format\n");
  }
  return r;
}

//-------------------------------------------------------------------------
//
// The blocking Paste routine
//
//-------------------------------------------------------------------------
NS_IMETHODIMP
nsClipboard::GetNativeClipboardData(nsITransferable * aTransferable)
{
  nsString *df;
  int i = 0;

#ifdef DEBUG_CLIPBOARD
  printf("nsClipboard::GetNativeClipboardData()\n");
#endif /* DEBUG_CLIPBOARD */

  // make sure we have a good transferable
  if (nsnull == aTransferable) {
    printf("  GetNativeClipboardData: Transferable is null!\n");
    return NS_ERROR_FAILURE;
  }

  // Get the transferable list of data flavors
  nsVoidArray *dfList;
  aTransferable->GetTransferDataFlavors(&dfList);

  // Walk through flavors and see which flavor matches the one being pasted:
  int cnt = dfList->Count();

  for (i=0;i<cnt;i++) {
    df = (nsString *)dfList->ElementAt(i);
    if (nsnull != df) {
      gint format = GetFormat(*df);

      if (DoConvert(format))
        break;
    }
  }


#ifdef DEBUG_CLIPBOARD
  printf("  Got the callback: '%s', %d\n",
         mSelectionData.data, mSelectionData.length);
#endif /* DEBUG_CLIPBOARD */

  // We're back from the callback, no longer blocking:
  mBlocking = PR_FALSE;

  // 
  // Now we have data in mSelectionData.data.
  // We just have to copy it to the transferable.
  // 

  
  nsString *name = new nsString((const char*)gdk_atom_name(mSelectionData.type));
  
  int format = GetFormat(*name);

  df->SetString((const char*)gdk_atom_name(sSelTypes[format]));
  aTransferable->SetTransferData(df,
                                 mSelectionData.data,
                                 mSelectionData.length);

  delete name;

  // transferable is now copying the data, so we can free it.
  //  g_free(mSelectionData.data);
  mSelectionData.data = nsnull;
  mSelectionData.length = 0;

  return NS_OK;
}

/**
 * Called when the data from a paste comes in (recieved from gdk_selection_convert)
 *
 * @param  aWidget the widget
 * @param  aSelectionData gtk selection stuff
 * @param  aTime time the selection was requested
 */
void
nsClipboard::SelectionReceivedCB (GtkWidget        *aWidget,
                                  GtkSelectionData *aSelectionData,
                                  guint             aTime)
{
#ifdef DEBUG_CLIPBOARD
  printf("      nsClipboard::SelectionReceivedCB\n      {\n");
#endif /* DEBUG_CLIPBOARD */
  nsClipboard *cb =(nsClipboard *)gtk_object_get_data(GTK_OBJECT(aWidget),
                                                      "cb");
  if (!cb)
  {
    g_print("no clipboard found.. this is bad.\n");
    return;
  }
  cb->SelectionReceiver(aWidget, aSelectionData);
#ifdef DEBUG_CLIPBOARD
  g_print("      }\n");
#endif
}


/**
 * local method (called from nsClipboard::SelectionReceivedCB)
 *
 * @param  aWidget the widget
 * @param  aSelectionData gtk selection stuff
 */
void
nsClipboard::SelectionReceiver (GtkWidget *aWidget,
                                GtkSelectionData *aSD)
{
  gint type;

  mBlocking = PR_FALSE;

  if (aSD->length < 0)
  {
    printf("        Error retrieving selection: length was %d\n",
           aSD->length);
    return;
  }

  type = TARGET_NONE;
  for (int i=0; i < TARGET_LAST; i++)
  {
    if (sSelTypes[i] == aSD->type)
    {
      type = i;
      break;
    }
  }

  switch (type)
  {
  case GDK_TARGET_STRING:
  case TARGET_TEXT_PLAIN:
  case TARGET_TEXT_XIF:
  case TARGET_TEXT_UNICODE:
  case TARGET_TEXT_HTML:
#ifdef DEBUG_CLIPBOARD
    g_print("        Copying mSelectionData pointer -- ");
#endif
    mSelectionData = *aSD;
    mSelectionData.data = g_new(guchar, aSD->length + 1);
#ifdef DEBUG_CLIPBOARD
    g_print("        Data = %s\n    Length = %i\n", aSD->data, aSD->length);
#endif
    memcpy(mSelectionData.data,
           aSD->data,
           aSD->length);
    // Null terminate in case anyone cares,
    // and so we can print the string for debugging:
    mSelectionData.data[aSD->length] = '\0';
    mSelectionData.length = aSD->length;
    return;

  default:
    mSelectionData = *aSD;
    mSelectionData.data = g_new(guchar, aSD->length + 1);
    memcpy(mSelectionData.data,
           aSD->data,
           aSD->length);
    mSelectionData.length = aSD->length;
    printf("        Can't convert type %s (%ld) to string\n",
           gdk_atom_name (aSD->type), aSD->type);
    return;
  }
}



/**
 * Some platforms support deferred notification for putting data on the clipboard
 * This method forces the data onto the clipboard in its various formats
 * This may be used if the application going away.
 *
 * @result NS_OK if successful.
 */
NS_IMETHODIMP nsClipboard::ForceDataToClipboard()
{
#ifdef DEBUG_CLIPBOARD
  printf("  nsClipboard::ForceDataToClipboard()\n");
#endif /* DEBUG_CLIPBOARD */

  // make sure we have a good transferable
  if (nsnull == mTransferable) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


/**
 * This is the callback which is called when another app
 * requests the selection.
 *
 * @param  widget The widget
 * @param  aSelectionData Selection data
 * @param  info Value passed in from the callback init
 * @param  time Time when the selection request came in
 */
void nsClipboard::SelectionGetCB(GtkWidget        *widget,
                                 GtkSelectionData *aSelectionData,
                                 guint            aInfo,
                                 guint            aTime)
{ 
#ifdef DEBUG_CLIPBOARD
  printf("nsClipboard::SelectionGetCB\n"); 
#endif /* DEBUG_CLIPBOARD */

  nsClipboard *cb = (nsClipboard *)gtk_object_get_data(GTK_OBJECT(widget),
                                                       "cb");

  void     *clipboardData;
  PRUint32 dataLength;
  nsresult rv;
  GdkAtom type = GDK_NONE;

  // Make sure we have a transferable:
  if (!cb->mTransferable) {
    printf("Clipboard has no transferable!\n");
    return;
  }

#ifdef DEBUG_CLIPBOARD
  g_print("  aInfo == %d -", aInfo);
#endif

  nsString dataFlavor;

  switch(aInfo)
    {
    case GDK_TARGET_STRING:
      type = GDK_TARGET_STRING;
      dataFlavor = kTextMime;
      break;
    case TARGET_TEXT_PLAIN:
      type = sSelTypes[aInfo];
      dataFlavor = kTextMime;
      break;
    case TARGET_TEXT_XIF:
      type = sSelTypes[aInfo];
      dataFlavor = kXIFMime;
      break;
    case TARGET_TEXT_UNICODE:
      type = sSelTypes[aInfo];
      dataFlavor = kUnicodeMime;
      break;
    case TARGET_TEXT_HTML:
      type = sSelTypes[aInfo];
      dataFlavor = kHTMLMime;
      break;
    case TARGET_AOLMAIL:
      type = sSelTypes[aInfo];
      dataFlavor = kAOLMailMime;
      break;
    case TARGET_IMAGE_PNG:
      type = sSelTypes[aInfo];
      dataFlavor = kPNGImageMime;
      break;
    case TARGET_IMAGE_JPEG:
      type = sSelTypes[aInfo];
      dataFlavor = kJPEGImageMime;
      break;
    case TARGET_IMAGE_GIF:
      type = sSelTypes[aInfo];
      dataFlavor = kGIFImageMime;
      break;
    default:
      {
        /* handle outside things */
        if (aInfo == sSelTypes[TARGET_NS4_HTML] ||
            aInfo == sSelTypes[TARGET_NS4_HTML_NETSCAPE])
        {
          type = aInfo;
          dataFlavor = kHTMLMime;
          break;
        }
      }
    }
#ifdef DEBUG_CLIPBOARD
  g_print("- aInfo is for %s\n", gdk_atom_name(type));
#endif

  // Get data out of transferable.
  rv = cb->mTransferable->GetTransferData(&dataFlavor, 
                                          &clipboardData,
                                          &dataLength);

  if (NS_SUCCEEDED(rv) && clipboardData && dataLength > 0) {
    gtk_selection_data_set(aSelectionData,
                           type, 8,
                           (unsigned char *)clipboardData,
                           dataLength);
  }
  else
    printf("Transferable didn't support the data flavor\n");
}


/**
 * Called when another app requests selection ownership
 *
 * @param  aWidget the widget
 * @param  aEvent the GdkEvent for the selection
 * @param  aData value passed in from the callback init
 */
void nsClipboard::SelectionClearCB(GtkWidget *aWidget,
                                   GdkEventSelection *aEvent,
                                   gpointer aData)
{
#ifdef DEBUG_CLIPBOARD
  printf("  nsClipboard::SelectionClearCB\n");
#endif /* DEBUG_CLIPBOARD */

  nsClipboard *cb = (nsClipboard *)gtk_object_get_data(GTK_OBJECT(aWidget),
                                                       "cb");

  cb->EmptyClipboard();
}


/**
 * The routine called when another app asks for the content of the selection
 *
 * @param  aWidget the widget
 * @param  aSelectionData gtk selection stuff
 * @param  aData value passed in from the callback init
 */
void
nsClipboard::SelectionRequestCB (GtkWidget *aWidget,
                                 GtkSelectionData *aSelectionData,
                                 gpointer aData)
{
#ifdef DEBUG_CLIPBOARD
  printf("  nsClipboard::SelectionRequestCB\n");
#endif /* DEBUG_CLIPBOARD */
}

/**
 * ...
 *
 * @param  aWidget the widget
 * @param  aSelectionData gtk selection stuff
 * @param  aData value passed in from the callback init
 */
void
nsClipboard::SelectionNotifyCB (GtkWidget *aWidget,
                                GtkSelectionData *aSelectionData,
                                gpointer aData)
{
#ifdef DEBUG_CLIPBOARD
  printf("  nsClipboard::SelectionNotifyCB\n");
#endif /* DEBUG_CLIPBOARD */
}
