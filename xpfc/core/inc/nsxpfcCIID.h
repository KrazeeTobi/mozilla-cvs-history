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

#ifndef nsxpfcCIID_h__
#define nsxpfcCIID_h__

#include "nsISupports.h"
#include "nsIFactory.h"
#include "nsRepository.h"

// c38e93c0-386e-11d2-9249-00805f8a7ab6
#define NS_XPFC_COMMAND_SERVER_CID      \
 { 0xc38e93c0, 0x386e, 0x11d2, \
   {0x92, 0x49, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

#define NS_XPFC_SHELL_INSTANCE_CID      \
 { 0x90487580, 0xfefe, 0x11d1, \
   {0xbe, 0xcd, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// 03dd8f40-3610-11d2-9248-00805f8a7ab6
#define NS_STREAM_MANAGER_CID      \
 { 0x03dd8f40, 0x3610, 0x11d2, \
   {0x92, 0x48, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// f7c349a0-4915-11d2-924a-00805f8a7ab6
#define NS_STREAM_OBJECT_CID      \
 { 0xf7c349a0, 0x4915, 0x11d2, \
   {0x92, 0x4a, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// cd5cd8b0-3b80-11d2-bee0-00805f8a8dbd
#define NS_XPFC_DIALOG_CID      \
 { 0xcd5cd8b0, 0x3b80, 0x11d2, \
   {0xbe, 0xe0, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// f156ec60-4cba-11d2-924a-00805f8a7ab6
#define NS_USER_CID      \
 { 0xf156ec60, 0x4cba, 0x11d2, \
   {0x92, 0x4a, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// 55395780-3b98-11d2-bee0-00805f8a8dbd
#define NS_XPFC_BUTTON_CID      \
 { 0x55395780, 0x3b98, 0x11d2, \
   {0xbe, 0xe0, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// 22070fb0-4a92-11d2-bee2-00805f8a8dbd
#define NS_XP_BUTTON_CID      \
 { 0x22070fb0, 0x4a92, 0x11d2, \
   {0xbe, 0xe2, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

//23432750-4a9c-11d2-bee2-00805f8a8dbd
#define NS_XP_ITEM_CID      \
 { 0x23432750, 0x4a9c, 0x11d2, \
   {0xbe, 0xe2, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// cd53e4b0-3b99-11d2-bee0-00805f8a8dbd
#define NS_XPFC_TEXTWIDGET_CID      \
 { 0xcd53e4b0, 0x3b99, 0x11d2, \
   {0xbe, 0xe0, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// dafa9a80-3b99-11d2-bee0-00805f8a8dbd
#define NS_XPFC_TABWIDGET_CID      \
 { 0xdafa9a80, 0x3b99, 0x11d2, \
   {0xbe, 0xe0, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// 5de8a290-35f9-11d2-9248-00805f8a7ab6
#define NS_XPFC_TOOLBAR_CID      \
 { 0x5de8a290, 0x35f9, 0x11d2, \
   {0x92, 0x48, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// 74222650-2d76-11d2-9246-00805f8a7ab6
#define NS_XPFCMENUBAR_CID      \
 { 0x74222650, 0x2d76, 0x11d2, \
   {0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// c78daa60-30a9-11d2-9247-00805f8a7ab6
#define NS_MENU_MANAGER_CID      \
 { 0xc78daa60, 0x30a9, 0x11d2, \
   {0x92, 0x47, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// 782ae760-35fb-11d2-9248-00805f8a7ab6
#define NS_XPFCTOOLBAR_MANAGER_CID      \
 { 0x782ae760, 0x35fb, 0x11d2, \
   {0x92, 0x48, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// 6c4e09a0-2f54-11d2-bede-00805f8a8dbd
#define NS_XPFCMENUCONTAINER_CID      \
 { 0x6c4e09a0, 0x2f54, 0x11d2, \
   {0xbe, 0xde, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd} }

// 7a01c6d0-2d76-11d2-9246-00805f8a7ab6
#define NS_XPFCMENUITEM_CID      \
 { 0x7a01c6d0, 0x2d76, 0x11d2, \
   {0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

#define NS_IAPPLICATIONSHELL_CID      \
 { 0x2293d960, 0xdeff, 0x11d1, \
   {0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// ce401b60-2e1e-11d2-9246-00805f8a7ab
#define NS_XPFCXMLCONTENTSINK_IID   \
{ 0xce401b60, 0x2e1e, 0x11d2, \
  {0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

// d422ca40-2e1e-11d2-9246-00805f8a7ab6
#define NS_IXPFCXML_DTD_IID   \
{ 0xd422ca40, 0x2e1e, 0x11d2, \
  {0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

//8c091510-eb84-11d1-9244-00805f8a7ab6
#define NS_VECTOR_CID \
{ 0x8c091510, 0xeb84, 0x11d1, \
{ 0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//24439be0-eb85-11d1-9244-00805f8a7ab6
#define NS_VECTOR_ITERATOR_CID \
{ 0x24439be0, 0xeb85, 0x11d1, \
{ 0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//f16fb1e0-27db-11d2-9246-00805f8a7ab6
#define NS_STACK_CID \
{ 0xf16fb1e0, 0x27db, 0x11d2, \
{ 0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//0156b2a0-f73d-11d1-9244-00805f8a7ab6
#define NS_LAYOUT_CID \
{ 0x0156b2a0, 0xf73d, 0x11d1, \
{ 0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//96562f50-fb29-11d1-becb-00805f8a8dbd
#define NS_BOXLAYOUT_CID \
{ 0x96562f50, 0xfb29, 0x11d1, \
{ 0xbe, 0xcb, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//f3dddca0-4b30-11d2-bee2-00805f8a8dbd
#define NS_LISTLAYOUT_CID \
{ 0xf3dddca0, 0x4b30, 0x11d2, \
{ 0xbe, 0xe2, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//0241c670-1f57-11d2-bed9-00805f8a8dbd
#define NS_XPFC_OBSERVERMANAGER_CID \
{ 0x0241c670, 0x1f57, 0x11d2, \
{ 0xbe, 0xd9, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//7a1e8ca0-1f53-11d2-bed9-00805f8a8dbd
#define NS_XPFC_OBSERVER_CID \
{ 0x7a1e8ca0, 0x1f53, 0x11d2, \
{ 0xbe, 0xd9, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//a4c7f8b0-1f55-11d2-bed9-00805f8a8dbd
#define NS_XPFC_SUBJECT_CID \
{ 0xa4c7f8b0, 0x1f55, 0x11d2, \
{ 0xbe, 0xd9, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//82613bd0-1f46-11d2-bed9-00805f8a8dbd
#define NS_XPFC_COMMAND_CID \
{ 0x82613bd0, 0x1f46, 0x11d2, \
{ 0xbe, 0xd9, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//2d093010-e9e7-11d1-9244-00805f8a7ab6
#define NS_XPFC_CANVAS_CID \
{ 0x2d093010, 0xe9e7, 0x11d1, \
{ 0x92, 0x44, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//0b4ff5b0-3dda-11d2-bee1-00805f8a8dbd
#define NS_XPFC_HTML_CANVAS_CID \
{ 0x0b4ff5b0, 0x3dda, 0x11d2, \
{ 0xbe, 0xe1, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//cc2e92e0-4a9f-11d2-bee2-00805f8a8dbd
#define NS_XP_FOLDER_CANVAS_CID \
{ 0xcc2e92e0, 0x4a9f, 0x11d2, \
{ 0xbe, 0xe2, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//92789710-28a6-11d2-9246-00805f8a7ab6
#define NS_XPFC_CANVASMANAGER_CID \
{ 0x92789710, 0x28a6, 0x11d2, \
{ 0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//8d2f7410-28c3-11d2-9246-00805f8a7ab6
#define NS_XPFC_METHODINVOKER_COMMAND_CID \
{ 0x8d2f7410, 0x28c3, 0x11d2, \
{ 0x92, 0x46, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//1eb182f0-62bb-11d2-bee8-00805f8a8dbd
#define NS_XPFC_NOTIFICATIONSTATE_COMMAND_CID \
{ 0x1eb182f0, 0x62bb, 0x11d2, \
{ 0xbe, 0xe8, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//b09314c0-62c2-11d2-bee8-00805f8a8dbd
#define NS_XPFC_MODELUPDATE_COMMAND_CID \
{ 0xb09314c0, 0x62c2, 0x11d2, \
{ 0xbe, 0xe8, 0x00, 0x80, 0x5f, 0x8a, 0x8d, 0xbd } }

//667b7530-33b5-11d2-9248-00805f8a7ab6
#define NS_XPFC_ACTION_COMMAND_CID \
{ 0x667b7530, 0x33b5, 0x11d2, \
{ 0x92, 0x48, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6 } }

//e72e08e0-6f78-11d2-8dbc-00805f8a7ab6
#define NS_SMTP_SERVICE_CID      \
 { 0xe72e08e0, 0x6f78, 0x11d2, \
   {0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

//57c616d0-6f90-11d2-8dbc-00805f8a7ab6
#define NS_MIME_SERVICE_CID      \
 { 0x57c616d0, 0x6f90, 0x11d2, \
   {0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

//1887e710-6f9a-11d2-8dbc-00805f8a7ab6
#define NS_MESSAGE_CID      \
 { 0x1887e710, 0x6f9a, 0x11d2, \
   {0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

//8afb9c70-7029-11d2-8dbc-00805f8a7ab6
#define NS_MIME_MESSAGE_CID      \
 { 0x8afb9c70, 0x7029, 0x11d2, \
   {0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }

//a30f09f0-703a-11d2-8dbc-00805f8a7ab6
#define NS_MIME_BODY_PART_CID      \
 { 0xa30f09f0, 0x703a, 0x11d2, \
   {0x8d, 0xbc, 0x00, 0x80, 0x5f, 0x8a, 0x7a, 0xb6} }


#endif

