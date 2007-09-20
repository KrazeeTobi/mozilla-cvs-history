/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * The Original Code is mozilla.org Code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsCOMPtr.h"
#include "nsEudoraAddress.h"

#include "nsIAddrDatabase.h"
#include "mdb.h"
#include "nsAbBaseCID.h"
#include "nsIAbCard.h"
#include "nsIServiceManager.h"
#include "nsEudoraImport.h"
#include "nsReadableUtils.h"
#include "nsMsgI18N.h"
#include "nsNativeCharsetUtils.h"
#include "nsTextFormatter.h"
#include "nsEudoraStringBundle.h"
#include "nsIStringBundle.h"
#include "nsNetUtil.h"
#include "nsILineInputStream.h"
#include "EudoraDebugLog.h"

#define  kWhitespace  " \t\b\r\n"

#define ADD_FIELD_TO_DB_ROW(pdb, func, dbRow, val, uniStr)    \
  if (!val.IsEmpty())                                         \
  {                                                           \
        NS_CopyNativeToUnicode(val, uniStr);                  \
        pdb->func(dbRow, NS_ConvertUTF16toUTF8(uniStr).get());\
  }


// If we get a line longer than 16K it's just toooooo bad!
#define kEudoraAddressBufferSz  (16 * 1024)


#ifdef IMPORT_DEBUG
void DumpAliasArray( nsVoidArray& a);
#endif

class CAliasData {
public:
  CAliasData() {}
  ~CAliasData() {}

  PRBool Process( const char *pLine, PRInt32 len);

public:
    nsCString   m_fullEntry;
    nsCString   m_nickName;
    nsCString   m_realName;
    nsCString   m_email;
};

class CAliasEntry {
public:
  CAliasEntry( nsCString& name) { m_name = name;}
  ~CAliasEntry() { EmptyList();}

  void EmptyList( void) {
    CAliasData *pData;
    for (PRInt32 i = 0; i < m_list.Count(); i++) {
      pData = (CAliasData *)m_list.ElementAt( i);
      delete pData;
    }
    m_list.Clear();
  }

public:
  nsCString  m_name;
  nsVoidArray  m_list;
  nsCString  m_notes;
};

nsEudoraAddress::nsEudoraAddress()
{
}

nsEudoraAddress::~nsEudoraAddress()
{
  EmptyAliases();
}


nsresult nsEudoraAddress::ImportAddresses( PRUint32 *pBytes, PRBool *pAbort,
                                          const PRUnichar *pName, nsIFile *pSrc,
                                          nsIAddrDatabase *pDb, nsString& errors)
{
  // Open the source file for reading, read each line and process it!

  EmptyAliases();
  nsCOMPtr<nsIInputStream> inputStream;
  nsresult rv = NS_NewLocalFileInputStream(getter_AddRefs(inputStream), pSrc);
  if (NS_FAILED(rv)) {
    IMPORT_LOG0( "*** Error opening address file for reading\n");
    return rv;
  }

  PRUint32 bytesLeft = 0;

  rv = inputStream->Available(&bytesLeft);
  if (NS_FAILED(rv)) {
    IMPORT_LOG0( "*** Error checking address file for eof\n");
    inputStream->Close();
    return rv;
  }

  nsCOMPtr<nsILineInputStream> lineStream(do_QueryInterface(inputStream, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool more = PR_TRUE;

  while ((!(*pAbort) && more && NS_SUCCEEDED( rv)))
  {
    nsCString line;
    rv = lineStream->ReadLine(line, &more);
    if (NS_SUCCEEDED( rv))
    {
      PRInt32  len = line.Length();
      ProcessLine( line.get(), len, errors);
      if (pBytes)
        *pBytes += (len / 2);
    }
  }
  rv = inputStream->Close();

  if (!more)
  {
    IMPORT_LOG0( "*** Error reading the address book, didn't reach the end\n");
    return( NS_ERROR_FAILURE);
  }
  // Run through the alias array and make address book entries...
#ifdef IMPORT_DEBUG
  DumpAliasArray( m_alias);
#endif

  BuildABCards( pBytes, pDb);

  return pDb->Commit(nsAddrDBCommitType::kLargeCommit);
  }


PRInt32 nsEudoraAddress::CountWhiteSpace( const char *pLine, PRInt32 len)
{
  PRInt32    cnt = 0;
  while (len && ((*pLine == ' ') || (*pLine == '\t'))) {
    len--;
    pLine++;
    cnt++;
  }

  return( cnt);
}

void nsEudoraAddress::EmptyAliases( void)
{
  CAliasEntry *pData;
  for (PRInt32 i = 0; i < m_alias.Count(); i++) {
    pData = (CAliasEntry *)m_alias.ElementAt( i);
    delete pData;
  }
  m_alias.Clear();
}

void nsEudoraAddress::ProcessLine( const char *pLine, PRInt32 len, nsString& errors)
{
  if (len < 6)
    return;

  PRInt32  cnt;
  CAliasEntry  *pEntry;

  if (!strncmp( pLine, "alias", 5)) {
    pLine += 5;
    len -= 5;
    cnt = CountWhiteSpace( pLine, len);
    if (cnt) {
      pLine += cnt;
      len -= cnt;
      if ((pEntry = ProcessAlias( pLine, len, errors)) != nsnull) {
        m_alias.AppendElement( pEntry);
      }
    }
  }
  else if (!strncmp( pLine, "note", 4)) {
    pLine += 4;
    len -= 4;
    cnt = CountWhiteSpace( pLine, len);
    if (cnt) {
      pLine += cnt;
      len -= cnt;
      ProcessNote( pLine, len, errors);
    }
  }

  // as far as I know everything must be on one line
  // if not, then I need to add a state variable.
}

PRInt32 nsEudoraAddress::GetAliasName( const char *pLine, PRInt32 len, nsCString& name)
{
  name.Truncate();
  if (!len)
    return( 0);
  const char *pStart = pLine;
  char  end[2] = {' ', '\t'};
  if (*pLine == '"') {
    pLine++;
    pStart++;
    len--;
    end[0] = '"';
    end[1] = 0;
  }

  PRInt32 cnt = 0;
  while (len) {
    if ((*pLine == end[0]) || (*pLine == end[1]))
      break;
    len--;
    pLine++;
    cnt++;
  }

  if (cnt)
    name.Append( pStart, cnt);

  if (end[0] == '"') {
    cnt++;
    if (len && (*pLine == '"')) {
      cnt++;
      pLine++;
      len--;
    }
  }

  cnt += CountWhiteSpace( pLine, len);

  return( cnt);
}


CAliasEntry *nsEudoraAddress::ProcessAlias( const char *pLine, PRInt32 len, nsString& errors)
{
  nsCString  name;
  PRInt32    cnt = GetAliasName( pLine, len, name);
  pLine += cnt;
  len -= cnt;

  // we have 3 known forms of addresses in Eudora
  // 1) real name <email@address>
  // 2) email@address
  // 3) <email@address>
  // 4) real name email@address
  // 5) <email@address> (Real name)

  CAliasEntry *pEntry = new CAliasEntry( name);
  if (!cnt || !len)
    return(pEntry);

  // Theoretically, an alias is just an RFC822 email adress, but it may contain
  // an alias to another alias as the email!  I general, it appears close
  // but unfortunately not exact so we can't use the nsIMsgHeaderParser to do
  // the work for us!

  // Very big bummer!

  const char *pStart;
  PRInt32    tLen;
  nsCString  alias;

  while ( len) {
    pStart = pLine;
    cnt = 0;
    while (len && (*pLine != ',')) {
      if (*pLine == '"') {
        tLen = CountQuote( pLine, len);
        pLine += tLen;
        len -= tLen;
        cnt += tLen;
      }
      else if (*pLine == '(') {
        tLen = CountComment( pLine, len);
        pLine += tLen;
        len -= tLen;
        cnt += tLen;
      }
      else if (*pLine == '<') {
        tLen = CountAngle( pLine, len);
        pLine += tLen;
        len -= tLen;
        cnt += tLen;
      }
      else {
        cnt++;
        pLine++;
        len--;
      }
    }
    if (cnt) {
      CAliasData *pData = new CAliasData();
      if (pData->Process( pStart, cnt)) {
        pEntry->m_list.AppendElement( pData);
      }
      else
        delete pData;
    }

    if (len && (*pLine == ',')) {
      pLine++;
      len--;
    }
  }

  // Always return the entry even if there's no other attribute associated with the contact.
  return( pEntry);
}


void nsEudoraAddress::ProcessNote( const char *pLine, PRInt32 len, nsString& errors)
{
  nsCString  name;
  PRInt32    cnt = GetAliasName( pLine, len, name);
  pLine += cnt;
  len -= cnt;
  if (!cnt || !len)
    return;

  // Find the alias for this note and store the note data there!
  CAliasEntry *pEntry = nsnull;
  PRInt32  idx = FindAlias( name);
  if (idx == -1)
    return;

  pEntry = (CAliasEntry *) m_alias.ElementAt( idx);
  pEntry->m_notes.Append( pLine, len);
  pEntry->m_notes.Trim( kWhitespace);
}



PRInt32 nsEudoraAddress::CountQuote( const char *pLine, PRInt32 len)
{
  if (!len)
    return( 0);

  PRInt32 cnt = 1;
  pLine++;
  len--;

  while (len && (*pLine != '"')) {
    cnt++;
    len--;
    pLine++;
  }

  if (len)
    cnt++;
  return( cnt);
}


PRInt32 nsEudoraAddress::CountAngle( const char *pLine, PRInt32 len)
{
  if (!len)
    return( 0);

  PRInt32 cnt = 1;
  pLine++;
  len--;

  while (len && (*pLine != '>')) {
    cnt++;
    len--;
    pLine++;
  }

  if (len)
    cnt++;
  return( cnt);
}

PRInt32 nsEudoraAddress::CountComment( const char *pLine, PRInt32 len)
{
  if (!len)
    return( 0);

  PRInt32  cCnt;
  PRInt32 cnt = 1;
  pLine++;
  len--;

  while (len && (*pLine != ')')) {
    if (*pLine == '(') {
      cCnt = CountComment( pLine, len);
      cnt += cCnt;
      pLine += cCnt;
      len -= cCnt;
    }
    else {
      cnt++;
      len--;
      pLine++;
    }
  }

  if (len)
    cnt++;
  return( cnt);
}

/*
  nsCString  m_nickName;
  nsCString  m_realName;
  nsCString  m_email;
*/

PRBool CAliasData::Process( const char *pLine, PRInt32 len)
{
  // Extract any comments first!
  nsCString  str;

  const char *pStart = pLine;
  PRInt32    tCnt = 0;
  PRInt32    cnt = 0;
  PRInt32    max = len;
  PRBool    endCollect = PR_FALSE;

    // Keep track of the full entry without any processing for potential alias
    // nickname resolution. Previously alias resolution was done with m_email,
    // but unfortunately that doesn't work for nicknames with spaces.
    // For example for the nickname "Joe Smith", "Smith" was being interpreted
    // as the potential email address and placed in m_email, but routines like
    // ResolveAlias were failing because "Smith" is not the full nickname.
    // Now we just stash the full entry for nickname resolution before processing
    // the line as a potential entry in its own right.
    m_fullEntry.Append(pLine, len);

  while (max) {
    if (*pLine == '"') {
      if (tCnt && !endCollect) {
        str.Trim( kWhitespace);
        if (!str.IsEmpty())
          str.Append( " ", 1);
        str.Append( pStart, tCnt);
      }
      cnt = nsEudoraAddress::CountQuote( pLine, max);
      if ((cnt > 2) && m_realName.IsEmpty()) {
        m_realName.Append( pLine + 1, cnt - 2);
      }
      pLine += cnt;
      max -= cnt;
      pStart = pLine;
      tCnt = 0;
    }
    else if (*pLine == '<') {
      if (tCnt && !endCollect) {
        str.Trim( kWhitespace);
        if (!str.IsEmpty())
          str.Append( " ", 1);
        str.Append( pStart, tCnt);
      }
      cnt = nsEudoraAddress::CountAngle( pLine, max);
      if ((cnt > 2) && m_email.IsEmpty()) {
        m_email.Append( pLine + 1, cnt - 2);
      }
      pLine += cnt;
      max -= cnt;
      pStart = pLine;
      tCnt = 0;
      endCollect = PR_TRUE;
    }
    else if (*pLine == '(') {
      if (tCnt && !endCollect) {
        str.Trim( kWhitespace);
        if (!str.IsEmpty())
          str.Append( " ", 1);
        str.Append( pStart, tCnt);
      }
      cnt = nsEudoraAddress::CountComment( pLine, max);
      if (cnt > 2) {
        if (!m_realName.IsEmpty() && m_nickName.IsEmpty())
          m_nickName = m_realName;
        m_realName.Truncate();
        m_realName.Append( pLine + 1, cnt - 2);
      }
      pLine += cnt;
      max -= cnt;
      pStart = pLine;
      tCnt = 0;
    }
    else {
      tCnt++;
      pLine++;
      max--;
    }
  }

  if (tCnt) {
    str.Trim( kWhitespace);
    if (!str.IsEmpty())
      str.Append( " ", 1);
    str.Append( pStart, tCnt);
  }

  str.Trim( kWhitespace);

  if (!m_realName.IsEmpty() && !m_email.IsEmpty())
    return( PR_TRUE);

  // now we should have a string with any remaining non-delimitted text
  // we assume that the last token is the email
  // anything before that is realName
  if (!m_email.IsEmpty()) {
    m_realName = str;
    return( PR_TRUE);
  }

  tCnt = str.RFindChar( ' ');
  if (tCnt == -1) {
    if (!str.IsEmpty()) {
      m_email = str;
      return( PR_TRUE);
    }
    return( PR_FALSE);
  }

  str.Right( m_email, str.Length() - tCnt - 1);
  str.Left( m_realName, tCnt);
  m_realName.Trim( kWhitespace);
  m_email.Trim( kWhitespace);

  return( !m_email.IsEmpty());
}

#ifdef IMPORT_DEBUG
void DumpAliasArray( nsVoidArray& a)
{
  CAliasEntry *pEntry;
  CAliasData *pData;

  PRInt32 cnt = a.Count();
  IMPORT_LOG1( "Alias list size: %ld\n", cnt);
  for (PRInt32 i = 0; i < cnt; i++) {
    pEntry = (CAliasEntry *)a.ElementAt( i);
    IMPORT_LOG1( "\tAlias: %s\n", pEntry->m_name.get());
    if (pEntry->m_list.Count() > 1) {
      IMPORT_LOG1( "\tList count #%ld\n", pEntry->m_list.Count());
      for (PRInt32 j = 0; j < pEntry->m_list.Count(); j++) {
        pData = (CAliasData *) pEntry->m_list.ElementAt( j);
        IMPORT_LOG0( "\t\t--------\n");
        IMPORT_LOG1( "\t\temail: %s\n", pData->m_email.get());
        IMPORT_LOG1( "\t\trealName: %s\n", pData->m_realName.get());
        IMPORT_LOG1( "\t\tnickName: %s\n", pData->m_nickName.get());
      }
    }
    else if (pEntry->m_list.Count()) {
      pData = (CAliasData *) pEntry->m_list.ElementAt( 0);
      IMPORT_LOG1( "\t\temail: %s\n", pData->m_email.get());
      IMPORT_LOG1( "\t\trealName: %s\n", pData->m_realName.get());
      IMPORT_LOG1( "\t\tnickName: %s\n", pData->m_nickName.get());
    }
  }
}
#endif

CAliasEntry *nsEudoraAddress::ResolveAlias( nsCString& name)
{
  PRInt32  max = m_alias.Count();
  CAliasEntry *pEntry;
  for (PRInt32 i = 0; i < max; i++) {
    pEntry = (CAliasEntry *) m_alias.ElementAt( i);
    if (name.Equals( pEntry->m_name, nsCaseInsensitiveCStringComparator()))
      return( pEntry);
  }

  return( nsnull);
}

void nsEudoraAddress::ResolveEntries( nsCString& name, nsVoidArray& list,
                                     nsVoidArray& result, PRBool addResolvedEntries,
                                     PRBool wasResolved, PRInt32& numResolved)
{
    /* a safe-guard against recursive entries */
    if (result.Count() > m_alias.Count())
        return;

    PRInt32         max = list.Count();
    PRInt32         i;
    CAliasData *    pData;
    CAliasEntry *   pEntry;
    for (i = 0; i < max; i++) {
        pData = (CAliasData *)list.ElementAt( i);
        // resolve the email to an existing alias!
        if ( !name.Equals(pData->m_email, nsCaseInsensitiveCStringComparator()) &&
             ((pEntry = ResolveAlias( pData->m_fullEntry)) != nsnull) ) {
            // This new entry has all of the entries for this puppie.
            // Resolve all of it's entries!
            numResolved++;  // Track the number of entries resolved

            // We pass in PR_TRUE for the 5th parameter so that we know that we're
            // calling ourselves recursively.
            ResolveEntries( pEntry->m_name, pEntry->m_list, result, addResolvedEntries, PR_TRUE, numResolved);
        }
        else if (addResolvedEntries || !wasResolved) {
            // This is either an ordinary entry (i.e. just contains the info) or we were told
            // to add resolved alias entries.
            result.AppendElement( pData);
        }
    }
}

PRInt32 nsEudoraAddress::FindAlias( nsCString& name)
{
  CAliasEntry *  pEntry;
  PRInt32      max = m_alias.Count();
  PRInt32      i;

  for (i = 0; i < max; i++) {
    pEntry = (CAliasEntry *) m_alias.ElementAt( i);
    if (pEntry->m_name == name)
      return( i);
  }

  return( -1);
}

void nsEudoraAddress::BuildABCards( PRUint32 *pBytes, nsIAddrDatabase *pDb)
{
  CAliasEntry *  pEntry;
  PRInt32      max = m_alias.Count();
  PRInt32      i;
  nsVoidArray    emailList;
  nsVoidArray membersArray;// Remember group members.
  nsVoidArray groupsArray; // Remember groups.

  // First off, run through the list and build person cards - groups/lists have to be done later
  for (i = 0; i < max; i++) {
    PRInt32   numResolved = 0;
    pEntry = (CAliasEntry *) m_alias.ElementAt( i);

    // PR_FALSE for 4th parameter tells ResolveEntries not to add resolved entries (avoids
    // duplicates as mailing lists are being resolved to other cards - the other cards that
    // are found have already been added and don't need to be added again).
    //
    // PR_FALSE for 5th parameter tells ResolveEntries that we're calling it - it's not being
    // called recursively by itself.
    ResolveEntries( pEntry->m_name, pEntry->m_list, emailList, PR_FALSE, PR_FALSE, numResolved);

    // Treat it as a group if there's more than one email address or if we
    // needed to resolve one or more aliases. We treat single aliases to
    // other aliases as a mailing list because there's no better equivalent.
    if ( (emailList.Count() > 1) || (numResolved > 0) )
    {
      // Remember group members uniquely and add them to db later.
      RememberGroupMembers(membersArray, emailList);
      // Remember groups and add them to db later.
      groupsArray.AppendElement(pEntry);
    }
    else
      AddSingleCard( pEntry, emailList, pDb);

    emailList.Clear();

    if (pBytes) {
      // This isn't exact but it will get us close enough
      *pBytes += (pEntry->m_name.Length() + pEntry->m_notes.Length() + 10);
    }
  }

  // Make sure group members exists before adding groups.
  nsresult rv = AddGroupMembersAsCards(membersArray, pDb);
  if (NS_FAILED(rv))
    return;

  // Now add the lists/groups (now that all cards have been added).
  max = groupsArray.Count();
  for (i = 0; i < max; i++)
  {
    PRInt32   numResolved = 0;
    pEntry = (CAliasEntry *) groupsArray.ElementAt(i);

    // PR_FALSE for 4th parameter tells ResolveEntries to add resolved entries so that we
    // can create the mailing list with references to all entries correctly.
    //
    // PR_FALSE for 5th parameter tells ResolveEntries that we're calling it - it's not being
    // called recursively by itself.
    ResolveEntries( pEntry->m_name, pEntry->m_list, emailList, PR_TRUE, PR_FALSE, numResolved);
    AddSingleList(pEntry, emailList, pDb);
    emailList.Clear();
  }
}

void nsEudoraAddress::ExtractNoteField( nsCString& note, nsCString& value, const char *pFieldName)
{
  value.Truncate();
  nsCString field("<");
  field.Append( pFieldName);
  field.Append( ':');

/*
    this is a bit of a cheat, but there's no reason it won't work
    fine for us, even better than Eudora in some cases!
*/

  PRInt32 idx = note.Find( field);
  if (idx != -1) {
    idx += field.Length();
    PRInt32 endIdx = note.FindChar( '>', idx);
    if (endIdx == -1)
      endIdx = note.Length() - 1;
    note.Mid( value, idx, endIdx - idx);
    idx -= field.Length();
    nsCString tempL;
    if (idx)
      note.Left( tempL, idx);
    nsCString tempR;
    note.Right( tempR, note.Length() - endIdx - 1);
    note = tempL;
    note.Append( tempR);
  }
}

void nsEudoraAddress::FormatExtraDataInNoteField(PRInt32 labelStringID, nsIStringBundle* bundle, nsCString& extraData, nsString& noteUTF16)
{
  nsAutoString    label;
  nsEudoraStringBundle::GetStringByID(labelStringID, label, bundle);

  noteUTF16.Append(label);
  noteUTF16.AppendLiteral("\n");
  noteUTF16.Append( NS_ConvertASCIItoUTF16(extraData) );
  noteUTF16.AppendLiteral("\n\n");
}

void nsEudoraAddress::SanitizeValue( nsCString& val)
{
  val.ReplaceSubstring( "\n", ", ");
  val.ReplaceChar( 13, ',');
  val.ReplaceChar( 10, ',');
}

void nsEudoraAddress::SplitString( nsCString& val1, nsCString& val2)
{
  nsCString  temp;

  // Find the last line if there is more than one!
  PRInt32 idx = val1.RFind( "\x0D\x0A");
  PRInt32  cnt = 2;
  if (idx == -1) {
    cnt = 1;
    idx = val1.RFindChar( 13);
  }
  if (idx == -1)
    idx= val1.RFindChar( 10);
  if (idx != -1) {
    val1.Right( val2, val1.Length() - idx - cnt);
    val1.Left( temp, idx);
    val1 = temp;
    SanitizeValue( val1);
  }
}

void nsEudoraAddress::AddSingleCard( CAliasEntry *pEntry, nsVoidArray &emailList, nsIAddrDatabase *pDb)
{
  // We always have a nickname and everything else is optional.
  // Map both home and work related fields to our address card. Eudora
  // fields that can't be mapped will be left in the 'note' field!
  nsIMdbRow* newRow = nsnull;
  pDb->GetNewRow( &newRow);
  if (!newRow)
    return;

  nsCString                   displayName, name, firstName, lastName;
  nsCString                   fax, secondaryFax, phone, mobile, secondaryMobile, webLink;
  nsCString                   address, address2, city, state, zip, country;
  nsCString                   phoneWK, webLinkWK, title, company;
  nsCString                   addressWK, address2WK, cityWK, stateWK, zipWK, countryWK;
  nsCString                   primaryLocation, otherPhone, otherWeb;
  nsCString                   additionalEmail, stillMoreEmail;
  nsCString                   note(pEntry->m_notes);
  nsString                    noteUTF16;
  PRBool                      isSecondaryMobileWorkNumber = PR_TRUE;
  PRBool                      isSecondaryFaxWorkNumber = PR_TRUE;
  nsCOMPtr<nsIStringBundle>   bundle( dont_AddRef(nsEudoraStringBundle::GetStringBundleProxy()) );

  if (!note.IsEmpty())
  {
    ExtractNoteField( note, fax, "fax");
    ExtractNoteField( note, secondaryFax, "fax2");
    ExtractNoteField( note, phone, "phone");
    ExtractNoteField( note, mobile, "mobile");
    ExtractNoteField( note, secondaryMobile, "mobile2");
    ExtractNoteField( note, address, "address");
    ExtractNoteField( note, city, "city");
    ExtractNoteField( note, state, "state");
    ExtractNoteField( note, zip, "zip");
    ExtractNoteField( note, country, "country");
    ExtractNoteField( note, name, "name");
    ExtractNoteField( note, firstName, "first");
    ExtractNoteField( note, lastName, "last");
    ExtractNoteField( note, webLink, "web");

    ExtractNoteField( note, addressWK, "address2");
    ExtractNoteField( note, cityWK, "city2");
    ExtractNoteField( note, stateWK, "state2");
    ExtractNoteField( note, zipWK, "zip2");
    ExtractNoteField( note, countryWK, "country2");
    ExtractNoteField( note, phoneWK, "phone2");
    ExtractNoteField( note, title, "title");
    ExtractNoteField( note, company, "company");
    ExtractNoteField( note, webLinkWK, "web2");

    ExtractNoteField( note, primaryLocation, "primary");
    ExtractNoteField( note, additionalEmail, "otheremail");
    ExtractNoteField( note, otherPhone, "otherphone");
    ExtractNoteField( note, otherWeb, "otherweb");

    // Is there any "extra" data that we may want to format nicely and place
    // in the notes field?
    if ( !additionalEmail.IsEmpty() || !otherPhone.IsEmpty() || !otherWeb.IsEmpty() )
    {
      nsCString     otherNotes(note);

      if ( !additionalEmail.IsEmpty() )
      {
        // Reconstitute line breaks for additional email
        additionalEmail.ReplaceSubstring( "\x03", "\n");

        // Try to figure out if there are multiple email addresses in additionalEmail
        PRInt32     idx = additionalEmail.FindCharInSet("\t\r\n,; ");

        if (idx != -1)
        {
          // We found a character that indicates that there's more than one email address here.
          // Separate out the addresses after the first one.
          additionalEmail.Right(stillMoreEmail, additionalEmail.Length() - idx - 1);
          stillMoreEmail.Trim(kWhitespace);

          // Separate out the first address.
          nsCString   tempStashEmail(additionalEmail);
          tempStashEmail.Left(additionalEmail, idx);
        }

        // If there were more than one additional email addresses store all the extra
        // ones in the notes field, labeled nicely.
        if ( !stillMoreEmail.IsEmpty() )
          FormatExtraDataInNoteField(EUDORAIMPORT_ADDRESS_LABEL_OTHEREMAIL, bundle, stillMoreEmail, noteUTF16);
      }

      if ( !otherPhone.IsEmpty() )
      {
        // Reconstitute line breaks for other phone numbers
        otherPhone.ReplaceSubstring( "\x03", "\n");

        // Store other phone numbers in the notes field, labeled nicely
        FormatExtraDataInNoteField(EUDORAIMPORT_ADDRESS_LABEL_OTHERPHONE, bundle, otherPhone, noteUTF16);
      }

      if ( !otherWeb.IsEmpty() )
      {
        // Reconstitute line breaks for other web sites
        otherWeb.ReplaceSubstring( "\x03", "\n");

        // Store other web sites in the notes field, labeled nicely
        FormatExtraDataInNoteField(EUDORAIMPORT_ADDRESS_LABEL_OTHERWEB, bundle, otherWeb, noteUTF16);
      }

      noteUTF16.Append( NS_ConvertASCIItoUTF16(note) );
    }
  }

  CAliasData *pData = emailList.Count() ? (CAliasData *)emailList.ElementAt(0) : nsnull;

  if (pData && !pData->m_realName.IsEmpty())
    displayName = pData->m_realName;
  else if (!name.IsEmpty())
    displayName = name;
  else
    displayName = pEntry->m_name;

  address.ReplaceSubstring( "\x03", "\n");
  SplitString( address, address2);
  note.ReplaceSubstring( "\x03", "\n");
  fax.ReplaceSubstring( "\x03", " ");
  secondaryFax.ReplaceSubstring( "\x03", " ");
  phone.ReplaceSubstring( "\x03", " ");
  name.ReplaceSubstring( "\x03", " ");
  city.ReplaceSubstring( "\x03", " ");
  state.ReplaceSubstring( "\x03", " ");
  zip.ReplaceSubstring( "\x03", " ");
  country.ReplaceSubstring( "\x03", " ");

  addressWK.ReplaceSubstring( "\x03", "\n");
  SplitString( addressWK, address2WK);
  phoneWK.ReplaceSubstring( "\x03", " ");
  cityWK.ReplaceSubstring( "\x03", " ");
  stateWK.ReplaceSubstring( "\x03", " ");
  zipWK.ReplaceSubstring( "\x03", " ");
  countryWK.ReplaceSubstring( "\x03", " ");
  title.ReplaceSubstring( "\x03", " ");
  company.ReplaceSubstring( "\x03", " ");

  if (newRow)
  {
    nsAutoString uniStr;

    // Home related fields.
    ADD_FIELD_TO_DB_ROW(pDb, AddDisplayName, newRow, displayName, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddNickName, newRow, pEntry->m_name, uniStr);
    if (pData)
      ADD_FIELD_TO_DB_ROW(pDb, AddPrimaryEmail, newRow, pData->m_email, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddFirstName, newRow, firstName, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddLastName, newRow, lastName, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWebPage2, newRow, webLink, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomePhone, newRow, phone, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeAddress, newRow, address, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeAddress2, newRow, address2, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeCity, newRow, city, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeZipCode, newRow, zip, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeState, newRow, state, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddHomeCountry, newRow, country, uniStr);

    // Work related fields.
    ADD_FIELD_TO_DB_ROW(pDb, AddJobTitle, newRow, title, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddCompany, newRow, company, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWebPage1, newRow, webLinkWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkPhone, newRow, phoneWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkAddress, newRow, addressWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkAddress2, newRow, address2WK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkCity, newRow, cityWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkZipCode, newRow, zipWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkState, newRow, stateWK, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddWorkCountry, newRow, countryWK, uniStr);

    if ( (primaryLocation.IsEmpty() || primaryLocation.LowerCaseEqualsASCII("home")) &&
         !mobile.IsEmpty() )
    {
      // Primary location field is either specified to be "home" or is not
      // specified and there is a home mobile number, so use that as the mobile number.
      ADD_FIELD_TO_DB_ROW(pDb, AddCellularNumber, newRow, mobile, uniStr);

      isSecondaryMobileWorkNumber = PR_TRUE;
    }
    else
    {
      // Primary location field is either specified to be "work" or there is no
      // home mobile number, so use work mobile number.
      ADD_FIELD_TO_DB_ROW(pDb, AddCellularNumber, newRow, secondaryMobile, uniStr);

      // Home mobile number (if any) is the secondary mobile number
      secondaryMobile = mobile;
      isSecondaryMobileWorkNumber = PR_FALSE;
    }

    if ( (primaryLocation.IsEmpty() || primaryLocation.LowerCaseEqualsASCII("home")) &&
         !fax.IsEmpty() )
    {
      // Primary location field is either specified to be "home" or is not
      // specified and there is a home fax number, so use that as the fax number.
      ADD_FIELD_TO_DB_ROW(pDb, AddFaxNumber, newRow, fax, uniStr);

      isSecondaryFaxWorkNumber = PR_TRUE;
    }
    else
    {
      // Primary location field is either specified to be "work" or there is no
      // home fax number, so use work fax number.
      ADD_FIELD_TO_DB_ROW(pDb, AddFaxNumber, newRow, secondaryFax, uniStr);

      // Home fax number (if any) is the secondary fax number
      secondaryFax = fax;
      isSecondaryFaxWorkNumber = PR_FALSE;
    }

    ADD_FIELD_TO_DB_ROW(pDb, Add2ndEmail, newRow, additionalEmail, uniStr);

    // Extra info fields
    PRInt32         stringID;
    nsString        pFormat;
    nsString        pCustomData;

    // Add second mobile number, if any, to the Custom 1 field
    if ( !secondaryMobile.IsEmpty() )
    {
      stringID = isSecondaryMobileWorkNumber ?
                 EUDORAIMPORT_ADDRESS_LABEL_WORKMOBILE : EUDORAIMPORT_ADDRESS_LABEL_HOMEMOBILE;
      pFormat.Adopt( nsEudoraStringBundle::GetStringByID(stringID, bundle) );
      pCustomData.Adopt( nsTextFormatter::smprintf(pFormat.get(), NS_ConvertASCIItoUTF16(secondaryMobile).get()) );
      pDb->AddCustom1( newRow, NS_ConvertUTF16toUTF8(pCustomData).get() );
    }

    // Add second fax number, if any, to the Custom 2 field
    if ( !secondaryFax.IsEmpty() )
    {
      stringID = isSecondaryFaxWorkNumber ?
                 EUDORAIMPORT_ADDRESS_LABEL_WORKFAX : EUDORAIMPORT_ADDRESS_LABEL_HOMEFAX;
      pFormat.Adopt( nsEudoraStringBundle::GetStringByID(stringID, bundle) );
      pCustomData.Adopt( nsTextFormatter::smprintf(pFormat.get(), NS_ConvertASCIItoUTF16(secondaryFax).get()) );
      pDb->AddCustom2( newRow, NS_ConvertUTF16toUTF8(pCustomData).get() );
    }

    // Lastly, note field.
    pDb->AddNotes( newRow, NS_ConvertUTF16toUTF8(noteUTF16).get() );

    pDb->AddCardRowToDB( newRow);

    IMPORT_LOG1( "Added card to db: %s\n", displayName.get());
  }
}

//
// Since there is no way to check if a card for a given email address already exists,
// elements in 'membersArray' are make unique. So for each email address in 'emailList'
// we check it in 'membersArray' and if it's not there then we add it to 'membersArray'.
//
void nsEudoraAddress::RememberGroupMembers(nsVoidArray &membersArray, nsVoidArray &emailList)
{
  PRInt32 cnt = emailList.Count();
  CAliasData *pData;

  for (PRInt32 i = 0; i < cnt; i++)
  {
    pData = (CAliasData *)emailList.ElementAt(i);
    if (!pData)
      continue;

    PRInt32 memberCnt = membersArray.Count();
    PRInt32 j = 0;
    for (j = 0; j < memberCnt; j++)
    {
      if (pData == membersArray.ElementAt(j))
        break;
    }
    if (j >= memberCnt)
      membersArray.AppendElement(pData); // add to member list
  }
}

nsresult nsEudoraAddress::AddGroupMembersAsCards(nsVoidArray &membersArray, nsIAddrDatabase *pDb)
{
  PRInt32 max = membersArray.Count();
  CAliasData *pData;
  nsresult rv = NS_OK;
  nsCOMPtr <nsIMdbRow> newRow;
  nsAutoString uniStr;
  nsCAutoString  displayName;

  for (PRInt32 i = 0; i < max; i++)
  {
    pData = (CAliasData *)membersArray.ElementAt(i);

    if (!pData || (pData->m_email.IsEmpty()))
      continue;

    rv = pDb->GetNewRow(getter_AddRefs(newRow));
    if (NS_FAILED(rv) || !newRow)
      return rv;

    if (!pData->m_realName.IsEmpty())
      displayName = pData->m_realName;
    else if (!pData->m_nickName.IsEmpty())
      displayName = pData->m_nickName;
    else
      displayName.Truncate();

    ADD_FIELD_TO_DB_ROW(pDb, AddDisplayName, newRow, displayName, uniStr);
    ADD_FIELD_TO_DB_ROW(pDb, AddPrimaryEmail, newRow, pData->m_email, uniStr);
    rv = pDb->AddCardRowToDB( newRow);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  return rv;
}

nsresult nsEudoraAddress::AddSingleList(CAliasEntry *pEntry, nsVoidArray &emailList, nsIAddrDatabase *pDb)
{
  // Create a list.
  nsCOMPtr <nsIMdbRow> newRow;
  nsresult rv = pDb->GetNewListRow(getter_AddRefs(newRow));
  if (NS_FAILED(rv) || !newRow)
      return rv;

  // Extract name from notes, if any
  nsCString     name;

  if ( !pEntry->m_notes.IsEmpty() )
  {
    nsCString     note(pEntry->m_notes);
    ExtractNoteField(note, name, "name");
  }

  // If we got a name from the notes, use that for the name otherwise use the
  // name in pEntry (which is the Eudora nickname).
  rv = pDb->AddListName(newRow, name.IsEmpty() ? pEntry->m_name.get() : name.get());
  NS_ENSURE_SUCCESS(rv, rv);

  // Add the name in pEntry as the list nickname, because it was the Eudora nickname
  rv = pDb->AddListNickName(newRow, pEntry->m_name.get());
  NS_ENSURE_SUCCESS(rv, rv);

  // Now add the members.
  PRInt32 max = emailList.Count();
  for (PRInt32 i = 0; i < max; i++)
  {
    CAliasData *pData = (CAliasData *)emailList.ElementAt(i);
    nsCAutoString ldifValue(NS_LITERAL_CSTRING("mail=") + nsDependentCString(pData->m_email.get()));
    rv = pDb->AddLdifListMember(newRow, ldifValue.get());
  }

  rv = pDb->AddCardRowToDB(newRow);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = pDb->AddListDirNode(newRow);
  return rv;
}

