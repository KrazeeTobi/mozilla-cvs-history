/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Corporation code.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <stuart@mozilla.com>
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

#include "gfxQuartzPDFSurface.h"

#include "cairo-nquartz.h"

#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#include <cups/cups.h>
#else
/* stupid apple doesn't bother to include cups.h in anything lower than
 * the 10.4 SDK yet they've shipped libcups since 10.2! wtf!
 * Apple is as bad as Linux!  come on!
 */
extern "C" {
typedef struct
{
    char *name;
    char *value;
} cups_option_t;
typedef struct
{
    char *name,
         *instance;
    int is_default;
    int num_options;
    cups_option_t *options;
} cups_dest_t;
extern int cupsPrintFile(const char *printer, const char *filename,
                         const char *title, int num_options,
                         cups_option_t *options);
extern void cupsFreeDests(int num_dests, cups_dest_t *dests);
extern cups_dest_t *cupsGetDest(const char *name, const char *instance,
                                int num_dests, cups_dest_t *dests);
extern int cupsGetDests(cups_dest_t **dests);
}
#endif

gfxQuartzPDFSurface::gfxQuartzPDFSurface(const char *filename, gfxSize aSizeInPoints)
    : mFilename(strdup(filename)), mAborted(PR_FALSE)
{
    mRect = CGRectMake(0.0, 0.0, aSizeInPoints.width, aSizeInPoints.height);

    CFStringRef file = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
    CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file, kCFURLPOSIXPathStyle, false);
    mCGContext = CGPDFContextCreateWithURL(fileURL, &mRect, NULL);

    CFRelease(file);
    CFRelease(fileURL);

    Init(cairo_nquartz_surface_create_for_cg_context(mCGContext, aSizeInPoints.width, aSizeInPoints.height, PR_FALSE));
}

gfxQuartzPDFSurface::~gfxQuartzPDFSurface()
{
    CGContextRelease(mCGContext);

    /* don't spool if we're aborted */
    if (mAborted) {
        printf("aborted!!\n");
        free(mFilename);
        return;
    }

    /* quick dirty spooling code */
    /* this doesn't do the right thing with the print options at all. */
    int ndests;
    cups_dest_t *dests;
    ndests = cupsGetDests(&dests);
#ifdef DEBUG_pavlov
    for (int i = 0; i < ndests; i++) {
        printf("%s\n", dests[i].name);
    }
#endif
    /* get the default printer */
    cups_dest_t *defaultDest = cupsGetDest(NULL, NULL, ndests, dests);
#ifdef DEBUG_pavlov
    printf("printing to: %s\n", defaultDest->name);
    printf("printing: %s\n", mFilename);
#endif

    /* print with the default printer */
    cupsPrintFile(defaultDest->name, mFilename, "Mozilla Print Job", defaultDest->num_options, defaultDest->options);

    cupsFreeDests(ndests, dests);

    free(mFilename);
}


nsresult
gfxQuartzPDFSurface::BeginPrinting(const nsAString& aTitle, const nsAString& aPrintToFileName)
{
    return NS_OK;
}

nsresult
gfxQuartzPDFSurface::EndPrinting()
{
    return NS_OK;
}

nsresult
gfxQuartzPDFSurface::AbortPrinting()
{
    mAborted = PR_TRUE;
    return NS_OK;
}

nsresult
gfxQuartzPDFSurface::BeginPage()
{
    CGContextBeginPage(mCGContext, &mRect);

    return NS_OK;
}

nsresult
gfxQuartzPDFSurface::EndPage()
{
    CGContextEndPage(mCGContext);
    return NS_OK;
}
