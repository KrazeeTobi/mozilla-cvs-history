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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Created by: Paul Sandoz   <paul.sandoz@sun.com>
 *   Dan Mosedale <dan.mosedale@oracle.com>
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

#include "nsAbDirectoryQuery.h"
#include "nsAbDirectoryQueryProxy.h"
#include "nsAbUtils.h"
#include "nsAbBooleanExpression.h"

#include "nsIRDFResource.h"

#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsIAbDirSearchListener.h"

NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbDirectoryQuerySimpleBooleanExpression, nsIAbBooleanExpression)

nsAbDirectoryQuerySimpleBooleanExpression::nsAbDirectoryQuerySimpleBooleanExpression() :
    mOperation (nsIAbBooleanOperationTypes::AND)
{
}

nsAbDirectoryQuerySimpleBooleanExpression::~nsAbDirectoryQuerySimpleBooleanExpression()
{
}

/* attribute nsAbBooleanOperationType operation; */
NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::GetOperation(nsAbBooleanOperationType *aOperation)
{
    if (!aOperation)
        return NS_ERROR_NULL_POINTER;

    *aOperation = mOperation;

    return NS_OK;
}
NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::SetOperation(nsAbBooleanOperationType aOperation)
{
    if (aOperation != nsIAbBooleanOperationTypes::AND &&
        aOperation != nsIAbBooleanOperationTypes::OR)
        return NS_ERROR_FAILURE;

    mOperation = aOperation;

    return NS_OK;
}

/* attribute nsISupportsArray expressions; */
NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::GetExpressions(nsISupportsArray** aExpressions)
{
    if (!aExpressions)
        return NS_ERROR_NULL_POINTER;

    if (!mExpressions)
        NS_NewISupportsArray(getter_AddRefs(mExpressions));

    NS_IF_ADDREF(*aExpressions = mExpressions);
    return NS_OK;
}

NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::SetExpressions(nsISupportsArray* aExpressions)
{
    if (!aExpressions)
        return NS_ERROR_NULL_POINTER;

    nsresult rv;
    PRUint32 count;
    rv = aExpressions->Count (&count);
    NS_ENSURE_SUCCESS(rv, rv);

    for (PRUint32 i = 0; i < count; i++)
    {
        nsCOMPtr<nsISupports> item;
        rv = aExpressions->GetElementAt (i, getter_AddRefs (item));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIAbBooleanConditionString> queryExpression (do_QueryInterface (item, &rv));
        NS_ENSURE_SUCCESS(rv, rv);
    }

    mExpressions = aExpressions;

    return NS_OK;
}

/* void asetExpressions (in unsigned long aExpressionsSize, [array, size_is (aExpressionsSize)] in nsISupports aExpressionsArray); */
NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::AsetExpressions(PRUint32 aExpressionsSize, nsISupports **aExpressionsArray)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void agetExpressions (out unsigned long aExpressionsSize, [array, size_is (aExpressionsSize), retval] out nsISupports aExpressionsArray); */
NS_IMETHODIMP nsAbDirectoryQuerySimpleBooleanExpression::AgetExpressions(PRUint32 *aExpressionsSize, nsISupports ***aExpressionsArray)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbDirectoryQueryArguments, nsIAbDirectoryQueryArguments)

nsAbDirectoryQueryArguments::nsAbDirectoryQueryArguments() :
    mQuerySubDirectories(PR_TRUE)
{
}

nsAbDirectoryQueryArguments::~nsAbDirectoryQueryArguments()
{
}

/* attribute nsISupportsArray matchItems; */
NS_IMETHODIMP nsAbDirectoryQueryArguments::GetExpression(nsISupports** aExpression)
{
    if (!aExpression)
        return NS_ERROR_NULL_POINTER;

    NS_IF_ADDREF(*aExpression = mExpression);
    return NS_OK;
}

NS_IMETHODIMP nsAbDirectoryQueryArguments::SetExpression(nsISupports* aExpression)
{
    mExpression = aExpression;
    return NS_OK;
}

/* attribute boolean querySubDirectories; */
NS_IMETHODIMP nsAbDirectoryQueryArguments::GetQuerySubDirectories(PRBool* aQuerySubDirectories)
{
    NS_ENSURE_ARG_POINTER(aQuerySubDirectories);
    *aQuerySubDirectories = mQuerySubDirectories;
    return NS_OK;
}

NS_IMETHODIMP nsAbDirectoryQueryArguments::SetQuerySubDirectories(PRBool aQuerySubDirectories)
{
    mQuerySubDirectories = aQuerySubDirectories;
    return NS_OK;
}

NS_IMETHODIMP nsAbDirectoryQueryArguments::GetTypeSpecificArg(nsISupports** aArg)
{
    NS_ENSURE_ARG_POINTER(aArg);

    NS_IF_ADDREF(*aArg = mTypeSpecificArg);
    return NS_OK;
}

NS_IMETHODIMP nsAbDirectoryQueryArguments::SetTypeSpecificArg(nsISupports* aArg)
{
    mTypeSpecificArg = aArg;
    return NS_OK;
}


NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbDirectoryQueryPropertyValue, nsIAbDirectoryQueryPropertyValue)

nsAbDirectoryQueryPropertyValue::nsAbDirectoryQueryPropertyValue()
{
}

nsAbDirectoryQueryPropertyValue::nsAbDirectoryQueryPropertyValue(const char* aName,
      const PRUnichar* aValue)
{
    mName = aName;
    mValue = aValue;
}

nsAbDirectoryQueryPropertyValue::nsAbDirectoryQueryPropertyValue(const char* aName,
      nsISupports* aValueISupports)
{
    mName = aName;
    mValueISupports = aValueISupports;
}

nsAbDirectoryQueryPropertyValue::~nsAbDirectoryQueryPropertyValue()
{
}

/* read only attribute string name; */
NS_IMETHODIMP nsAbDirectoryQueryPropertyValue::GetName(char*  *aName)
{
    *aName = mName.IsEmpty() ? 0 : ToNewCString(mName);

    return NS_OK;
}

/* read only attribute wstring value; */
NS_IMETHODIMP nsAbDirectoryQueryPropertyValue::GetValue(PRUnichar*  *aValue)
{
    *aValue = ToNewUnicode(mValue);
    if (!(*aValue)) 
        return NS_ERROR_OUT_OF_MEMORY;
    else
        return NS_OK;
}

/* readonly attribute nsISupports valueISupports; */
NS_IMETHODIMP nsAbDirectoryQueryPropertyValue::GetValueISupports(nsISupports*  *aValueISupports)
{
    if (!mValueISupports)
        return NS_ERROR_NULL_POINTER;

    NS_IF_ADDREF(*aValueISupports = mValueISupports);
    return NS_OK;
}

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAbDirectoryQuery, nsIAbDirectoryQuery)

nsAbDirectoryQuery::nsAbDirectoryQuery()
{
}

nsAbDirectoryQuery::~nsAbDirectoryQuery()
{
}

NS_IMETHODIMP nsAbDirectoryQuery::DoQuery(nsIAbDirectory *aDirectory,
                                          nsIAbDirectoryQueryArguments* arguments,
                                          nsIAbDirSearchListener* listener,
                                          PRInt32 resultLimit, PRInt32 timeOut,
                                          PRInt32* _retval)
{
  NS_ENSURE_ARG_POINTER(aDirectory);

  nsresult rv = query(aDirectory, arguments, listener, &resultLimit);

  rv = NS_FAILED(rv) ? queryError(listener) : queryFinished(listener);

  *_retval = 0;
  return rv;
}

/* void stopQuery (in long contextID); */
NS_IMETHODIMP nsAbDirectoryQuery::StopQuery(PRInt32 contextID)
{
    return NS_OK;
}


nsresult nsAbDirectoryQuery::query (nsIAbDirectory* directory,
    nsIAbDirectoryQueryArguments* arguments,
    nsIAbDirSearchListener* listener,
    PRInt32* resultLimit)
{
    if (*resultLimit == 0)
        return NS_OK;

    nsresult rv = queryCards (directory, arguments, listener, resultLimit);
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool doSubDirectories;
    arguments->GetQuerySubDirectories (&doSubDirectories);
    if (doSubDirectories && *resultLimit != 0) {
        rv = queryChildren(directory, arguments, listener, resultLimit);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    return rv;
}

nsresult nsAbDirectoryQuery::queryChildren (nsIAbDirectory* directory,
    nsIAbDirectoryQueryArguments* arguments,
    nsIAbDirSearchListener* listener,
    PRInt32* resultLimit)
{
    nsresult rv = NS_OK;

    nsCOMPtr<nsISimpleEnumerator> subDirectories;
    rv = directory->GetChildNodes(getter_AddRefs(subDirectories));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool hasMore;
    while (NS_SUCCEEDED(rv = subDirectories->HasMoreElements(&hasMore)) && hasMore)
    {
        nsCOMPtr<nsISupports> item;
        rv = subDirectories->GetNext (getter_AddRefs (item));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIAbDirectory> subDirectory(do_QueryInterface(item, &rv));
        NS_ENSURE_SUCCESS(rv, rv);
        
        rv = query (subDirectory, arguments, listener, resultLimit);
        NS_ENSURE_SUCCESS(rv, rv);

    }
    return NS_OK;
}

nsresult nsAbDirectoryQuery::queryCards (nsIAbDirectory* directory,
    nsIAbDirectoryQueryArguments* arguments,
    nsIAbDirSearchListener* listener,
    PRInt32* resultLimit)
{
    nsresult rv = NS_OK;

    nsCOMPtr<nsISimpleEnumerator> cards;
    rv = directory->GetChildCards(getter_AddRefs(cards));
    if (NS_FAILED(rv))
    {
        if (rv != NS_ERROR_NOT_IMPLEMENTED)
            NS_ENSURE_SUCCESS(rv, rv);
        else
            return NS_OK;
    }

    if (!cards)
        return NS_OK;

    PRBool more;
    while (NS_SUCCEEDED(cards->HasMoreElements(&more)) && more)
    {
        nsCOMPtr<nsISupports> item;
        rv = cards->GetNext(getter_AddRefs(item));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIAbCard> card(do_QueryInterface(item, &rv));
        NS_ENSURE_SUCCESS(rv, rv);
        
        rv = matchCard (card, arguments, listener, resultLimit);
        NS_ENSURE_SUCCESS(rv, rv);

        if (*resultLimit == 0)
            return NS_OK;
    }

    return NS_OK;
}

nsresult nsAbDirectoryQuery::matchCard (nsIAbCard* card,
    nsIAbDirectoryQueryArguments* arguments,
    nsIAbDirSearchListener* listener,
    PRInt32* resultLimit)
{
    nsCOMPtr<nsISupports> supportsExpression;
    nsresult rv = arguments->GetExpression (getter_AddRefs (supportsExpression));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIAbBooleanExpression> expression(do_QueryInterface(supportsExpression, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool matchFound = PR_FALSE;
    rv = matchCardExpression (card, expression, &matchFound);
    NS_ENSURE_SUCCESS(rv, rv);

    if (matchFound)
    {
        (*resultLimit)--;
        rv = queryMatch(card, listener);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    return rv;
}

nsresult nsAbDirectoryQuery::matchCardExpression (nsIAbCard* card,
    nsIAbBooleanExpression* expression,
    PRBool* result)
{
    nsAbBooleanOperationType operation;
    nsresult rv = expression->GetOperation (&operation);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsISupportsArray> childExpressions;
    rv = expression->GetExpressions (getter_AddRefs (childExpressions));
    NS_ENSURE_SUCCESS(rv, rv);
    
    PRUint32 count;
    rv = childExpressions->Count (&count);
    NS_ENSURE_SUCCESS(rv, rv);

    if (operation == nsIAbBooleanOperationTypes::NOT &&
        count > 1)
        return NS_ERROR_FAILURE;

    PRBool value = *result = PR_FALSE;
    for (PRUint32 i = 0; i < count; i++)
    {
        nsCOMPtr<nsISupports> item;
        rv = childExpressions->GetElementAt (i, getter_AddRefs (item));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIAbBooleanConditionString>
            childCondition(do_QueryInterface(item, &rv));
        if (NS_SUCCEEDED(rv))
        {
            rv = matchCardCondition (card, childCondition, &value);
            NS_ENSURE_SUCCESS(rv, rv);
        }
        else
        {
            nsCOMPtr<nsIAbBooleanExpression>
                childExpression(do_QueryInterface(item, &rv));
            if (NS_SUCCEEDED(rv))
            {
                rv = matchCardExpression (card, childExpression, &value);
                NS_ENSURE_SUCCESS(rv, rv);
            }
            else
                return NS_ERROR_FAILURE;
        }
        if (operation == nsIAbBooleanOperationTypes::OR && value)
            break;
        else if (operation == nsIAbBooleanOperationTypes::AND && !value)
            break;
        else if (operation == nsIAbBooleanOperationTypes::NOT)
            value = !value;
    }
    *result = value;

    return NS_OK;
}

nsresult nsAbDirectoryQuery::matchCardCondition (nsIAbCard* card,
    nsIAbBooleanConditionString* condition,
    PRBool* matchFound)
{
    nsAbBooleanConditionType conditionType;
    nsresult rv = condition->GetCondition (&conditionType);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCString name;
    rv = condition->GetName (getter_Copies (name));
    NS_ENSURE_SUCCESS(rv, rv);

    if (name.Equals ("card:nsIAbCard"))
    {
      *matchFound = (conditionType == nsIAbBooleanConditionTypes::Exists);
      return NS_OK;
    }

    nsString value;
    rv = card->GetCardValue(name.get(), value);
    NS_ENSURE_SUCCESS(rv, rv);

    if (value.IsEmpty())
    {
      *matchFound = (conditionType == nsIAbBooleanConditionTypes::DoesNotExist) ?
          PR_TRUE : PR_FALSE;
      return NS_OK;
    }

    nsString matchValue;
    rv = condition->GetValue (getter_Copies (matchValue));
    NS_ENSURE_SUCCESS(rv, rv);

    /* TODO
     * What about allowing choice between case insensitive
     * and case sensitive comparisons?
     *
     */
    switch (conditionType)
    {
        case nsIAbBooleanConditionTypes::Exists:
            *matchFound = PR_TRUE;
            break;
        case nsIAbBooleanConditionTypes::Contains:
            // When we move to frozen linkage, this should be:
            // *matchFound = value.Find(matchValue, CaseInsensitiveCompare) >= 0;
            *matchFound = FindInReadable(matchValue, value, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::DoesNotContain:
            // When we move to frozen linkage, this should be:
            // *matchFound = value.Find(matchValue, CaseInsensitiveCompare) == -1;
            *matchFound = !FindInReadable(matchValue, value, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::Is:
            *matchFound = value.Equals(matchValue, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::IsNot:
            *matchFound = !value.Equals(matchValue, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::BeginsWith:
            *matchFound = StringBeginsWith(value, matchValue, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::LessThan:
            *matchFound = Compare(value, matchValue, nsCaseInsensitiveStringComparator()) < 0;
            break;
        case nsIAbBooleanConditionTypes::GreaterThan:
            *matchFound = Compare(value, matchValue, nsCaseInsensitiveStringComparator()) > 0;
            break;
        case nsIAbBooleanConditionTypes::EndsWith:
            *matchFound = StringEndsWith(value, matchValue, nsCaseInsensitiveStringComparator());
            break;
        case nsIAbBooleanConditionTypes::SoundsLike:
        case nsIAbBooleanConditionTypes::RegExp:
            *matchFound = PR_FALSE;
            break;
        default:
            *matchFound = PR_FALSE;
    }

    return rv;
}

nsresult nsAbDirectoryQuery::queryMatch(nsIAbCard* card,
    nsIAbDirSearchListener* listener)
{
  return listener->OnSearchFoundCard(card);
}

nsresult nsAbDirectoryQuery::queryFinished(nsIAbDirSearchListener* listener)
{
    return listener->OnSearchFinished(nsIAbDirectoryQueryResultListener::queryResultComplete, EmptyString());
}

nsresult nsAbDirectoryQuery::queryError(nsIAbDirSearchListener* listener)
{
    return listener->OnSearchFinished(nsIAbDirectoryQueryResultListener::queryResultError, EmptyString());
}
