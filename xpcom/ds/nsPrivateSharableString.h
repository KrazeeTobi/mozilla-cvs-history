/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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
 * The Original Code is Mozilla strings.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Scott Collins <scc@mozilla.org> (original author)
 *
 */

#ifndef nsPrivateSharableString_h___
#define nsPrivateSharableString_h___

#include "nsBufferHandle.h"

  /**
   *
   */
template <class CharT>
class nsPrivateSharableString
  {
    public:
      virtual const nsBufferHandle<CharT>*        GetBufferHandle() const;
      virtual const nsSharedBufferHandle<CharT>*  GetSharedBufferHandle() const;
  };

template <class CharT>
nsBufferHandle<CharT>*
nsPrivateSharableString<CharT>::GetBufferHandle() const
  {
    return GetSharedBufferHandle();
  }

template <class CharT>
nsBufferHandle<CharT>*
nsPrivateSharableString<CharT>::GetSharedBufferHandle() const
  {
    return 0;
  }

#endif // !defined(nsPrivateSharableString_h___)
