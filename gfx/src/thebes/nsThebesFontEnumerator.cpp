/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * mozilla.org.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir@mozilla.com>
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

#include "nsThebesFontEnumerator.h"

#include "nsMemory.h"

#include "gfxPlatform.h"

NS_IMPL_ISUPPORTS1(nsThebesFontEnumerator, nsIFontEnumerator)

nsThebesFontEnumerator::nsThebesFontEnumerator()
{
}

NS_IMETHODIMP
nsThebesFontEnumerator::EnumerateAllFonts(PRUint32 *aCount,
                                          PRUnichar ***aResult)
{
    return EnumerateFonts (nsnull, nsnull, aCount, aResult);
}

NS_IMETHODIMP
nsThebesFontEnumerator::EnumerateFonts(const char *aLangGroup,
                                       const char *aGeneric,
                                       PRUint32 *aCount,
                                       PRUnichar ***aResult)
{
    NS_ENSURE_ARG_POINTER(aCount);
    NS_ENSURE_ARG_POINTER(aResult);

    nsStringArray fontList;
    nsresult rv;

    nsCAutoString langGroup;
    nsCAutoString generic;

    if (aLangGroup)
        langGroup.Assign(aLangGroup);
    else
        langGroup.SetIsVoid(PR_TRUE);

    if (aGeneric)
        generic.Assign(aGeneric);
    else
        generic.SetIsVoid(PR_TRUE);

    // XXX windows doesn't implement gfxPlatform yet
    if (gfxPlatform::GetPlatform())
        rv = gfxPlatform::GetPlatform()->GetFontList(langGroup, generic, fontList);
    else
        rv = NS_ERROR_NOT_IMPLEMENTED;

    if (NS_FAILED(rv)) {
        *aCount = 0;
        *aResult = nsnull;
        /* XXX in this case, do we want to return the CSS generics? */
        return NS_OK;
    }

    PRUnichar **fs = NS_STATIC_CAST(PRUnichar **,
                                    nsMemory::Alloc(fontList.Count() * sizeof(PRUnichar*)));
    for (int i = 0; i < fontList.Count(); i++) {
        fs[i] = ToNewUnicode(*fontList[i]);
    }

    *aResult = fs;
    *aCount = fontList.Count();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesFontEnumerator::HaveFontFor(const char *aLangGroup,
                                    PRBool *aResult)
{
    NS_ENSURE_ARG_POINTER(*aResult);
    NS_ENSURE_ARG_POINTER(*aLangGroup);

    *aResult = PR_TRUE;
    return NS_OK;
}

NS_IMETHODIMP
nsThebesFontEnumerator::GetDefaultFont(const char *aLangGroup,
                                       const char *aGeneric,
                                       PRUnichar **aResult)
{
    NS_ENSURE_ARG_POINTER(aResult);
    *aResult = nsnull;
    return NS_OK;
}

NS_IMETHODIMP
nsThebesFontEnumerator::UpdateFontList(PRBool *_retval)
{
    *_retval = PR_FALSE; // always return false for now
    return NS_OK;
}
