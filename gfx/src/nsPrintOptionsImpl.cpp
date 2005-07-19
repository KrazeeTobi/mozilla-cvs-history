/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jessica Blanco <jblanco@us.ibm.com>
 *   Bastiaan Jacques <baafie@planet.nl>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsPrintOptionsImpl.h"
#include "nsCoord.h"
#include "nsUnitConversion.h"
#include "nsReadableUtils.h"
#include "nsPrintSettingsImpl.h"

#include "nsIDOMWindow.h"
#include "nsIServiceManager.h"
#include "nsIDialogParamBlock.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIWindowWatcher.h"
#include "nsIDOMWindowInternal.h"
#include "nsVoidArray.h"
#include "nsSupportsArray.h"
#include "prprf.h"

// For Prefs
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsIServiceManager.h"

#include "nsISimpleEnumerator.h"
#include "nsISupportsPrimitives.h"
#include "nsGfxCIID.h"
#include "stdlib.h"
#include "nsAutoPtr.h"

static NS_DEFINE_IID(kCPrinterEnumerator, NS_PRINTER_ENUMERATOR_CID);

NS_IMPL_ISUPPORTS2(nsPrintOptions, nsIPrintOptions, nsIPrintSettingsService)

// Pref Constants
    static const char kMarginTop[]       = "print_margin_top";
static const char kMarginLeft[]      = "print_margin_left";
static const char kMarginBottom[]    = "print_margin_bottom";
static const char kMarginRight[]     = "print_margin_right";

// Prefs for Print Options
static const char kPrintEvenPages[]       = "print_evenpages";
static const char kPrintOddPages[]        = "print_oddpages";
static const char kPrintHeaderStrLeft[]   = "print_headerleft";
static const char kPrintHeaderStrCenter[] = "print_headercenter";
static const char kPrintHeaderStrRight[]  = "print_headerright";
static const char kPrintFooterStrLeft[]   = "print_footerleft";
static const char kPrintFooterStrCenter[] = "print_footercenter";
static const char kPrintFooterStrRight[]  = "print_footerright";

// Additional Prefs
static const char kPrintPaperSize[]     = "print_paper_size"; // deprecated

static const char kPrintReversed[]      = "print_reversed";
static const char kPrintInColor[]       = "print_in_color";
static const char kPrintPaperName[]     = "print_paper_name";
static const char kPrintPlexName[]      = "print_plex_name";
static const char kPrintPaperSizeType[] = "print_paper_size_type";
static const char kPrintPaperData[]     = "print_paper_data";
static const char kPrintPaperSizeUnit[] = "print_paper_size_unit";
static const char kPrintPaperWidth[]    = "print_paper_width";
static const char kPrintPaperHeight[]   = "print_paper_height";
static const char kPrintColorspace[]    = "print_colorspace";
static const char kPrintResolutionName[]= "print_resolution_name";
static const char kPrintDownloadFonts[] = "print_downloadfonts";
static const char kPrintOrientation[]   = "print_orientation";
static const char kPrintCommand[]       = "print_command";
static const char kPrinterName[]        = "print_printer";
static const char kPrintToFile[]        = "print_to_file";
static const char kPrintToFileName[]    = "print_to_filename";
static const char kPrintPageDelay[]     = "print_pagedelay";
static const char kPrintBGColors[]      = "print_bgcolor";
static const char kPrintBGImages[]      = "print_bgimages";
static const char kPrintShrinkToFit[]   = "print_shrink_to_fit";
static const char kPrintScaling[]       = "print_scaling";

static const char kJustLeft[]   = "left";
static const char kJustCenter[] = "center";
static const char kJustRight[]  = "right";

static NS_DEFINE_IID(kPrinterEnumeratorCID, NS_PRINTER_ENUMERATOR_CID);


nsPrintOptions::nsPrintOptions()
{
}

nsPrintOptions::~nsPrintOptions()
{
  if (mDefaultFont) {
    delete mDefaultFont;
  }
}



nsresult
nsPrintOptions::Init()
{
  mDefaultFont = new nsFont("Times", NS_FONT_STYLE_NORMAL,
                            NS_FONT_VARIANT_NORMAL, NS_FONT_WEIGHT_NORMAL, 0,
                            NSIntPointsToTwips(10));
  NS_ENSURE_TRUE(mDefaultFont, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv;
  nsCOMPtr<nsIPrefService> prefService =
      do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return prefService->GetBranch("print.", getter_AddRefs(mPrefBranch));
}


//**************************************************************
//** PageList Enumerator
//**************************************************************
class
    nsPrinterListEnumerator : public nsISimpleEnumerator
{
  public:
    nsPrinterListEnumerator();
    virtual ~nsPrinterListEnumerator();

    //nsISupports interface
    NS_DECL_ISUPPORTS

    //nsISimpleEnumerator interface
    NS_DECL_NSISIMPLEENUMERATOR

    NS_IMETHOD Init();

  protected:
    PRUnichar **mPrinters;
    PRUint32 mCount;
    PRUint32 mIndex;
};

nsPrinterListEnumerator::nsPrinterListEnumerator() :
    mPrinters(nsnull), mCount(0), mIndex(0)
{
}

nsPrinterListEnumerator::~nsPrinterListEnumerator()
{
  if (!mPrinters)
    return;

  PRUint32 i;
  for (i = 0; i < mCount; i++ ) {
    nsMemory::Free(mPrinters[i]);
  }
  nsMemory::Free(mPrinters);
}

NS_IMPL_ISUPPORTS1(nsPrinterListEnumerator, nsISimpleEnumerator)

NS_IMETHODIMP
nsPrinterListEnumerator::Init()
{
  nsresult rv;
  nsCOMPtr<nsIPrinterEnumerator> printerEnumerator;

  printerEnumerator = do_CreateInstance(kCPrinterEnumerator, &rv);
  if (NS_FAILED(rv))
    return rv;

  return printerEnumerator->EnumeratePrinters(&mCount, &mPrinters);
}

NS_IMETHODIMP
nsPrinterListEnumerator::HasMoreElements(PRBool *result)
{
  *result = (mIndex < mCount);
  return NS_OK;
}

NS_IMETHODIMP
nsPrinterListEnumerator::GetNext(nsISupports **aPrinter)
{
  if (mIndex >= mCount) {
    return NS_ERROR_UNEXPECTED;
  }

  PRUnichar *printerName = mPrinters[mIndex++];
  nsCOMPtr<nsISupportsString> printerNameWrapper;
  nsresult rv;

  printerNameWrapper = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  printerNameWrapper->SetData(nsDependentString(printerName));
  *aPrinter = NS_STATIC_CAST(nsISupports*, printerNameWrapper);
  NS_ADDREF(*aPrinter);
  return NS_OK;
}

NS_IMETHODIMP 
nsPrintOptions::SetFontNamePointSize(const nsAString& aFontName,
                                     PRInt32 aPointSize)
{
  if (mDefaultFont && !aFontName.IsEmpty() && aPointSize > 0) {
    mDefaultFont->name = aFontName;
    mDefaultFont->size = NSIntPointsToTwips(aPointSize);
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsPrintOptions::SetDefaultFont(nsFont &aFont)
{
  if (mDefaultFont)
    delete mDefaultFont;

  mDefaultFont = new nsFont(aFont);
  NS_ENSURE_TRUE(mDefaultFont, NS_ERROR_OUT_OF_MEMORY);

  return NS_OK;
}

NS_IMETHODIMP
nsPrintOptions::GetDefaultFont(nsFont &aFont)
{
  NS_ENSURE_STATE(mDefaultFont);
  aFont = *mDefaultFont;
  return NS_OK;
}

NS_IMETHODIMP
nsPrintOptions::ShowPrintSetupDialog(nsIPrintSettings *aPS)
{
  NS_ENSURE_ARG_POINTER(aPS);
  nsresult rv;

  // create a nsISupportsArray of the parameters
  // being passed to the window
  nsCOMPtr<nsISupportsArray> array;
  rv = NS_NewISupportsArray(getter_AddRefs(array));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsISupports> psSupports = do_QueryInterface(aPS);
  NS_ASSERTION(psSupports, "PrintSettings must be a supports");
  array->AppendElement(psSupports);

  nsCOMPtr<nsIDialogParamBlock> ioParamBlock =
      do_CreateInstance(NS_DIALOGPARAMBLOCK_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  ioParamBlock->SetInt(0, 0);

  nsCOMPtr<nsISupports> blkSupps = do_QueryInterface(ioParamBlock);
  NS_ASSERTION(blkSupps, "IOBlk must be a supports");
  array->AppendElement(blkSupps);

  nsCOMPtr<nsIWindowWatcher> wwatch =
      do_GetService(NS_WINDOWWATCHER_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMWindow> active;
  wwatch->GetActiveWindow(getter_AddRefs(active));
  nsCOMPtr<nsIDOMWindowInternal> parent = do_QueryInterface(active);
  // null |parent| is non-fatal

  nsCOMPtr<nsIDOMWindow> newWindow;

  return wwatch->OpenWindow(parent,
                            "chrome://global/content/printPageSetup.xul",
                            "_blank","chrome,modal,centerscreen", array,
                            getter_AddRefs(newWindow));
}

/** ---------------------------------------------------
 *  Helper function - Creates the "prefix" for the pref
 *  It is either "print."
 *  or "print.printer_<print name>."
 */
const char*
nsPrintOptions::GetPrefName(const char * aPrefName,
                            const nsAString& aPrinterName)
{
  if (!aPrefName || !*aPrefName) {
    NS_ERROR("Must have a valid pref name!");
    return aPrefName;
  }

  mPrefName.Truncate(); /* mPrefName = ""; */

  if (aPrinterName.Length()) {
    mPrefName.Append("printer_");
    AppendUTF16toUTF8(aPrinterName, mPrefName);
    mPrefName.Append(".");
  }
  mPrefName += aPrefName;

  return mPrefName.get();
}

//----------------------------------------------------------------------
// Testing of read/write prefs
// This define controls debug output
#ifdef DEBUG_rods_X
static void WriteDebugStr(const char* aArg1, const char* aArg2,
                          const PRUnichar* aStr)
{
  nsString str(aStr);
  PRUnichar s = '&';
  PRUnichar r = '_';
  str.ReplaceChar(s, r);

  printf("%s %s = %s \n", aArg1, aArg2, ToNewUTF8String(str));
}
const char* kWriteStr = "Write Pref:";
const char* kReadStr  = "Read Pref:";
#define DUMP_STR(_a1, _a2, _a3)  WriteDebugStr((_a1), GetPrefName((_a2), \
aPrefName), (_a3));
#define DUMP_BOOL(_a1, _a2, _a3) printf("%s %s = %s \n", (_a1), \
GetPrefName((_a2), aPrefName), (_a3)?"T":"F");
#define DUMP_INT(_a1, _a2, _a3)  printf("%s %s = %d \n", (_a1), \
GetPrefName((_a2), aPrefName), (_a3));
#define DUMP_DBL(_a1, _a2, _a3)  printf("%s %s = %10.5f \n", (_a1), \
GetPrefName((_a2), aPrefName), (_a3));
#else
#define DUMP_STR(_a1, _a2, _a3)
#define DUMP_BOOL(_a1, _a2, _a3)
#define DUMP_INT(_a1, _a2, _a3)
#define DUMP_DBL(_a1, _a2, _a3)
#endif /* DEBUG_rods_X */
//----------------------------------------------------------------------

/**
 *  This will either read in the generic prefs (not specific to a printer)
 *  or read the prefs in using the printer name to qualify.
 *  It is either "print.attr_name" or "print.printer_HPLasr5.attr_name"
 */
nsresult 
nsPrintOptions::ReadPrefs(nsIPrintSettings* aPS, const nsAString& aPrinterName,
                          PRUint32 aFlags)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPS);

  if (aFlags & nsIPrintSettings::kInitSaveMargins) {
    nscoord halfInch = NS_INCHES_TO_TWIPS(0.5);
    nsMargin margin;
    margin.SizeTo(halfInch, halfInch, halfInch, halfInch);
    ReadInchesToTwipsPref(GetPrefName(kMarginTop, aPrinterName), margin.top,
                          kMarginTop);
    DUMP_INT(kReadStr, kMarginTop, margin.top);
    ReadInchesToTwipsPref(GetPrefName(kMarginLeft, aPrinterName), margin.left,
                          kMarginLeft);
    DUMP_INT(kReadStr, kMarginLeft, margin.left);
    ReadInchesToTwipsPref(GetPrefName(kMarginBottom, aPrinterName),
                          margin.bottom, kMarginBottom);
    DUMP_INT(kReadStr, kMarginBottom, margin.bottom);
    ReadInchesToTwipsPref(GetPrefName(kMarginRight, aPrinterName), margin.right,
                          kMarginRight);
    DUMP_INT(kReadStr, kMarginRight, margin.right);
    aPS->SetMarginInTwips(margin);
  }

  PRBool   b;
  nsAutoString str;
  PRInt32  iVal;
  double   dbl;

#define GETBOOLPREF(_prefname, _retval)                 \
  NS_SUCCEEDED(                                         \
    mPrefBranch->GetBoolPref(                           \
      GetPrefName(_prefname, aPrinterName), _retval     \
    )                                                   \
  )

#define GETSTRPREF(_prefname, _retval)                  \
  NS_SUCCEEDED(                                         \
    ReadPrefString(                                     \
      GetPrefName(_prefname, aPrinterName), _retval     \
    )                                                   \
  )

#define GETINTPREF(_prefname, _retval)                  \
  NS_SUCCEEDED(                                         \
    mPrefBranch->GetIntPref(                            \
      GetPrefName(_prefname, aPrinterName), _retval     \
    )                                                   \
  )

#define GETDBLPREF(_prefname, _retval)                  \
  NS_SUCCEEDED(                                         \
    ReadPrefDouble(                                     \
      GetPrefName(_prefname, aPrinterName), _retval     \
    )                                                   \
  )

  if (aFlags & nsIPrintSettings::kInitSaveOddEvenPages) {
  if (GETBOOLPREF(kPrintEvenPages, &b)) {
    aPS->SetPrintOptions(nsIPrintSettings::kPrintEvenPages, b);
    DUMP_BOOL(kReadStr, kPrintEvenPages, b);
  }
  }

  if (aFlags & nsIPrintSettings::kInitSaveOddEvenPages) {
    if (GETBOOLPREF(kPrintOddPages, &b)) {
      aPS->SetPrintOptions(nsIPrintSettings::kPrintOddPages, b);
      DUMP_BOOL(kReadStr, kPrintOddPages, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderLeft) {
    if (GETSTRPREF(kPrintHeaderStrLeft, str)) {
      aPS->SetHeaderStrLeft(str.get());
      DUMP_STR(kReadStr, kPrintHeaderStrLeft, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderCenter) {
    if (GETSTRPREF(kPrintHeaderStrCenter, str)) {
      aPS->SetHeaderStrCenter(str.get());
      DUMP_STR(kReadStr, kPrintHeaderStrCenter, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderRight) {
    if (GETSTRPREF(kPrintHeaderStrRight, str)) {
      aPS->SetHeaderStrRight(str.get());
      DUMP_STR(kReadStr, kPrintHeaderStrRight, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterLeft) {
    if (GETSTRPREF(kPrintFooterStrLeft, str)) {
      aPS->SetFooterStrLeft(str.get());
      DUMP_STR(kReadStr, kPrintFooterStrLeft, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterCenter) {
    if (GETSTRPREF(kPrintFooterStrCenter, str)) {
      aPS->SetFooterStrCenter(str.get());
      DUMP_STR(kReadStr, kPrintFooterStrCenter, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterRight) {
    if (GETSTRPREF(kPrintFooterStrRight, str)) {
      aPS->SetFooterStrRight(str.get());
      DUMP_STR(kReadStr, kPrintFooterStrRight, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveBGColors) {
    if (GETBOOLPREF(kPrintBGColors, &b)) {
      aPS->SetPrintBGColors(b);
      DUMP_BOOL(kReadStr, kPrintBGColors, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveBGImages) {
    if (GETBOOLPREF(kPrintBGImages, &b)) {
      aPS->SetPrintBGImages(b);
      DUMP_BOOL(kReadStr, kPrintBGImages, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperSize) {
    if (GETINTPREF(kPrintPaperSize, &iVal)) { // this has been deprecated
      aPS->SetPaperSize(iVal);
      DUMP_INT(kReadStr, kPrintPaperSize, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveReversed) {
    if (GETBOOLPREF(kPrintReversed, &b)) {
      aPS->SetPrintReversed(b);
      DUMP_BOOL(kReadStr, kPrintReversed, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveInColor) {
    if (GETBOOLPREF(kPrintInColor, &b)) {
      aPS->SetPrintInColor(b);
      DUMP_BOOL(kReadStr, kPrintInColor, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperName) {
    if (GETSTRPREF(kPrintPaperName, str)) {
      aPS->SetPaperName(str.get());
      DUMP_STR(kReadStr, kPrintPaperName, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePlexName) {
    if (GETSTRPREF(kPrintPlexName, str)) {
      aPS->SetPlexName(str.get());
      DUMP_STR(kReadStr, kPrintPlexName, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperSizeUnit) {
    if (GETINTPREF(kPrintPaperSizeUnit, &iVal)) {
      aPS->SetPaperSizeUnit(iVal);
      DUMP_INT(kReadStr, kPrintPaperSizeUnit, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperSizeType) {
    if (GETINTPREF(kPrintPaperSizeType, &iVal)) {
      aPS->SetPaperSizeType(iVal);
      DUMP_INT(kReadStr, kPrintPaperSizeType, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperData) {
    if (GETINTPREF(kPrintPaperData, &iVal)) {
      aPS->SetPaperData(iVal);
      DUMP_INT(kReadStr, kPrintPaperData, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperWidth) {
    if (GETDBLPREF(kPrintPaperWidth, dbl)) {
      aPS->SetPaperWidth(dbl);
      DUMP_DBL(kReadStr, kPrintPaperWidth, dbl);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperHeight) {
    if (GETDBLPREF(kPrintPaperHeight, dbl)) {
      aPS->SetPaperHeight(dbl);
      DUMP_DBL(kReadStr, kPrintPaperHeight, dbl);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveColorspace) {
    if (GETSTRPREF(kPrintColorspace, str)) {
      aPS->SetColorspace(str.get());
      DUMP_STR(kReadStr, kPrintColorspace, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveResolutionName) {
    if (GETSTRPREF(kPrintResolutionName, str)) {
      aPS->SetResolutionName(str.get());
      DUMP_STR(kReadStr, kPrintResolutionName, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveDownloadFonts) {
    if (GETBOOLPREF(kPrintDownloadFonts, &b)) {
      aPS->SetDownloadFonts(b);
      DUMP_BOOL(kReadStr, kPrintDownloadFonts, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveOrientation) {
    if (GETINTPREF(kPrintOrientation, &iVal)) {
      aPS->SetOrientation(iVal);
      DUMP_INT(kReadStr, kPrintOrientation, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrintCommand) {
    if (GETSTRPREF(kPrintCommand, str)) {
      aPS->SetPrintCommand(str.get());
      DUMP_STR(kReadStr, kPrintCommand, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrinterName) {
    if (GETSTRPREF(kPrinterName, str)) {
      aPS->SetPrinterName(str.get());
      DUMP_STR(kReadStr, kPrinterName, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrintToFile) {
    if (GETBOOLPREF(kPrintToFile, &b)) {
      aPS->SetPrintToFile(b);
      DUMP_BOOL(kReadStr, kPrintToFile, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveToFileName) {
    if (GETSTRPREF(kPrintToFileName, str)) {
      aPS->SetToFileName(str.get());
      DUMP_STR(kReadStr, kPrintToFileName, str.get());
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePageDelay) {
    if (GETINTPREF(kPrintPageDelay, &iVal)) {
      aPS->SetPrintPageDelay(iVal);
      DUMP_INT(kReadStr, kPrintPageDelay, iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveShrinkToFit) {
    if (GETBOOLPREF(kPrintShrinkToFit, &b)) {
      aPS->SetShrinkToFit(b);
      DUMP_BOOL(kReadStr, kPrintShrinkToFit, b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveScaling) {
    if (GETDBLPREF(kPrintScaling, dbl)) {
      aPS->SetScaling(dbl);
      DUMP_DBL(kReadStr, kPrintScaling, dbl);
    }
  }

  // Not Reading In:
  //   Number of Copies

  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsPrintOptionsImpl.h
 *  @update 1/12/01 rods
 */
nsresult 
nsPrintOptions::WritePrefs(nsIPrintSettings *aPS, const nsAString& aPrinterName,
                           PRUint32 aFlags)
{
  NS_ENSURE_ARG_POINTER(aPS);
  NS_ENSURE_STATE(mPrefBranch);

  nsMargin margin;
  if (aFlags & nsIPrintSettings::kInitSaveMargins) {
    if (NS_SUCCEEDED(aPS->GetMarginInTwips(margin))) {
      WriteInchesFromTwipsPref(GetPrefName(kMarginTop, aPrinterName),
                               margin.top);
      DUMP_INT(kWriteStr, kMarginTop, margin.top);
      WriteInchesFromTwipsPref(GetPrefName(kMarginLeft, aPrinterName),
                               margin.left);
      DUMP_INT(kWriteStr, kMarginLeft, margin.top);
      WriteInchesFromTwipsPref(GetPrefName(kMarginBottom, aPrinterName),
                               margin.bottom);
      DUMP_INT(kWriteStr, kMarginBottom, margin.top);
      WriteInchesFromTwipsPref(GetPrefName(kMarginRight, aPrinterName),
                               margin.right);
      DUMP_INT(kWriteStr, kMarginRight, margin.top);
    }
  }

  PRBool     b;
  PRUnichar* uStr;
  PRInt32    iVal;
  PRInt16    iVal16;
  double     dbl;

  if (aFlags & nsIPrintSettings::kInitSaveOddEvenPages) {
    if (NS_SUCCEEDED(aPS->GetPrintOptions(nsIPrintSettings::kPrintEvenPages,
                                          &b))) {
          DUMP_BOOL(kWriteStr, kPrintEvenPages, b);
          mPrefBranch->SetBoolPref(GetPrefName(kPrintEvenPages, aPrinterName),
                                   b);
        }
  }

  if (aFlags & nsIPrintSettings::kInitSaveOddEvenPages) {
    if (NS_SUCCEEDED(aPS->GetPrintOptions(nsIPrintSettings::kPrintOddPages,
                                          &b))) {
          DUMP_BOOL(kWriteStr, kPrintOddPages, b);
          mPrefBranch->SetBoolPref(GetPrefName(kPrintOddPages, aPrinterName),
                                   b);
        }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderLeft) {
    if (NS_SUCCEEDED(aPS->GetHeaderStrLeft(&uStr))) {
      DUMP_STR(kWriteStr, kPrintHeaderStrLeft, uStr);
      WritePrefString(uStr, GetPrefName(kPrintHeaderStrLeft, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderCenter) {
    if (NS_SUCCEEDED(aPS->GetHeaderStrCenter(&uStr))) {
      DUMP_STR(kWriteStr, kPrintHeaderStrCenter, uStr);
      WritePrefString(uStr, GetPrefName(kPrintHeaderStrCenter, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveHeaderRight) {
    if (NS_SUCCEEDED(aPS->GetHeaderStrRight(&uStr))) {
      DUMP_STR(kWriteStr, kPrintHeaderStrRight, uStr);
      WritePrefString(uStr, GetPrefName(kPrintHeaderStrRight, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterLeft) {
    if (NS_SUCCEEDED(aPS->GetFooterStrLeft(&uStr))) {
      DUMP_STR(kWriteStr, kPrintFooterStrLeft, uStr);
      WritePrefString(uStr, GetPrefName(kPrintFooterStrLeft, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterCenter) {
    if (NS_SUCCEEDED(aPS->GetFooterStrCenter(&uStr))) {
      DUMP_STR(kWriteStr, kPrintFooterStrCenter, uStr);
      WritePrefString(uStr, GetPrefName(kPrintFooterStrCenter, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveFooterRight) {
    if (NS_SUCCEEDED(aPS->GetFooterStrRight(&uStr))) {
      DUMP_STR(kWriteStr, kPrintFooterStrRight, uStr);
      WritePrefString(uStr, GetPrefName(kPrintFooterStrRight, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveBGColors) {
    if (NS_SUCCEEDED(aPS->GetPrintBGColors(&b))) {
      DUMP_BOOL(kWriteStr, kPrintBGColors, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintBGColors, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveBGImages) {
    if (NS_SUCCEEDED(aPS->GetPrintBGImages(&b))) {
      DUMP_BOOL(kWriteStr, kPrintBGImages, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintBGImages, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperSize) {
    if (NS_SUCCEEDED(aPS->GetPaperSize(&iVal))) {
      DUMP_INT(kWriteStr, kPrintPaperSize, iVal);
      mPrefBranch->SetIntPref(GetPrefName(kPrintPaperSize, aPrinterName), iVal);
      // this has been deprecated
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveReversed) {
    if (NS_SUCCEEDED(aPS->GetPrintReversed(&b))) {
      DUMP_BOOL(kWriteStr, kPrintReversed, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintReversed, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveInColor) {
    if (NS_SUCCEEDED(aPS->GetPrintInColor(&b))) {
      DUMP_BOOL(kWriteStr, kPrintInColor, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintInColor, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperName) {
    if (NS_SUCCEEDED(aPS->GetPaperName(&uStr))) {
      DUMP_STR(kWriteStr, kPrintPaperName, uStr);
      WritePrefString(uStr, GetPrefName(kPrintPaperName, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePlexName) {
    if (NS_SUCCEEDED(aPS->GetPlexName(&uStr))) {
      DUMP_STR(kWriteStr, kPrintPlexName, uStr);
      WritePrefString(uStr, GetPrefName(kPrintPlexName, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperSizeUnit) {
    if (NS_SUCCEEDED(aPS->GetPaperSizeUnit(&iVal16))) {
      DUMP_INT(kWriteStr, kPrintPaperSizeUnit, iVal16);
      mPrefBranch->SetIntPref(GetPrefName(kPrintPaperSizeUnit, aPrinterName),
                              PRInt32(iVal16));
    }
  }
 
  if (aFlags & nsIPrintSettings::kInitSavePaperSizeType) {
    if (NS_SUCCEEDED(aPS->GetPaperSizeType(&iVal16))) {
      DUMP_INT(kWriteStr, kPrintPaperSizeType, iVal16);
      mPrefBranch->SetIntPref(GetPrefName(kPrintPaperSizeType, aPrinterName),
                              PRInt32(iVal16));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperData) {
    if (NS_SUCCEEDED(aPS->GetPaperData(&iVal16))) {
      DUMP_INT(kWriteStr, kPrintPaperData, iVal16);
      mPrefBranch->SetIntPref(GetPrefName(kPrintPaperData, aPrinterName),
                              PRInt32(iVal16));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperWidth) {
    if (NS_SUCCEEDED(aPS->GetPaperWidth(&dbl))) {
      DUMP_DBL(kWriteStr, kPrintPaperWidth, dbl);
      WritePrefDouble(GetPrefName(kPrintPaperWidth, aPrinterName), dbl);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePaperHeight) {
    if (NS_SUCCEEDED(aPS->GetPaperHeight(&dbl))) {
      DUMP_DBL(kWriteStr, kPrintPaperHeight, dbl);
      WritePrefDouble(GetPrefName(kPrintPaperHeight, aPrinterName), dbl);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveColorspace) {
    if (NS_SUCCEEDED(aPS->GetColorspace(&uStr))) {
      DUMP_STR(kWriteStr, kPrintColorspace, uStr);
      WritePrefString(uStr, GetPrefName(kPrintColorspace, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveResolutionName) {
    if (NS_SUCCEEDED(aPS->GetResolutionName(&uStr))) {
      DUMP_STR(kWriteStr, kPrintResolutionName, uStr);
      WritePrefString(uStr, GetPrefName(kPrintResolutionName, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveDownloadFonts) {
    if (NS_SUCCEEDED(aPS->GetDownloadFonts(&b))) {
      DUMP_BOOL(kWriteStr, kPrintDownloadFonts, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintDownloadFonts, aPrinterName),
                               b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveOrientation) {
    if (NS_SUCCEEDED(aPS->GetOrientation(&iVal))) {
      DUMP_INT(kWriteStr, kPrintOrientation, iVal);
      mPrefBranch->SetIntPref(GetPrefName(kPrintOrientation, aPrinterName),
                              iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrintCommand) {
    if (NS_SUCCEEDED(aPS->GetPrintCommand(&uStr))) {
      DUMP_STR(kWriteStr, kPrintCommand, uStr);
      WritePrefString(uStr, GetPrefName(kPrintCommand, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrinterName) {
    if (NS_SUCCEEDED(aPS->GetPrinterName(&uStr))) {
      DUMP_STR(kWriteStr, kPrinterName, uStr);
      WritePrefString(uStr, GetPrefName(kPrinterName, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePrintToFile) {
    if (NS_SUCCEEDED(aPS->GetPrintToFile(&b))) {
      DUMP_BOOL(kWriteStr, kPrintToFile, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintToFile, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveToFileName) {
    if (NS_SUCCEEDED(aPS->GetToFileName(&uStr))) {
      DUMP_STR(kWriteStr, kPrintToFileName, uStr);
      WritePrefString(uStr, GetPrefName(kPrintToFileName, aPrinterName));
    }
  }

  if (aFlags & nsIPrintSettings::kInitSavePageDelay) {
    if (NS_SUCCEEDED(aPS->GetPrintPageDelay(&iVal))) {
      DUMP_INT(kWriteStr, kPrintPageDelay, iVal);
      mPrefBranch->SetIntPref(GetPrefName(kPrintPageDelay, aPrinterName), iVal);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveShrinkToFit) {
    if (NS_SUCCEEDED(aPS->GetShrinkToFit(&b))) {
      DUMP_BOOL(kWriteStr, kPrintShrinkToFit, b);
      mPrefBranch->SetBoolPref(GetPrefName(kPrintShrinkToFit, aPrinterName), b);
    }
  }

  if (aFlags & nsIPrintSettings::kInitSaveScaling) {
    if (NS_SUCCEEDED(aPS->GetScaling(&dbl))) {
      DUMP_DBL(kWriteStr, kPrintScaling, dbl);
      WritePrefDouble(GetPrefName(kPrintScaling, aPrinterName), dbl);
    }
  }

  // Not Writing Out:
  //   Number of Copies

  return NS_OK;
}

/* create and return a new |nsPrinterListEnumerator| */
NS_IMETHODIMP
nsPrintOptions::AvailablePrinters(nsISimpleEnumerator **aPrinterEnumerator)
{
  nsRefPtr<nsPrinterListEnumerator> printerListEnum =
      new nsPrinterListEnumerator();
  NS_ENSURE_TRUE(printerListEnum, NS_ERROR_OUT_OF_MEMORY);

  NS_ADDREF(*aPrinterEnumerator = printerListEnum.get());

  nsresult rv = printerListEnum->Init();
  if (NS_FAILED(rv))
    NS_RELEASE(*aPrinterEnumerator);

  return rv;
}

NS_IMETHODIMP
nsPrintOptions::DisplayJobProperties(const PRUnichar *aPrinter,
                                     nsIPrintSettings* aPrintSettings,
                                     PRBool *aDisplayed)
{
  NS_ENSURE_ARG_POINTER(aPrinter);
  *aDisplayed = PR_FALSE;

  nsresult rv;
  nsCOMPtr<nsIPrinterEnumerator> propDlg;

  propDlg = do_CreateInstance(kCPrinterEnumerator, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  NS_ENSURE_ARG_POINTER(aPrintSettings);
  rv = propDlg->DisplayPropertiesDlg(aPrinter, aPrintSettings);
  NS_ENSURE_SUCCESS(rv, rv);

  *aDisplayed = PR_TRUE;

  return rv;
}

NS_IMETHODIMP nsPrintOptions::GetNativeData(PRInt16 aDataType, void * *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

nsresult nsPrintOptions::_CreatePrintSettings(nsIPrintSettings **_retval)
{
  // does not initially ref count
  nsPrintSettings * printSettings = new nsPrintSettings();
  NS_ENSURE_TRUE(printSettings, NS_ERROR_OUT_OF_MEMORY);

  NS_ADDREF(*_retval = printSettings); // ref count
  (void)InitPrintSettingsFromPrefs(*_retval, PR_FALSE,
                                   nsIPrintSettings::kInitSaveAll);

  return NS_OK;
}

NS_IMETHODIMP nsPrintOptions::CreatePrintSettings(nsIPrintSettings **_retval)
{
  return _CreatePrintSettings(_retval);
}

NS_IMETHODIMP
nsPrintOptions::GetGlobalPrintSettings(nsIPrintSettings **aGlobalPrintSettings)
{
  nsresult rv;

  rv = CreatePrintSettings(getter_AddRefs(mGlobalPrintSettings));
  NS_ENSURE_SUCCESS(rv, rv);

  NS_ADDREF(*aGlobalPrintSettings = mGlobalPrintSettings.get());

  return rv;
}

NS_IMETHODIMP
nsPrintOptions::GetNewPrintSettings(nsIPrintSettings * *aNewPrintSettings)
{
  return CreatePrintSettings(aNewPrintSettings);
}

NS_IMETHODIMP
nsPrintOptions::GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName)
{
  nsresult rv;
  nsCOMPtr<nsIPrinterEnumerator> prtEnum = do_GetService(kPrinterEnumeratorCID,
                                                         &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return prtEnum->GetDefaultPrinterName(aDefaultPrinterName);
}

NS_IMETHODIMP
nsPrintOptions::InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName,
                                             nsIPrintSettings *aPrintSettings)
{
  NS_ENSURE_ARG_POINTER(aPrintSettings);
  NS_ENSURE_ARG_POINTER(aPrinterName);

#ifdef NS_DEBUG
  nsXPIDLString printerName;
  aPrintSettings->GetPrinterName(getter_Copies(printerName));
  if (!printerName.Equals(aPrinterName)) {
    NS_WARNING("Printer names should match!");
  }
#endif

  PRBool isInitialized;
  aPrintSettings->GetIsInitializedFromPrinter(&isInitialized);
  if (isInitialized)
    return NS_OK;

  nsresult rv;
  nsCOMPtr<nsIPrinterEnumerator> prtEnum = do_GetService(kPrinterEnumeratorCID,
                                                         &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = prtEnum->InitPrintSettingsFromPrinter(aPrinterName, aPrintSettings);
  NS_ENSURE_SUCCESS(rv, rv);

  aPrintSettings->SetIsInitializedFromPrinter(PR_TRUE);
  return rv;
}

/** ---------------------------------------------------
 *  Helper function - Returns either the name or sets the length to zero
 */
static void
GetAdjustedPrinterName(nsIPrintSettings* aPS, PRBool aUsePNP,
                       nsAString& aPrinterName)
{
  if(!aPS) return;
  aPrinterName.Truncate();

  // Get the Printer Name from the PrintSettings 
  // to use as a prefix for Pref Names
  PRUnichar* prtName = nsnull;
  if (aUsePNP && NS_SUCCEEDED(aPS->GetPrinterName(&prtName))) {
    if (prtName && !*prtName) {
      nsMemory::Free(prtName);
      prtName = nsnull;
    }
  }

  if (!prtName) return;

  aPrinterName = nsDependentString(prtName);

  // Convert any whitespaces, carriage returns or newlines to _
  // The below algorithm is supposedly faster than using iterators
  NS_NAMED_LITERAL_STRING(replSubstr, "_");
  const char* replaceStr = " \n\r";

  PRInt32 x;
  for (x=0; x < (PRInt32)strlen(replaceStr); x++) {
    PRUnichar uChar = replaceStr[x];

    PRInt32 i = 0;
    while ((i = aPrinterName.FindChar(uChar, i)) != kNotFound) {
      aPrinterName.Replace(i, 1, replSubstr);
      i++;
    }
  }
}

NS_IMETHODIMP
nsPrintOptions::GetPrinterPrefInt(nsIPrintSettings *aPrintSettings,
                                  const PRUnichar *aPrefName, PRInt32 *_retval)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPrintSettings);
  NS_ENSURE_ARG_POINTER(aPrefName);

  nsAutoString prtName;
  // Get the Printer Name from the PrintSettings
  // to use as a prefix for Pref Names
  GetAdjustedPrinterName(aPrintSettings, PR_TRUE, prtName);

  const char* prefName =
    GetPrefName(NS_LossyConvertUCS2toASCII(aPrefName).get(), prtName);

  NS_ENSURE_TRUE(prefName, NS_ERROR_FAILURE);

  PRInt32 iVal;
  nsresult rv = mPrefBranch->GetIntPref(prefName, &iVal);
  NS_ENSURE_SUCCESS(rv, rv);

  *_retval = iVal;
  return rv;
}

NS_IMETHODIMP 
nsPrintOptions::InitPrintSettingsFromPrefs(nsIPrintSettings* aPS,
                                           PRBool aUsePNP, PRUint32 aFlags)
{
  NS_ENSURE_ARG_POINTER(aPS);

  PRBool isInitialized;
  aPS->GetIsInitializedFromPrefs(&isInitialized);

  if (isInitialized)
    return NS_OK;

  nsAutoString prtName;
  // read any non printer specific prefs
  // with empty printer name
  nsresult rv = ReadPrefs(aPS, prtName, aFlags);
  NS_ENSURE_SUCCESS(rv, rv);

  // Get the Printer Name from the PrintSettings
  // to use as a prefix for Pref Names
  GetAdjustedPrinterName(aPS, aUsePNP, prtName);
  NS_ENSURE_FALSE(prtName.IsEmpty(), NS_OK);

  // Now read any printer specific prefs
  rv = ReadPrefs(aPS, prtName, aFlags);
  if (NS_SUCCEEDED(rv))
    aPS->SetIsInitializedFromPrefs(PR_TRUE);

  return NS_OK;
}

/** ---------------------------------------------------
 *  This will save into prefs most all the PrintSettings either generically (not
 *  specified printer) or to a specific printer.
 */
nsresult
nsPrintOptions::SavePrintSettingsToPrefs(nsIPrintSettings *aPS,
                                         PRBool aUsePrinterNamePrefix,
                                         PRUint32 aFlags)
{
  NS_ENSURE_ARG_POINTER(aPS);
  nsAutoString prtName;

  // Get the Printer Name from the PrinterSettings
  // to use as a prefix for Pref Names
  GetAdjustedPrinterName(aPS, aUsePrinterNamePrefix, prtName);
  NS_ENSURE_FALSE(prtName.IsEmpty(), NS_ERROR_FAILURE);

  // Now write any printer specific prefs
  return WritePrefs(aPS, prtName, aFlags);
}


//-----------------------------------------------------
//-- Protected Methods --------------------------------
//-----------------------------------------------------
nsresult
nsPrintOptions::ReadPrefString(const char * aPrefId, nsAString& aString)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPrefId);

  nsXPIDLCString str;
  nsresult rv = mPrefBranch->GetCharPref(aPrefId, getter_Copies(str));
  NS_ENSURE_SUCCESS(rv, rv);

  CopyUTF8toUTF16(str.get(), aString);

  return rv;
}

nsresult
nsPrintOptions::WritePrefString(PRUnichar*& aStr, const char* aPrefId)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aStr);
  NS_ENSURE_ARG_POINTER(aPrefId);

  nsresult rv = mPrefBranch->SetCharPref(aPrefId,
                                         NS_ConvertUTF16toUTF8(aStr).get());

  nsMemory::Free(aStr);
  aStr = nsnull;
  return rv;
}

nsresult
nsPrintOptions::WritePrefString(const char * aPrefId, const nsAString& aString)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPrefId);

  return mPrefBranch->SetCharPref(aPrefId,
                                  NS_ConvertUTF16toUTF8(aString).get());
}

nsresult
nsPrintOptions::ReadPrefDouble(const char * aPrefId, double& aVal)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPrefId);

  char * str;
  nsresult rv = mPrefBranch->GetCharPref(aPrefId, &str);
  if (NS_SUCCEEDED(rv) && str) {
    aVal = atof(str);
    nsMemory::Free(str);
  }
  return rv;
}

nsresult
nsPrintOptions::WritePrefDouble(const char * aPrefId, double aVal)
{
  NS_ENSURE_STATE(mPrefBranch);
  NS_ENSURE_ARG_POINTER(aPrefId);

  char str[16]; // max 9 chars in below snprintf(), 16 will do nicely
  int ret = snprintf(str, sizeof(str), "%6.2f", aVal);
  NS_ENSURE_TRUE(ret >= 0, NS_ERROR_FAILURE);

  return mPrefBranch->SetCharPref(aPrefId, str);
}

void
nsPrintOptions::ReadInchesToTwipsPref(const char * aPrefId, nscoord& aTwips,
                                      const char * aMarginPref)
{
  if (!mPrefBranch) {
    return;
  }

  char * str = nsnull;
  nsresult rv = mPrefBranch->GetCharPref(aPrefId, &str);
  if (NS_FAILED(rv) || !str)
    rv = mPrefBranch->GetCharPref(aMarginPref, &str);
  if (NS_SUCCEEDED(rv) && str) {
    nsAutoString justStr;
    justStr.AssignWithConversion(str);
    PRInt32 errCode;
    float inches = justStr.ToFloat(&errCode);
    if (NS_SUCCEEDED(errCode)) {
      aTwips = NS_INCHES_TO_TWIPS(inches);
    } else {
      aTwips = 0;
    }
    nsMemory::Free(str);
  }
}

void
nsPrintOptions::WriteInchesFromTwipsPref(const char * aPrefId, nscoord aTwips)
{
  if (!mPrefBranch) {
    return;
  }

  double inches = NS_TWIPS_TO_INCHES(aTwips);
  nsCAutoString inchesStr;
  inchesStr.AppendFloat(inches);

  mPrefBranch->SetCharPref(aPrefId, inchesStr.get());
}

void
nsPrintOptions::ReadJustification(const char * aPrefId, PRInt16& aJust,
                                  PRInt16 aInitValue)
{
  aJust = aInitValue;
  nsAutoString justStr;
  if (NS_SUCCEEDED(ReadPrefString(aPrefId, justStr))) {
    if (justStr.EqualsASCII(kJustRight)) {
      aJust = nsIPrintSettings::kJustRight;

    } else if (justStr.EqualsASCII(kJustCenter)) {
      aJust = nsIPrintSettings::kJustCenter;

    } else {
      aJust = nsIPrintSettings::kJustLeft;
    }
  }
}

//---------------------------------------------------
void
nsPrintOptions::WriteJustification(const char * aPrefId, PRInt16 aJust)
{
  switch (aJust) {
    case nsIPrintSettings::kJustLeft:
      mPrefBranch->SetCharPref(aPrefId, kJustLeft);
      break;

    case nsIPrintSettings::kJustCenter:
      mPrefBranch->SetCharPref(aPrefId, kJustCenter);
      break;

    case nsIPrintSettings::kJustRight:
      mPrefBranch->SetCharPref(aPrefId, kJustRight);
      break;
  } //switch
}

//----------------------------------------------------------------------
// Testing of read/write prefs
// This define turns on the testing module below
// so at start up it writes and reads the prefs.
#ifdef DEBUG_rods_X
class Tester {
  public:
    Tester();
};
Tester::Tester()
{
  nsCOMPtr<nsIPrintSettings> ps;
  nsresult rv;
  nsCOMPtr<nsIPrintOptions> printService =
      do_GetService("@mozilla.org/gfx/printsettings-service;1", &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = printService->CreatePrintSettings(getter_AddRefs(ps));
  }

  if (ps) {
    ps->SetPrintOptions(nsIPrintSettings::kPrintOddPages,  PR_TRUE);
    ps->SetPrintOptions(nsIPrintSettings::kPrintEvenPages,  PR_FALSE);
    ps->SetMarginTop(1.0);
    ps->SetMarginLeft(1.0);
    ps->SetMarginBottom(1.0);
    ps->SetMarginRight(1.0);
    ps->SetScaling(0.5);
    ps->SetPrintBGColors(PR_TRUE);
    ps->SetPrintBGImages(PR_TRUE);
    ps->SetPrintRange(15);
    ps->SetHeaderStrLeft(NS_ConvertUTF8toUCS2("Left").get());
    ps->SetHeaderStrCenter(NS_ConvertUTF8toUCS2("Center").get());
    ps->SetHeaderStrRight(NS_ConvertUTF8toUCS2("Right").get());
    ps->SetFooterStrLeft(NS_ConvertUTF8toUCS2("Left").get());
    ps->SetFooterStrCenter(NS_ConvertUTF8toUCS2("Center").get());
    ps->SetFooterStrRight(NS_ConvertUTF8toUCS2("Right").get());
    ps->SetPaperName(NS_ConvertUTF8toUCS2("Paper Name").get());
    ps->SetPlexName(NS_ConvertUTF8toUCS2("Plex Name").get());
    ps->SetPaperSizeType(10);
    ps->SetPaperData(1);
    ps->SetPaperWidth(100.0);
    ps->SetPaperHeight(50.0);
    ps->SetPaperSizeUnit(nsIPrintSettings::kPaperSizeMillimeters);
    ps->SetPrintReversed(PR_TRUE);
    ps->SetPrintInColor(PR_TRUE);
    ps->SetPaperSize(5);
    ps->SetOrientation(nsIPrintSettings::kLandscapeOrientation);
    ps->SetPrintCommand(NS_ConvertUTF8toUCS2("Command").get());
    ps->SetNumCopies(2);
    ps->SetPrinterName(NS_ConvertUTF8toUCS2("Printer Name").get());
    ps->SetPrintToFile(PR_TRUE);
    ps->SetToFileName(NS_ConvertUTF8toUCS2("File Name").get());
    ps->SetPrintPageDelay(1000);
    ps->SetShrinkToFit(PR_TRUE);

    struct SettingsType {
      const char* mName;
      PRUint32    mFlag;
    };
    SettingsType gSettings[] = {
      {"OddEven", nsIPrintSettings::kInitSaveOddEvenPages},
      {kPrintHeaderStrLeft, nsIPrintSettings::kInitSaveHeaderLeft},
      {kPrintHeaderStrCenter, nsIPrintSettings::kInitSaveHeaderCenter},
      {kPrintHeaderStrRight, nsIPrintSettings::kInitSaveHeaderRight},
      {kPrintFooterStrLeft, nsIPrintSettings::kInitSaveFooterLeft},
      {kPrintFooterStrCenter, nsIPrintSettings::kInitSaveFooterCenter},
      {kPrintFooterStrRight, nsIPrintSettings::kInitSaveFooterRight},
      {kPrintBGColors, nsIPrintSettings::kInitSaveBGColors},
      {kPrintBGImages, nsIPrintSettings::kInitSaveBGImages},
      {kPrintShrinkToFit, nsIPrintSettings::kInitSaveShrinkToFit},
      {kPrintPaperSize, nsIPrintSettings::kInitSavePaperSize},
      {kPrintPaperName, nsIPrintSettings::kInitSavePaperName},
      {kPrintPlexName, nsIPrintSettings::kInitSavePlexName},
      {kPrintPaperSizeUnit, nsIPrintSettings::kInitSavePaperSizeUnit},
      {kPrintPaperSizeType, nsIPrintSettings::kInitSavePaperSizeType},
      {kPrintPaperData, nsIPrintSettings::kInitSavePaperData},
      {kPrintPaperWidth, nsIPrintSettings::kInitSavePaperWidth},
      {kPrintPaperHeight, nsIPrintSettings::kInitSavePaperHeight},
      {kPrintReversed, nsIPrintSettings::kInitSaveReversed},
      {kPrintInColor, nsIPrintSettings::kInitSaveInColor},
      {kPrintColorspace, nsIPrintSettings::kInitSaveColorspace},
      {kPrintResolutionName, nsIPrintSettings::kInitSaveResolutionName},
      {kPrintDownloadFonts, nsIPrintSettings::kInitSaveDownloadFonts},
      {kPrintOrientation, nsIPrintSettings::kInitSaveOrientation},
      {kPrintCommand, nsIPrintSettings::kInitSavePrintCommand},
      {kPrinterName, nsIPrintSettings::kInitSavePrinterName},
      {kPrintToFile, nsIPrintSettings::kInitSavePrintToFile},
      {kPrintToFileName, nsIPrintSettings::kInitSaveToFileName},
      {kPrintPageDelay, nsIPrintSettings::kInitSavePageDelay},
      {"Margins", nsIPrintSettings::kInitSaveMargins},
      {"All", nsIPrintSettings::kInitSaveAll},
      {nsnull, 0}};

      nsString prefix; prefix.AssignLiteral("Printer Name");
      PRInt32 i = 0;
      while (gSettings[i].mName != nsnull) {
        printf("------------------------------------------------\n");
        printf("%d) %s -> 0x%X\n", i, gSettings[i].mName, gSettings[i].mFlag);
        printService->SavePrintSettingsToPrefs(ps, PR_TRUE, gSettings[i].mFlag);
        printService->InitPrintSettingsFromPrefs(ps, PR_TRUE,
                                                 gSettings[i].mFlag);
        i++;
      }
  }

}
Tester gTester;
#endif
