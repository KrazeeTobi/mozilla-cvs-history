/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * Copyright (C) 1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "msgCore.h"
#include "nsMsgSearchCore.h"
#include "nsMsgUtils.h"
#include "nsIMsgDatabase.h"

//---------------------------------------------------------------------------
// nsMsgSearchTerm specifies one criterion, e.g. name contains phil
//---------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//-------------------- Implementation of nsMsgSearchTerm -----------------------
//-----------------------------------------------------------------------------


typedef struct
{
	nsMsgSearchAttribute	attrib;
	const char			*attribName;
} nsMsgSearchAttribEntry;

nsMsgSearchAttribEntry SearchAttribEntryTable[] =
{
    {nsMsgSearchAttribSubject,		"subject"},
    {nsMsgSearchAttribSender,		"from"},
    {nsMsgSearchAttribBody,		"body"},
    {nsMsgSearchAttribDate,		"date"},
    {nsMsgSearchAttribPriority,	"priority"},
    {nsMsgSearchAttribMsgStatus,	"status"},	
    {nsMsgSearchAttribTo,			"to"},
    {nsMsgSearchAttribCC,			"CC"},
    {nsMsgSearchAttribToOrCC,		"to or CC"}
};

// Take a string which starts off with an attribute
// return the matching attribute. If the string is not in the table, then we can conclude that it is an arbitrary header
nsresult NS_MsgGetAttributeFromString(const char *string, int16 *attrib)
{
	if (NULL == string || NULL == attrib)
		return NS_ERROR_NULL_POINTER;
	PRBool found = FALSE;
	for (int idxAttrib = 0; idxAttrib < sizeof(SearchAttribEntryTable) / sizeof(nsMsgSearchAttribEntry); idxAttrib++)
	{
		if (!PL_strcasecmp(string, SearchAttribEntryTable[idxAttrib].attribName))
		{
			found = PR_TRUE;
			*attrib = SearchAttribEntryTable[idxAttrib].attrib;
			break;
		}
	}
	if (!found)
		*attrib = nsMsgSearchAttribOtherHeader; // assume arbitrary header if we could not find the header in the table
	return NS_OK;      // we always succeed now
}

nsresult NS_MsgGetStringForAttribute(int16 attrib, const char **string)
{
	if (NULL == string)
		return NS_ERROR_NULL_POINTER;
	PRBool found = FALSE;
	for (int idxAttrib = 0; idxAttrib < sizeof(SearchAttribEntryTable) / sizeof(nsMsgSearchAttribEntry); idxAttrib++)
	{
		// I'm using the idx's as aliases into MSG_SearchAttribute and 
		// MSG_SearchOperator enums which is legal because of the way the
		// enums are defined (starts at 0, numItems at end)
		if (attrib == SearchAttribEntryTable[idxAttrib].attrib)
		{
			found = PR_TRUE;
			*string = SearchAttribEntryTable[idxAttrib].attribName;
			break;
		}
	}
	// we no longer return invalid attribute. If we cannot find the string in the table, 
	// then it is an arbitrary header. Return success regardless if found or not
//	return (found) ? SearchError_Success : SearchError_InvalidAttribute;
	return NS_OK;
}

typedef struct
{
	nsMsgSearchOperator	op;
	const char			*opName;
} nsMsgSearchOperatorEntry;

nsMsgSearchOperatorEntry SearchOperatorEntryTable[] =
{
	{nsMsgSearchOpContains,	"contains"},
    {nsMsgSearchOpDoesntContain,"doesn't contain"},
    {nsMsgSearchOpIs,           "is"},
    {nsMsgSearchOpIsnt,		"isn't"},
    {nsMsgSearchOpIsEmpty,		"is empty"},
	{nsMsgSearchOpIsBefore,    "is before"},
    {nsMsgSearchOpIsAfter,		"is after"},
    {nsMsgSearchOpIsHigherThan, "is higher than"},
    {nsMsgSearchOpIsLowerThan,	"is lower than"},
    {nsMsgSearchOpBeginsWith,  "begins with"},
	{nsMsgSearchOpEndsWith,	"ends with"}
};

nsresult NS_MsgGetOperatorFromString(const char *string, int16 *op)
{
	if (NULL == string || NULL == op)
		return NS_ERROR_NULL_POINTER;
	
	XP_Bool found = FALSE;
	for (int idxOp = 0; idxOp < sizeof(SearchOperatorEntryTable) / sizeof(nsMsgSearchOperatorEntry); idxOp++)
	{
		// I'm using the idx's as aliases into MSG_SearchAttribute and 
		// MSG_SearchOperator enums which is legal because of the way the
		// enums are defined (starts at 0, numItems at end)
		if (!PL_strcasecmp(string, SearchOperatorEntryTable[idxOp].opName))
		{
			found = PR_TRUE;
			*op = SearchOperatorEntryTable[idxOp].op;
			break;
		}
	}
	return (found) ? NS_OK : NS_ERROR_INVALID_ARG;
}

nsresult NS_MsgGetStringForOperator(int16 op, const char **string)
{
	if (NULL == string)
		return NS_ERROR_NULL_POINTER;
	PRBool found = FALSE;
	for (int idxOp = 0; idxOp < sizeof(SearchOperatorEntryTable) / sizeof(nsMsgSearchOperatorEntry); idxOp++)
	{
		// I'm using the idx's as aliases into MSG_SearchAttribute and 
		// MSG_SearchOperator enums which is legal because of the way the
		// enums are defined (starts at 0, numItems at end)
		if (op == SearchOperatorEntryTable[idxOp].op)
		{
			found = PR_TRUE;
			*string = SearchOperatorEntryTable[idxOp].opName;
			break;
		}
	}

	return (found) ? NS_OK : NS_ERROR_INVALID_ARG;
}

void NS_MsgGetUntranslatedStatusName (uint32 s, nsString2 *outName)
{
	char *tmpOutName = NULL;
#define MSG_STATUS_MASK (MSG_FLAG_READ | MSG_FLAG_REPLIED | MSG_FLAG_FORWARDED | MSG_FLAG_NEW)
	PRUint32 maskOut = (s & MSG_STATUS_MASK);

	// diddle the flags to pay attention to the most important ones first, if multiple
	// flags are set. Should remove this code from the winfe.
	if (maskOut & MSG_FLAG_NEW) 
		maskOut = MSG_FLAG_NEW;
	if ( maskOut & MSG_FLAG_REPLIED &&
		 maskOut & MSG_FLAG_FORWARDED ) 
		maskOut = MSG_FLAG_REPLIED|MSG_FLAG_FORWARDED;
	else if ( maskOut & MSG_FLAG_FORWARDED )
		maskOut = MSG_FLAG_FORWARDED;
	else if ( maskOut & MSG_FLAG_REPLIED ) 
		maskOut = MSG_FLAG_REPLIED;

	switch (maskOut)
	{
	case MSG_FLAG_READ:
		tmpOutName = "read";
		break;
	case MSG_FLAG_REPLIED:
		tmpOutName = "replied";
		break;
	case MSG_FLAG_FORWARDED:
		tmpOutName = "forwarded";
		break;
	case MSG_FLAG_FORWARDED|MSG_FLAG_REPLIED:
		tmpOutName = "replied and forwarded";
		break;
	case MSG_FLAG_NEW:
		tmpOutName = "new";
		break;
	default:
		// This is fine, status may be "unread" for example
        break;
	}

	if (tmpOutName)
		*outName = tmpOutName;
}


PRInt32 NS_MsgGetStatusValueFromName(char *name)
{
	if (PL_strcmp("read", name))
		return MSG_FLAG_READ;
	if (PL_strcmp("replied", name))
		return MSG_FLAG_REPLIED;
	if (PL_strcmp("forwarded", name))
		return MSG_FLAG_FORWARDED;
	if (PL_strcmp("replied and forwarded", name))
		return MSG_FLAG_FORWARDED|MSG_FLAG_REPLIED;
	if (PL_strcmp("new", name))
		return MSG_FLAG_NEW;
	return 0;
}


// Needed for DeStream method.
nsMsgSearchTerm::nsMsgSearchTerm()
{
}


nsMsgSearchTerm::nsMsgSearchTerm (
	nsMsgSearchAttribute attrib, 
	nsMsgSearchOperator op, 
	nsMsgSearchValue *val,
	PRBool booleanAND,
	char * arbitraryHeader) 
{
	m_operator = op;
	m_booleanOp = (booleanAND) ? nsMsgSearchBooleanAND : nsMsgSearchBooleanOR;
	if (attrib == nsMsgSearchAttribOtherHeader && arbitraryHeader)
		m_arbitraryHeader = arbitraryHeader;
	m_attribute = attrib;

	nsMsgResultElement::AssignValues (val, &m_value);
}

nsMsgSearchTerm::nsMsgSearchTerm (
	nsMsgSearchAttribute attrib, 
	nsMsgSearchOperator op, 
	nsMsgSearchValue *val,
	nsMsgSearchBooleanOp boolOp,
	char * arbitraryHeader) 
{
	m_operator = op;
	m_attribute = attrib;
	m_booleanOp = boolOp;
	if (attrib == nsMsgSearchAttribOtherHeader && arbitraryHeader)
		m_arbitraryHeader = arbitraryHeader;
	nsMsgResultElement::AssignValues (val, &m_value);
}



nsMsgSearchTerm::~nsMsgSearchTerm ()
{
	if (IsStringAttribute (m_attribute))
		PR_Free(m_value.u.string);
}

// Perhaps we could find a better place for this?
// Caller needs to free.
/* static */char *nsMsgSearchTerm::EscapeQuotesInStr(const char *str)
{
	int	numQuotes = 0;
	for (const char *strPtr = str; *strPtr; strPtr++)
		if (*strPtr == '"')
			numQuotes++;
	int escapedStrLen = PL_strlen(str) + numQuotes;
	char	*escapedStr = (char *) PR_Malloc(escapedStrLen + 1);
	if (escapedStr)
	{
		char *destPtr;
		for (destPtr = escapedStr; *str; str++)
		{
			if (*str == '"')
				*destPtr++ = '\\';
			*destPtr++ = *str;
		}
		*destPtr = '\0';
	}
	return escapedStr;
}


nsresult nsMsgSearchTerm::OutputValue(nsString2 &outputStr)
{
	if (IsStringAttribute(m_attribute))
	{
		PRBool	quoteVal = PR_FALSE;
		// need to quote strings with ')' - filter code will escape quotes
		if (PL_strchr(m_value.u.string, ')'))
		{
			quoteVal = PR_TRUE;
			outputStr += "\"";
		}
		if (PL_strchr(m_value.u.string, '"'))
		{
			char *escapedString = nsMsgSearchTerm::EscapeQuotesInStr(m_value.u.string);
			if (escapedString)
			{
				outputStr += escapedString;
				PR_Free(escapedString);
			}

		}
		else
		{
			outputStr += m_value.u.string;
		}
		if (quoteVal)
			outputStr += "\"";
	}
	else
	{
	    switch (m_attribute)
		{
		case nsMsgSearchAttribDate:
		{
			struct tm *p = localtime(&m_value.u.date);
			if (p)
			{
				// wow, so tm_mon is 0 based, tm_mday is 1 based.
				char dateBuf[100];
				strftime (dateBuf, sizeof(dateBuf), "%d-%b-%Y", p);
				outputStr += dateBuf;
			}
			else 
				outputStr += "01/01/70";
			break;
		}
		case nsMsgSearchAttribMsgStatus:
		{
			nsString2 status(eOneByte);
			NS_MsgGetUntranslatedStatusName (m_value.u.msgStatus, &status);
			outputStr += status;
			break;
		}
		case nsMsgSearchAttribPriority:
		{
			nsString2 priority(eOneByte);
			NS_MsgGetUntranslatedPriorityName( m_value.u.priority, 
											 &priority);
			outputStr += priority;
			break;
		}
		default:
			NS_ASSERTION(PR_FALSE, "trying to output invalid attribute");
			break;
		}
	}
	return NS_OK;
}

nsresult nsMsgSearchTerm::EnStreamNew (nsString2 &outStream)
{
	const char	*attrib, *operatorStr;
	nsString2	outputStr(eOneByte);
	nsresult	ret;

	ret = NS_MsgGetStringForAttribute(m_attribute, &attrib);
	if (ret != NS_OK)
		return ret;

	if (m_attribute == nsMsgSearchAttribOtherHeader)  // if arbitrary header, use it instead!
	{
		outputStr = "\"";
		outputStr += m_arbitraryHeader;
		outputStr += "\"";
	}
	else
		outputStr = attrib;

	outputStr += ',';

	ret = NS_MsgGetStringForOperator(m_operator, &operatorStr);
	if (ret != NS_OK)
		return ret;

	outputStr += operatorStr;
	outputStr += ',';

	OutputValue(outputStr);
	outStream = outputStr;
	return NS_OK;
}

// fill in m_value from the input stream.
nsresult nsMsgSearchTerm::ParseValue(char *inStream)
{
	if (IsStringAttribute(m_attribute))
	{
		PRBool	quoteVal = PR_FALSE;
		while (IS_SPACE(*inStream))
			inStream++;

		// need to remove pair of '"', if present
		if (*inStream == '"')
		{
			quoteVal = PR_TRUE;
			inStream++;
		}
		int valueLen = PL_strlen(inStream);
		if (quoteVal && inStream[valueLen - 1] == '"')
			valueLen--;

		m_value.u.string = (char *) PR_Malloc(valueLen + 1);
		PL_strncpy(m_value.u.string, inStream, valueLen + 1);
		m_value.u.string[valueLen] = '\0';
	}
	else
	{
	    switch (m_attribute)
		{
		case nsMsgSearchAttribDate:
#ifdef DO_DATE_YET
			m_value.u.date = XP_ParseTimeString (inStream, PR_FALSE);
#endif
			break;
		case nsMsgSearchAttribMsgStatus:
			m_value.u.msgStatus = NS_MsgGetStatusValueFromName(inStream);
			break;
		case nsMsgSearchAttribPriority:
			NS_MsgGetPriorityFromString(inStream, &m_value.u.priority);
			break;
		default:
			NS_ASSERTION(PR_FALSE, "invalid attribute parsing search term value");
			break;
		}
	}
	m_value.attribute = m_attribute;
	return NS_OK;
}

// find the operator code for this operator string.
nsMsgSearchOperator nsMsgSearchTerm::ParseOperator(char *inStream)
{
	int16				operatorVal;
	nsresult		err;

	while (IS_SPACE(*inStream))
		inStream++;

	char *commaSep = PL_strchr(inStream, ',');

	if (commaSep)
		*commaSep = '\0';

	err = NS_MsgGetOperatorFromString(inStream, &operatorVal);
	return (nsMsgSearchOperator) operatorVal;
}

// find the attribute code for this comma-delimited attribute. 
nsMsgSearchAttribute nsMsgSearchTerm::ParseAttribute(char *inStream)
{
	nsString2			attributeStr(eOneByte);
	int16				attributeVal;
	nsresult		err;

	while (IS_SPACE(*inStream))
		inStream++;

	// if we are dealing with an arbitrary header, it may be quoted....
	PRBool quoteVal = PR_FALSE;
	if (*inStream == '"')
	{
		quoteVal = PR_TRUE;
		inStream++;
	}

	char *separator;
	if (quoteVal)      // arbitrary headers are quoted...
		separator = PL_strchr(inStream, '"');
	else
		separator = PL_strchr(inStream, ',');
	
	if (separator)
		*separator = '\0';

	err = NS_MsgGetAttributeFromString(inStream, &attributeVal);
	nsMsgSearchAttribute attrib = (nsMsgSearchAttribute) attributeVal;
	
	if (attrib == nsMsgSearchAttribOtherHeader)  // if we are dealing with an arbitrary header....
		m_arbitraryHeader =  inStream;
	
	return attrib;
}

// De stream one search term. If the condition looks like
// condition = "(to or CC, contains, r-thompson) AND (body, doesn't contain, fred)"
// This routine should get called twice, the first time
// with "to or CC, contains, r-thompson", the second time with
// "body, doesn't contain, fred"

nsresult nsMsgSearchTerm::DeStreamNew (char *inStream, int16 /*length*/)
{
	char *commaSep = PL_strchr(inStream, ',');
	m_attribute = ParseAttribute(inStream);  // will allocate space for arbitrary header if necessary
	if (!commaSep)
		return NS_ERROR_INVALID_ARG;
	char *secondCommaSep = PL_strchr(commaSep + 1, ',');
	if (commaSep)
		m_operator = ParseOperator(commaSep + 1);
	if (secondCommaSep)
		ParseValue(secondCommaSep + 1);
	return NS_OK;
}


void nsMsgSearchTerm::StripQuotedPrintable (unsigned char *src)
{
	// decode quoted printable text in place

	unsigned char *dest = src;
	int srcIdx = 0, destIdx = 0;
	
	while (src[srcIdx] != 0)
	{
		if (src[srcIdx] == '=')
		{
			unsigned char *token = &src[srcIdx];
			unsigned char c = 0;

			// decode the first quoted char
			if (token[1] >= '0' && token[1] <= '9')
				c = token[1] - '0';
			else if (token[1] >= 'A' && token[1] <= 'F')
				c = token[1] - ('A' - 10);
			else if (token[1] >= 'a' && token[1] <= 'f')
				c = token[1] - ('a' - 10);
			else
			{
				// first char after '=' isn't hex. copy the '=' as a normal char and keep going
				dest[destIdx++] = src[srcIdx++]; // aka token[0]
				continue;
			}
			
			// decode the second quoted char
			c = (c << 4);
			if (token[2] >= '0' && token[2] <= '9')
				c += token[2] - '0';
			else if (token[2] >= 'A' && token[2] <= 'F')
				c += token[2] - ('A' - 10);
			else if (token[2] >= 'a' && token[2] <= 'f')
				c += token[2] - ('a' - 10);
			else
			{
				// second char after '=' isn't hex. copy the '=' as a normal char and keep going
				dest[destIdx++] = src[srcIdx++]; // aka token[0]
				continue;
			}

			// if we got here, we successfully decoded a quoted printable sequence,
			// so bump each pointer past it and move on to the next char;
			dest[destIdx++] = c; 
			srcIdx += 3;

		}
		else
			dest[destIdx++] = src[srcIdx++];
	}

	dest[destIdx] = src[srcIdx]; // null terminate
}

#define EMPTY_MESSAGE_LINE(buf) (buf[0] == CR || buf[0] == LF || buf[0] == '\0')

// Looks in the MessageDB for the user specified arbitrary header, if it finds the header, it then looks for a match against
// the value for the header. 
nsresult nsMsgSearchTerm::MatchArbitraryHeader (nsMsgSearchScopeTerm *scope, PRUint32 offset, PRUint32 length /* in lines*/, const char *charset,
														nsIMsgDBHdr *msg, nsIMsgDatabase* db, char * headers, PRUint32 headersSize, PRBool ForFiltering)
{
	nsresult err = NS_COMFALSE;
	nsMsgBodyHandler * bodyHan = new nsMsgBodyHandler (scope, offset,length, msg, db, headers, headersSize, ForFiltering);
	if (!bodyHan)
		return NS_ERROR_OUT_OF_MEMORY;

	bodyHan->SetStripHeaders (PR_FALSE);

	if (MatchAllBeforeDeciding())
		err = NS_OK;
	else
		err = NS_COMFALSE;

	const int kBufSize = 512; // max size of a line??
	char * buf = (char *) PR_Malloc(kBufSize);
	if (buf)
	{
		PRBool searchingHeaders = PR_TRUE;
		while (searchingHeaders && bodyHan->GetNextLine(buf, kBufSize))
		{
			char * buf_end = buf + PL_strlen(buf);
			int headerLength = m_arbitraryHeader.Length();
			if (m_arbitraryHeader.Equals(buf))
			{
				char * headerValue = buf + headerLength; // value occurs after the header name...
				if (headerValue < buf_end && headerValue[0] == ':')  // + 1 to account for the colon which is MANDATORY
					headerValue++; 

				// strip leading white space
				while (headerValue < buf_end && IS_SPACE(*headerValue))
					headerValue++; // advance to next character
				
				// strip trailing white space
				char * end = buf_end - 1; 
				while (end > headerValue && IS_SPACE(*end)) // while we haven't gone back past the start and we are white space....
				{
					*end = '\0';	// eat up the white space
					end--;			// move back and examine the previous character....
				}
					
				if (headerValue < buf_end && *headerValue) // make sure buf has info besides just the header
				{
					nsString2 headerStr = headerValue;
					nsresult err2 = MatchString(&headerStr, charset);  // match value with the other info...
					if (err != err2) // if we found a match
					{
						searchingHeaders = PR_FALSE;   // then stop examining the headers
						err = err2;
					}
				}
				else
					NS_ASSERTION(PR_FALSE, "error matching arbitrary headers"); // mscott --> i'd be curious if there is a case where this fails....
			}
			if (EMPTY_MESSAGE_LINE(buf))
				searchingHeaders = PR_FALSE;
		}
		delete bodyHan;
		PR_Free(buf);
		return err;
	}
	else
	{
		delete bodyHan;
		return NS_ERROR_OUT_OF_MEMORY;
	}
}

nsresult nsMsgSearchTerm::MatchBody (nsMsgSearchScopeTerm *scope, PRUint32 offset, PRUint32 length /*in lines*/, const char *folderCharset,
										   nsIMsgDBHdr *msg, nsIMsgDatabase* db)
{
	nsresult err = NS_COMFALSE;

	// Small hack so we don't look all through a message when someone has
	// specified "BODY IS foo"
	if ((length > 0) && (m_operator == nsMsgSearchOpIs || m_operator == nsMsgSearchOpIsnt))
		length = PL_strlen (m_value.u.string);

	nsMsgBodyHandler * bodyHan  = new nsMsgBodyHandler (scope, offset, length, msg, db);
	if (!bodyHan)
		return NS_ERROR_OUT_OF_MEMORY;

	const int kBufSize = 512; // max size of a line???
#ifdef HAVE_I18N
	char *buf = (char*) PR_Malloc(kBufSize);
	if (buf)
	{
		PRBool endOfFile = PR_FALSE;  // if retValue == 0, we've hit the end of the file
		uint32 lines = 0;

		CCCDataObject conv = INTL_CreateCharCodeConverter();
		PRBool getConverter = PR_FALSE;
		int16 win_csid = INTL_DocToWinCharSetID(foldcsid);
		int16 mail_csid = INTL_DefaultMailCharSetID(win_csid);    // to default mail_csid (e.g. JIS for Japanese)
		if ((nsnull != conv) && INTL_GetCharCodeConverter(mail_csid, win_csid, conv)) 
			getConverter = PR_TRUE;

		// Change the sense of the loop so we don't bail out prematurely
		// on negative terms. i.e. opDoesntContain must look at all lines
		nsresult errContinueLoop;
		if (MatchAllBeforeDeciding())
			err = errContinueLoop = NS_OK;
		else
			err = errContinueLoop = NS_COMFALSE;

		// If there's a '=' in the search term, then we're not going to do
		// quoted printable decoding. Otherwise we assume everything is
		// quoted printable. Obviously everything isn't quoted printable, but
		// since we don't have a MIME parser handy, and we want to err on the
		// side of too many hits rather than not enough, we'll assume in that
		// general direction. Blech. ### FIX ME 
		// bug fix #88935: for stateful csids like JIS, we don't want to decode
		// quoted printable since it contains '='.
		PRBool isQuotedPrintable = !(mail_csid & STATEFUL) &&
										(PL_strchr (m_value.u.string, '=') == nsnull);

		while (!endOfFile && err == errContinueLoop)
		{
			if (bodyHan->GetNextLine(buf, kBufSize))
			{
				// Do in-place decoding of quoted printable
				if (isQuotedPrintable)
					StripQuotedPrintable ((unsigned char*)buf);

				char *compare = buf;
				if (getConverter) 
				{	
					// In here we do I18N conversion if we get the converter
					char *newBody = nsnull;
					newBody = (char *)INTL_CallCharCodeConverter(conv, (unsigned char *) buf, (int32) PL_strlen(buf));
					if (newBody && (newBody != buf))
					{
						// CharCodeConverter return the char* to the orginal string
						// we don't want to free body in that case 
						compare = newBody;
					}
				}
				if (*compare && *compare != CR && *compare != LF)
				{
					err = MatchString (compare, win_csid, PR_TRUE);
					lines++; 
				}
				if (compare != buf)
					XP_FREEIF(compare);
			}
			else 
				endOfFile = PR_TRUE;
		}

		if(conv) 
			INTL_DestroyCharCodeConverter(conv);
		PR_FREEIF(buf);
		delete bodyHan;
	}
	else
		err = NS_ERROR_OUT_OF_MEMORY;
#endif // HAVE_I18N
	return err;
}


// returns NS_COMFALSE when strings don't match, NS_OK if they do.
nsresult nsMsgSearchTerm::MatchString (nsString2 *stringToMatch, const char *charset, PRBool body)
{
	nsresult err = NS_COMFALSE;
	nsString2 n_str(eOneByte);
	const char* n_header = nsnull;
	if(nsMsgSearchOpIsEmpty != m_operator)	// Save some performance for opIsEmpty
	{
#ifdef DO_I18N
		n_str = INTL_GetNormalizeStr(csid , (unsigned char*)m_value.u.string);	// Always new buffer unless not enough memory
		if (!body)
			n_header = INTL_GetNormalizeStrFromRFC1522(csid , (unsigned char*)stringToMatch);	// Always new buffer unless not enough memory
		else
			n_header = INTL_GetNormalizeStr(csid , (unsigned char*)stringToMatch);	// Always new buffer unless not enough memory

		NS_ASSERTION(n_str, "failed get normalized string");
		NS_ASSERTION(n_header, "failed get normalized header");
#else
		n_header = stringToMatch->GetBuffer() ;
		n_str = m_value.u.string;
#endif // DO_I18N
	}
	switch (m_operator)
	{
	case nsMsgSearchOpContains:
		if ((nsnull != n_header) && (n_str[0]) && /* INTL_StrContains(csid, n_header, n_str) */
			stringToMatch->Find(n_str, PR_TRUE) != -1)
			err = NS_OK;
		break;
	case nsMsgSearchOpDoesntContain:
		if ((nsnull != n_header) && (n_str[0]) &&  /* !INTL_StrContains(csid, n_header, n_str) */
			stringToMatch->Find(n_str, PR_TRUE) == -1)
			err = NS_OK;
		break;
	case nsMsgSearchOpIs:
		if(n_header)
		{
			if (n_str[0])
			{
				if (n_str.Equals(*stringToMatch, PR_TRUE) != -1/* INTL_StrIs(csid, n_header, n_str)*/ )
					err = NS_OK;
			}
			else if (n_header[0] == '\0') // Special case for "is <the empty string>"
				err = NS_OK;
		}
		break;
	case nsMsgSearchOpIsnt:
		if(n_header)
		{
			if (n_str[0])
			{
				if (!n_str.Equals(*stringToMatch, PR_TRUE)/* INTL_StrIs(csid, n_header, n_str)*/ )
					err = NS_OK;
			}
			else if (n_header[0] != '\0') // Special case for "isn't <the empty string>"
				err = NS_OK;
		}
		break;
	case nsMsgSearchOpIsEmpty:
		if (stringToMatch->Length() == 0)
			err = NS_OK;
		break;
	case nsMsgSearchOpBeginsWith:
#ifdef DO_I18N_YET
		if((nsnull != n_str) && (nsnull != n_header) && INTL_StrBeginWith(csid, n_header, n_str))
			err = NS_OK;
#else
		// ### DMB - not the  most efficient way to do this.
		if (stringToMatch->Find(n_str, PR_TRUE) == 0)
			err = NS_OK;
#endif
		break;
	case nsMsgSearchOpEndsWith: 
#ifdef DO_I18N_YET
		{
		if((nsnull != n_str) && (nsnull != n_header) && INTL_StrEndWith(csid, n_header, n_str))
			err = NS_OK;
		}
#else
		NS_ASSERTION(PR_FALSE, "not implemented yet");
#endif
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid operator matching search results");
	}


	return err;
}

PRBool nsMsgSearchTerm::MatchAllBeforeDeciding ()
{
	if (m_operator == nsMsgSearchOpDoesntContain || m_operator == nsMsgSearchOpIsnt)
		return PR_TRUE;
	return PR_FALSE;
}


nsresult nsMsgSearchTerm::MatchRfc822String (const char *string, const char *charset)
{
	nsresult err = InitHeaderAddressParser();
	if (!NS_SUCCEEDED(err))
		return err;
	// Isolate the RFC 822 parsing weirdnesses here. MSG_ParseRFC822Addresses
	// returns a catenated string of null-terminated strings, which we walk
	// across, tring to match the target string to either the name OR the address

	char *names = nsnull, *addresses = nsnull;

	// Change the sense of the loop so we don't bail out prematurely
	// on negative terms. i.e. opDoesntContain must look at all recipients
	nsresult errContinueLoop;
	if (MatchAllBeforeDeciding())
		err = errContinueLoop = NS_OK;
	else
		err = errContinueLoop = NS_COMFALSE;

	PRUint32 count;
	nsresult parseErr = m_headerAddressParser->ParseHeaderAddresses(charset, string, &names, &addresses, count) ;

	if (NS_SUCCEEDED(parseErr) && count > 0)
	{
		NS_ASSERTION(names, "couldn't get names");
		NS_ASSERTION(addresses, "couldn't get addresses");
		if (!names || !addresses)
			return err;

		nsString2 walkNames(names, eOneByte);
		nsString2 walkAddresses(addresses, eOneByte);
		PRInt32 namePos = 0;
		PRInt32 addressPos = 0;
		for (int i = 0; i < count && err == errContinueLoop; i++)
		{
			err = MatchString (&walkNames, charset);
			if (errContinueLoop == err)
				err = MatchString (&walkAddresses, charset);

			namePos += walkNames.Length() + 1;
			addressPos += walkAddresses.Length() + 1;
			walkNames = names + namePos;
			walkAddresses = addresses + addressPos;;
		}

		PR_FREEIF(names);
		PR_FREEIF(addresses);
	}
	return err;
}


nsresult nsMsgSearchTerm::GetLocalTimes (time_t a, time_t b, struct tm &aTm, struct tm &bTm)
{
	// Isolate the RTL time weirdnesses here: 
	// (1) Must copy the tm since localtime has a static tm
	// (2) localtime can fail if it doesn't like the time_t. Must check the tm* for nsnull

	struct tm *p = localtime(&a);
	if (p)
	{
		memcpy (&aTm, p, sizeof(struct tm));
		p = localtime(&b);
		if (p) 
		{
			memcpy (&bTm, p, sizeof(struct tm));
			return NS_OK;
		}
	}
	return NS_ERROR_INVALID_ARG;
}


nsresult nsMsgSearchTerm::MatchDate (time_t dateToMatch)
{
	nsresult err = NS_COMFALSE;
	switch (m_operator)
	{
	case nsMsgSearchOpIsBefore:
		if (dateToMatch < m_value.u.date)
			err = NS_OK;
		break;
	case nsMsgSearchOpIsAfter:
		{
			time_t adjustedDate = m_value.u.date + 60*60*24; // we want to be greater than the next day....
			if (dateToMatch > adjustedDate)
				err = NS_OK;
		}
		break;
	case nsMsgSearchOpIs:
		{
			struct tm tmToMatch, tmThis;
			if (NS_OK == GetLocalTimes (dateToMatch, m_value.u.date, tmToMatch, tmThis))
			{
				if (tmThis.tm_year == tmToMatch.tm_year &&
					tmThis.tm_mon == tmToMatch.tm_mon &&
					tmThis.tm_mday == tmToMatch.tm_mday)
					err = NS_OK;
			}
		}
		break;
	case nsMsgSearchOpIsnt:
		{
			struct tm tmToMatch, tmThis;
			if (NS_OK == GetLocalTimes (dateToMatch, m_value.u.date, tmToMatch, tmThis))
			{
				if (tmThis.tm_year != tmToMatch.tm_year ||
					tmThis.tm_mon != tmToMatch.tm_mon ||
					tmThis.tm_mday != tmToMatch.tm_mday)
					err = NS_OK;
			}
		}
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid compare op for dates");
	}
	return err;
}


nsresult nsMsgSearchTerm::MatchAge (time_t msgDate)
{
	nsresult err = NS_COMFALSE;
#ifdef DO_AGE_YET
	time_t now = XP_TIME();
	time_t matchDay = now - (m_value.u.age * 60 * 60 * 24);
	struct tm * matchTime = localtime(&matchDay);
	
	// localTime axes previous results so save these.
	int day = matchTime->tm_mday;
	int month = matchTime->tm_mon;
	int year = matchTime->tm_year;

	struct tm * msgTime = localtime(&msgDate);

	switch (m_operator)
	{
	case nsMsgSearchOpIsGreaterThan: // is older than 
		if (msgDate < matchDay)
			err = NS_OK;
		break;
	case nsMsgSearchOpIsLessThan: // is younger than 
		if (msgDate > matchDay)
			err = NS_OK;
		break;
	case nsMsgSearchOpIs:
		if (matchTime && msgTime)
			if ((day == msgTime->tm_mday) 
				&& (month == msgTime->tm_mon)
				&& (year == msgTime->tm_year))
				err = NS_OK;
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid compare op comparing msg age");
	}
#endif // DO_AGE_YET
	return err;
}


nsresult nsMsgSearchTerm::MatchSize (PRUint32 sizeToMatch)
{
	nsresult err = NS_COMFALSE;
	switch (m_operator)
	{
	case nsMsgSearchOpIsHigherThan:
		if (sizeToMatch > m_value.u.size)
			err = NS_OK;
		break;
	case nsMsgSearchOpIsLowerThan:
		if (sizeToMatch < m_value.u.size)
			err = NS_OK;
		break;
	default:
		break;
	}
	return err;
}


nsresult nsMsgSearchTerm::MatchStatus (PRUint32 statusToMatch)
{
	nsresult err = NS_COMFALSE;
	PRBool matches = PR_FALSE;

	if (statusToMatch & m_value.u.msgStatus)
		matches = PR_TRUE;

	switch (m_operator)
	{
	case nsMsgSearchOpIs:
		if (matches)
			err = NS_OK;
		break;
	case nsMsgSearchOpIsnt:
		if (!matches)
			err = NS_OK;
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid comapre op for msg status");
	}

	return err;	
}


nsresult nsMsgSearchTerm::MatchPriority (nsMsgPriority priorityToMatch)
{
	nsresult err = NS_COMFALSE;

	// Use this ugly little hack to get around the fact that enums don't have
	// integer compare operators
	int p1 = (int) priorityToMatch;
	int p2 = (int) m_value.u.priority;

	switch (m_operator)
	{
	case nsMsgSearchOpIsHigherThan:
		if (p1 > p2)
			err = NS_OK;
		break;
	case nsMsgSearchOpIsLowerThan:
		if (p1 < p2)
			err = NS_OK;
		break;
	case nsMsgSearchOpIs:
		if (p1 == p2)
			err = NS_OK;
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid match operator");
	}
	return err;
}

// Lazily initialize the rfc822 header parser we're going to use to do
// header matching.
nsresult nsMsgSearchTerm::InitHeaderAddressParser()
{
	nsresult res = NS_OK;
	NS_DEFINE_CID(kMsgHeaderParserCID, NS_MSGHEADERPARSER_CID);
    
	if (!m_headerAddressParser)
	{
		res = nsComponentManager::CreateInstance(kMsgHeaderParserCID,
                                       nsnull,
                                       nsIMsgHeaderParser::GetIID(),
                                       (void **) getter_AddRefs(m_headerAddressParser));
	}
	return res;
}

//-----------------------------------------------------------------------------
// nsMsgSearchScopeTerm implementation
//-----------------------------------------------------------------------------
nsMsgSearchScopeTerm::nsMsgSearchScopeTerm (nsMsgSearchScopeAttribute attribute, nsIMsgFolder *folder)
{
	m_attribute = attribute;
	m_folder = folder;
	m_searchServer = PR_TRUE;
}

nsMsgSearchScopeTerm::nsMsgSearchScopeTerm ()
{
	m_searchServer = PR_TRUE;
}

nsMsgSearchScopeTerm::~nsMsgSearchScopeTerm ()
{
}

// ### purely temporary
static PRBool NET_IsOffline()
{
	return PR_FALSE;
}

PRBool nsMsgSearchScopeTerm::IsOfflineNews()
{
	switch (m_attribute)
	{
	case nsMsgSearchScopeNewsgroup:
	case nsMsgSearchScopeAllSearchableGroups:
		if (NET_IsOffline() || !m_searchServer)
			return PR_TRUE;
		else
			return PR_FALSE;
	case nsMsgSearchScopeOfflineNewsgroup:
		return PR_TRUE;
	default:
		return PR_FALSE;
	}
}

PRBool nsMsgSearchScopeTerm::IsOfflineMail ()
{
	// Find out whether "this" mail folder is online or offline
	NS_ASSERTION(m_folder, "scope doesn't have folder");
//	if (m_folder->GetType() == FOLDER_IMAPMAIL && !NET_IsOffline() && m_searchServer)    // make sure we are not in offline IMAP (mscott)
//		return PR_FALSE;
	return PR_TRUE;  // if POP or IMAP in offline mode
}

PRBool nsMsgSearchScopeTerm::IsOfflineIMAPMail()
{
	// Find out whether "this" mail folder is an offline IMAP folder
	NS_ASSERTION(m_folder, "scope doesn't have folder");
//	if (m_folder->GetType() == FOLDER_IMAPMAIL && (NET_IsOffline() || !m_searchServer))
//		return PR_TRUE;
	return PR_FALSE;       // we are not an IMAP folder that is offline
}

const char *nsMsgSearchScopeTerm::GetMailPath()
{
	return nsnull;
}

nsresult nsMsgSearchScopeTerm::TimeSlice ()
{
	return NS_OK;
}


nsresult nsMsgSearchScopeTerm::InitializeAdapter (nsMsgSearchTermArray &termList)
{
	return NS_OK;
}


char *nsMsgSearchScopeTerm::GetStatusBarName ()
{
	return nsnull;
}


//-----------------------------------------------------------------------------
// nsMsgResultElement implementation
//-----------------------------------------------------------------------------


nsMsgResultElement::nsMsgResultElement(nsIMsgSearchAdapter *adapter)
{
	m_adapter = adapter;
}


nsMsgResultElement::~nsMsgResultElement () 
{
	for (int i = 0; i < m_valueList.Count(); i++)
	{
		nsMsgSearchValue *value = m_valueList.ElementAt(i);
#ifdef HUH_WHATS_THIS
		if (value->attribute == nsMsgSearchAttribJpegFile)
		{
			char *url = XP_PlatformFileToURL (value->u.string);
			char *tmp = url + PL_strlen("file://");
			XP_FileRemove (tmp, xpMailFolder /*###phil hacky*/);
			XP_FREE(url);
		}
#endif
		nsMsgResultElement::DestroyValue (value);
	}
}


nsresult nsMsgResultElement::AddValue (nsMsgSearchValue *value)
{ 
	m_valueList.AppendElement (value); 
	return NS_OK;
}


nsresult nsMsgResultElement::DestroyValue (nsMsgSearchValue *value)
{
	if (IsStringAttribute(value->attribute))
	{
		NS_ASSERTION(value->u.string, "got null result value string");
		PR_Free (value->u.string);
	}
	delete value;
	return NS_OK;
}


nsresult nsMsgResultElement::AssignValues (nsMsgSearchValue *src, nsMsgSearchValue *dst)
{
	// Yes, this could be an operator overload, but nsMsgSearchValue is totally public, so I'd
	// have to define a derived class with nothing by operator=, and that seems like a bit much
	nsresult err = NS_OK;
	switch (src->attribute)
	{
	case nsMsgSearchAttribPriority:
		dst->attribute = src->attribute;
		dst->u.priority = src->u.priority;
		break;
	case nsMsgSearchAttribDate:
		dst->attribute = src->attribute;
		dst->u.date = src->u.date;
		break;
	case nsMsgSearchAttribMsgStatus:
		dst->attribute = src->attribute;
		dst->u.msgStatus = src->u.msgStatus;
		break;
	case nsMsgSearchAttribMessageKey:
		dst->attribute = src->attribute;
		dst->u.key = src->u.key;
		break;
	case nsMsgSearchAttribAgeInDays:
		dst->attribute = src->attribute;
		dst->u.age = src->u.age;
		break;
	default:
		if (src->attribute < kNumMsgSearchAttributes)
		{
			NS_ASSERTION(IsStringAttribute(src->attribute), "assigning non-string result");
			dst->attribute = src->attribute;
			dst->u.string = PL_strdup(src->u.string);
			if (!dst->u.string)
				err = NS_ERROR_OUT_OF_MEMORY;
		}
		else
			err = NS_ERROR_INVALID_ARG;
	}
	return err;
}


nsresult nsMsgResultElement::GetValue (nsMsgSearchAttribute attrib, nsMsgSearchValue **outValue) const
{
	nsresult err = NS_OK;
	nsMsgSearchValue *value = NULL;
	*outValue = NULL;

	for (int i = 0; i < m_valueList.Count() && err != NS_OK; i++)
	{
		value = m_valueList.ElementAt(i);
		if (attrib == value->attribute)
		{
			*outValue = new nsMsgSearchValue;
			if (*outValue)
			{
				err = AssignValues (value, *outValue);
				err = NS_OK;
			}
			else
				err = NS_ERROR_OUT_OF_MEMORY;
		}
	}
#ifdef HAVE_SEARCH_PORT
	// No need to store the folderInfo separately; we can always get it if/when
	// we need it. This code is to support "view thread context" in the search dialog
	if (SearchError_ScopeAgreement == err && attrib == nsMsgSearchAttribFolderInfo)
	{
		nsMsgFolderInfo *targetFolder = m_adapter->FindTargetFolder (this);
		if (targetFolder)
		{
			*outValue = new nsMsgSearchValue;
			if (*outValue)
			{
				(*outValue)->u.folder = targetFolder;
				(*outValue)->attribute = nsMsgSearchAttribFolderInfo;
				err = NS_OK;
			}
		}
	}
#endif
	return err;
}


const nsMsgSearchValue *nsMsgResultElement::GetValueRef (nsMsgSearchAttribute attrib) const 
{
	nsMsgSearchValue *value =  NULL;
	for (int i = 0; i < m_valueList.Count(); i++)
	{
		value = m_valueList.ElementAt(i);
		if (attrib == value->attribute)
			return value;
	}
	return NULL;
}


nsresult nsMsgResultElement::GetPrettyName (nsMsgSearchValue **value)
{
	nsresult err = GetValue (nsMsgSearchAttribLocation, value);
#ifdef HAVE_SEARCH_PORT
	if (NS_OK == err)
	{
		nsMsgFolderInfo *folder = m_adapter->m_scope->m_folder;
		nsMsgNewsHost *host = NULL;
		if (folder)
		{
			// Find the news host because only the host knows whether pretty
			// names are supported. 
			if (FOLDER_CONTAINERONLY == folder->GetType())
				host = ((nsMsgNewsFolderInfoContainer*) folder)->GetHost();
			else if (folder->IsNews())
				host = folder->GetNewsFolderInfo()->GetHost();

			// Ask the host whether it knows pretty names. It isn't strictly
			// necessary to avoid calling folder->GetPrettiestName() since it
			// does the right thing. But we do have to find the folder from the host.
			if (host && host->QueryExtension ("LISTPNAMES"))
			{
				folder = host->FindGroup ((*value)->u.string);
				if (folder)
				{
					char *tmp = XP_STRDUP (folder->GetPrettiestName());
					if (tmp)
					{
						XP_FREE ((*value)->u.string);
						(*value)->u.string = tmp;
					}
				}
			}
		}
	}
#endif // HAVE_SEARCH_PORT
	return err;
}

int nsMsgResultElement::CompareByFolderInfoPtrs (const void *e1, const void *e2)
{
#ifdef HAVE_SEARCH_PORT
	nsMsgResultElement * re1 = *(nsMsgResultElement **) e1;
	nsMsgResultElement * re2 = *(nsMsgResultElement **) e2;

	// get the src folder for each one
	
	const nsMsgSearchValue * v1 = re1->GetValueRef(attribFolderInfo);
	const nsMsgSearchValue * v2 = re2->GetValueRef(attribFolderInfo);

	if (!v1 || !v2)
		return 0;

	return (v1->u.folder - v2->u.folder);
#else
	return -1;
#endif // HAVE_SEARCH_PORT
}



int nsMsgResultElement::Compare (const void *e1, const void *e2)
{
	int ret = 0;
#ifdef HAVE_SEARCH_PORT
	// Bad karma to cast away const, but they're my objects anway.
	// Maybe if we go through and const everything this should be a const ptr.
	nsMsgResultElement *re1 = *(nsMsgResultElement**) e1;
	nsMsgResultElement *re2 = *(nsMsgResultElement**) e2;

	NS_ASSERTION(re1->IsValid(), "invalid result element1 in resultElement::Compare");
	NS_ASSERTION(re2->IsValid(), "invalid result element2 in resultElement::Compare");

	nsMsgSearchAttribute attrib = re1->m_adapter->m_scope->m_frame->m_sortAttribute;

	const nsMsgSearchValue *v1 = re1->GetValueRef (attrib);
	const nsMsgSearchValue *v2 = re2->GetValueRef (attrib);

	if (!v1 || !v2)
		return ret; // search result doesn't contain the attrib we want to sort on

	switch (attrib)
	{
	case nsMsgSearchAttribDate:
		{
			// on Win 3.1, the requisite 'int' return type is a short, so use a 
			// real time_t for comparison
			time_t date = v1->u.date - v2->u.date;
			if (date)
				ret = ((long)date) < 0 ? -1 : 1;
			else
				ret = 0;
		}
		break;
	case nsMsgSearchAttribPriority:
		ret = v1->u.priority - v2->u.priority;
		break;
	case nsMsgSearchAttribMsgStatus:
		{
			// Here's a totally arbitrary sorting protocol for msg status
			uint32 s1, s2;

			s1 = v1->u.msgStatus & ~MSG_FLAG_REPLIED;
			s2 = v2->u.msgStatus & ~MSG_FLAG_REPLIED;
			if (s1 || s2)
				ret = s1 - s2;
			else
			{
				s1 = v1->u.msgStatus & ~MSG_FLAG_FORWARDED;
				s2 = v2->u.msgStatus & ~MSG_FLAG_FORWARDED;
				if (s1 || s2)
					ret = s1 - s2;
				else
				{
					s1 = v1->u.msgStatus & ~MSG_FLAG_READ;
					s2 = v2->u.msgStatus & ~MSG_FLAG_READ;
					if (s1 || s2)
						ret = s1 - s2;
					else
						// statuses don't contain any flags we're interested in, 
						// so they're equal as far as we care
						ret = 0;
				}
			}
		}
		break;
	default:
		if (attrib == nsMsgSearchAttribSubject)
		{
			// Special case for subjects, so "Re:foo" sorts under 'f' not 'r'
			const char *s1 = v1->u.string;
			const char *s2 = v2->u.string;
			msg_StripRE (&s1, NULL);
			msg_StripRE (&s2, NULL);
			ret = PL_strcasecomp (s1, s2);
		}
		else
			ret = strcasecomp (v1->u.string, v2->u.string);
	}
	// ### need different hack for this.
	// qsort's default sort order is ascending, so in order to get descending
	// behavior, we'll tell qsort a lie and reverse the comparison order.
	if (re1->m_adapter->m_scope->m_frame->m_descending && ret != 0)
		if (ret < 0)
			ret = 1;
		else
			ret = -1;

	// <0 --> e1 less than e2
	// 0  --> e1 equal to e2
	// >0 --> e1 greater than e2
#endif
	return ret;
}

#ifdef HAVE_SEARCH_PORT
MWContextType nsMsgResultElement::GetContextType ()
{
	MWContextType type=(MWContextType)0;
	switch (m_adapter->m_scope->m_attribute)
	{
	case nsMsgSearchScopeMailFolder:
		type = MWContextMailMsg;
		break;
	case nsMsgSearchScopeOfflineNewsgroup:    // added by mscott could be bug fix...
	case nsMsgSearchScopeNewsgroup:
	case nsMsgSearchScopeAllSearchableGroups:
		type = MWContextNewsMsg;
		break;
	case nsMsgSearchScopeLdapDirectory:
		type = MWContextBrowser;
		break;
	default:
		NS_ASSERTION(PR_FALSE, "invalid scope"); // should never happen
	}
	return type;
}

#endif
nsresult nsMsgResultElement::Open (void *window)
{
#ifdef HAVE_SEARCH_PORT
	// ###phil this is a little ugly, but I'm not inclined to invest more in it
	// until the libnet rework is done and I know what kind of context we'll end up with

	if (window)
	{
		if (m_adapter->m_scope->m_attribute != nsMsgSearchScopeLdapDirectory)
		{
			msgPane = (MSG_MessagePane *) window; 
			XP_ASSERT (MSG_MESSAGEPANE == msgPane->GetPaneType());
			return m_adapter->OpenResultElement (msgPane, this);
		}
		else
		{
			context = (MWContext*) window;
			XP_ASSERT (MWContextBrowser == context->type);
			msg_SearchLdap *thisAdapter = (msg_SearchLdap*) m_adapter;
			return thisAdapter->OpenResultElement (context, this);
		}
	}
#endif
	return NS_ERROR_NULL_POINTER;
}


