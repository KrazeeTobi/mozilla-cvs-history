/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Created by: Rajiv Dayal <rdayal@netscape.com> 
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <windows.h>
#include <tchar.h>

#include "nsAbIPCCard.h"
#include "nsUnicharUtils.h"
#include "nsAbMDBCard.h"
#include "nsAddrDatabase.h"
#include "prdtoa.h"

NS_IMPL_ISUPPORTS_INHERITED1(nsAbIPCCard, nsAbCardProperty, nsIAbMDBCard)

nsAbIPCCard::nsAbIPCCard()
{
    mRecordId = 0;
    mCategoryId = -1;
    mStatus = -1;
}

nsAbIPCCard::~nsAbIPCCard()
{
   
}

nsAbIPCCard::nsAbIPCCard(nsIAbCard *card)
{
    Copy(card);
}

nsAbIPCCard::nsAbIPCCard(nsABCOMCardStruct *card, PRBool isUnicode)
{
    if(isUnicode)
        Copy(card);
    else
        ConvertToUnicodeAndCopy(card);
}

NS_IMETHODIMP nsAbIPCCard::Copy(nsIAbCard *srcCard)
{
    NS_ENSURE_ARG_POINTER(srcCard);

    nsresult rv = NS_OK;
    nsCOMPtr<nsIAbMDBCard> dbCard;
    dbCard = do_QueryInterface(srcCard, &rv);
    if(NS_SUCCEEDED(rv) && dbCard) {
        nsXPIDLString palmIDStr;
        nsresult rv = dbCard->GetStringAttribute(CARD_ATTRIB_PALMID, getter_Copies(palmIDStr));
        if(NS_SUCCEEDED(rv) && palmIDStr.get()) {
            PRFloat64 f = PR_strtod(NS_LossyConvertUCS2toASCII(palmIDStr).get(), nsnull);
            PRInt64 l;
            LL_D2L(l, f);
            LL_L2UI(mRecordId, l);
        }
        else
            mRecordId = 0;
        // set tableID, RowID and Key for the card
        PRUint32 tableID=0;
        dbCard->GetDbTableID(&tableID);
        SetDbTableID(tableID);
        PRUint32 rowID=0;
        dbCard->GetDbRowID(&rowID);
        SetDbRowID(rowID);
        PRUint32 key;
        dbCard->GetKey(&key);
        SetKey(key);
    }
    PRUint32 lastModifiedDate = 0;
    srcCard->GetLastModifiedDate(&lastModifiedDate);
    mStatus = (lastModifiedDate) ? ATTR_MODIFIED : ATTR_NEW;

    return nsAbCardProperty::Copy(srcCard);
}

nsresult nsAbIPCCard::Copy(nsABCOMCardStruct * srcCard)
{
    NS_ENSURE_ARG_POINTER(srcCard);

    mRecordId = srcCard->dwRecordId;
    mCategoryId = srcCard->dwCategoryId;
    mStatus = srcCard->dwStatus;

    SetFirstName(srcCard->firstName);
    SetLastName(srcCard->lastName);
    SetDisplayName(srcCard->displayName);
    SetNickName(srcCard->nickName);
    SetPrimaryEmail(srcCard->primaryEmail);
    SetSecondEmail(srcCard->secondEmail);
    SetPreferMailFormat(srcCard->preferMailFormat);
    SetWorkPhone(srcCard->workPhone);
    SetHomePhone(srcCard->homePhone);
    SetFaxNumber(srcCard->faxNumber);
    SetPagerNumber(srcCard->pagerNumber);
    SetCellularNumber(srcCard->cellularNumber);
    SetHomeAddress(srcCard->homeAddress);
    SetHomeAddress2(srcCard->homeAddress2);
    SetHomeCity(srcCard->homeCity);
    SetHomeState(srcCard->homeState);
    SetHomeZipCode(srcCard->homeZipCode);
    SetHomeCountry(srcCard->homeCountry);
    SetWorkAddress(srcCard->workAddress);
    SetWorkAddress2(srcCard->workAddress2);
    SetWorkCity(srcCard->workCity);
    SetWorkState(srcCard->workState);
    SetWorkZipCode(srcCard->workZipCode);
    SetWorkCountry(srcCard->workCountry);
    SetJobTitle(srcCard->jobTitle);
    SetDepartment(srcCard->department);
    SetCompany(srcCard->company);
    SetWebPage1(srcCard->webPage1);
    SetWebPage2(srcCard->webPage2);
    SetBirthYear(srcCard->birthYear);
    SetBirthMonth(srcCard->birthMonth);
    SetBirthDay(srcCard->birthDay);
    SetCustom1(srcCard->custom1);
    SetCustom2(srcCard->custom2);
    SetCustom3(srcCard->custom3);
    SetCustom4(srcCard->custom4);
    SetNotes(srcCard->notes);
    SetLastModifiedDate(srcCard->lastModifiedDate);
    SetIsMailList(srcCard->isMailList);
    SetMailListURI(srcCard->mailListURI);

    return NS_OK;
}

#define CONVERT_ASSIGNTO_UNICODE(d, s)   d.SetLength(0); if((char*) s) d=NS_ConvertUTF8toUCS2((char*)s);

nsresult nsAbIPCCard::ConvertToUnicodeAndCopy(nsABCOMCardStruct * srcCard)
{
    NS_ENSURE_ARG_POINTER(srcCard);

    mRecordId = srcCard->dwRecordId;
    mCategoryId = srcCard->dwCategoryId;
    mStatus = srcCard->dwStatus;

    nsAutoString str;

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->firstName);
    SetFirstName(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->lastName);
    SetLastName(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->displayName);
    SetDisplayName(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->nickName);
    SetNickName(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->primaryEmail);
    SetPrimaryEmail(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->secondEmail);
    SetSecondEmail(str.get());

    SetPreferMailFormat(srcCard->preferMailFormat);

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workPhone);
    SetWorkPhone(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homePhone);
    SetHomePhone(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->faxNumber);
    SetFaxNumber(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->pagerNumber);
    SetPagerNumber(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->cellularNumber);
    SetCellularNumber(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeAddress);
    SetHomeAddress(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeAddress2);
    SetHomeAddress2(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeCity);
    SetHomeCity(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeState);
    SetHomeState(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeZipCode);
    SetHomeZipCode(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->homeCountry);
    SetHomeCountry(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workAddress);
    SetWorkAddress(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workAddress2);
    SetWorkAddress2(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workCity);
    SetWorkCity(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workState);
    SetWorkState(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workZipCode);
    SetWorkZipCode(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->workCountry);
    SetWorkCountry(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->jobTitle);
    SetJobTitle(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->department);
    SetDepartment(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->company);
    SetCompany(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->webPage1);
    SetWebPage1(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->webPage2);
    SetWebPage2(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->birthYear);
    SetBirthYear(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->birthMonth);
    SetBirthMonth(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->birthDay);
    SetBirthDay(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->custom1);
    SetCustom1(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->custom2);
    SetCustom2(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->custom3);
    SetCustom3(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->custom4);
    SetCustom4(str.get());

    CONVERT_ASSIGNTO_UNICODE(str, srcCard->notes);
    SetNotes(str.get());

    SetLastModifiedDate(srcCard->lastModifiedDate);
    SetIsMailList(srcCard->isMailList);
    SetMailListURI(srcCard->mailListURI);

    return NS_OK;
}


PRBool nsAbIPCCard::EqualsAfterUnicodeConversion(nsABCOMCardStruct * card, nsStringArray & differingAttrs)
{
    if(!card)
        return PR_FALSE;

    // convert to Unicode first
    nsAbIPCCard card1(card, PR_FALSE);
    nsABCOMCardStruct * newCard = new nsABCOMCardStruct;
    // get the unicode nsABCOMCardStruct and compare
    card1.GetABCOMCardStruct(PR_TRUE, newCard);
    PRBool ret = Equals(newCard, differingAttrs);
    delete newCard;
    return ret;
}


PRBool nsAbIPCCard::Equals(nsABCOMCardStruct * card, nsStringArray & differingAttrs)
{
    if(!card)
        return PR_FALSE;

    differingAttrs.Clear();

    if(card->firstName)
        if (Compare(nsDependentString(card->firstName), m_FirstName, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kFirstNameColumn));
    if(card->lastName)
        if (Compare(nsDependentString(card->lastName), m_LastName, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kLastNameColumn));
    if(card->displayName)
        if (Compare(nsDependentString(card->displayName), m_DisplayName, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kDisplayNameColumn));
    if(card->nickName)
        if (Compare(nsDependentString(card->nickName), m_NickName, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kNicknameColumn));
    if(card->primaryEmail)
        if (Compare(nsDependentString(card->primaryEmail), m_PrimaryEmail, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kPriEmailColumn));
    if(card->secondEmail)
        if (Compare(nsDependentString(card->secondEmail), m_SecondEmail, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(k2ndEmailColumn));
    if(card->workPhone)
        if (Compare(nsDependentString(card->workPhone), m_WorkPhone, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkPhoneColumn));
    if(card->homePhone)
        if (Compare(nsDependentString(card->homePhone), m_HomePhone, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomePhoneColumn));
    if(card->faxNumber)
        if (Compare(nsDependentString(card->faxNumber), m_FaxNumber, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kFaxColumn));
    if(card->pagerNumber)
        if (Compare(nsDependentString(card->pagerNumber), m_PagerNumber, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kPagerColumn));
    if(card->cellularNumber)
        if (Compare(nsDependentString(card->cellularNumber), m_CellularNumber, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCellularColumn));
    if(card->homeAddress)
        if (Compare(nsDependentString(card->homeAddress), m_HomeAddress, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeAddressColumn));
    if(card->homeAddress2)
        if (Compare(nsDependentString(card->homeAddress2), m_HomeAddress2, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeAddress2Column));
    if(card->homeCity)
        if (Compare(nsDependentString(card->homeCity), m_HomeCity, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeCityColumn));
    if(card->homeState)
        if (Compare(nsDependentString(card->homeState), m_HomeState, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeStateColumn));
    if(card->homeZipCode)
        if (Compare(nsDependentString(card->homeZipCode), m_HomeZipCode, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeZipCodeColumn));
    if(card->homeCountry)
        if (Compare(nsDependentString(card->homeCountry), m_HomeCountry, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kHomeCountryColumn));
    if(card->workAddress)
        if (Compare(nsDependentString(card->workAddress), m_WorkAddress, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkAddressColumn));
    if(card->workAddress2)
        if (Compare(nsDependentString(card->workAddress2), m_WorkAddress2, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkAddress2Column));
    if(card->workCity)
        if (Compare(nsDependentString(card->workCity), m_WorkCity, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkCityColumn));
    if(card->workState)
        if (Compare(nsDependentString(card->workState), m_WorkState, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkStateColumn));
    if(card->workZipCode)
        if (Compare(nsDependentString(card->workZipCode), m_WorkZipCode, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkZipCodeColumn));
    if(card->workCountry)
        if (Compare(nsDependentString(card->workCountry), m_WorkCountry, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWorkCountryColumn));
    if(card->jobTitle)
        if (Compare(nsDependentString(card->jobTitle), m_JobTitle, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kJobTitleColumn));
    if(card->department)
        if (Compare(nsDependentString(card->department), m_Department, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kDepartmentColumn));
    if(card->company)
        if (Compare(nsDependentString(card->company), m_Company, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCompanyColumn));
    if(card->webPage1)
        if (Compare(nsDependentString(card->webPage1), m_WebPage1, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWebPage1Column));
    if(card->webPage2)
        if (Compare(nsDependentString(card->webPage2), m_WebPage2, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kWebPage2Column));
    if(card->birthYear)
        if (Compare(nsDependentString(card->birthYear), m_BirthYear, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kBirthYearColumn));
    if(card->birthMonth)
        if (Compare(nsDependentString(card->birthMonth), m_BirthMonth, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kBirthMonthColumn));
    if(card->birthDay)
        if (Compare(nsDependentString(card->birthDay), m_BirthDay, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kBirthDayColumn));
    if(card->custom1)
        if (Compare(nsDependentString(card->custom1), m_Custom1, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCustom1Column));
    if(card->custom2)
        if (Compare(nsDependentString(card->custom2), m_Custom2, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCustom2Column));
    if(card->custom3)
        if (Compare(nsDependentString(card->custom3), m_Custom3, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCustom3Column));
    if(card->custom4)
        if (Compare(nsDependentString(card->custom4), m_Custom4, nsCaseInsensitiveStringComparator()))
            differingAttrs.AppendString(NS_LITERAL_STRING(kCustom4Column));
    if (card->isMailList != m_IsMailList)
        differingAttrs.AppendString(NS_LITERAL_STRING(kMailListName));
    if(card->mailListURI) {
        nsCAutoString str(card->mailListURI);
        if (str.Compare(m_MailListURI, PR_TRUE))
            differingAttrs.AppendString(NS_LITERAL_STRING(kMailListDescription));
    }

    return (differingAttrs.Count() == 0);
}


NS_IMETHODIMP nsAbIPCCard::Equals(nsIAbCard *card, PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(card);
    NS_ENSURE_ARG_POINTER(_retval);

    nsXPIDLString str;
    *_retval = PR_FALSE;

    card->GetFirstName(getter_Copies(str));
    if (Compare(str, m_FirstName, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetLastName(getter_Copies(str));
    if (Compare(str, m_LastName, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetDisplayName(getter_Copies(str));
    if (Compare(str, m_DisplayName, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetNickName(getter_Copies(str));
    if (Compare(str, m_NickName, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetPrimaryEmail(getter_Copies(str));
    if (Compare(str, m_PrimaryEmail, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetSecondEmail(getter_Copies(str));
    if (Compare(str, m_SecondEmail, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkPhone(getter_Copies(str));
    if (Compare(str, m_WorkPhone, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomePhone(getter_Copies(str));
    if (Compare(str, m_HomePhone, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetFaxNumber(getter_Copies(str));
    if (Compare(str, m_FaxNumber, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetPagerNumber(getter_Copies(str));
    if (Compare(str, m_PagerNumber, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCellularNumber(getter_Copies(str));
    if (Compare(str, m_CellularNumber, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeAddress(getter_Copies(str));
    if (Compare(str, m_HomeAddress, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeAddress2(getter_Copies(str));
    if (Compare(str, m_HomeAddress2, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeCity(getter_Copies(str));
    if (Compare(str, m_HomeCity, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeState(getter_Copies(str));
    if (Compare(str, m_HomeState, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeZipCode(getter_Copies(str));
    if (Compare(str, m_HomeZipCode, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetHomeCountry(getter_Copies(str));
    if (Compare(str, m_HomeCountry, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkAddress(getter_Copies(str));
    if (Compare(str, m_WorkAddress, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkAddress2(getter_Copies(str));
    if (Compare(str, m_WorkAddress2, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkCity(getter_Copies(str));
    if (Compare(str, m_WorkCity, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkState(getter_Copies(str));
    if (Compare(str, m_WorkState, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkZipCode(getter_Copies(str));
    if (Compare(str, m_WorkZipCode, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWorkCountry(getter_Copies(str));
    if (Compare(str, m_WorkCountry, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetJobTitle(getter_Copies(str));
    if (Compare(str, m_JobTitle, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetDepartment(getter_Copies(str));
    if (Compare(str, m_Department, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCompany(getter_Copies(str));
    if (Compare(str, m_Company, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWebPage1(getter_Copies(str));
    if (Compare(str, m_WebPage1, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetWebPage2(getter_Copies(str));
    if (Compare(str, m_WebPage2, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetBirthYear(getter_Copies(str));
    if (Compare(str, m_BirthYear, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetBirthMonth(getter_Copies(str));
    if (Compare(str, m_BirthMonth, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetBirthDay(getter_Copies(str));
    if (Compare(str, m_BirthDay, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCustom1(getter_Copies(str));
    if (Compare(str, m_Custom1, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCustom2(getter_Copies(str));
    if (Compare(str, m_Custom2, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCustom3(getter_Copies(str));
    if (Compare(str, m_Custom3, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    card->GetCustom4(getter_Copies(str));
    if (Compare(str, m_Custom4, nsCaseInsensitiveStringComparator()))
        return NS_OK;

    PRBool isMailList=PR_FALSE;
    card->GetIsMailList(&isMailList);
    if (isMailList != m_IsMailList)
        return NS_OK;

    nsCAutoString tempStr(m_MailListURI);
    nsXPIDLCString str2;
    card->GetMailListURI(getter_Copies(str2));
    if (tempStr.Compare(str2.get(), PR_TRUE))
        return NS_OK;

    *_retval = PR_TRUE;
    return NS_OK;
}

PRBool nsAbIPCCard::CompareValue(PRBool isUnicode, LPTSTR cardValue, nsString & attribValue)
{
    if(cardValue) {
        if(isUnicode) {
            if (Compare(nsDependentString(cardValue), attribValue, nsCaseInsensitiveStringComparator()))
                return PR_FALSE;
        }
        else {
            nsAutoString str;
            CONVERT_ASSIGNTO_UNICODE(str, cardValue);
            if (Compare(str, attribValue, nsCaseInsensitiveStringComparator()))
                return PR_FALSE;
        }
    }

    return PR_TRUE;
}

PRBool nsAbIPCCard::Same(nsABCOMCardStruct * card, PRBool isUnicode)
{
    if(!card)
        return PR_FALSE;

    if(mRecordId && card->dwRecordId) {
        if(mRecordId == card->dwRecordId)
            return PR_TRUE;
        else
            return PR_FALSE;
    }

    if(CompareValue(isUnicode, card->firstName, m_FirstName))
        if(CompareValue(isUnicode, card->lastName, m_LastName))
            if(CompareValue(isUnicode, card->displayName, m_DisplayName))
                if(CompareValue(isUnicode, card->nickName, m_NickName))
                    return PR_TRUE;

    return PR_FALSE;
}


PRBool nsAbIPCCard::Same(nsIAbCard *card)
{
    if(!card)
        return PR_FALSE;

    nsresult rv = NS_OK;
    nsCOMPtr<nsIAbMDBCard> dbCard;
    dbCard = do_QueryInterface(card, &rv);
    if(NS_SUCCEEDED(rv)) {
        // first check the palmID for the cards if they exist
        nsXPIDLString palmIDStr;
        rv = dbCard->GetStringAttribute(CARD_ATTRIB_PALMID, getter_Copies(palmIDStr));
        if(NS_SUCCEEDED(rv) && palmIDStr.get()) {
            PRInt32 palmID=0;
            PRFloat64 f = PR_strtod(NS_LossyConvertUCS2toASCII(palmIDStr).get(), nsnull);
            PRInt64 l;
            LL_D2L(l, f);
            LL_L2UI(palmID, l);
            if(palmID && mRecordId)
                return mRecordId == palmID;
        }
    }

    nsXPIDLString str;
    card->GetFirstName(getter_Copies(str));
    if (Compare(str, m_FirstName, nsCaseInsensitiveStringComparator()))
        return PR_FALSE;
    card->GetLastName(getter_Copies(str));
    if (Compare(str, m_LastName, nsCaseInsensitiveStringComparator()))
        return PR_FALSE;
    card->GetDisplayName(getter_Copies(str));
    if (Compare(str, m_DisplayName, nsCaseInsensitiveStringComparator()))
        return PR_FALSE;
    card->GetNickName(getter_Copies(str));
    if (Compare(str, m_NickName, nsCaseInsensitiveStringComparator()))
        return PR_FALSE;

    return PR_TRUE;
}


void nsAbIPCCard::CopyValue(PRBool isUnicode, nsString & attribValue, LPTSTR * result)
{
    *result = NULL;
    if(attribValue.Length() && attribValue.get()) {
        if(isUnicode) {                                 
            PRUnichar * Str = (PRUnichar *) CoTaskMemAlloc(sizeof(PRUnichar) * attribValue.Length()+1);
            wcscpy(Str, attribValue.get());
            *result = Str;
        } 
        else { 
            nsCAutoString cStr; 
            cStr = NS_ConvertUCS2toUTF8(attribValue);
            char * str = (char *) CoTaskMemAlloc(cStr.Length()+1);
            strcpy(str, cStr.get()); 
            *result = (LPTSTR) str;
        } 
    }
}


nsresult nsAbIPCCard::GetABCOMCardStruct(PRBool isUnicode, nsABCOMCardStruct * card)
{
    NS_ENSURE_ARG_POINTER(card);

    memset(card, 0, sizeof(nsABCOMCardStruct));

    card->dwRecordId = mRecordId;
    card->dwCategoryId = mCategoryId;
    card->dwStatus = mStatus;

    CopyValue(isUnicode, m_FirstName, &card->firstName);
    CopyValue(isUnicode, m_LastName, &card->lastName);
    CopyValue(isUnicode, m_DisplayName, &card->displayName);
    CopyValue(isUnicode, m_NickName, &card->nickName);
    CopyValue(isUnicode, m_PrimaryEmail, &card->primaryEmail);
    CopyValue(isUnicode, m_SecondEmail, &card->secondEmail);
    CopyValue(isUnicode, m_WorkPhone, &card->workPhone);
    CopyValue(isUnicode, m_HomePhone, &card->homePhone);
    CopyValue(isUnicode, m_FaxNumber, &card->faxNumber);
    CopyValue(isUnicode, m_PagerNumber, &card->pagerNumber);
    CopyValue(isUnicode, m_CellularNumber, &card->cellularNumber);
    CopyValue(isUnicode, m_HomeAddress, &card->homeAddress);
    CopyValue(isUnicode, m_HomeAddress2, &card->homeAddress2);
    CopyValue(isUnicode, m_HomeCity, &card->homeCity);
    CopyValue(isUnicode, m_HomeState, &card->homeState);
    CopyValue(isUnicode, m_HomeZipCode, &card->homeZipCode);
    CopyValue(isUnicode, m_HomeCountry, &card->homeCountry);
    CopyValue(isUnicode, m_WorkAddress, &card->workAddress);
    CopyValue(isUnicode, m_WorkAddress2, &card->workAddress2);
    CopyValue(isUnicode, m_WorkCity, &card->workCity);
    CopyValue(isUnicode, m_WorkState, &card->workState);
    CopyValue(isUnicode, m_WorkZipCode, &card->workZipCode);
    CopyValue(isUnicode, m_WorkCountry, &card->workCountry);
    CopyValue(isUnicode, m_JobTitle, &card->jobTitle);
    CopyValue(isUnicode, m_Department, &card->department);
    CopyValue(isUnicode, m_Company, &card->company);
    CopyValue(isUnicode, m_WebPage1, &card->webPage1);
    CopyValue(isUnicode, m_WebPage2, &card->webPage2);
    CopyValue(isUnicode, m_BirthYear, &card->birthYear);
    CopyValue(isUnicode, m_BirthMonth, &card->birthMonth);
    CopyValue(isUnicode, m_BirthDay, &card->birthDay);
    CopyValue(isUnicode, m_Custom1, &card->custom1);
    CopyValue(isUnicode, m_Custom2, &card->custom2);
    CopyValue(isUnicode, m_Custom3, &card->custom3);
    CopyValue(isUnicode, m_Custom4, &card->custom4);
    CopyValue(isUnicode, m_Note, &card->notes);

    card->lastModifiedDate = m_LastModDate;
    card->preferMailFormat = m_PreferMailFormat;

    card->isMailList = m_IsMailList;
    if(m_MailListURI) {
        card->mailListURI = new char[strlen(m_MailListURI)+1];
        strcpy(card->mailListURI, m_MailListURI);
    }

    return NS_OK;
}


