/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "nsISupports.h"
#include "nscore.h"
#include "nsString.h"
#include "nsILocale.h"
#include "nsPosixLocale.h"
#include "nsLocaleCID.h"
#include "prprf.h"
#include "nsFileSpec.h"

NS_DEFINE_IID(kIPosixLocaleIID, NS_IPOSIXLOCALE_IID);
NS_DEFINE_IID(kPosixLocaleCID, NS_POSIXLOCALE_CID);

/* nsPosixLocale ISupports */
NS_IMPL_ISUPPORTS(nsPosixLocale,kIPosixLocaleIID)

nsPosixLocale::nsPosixLocale(void)
{
  NS_INIT_REFCNT();
}

nsPosixLocale::~nsPosixLocale(void)
{

}

NS_IMETHODIMP 
nsPosixLocale::GetPlatformLocale(const nsString* locale,char* posixLocale, size_t length)
{
  char  country_code[MAX_COUNTRY_CODE_LEN+1];
  char  lang_code[MAX_LANGUAGE_CODE_LEN+1];
  char  extra[MAX_EXTRA_LEN+1];
  char  posix_locale[MAX_LOCALE_LEN+1];
  nsAutoCString xp_locale(*locale);

  if ((const char *)xp_locale!=nsnull) {
    if (!ParseLocaleString((const char *)xp_locale,lang_code,country_code,extra,'-')) {
//      strncpy(posixLocale,"C",length);
      PL_strncpyz(posixLocale,(const char *)xp_locale,length);  // use xp locale if parse failed
      return NS_OK;
    }

    if (*country_code) {
      if (*extra) {
        PR_snprintf(posix_locale,sizeof(posix_locale),"%s_%s.%s",lang_code,country_code,extra);
      }
      else {
        PR_snprintf(posix_locale,sizeof(posix_locale),"%s_%s",lang_code,country_code);
      }
    }
    else {
      if (*extra) {
        PR_snprintf(posix_locale,sizeof(posix_locale),"%s.%s",lang_code,extra);
      }
      else {
        PR_snprintf(posix_locale,sizeof(posix_locale),"%s",lang_code);
      }
    }

    strncpy(posixLocale,posix_locale,length);
    return NS_OK;
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsPosixLocale::GetXPLocale(const char* posixLocale, nsString* locale)
{
  char  country_code[MAX_COUNTRY_CODE_LEN+1];
  char  lang_code[MAX_LANGUAGE_CODE_LEN+1];
  char  extra[MAX_EXTRA_LEN+1];
  char  posix_locale[MAX_LOCALE_LEN+1];

  if (posixLocale!=nsnull) {
    if (strcmp(posixLocale,"C")==0 || strcmp(posixLocale,"POSIX")==0) {
      locale->AssignWithConversion("en-US");
      return NS_OK;
    }
    if (!ParseLocaleString(posixLocale,lang_code,country_code,extra,'_')) {
//      * locale = "x-user-defined";
      locale->AssignWithConversion(posixLocale);  // use posix if parse failed
      return NS_OK;
    }

    if (*country_code) {
      PR_snprintf(posix_locale,sizeof(posix_locale),"%s-%s",lang_code,country_code);
    } 
    else {
      PR_snprintf(posix_locale,sizeof(posix_locale),"%s",lang_code);
    }

    locale->AssignWithConversion(posix_locale);
    return NS_OK;

  }

    return NS_ERROR_FAILURE;

}

//
// returns PR_FALSE/PR_TRUE depending on if it was of the form LL-CC.Extra
PRBool
nsPosixLocale::ParseLocaleString(const char* locale_string, char* language, char* country, char* extra, char separator)
{
  const char *src = locale_string;
  char *dest;
  int dest_space, len;

  *language = '\0';
  *country = '\0';
  *extra = '\0';
  if (strlen(locale_string) < 2) {
    return(PR_FALSE);
  }

  //
  // parse the language part
  //
  dest = language;
  dest_space = MAX_LANGUAGE_CODE_LEN;
  while ((*src) && (isalpha(*src)) && (dest_space--)) {
    *dest++ = tolower(*src++);
  }
  *dest = '\0';
  len = dest - language;
  if ((len != 2) && (len != 3)) {
    NS_ASSERTION((len == 2) || (len == 3), "language code too short");
    NS_ASSERTION(len < 3, "reminder: verify we can handle 3+ character language code in all parts of the system; eg: language packs");
    *language = '\0';
    return(PR_FALSE);
  }

  // check if all done
  if (*src == '\0') {
    return(PR_TRUE);
  }

  if ((*src != '_') && (*src != '-') && (*src != '.')) {
    NS_ASSERTION(isalpha(*src), "language code too long");
    NS_ASSERTION(!isalpha(*src), "unexpected language/country separator");
    *language = '\0';
    return(PR_FALSE);
  }

  //
  // parse the country part
  //
  if ((*src == '_') || (*src == '-')) { 
    src++;
    dest = country;
    dest_space = MAX_COUNTRY_CODE_LEN;
    while ((*src) && (isalpha(*src)) && (dest_space--)) {
      *dest++ = toupper(*src++);
    }
    *dest = '\0';
    len = dest - country;
    if (len != 2) {
      NS_ASSERTION(len == 2, "unexpected country code length");
      *language = '\0';
      *country = '\0';
      return(PR_FALSE);
    }
  }

  // check if all done
  if (*src == '\0') {
    return(PR_TRUE);
  }

  if (*src != '.') {
    NS_ASSERTION(isalpha(*src), "country code too long");
    NS_ASSERTION(!isalpha(*src), "unexpected country/extra separator");
    *language = '\0';
    *country = '\0';
    return(PR_FALSE);
  }

  //
  // handle the extra part
  //
  src++;  // move past the extra part separator
  dest = extra;
  dest_space = MAX_EXTRA_LEN;
  while ((*src) && (dest_space--)) {
    *dest++ = *src++;
  }
  *dest = '\0';
  len = dest - extra;
  if (len < 1) {
    NS_ASSERTION(len > 0, "found country/extra separator but no extra code");
    *language = '\0';
    *country = '\0';
    *extra = '\0';
    return(PR_FALSE);
  }

  // check if all done
  if (*src == '\0') {
    return(PR_TRUE);
  }

  NS_ASSERTION(*src == '\0', "extra code too long");
  *language = '\0';
  *country = '\0';
  *extra = '\0';

  return(PR_FALSE);
}

