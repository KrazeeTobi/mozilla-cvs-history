/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 *   David Epstein <depstein@netscape.com> 
 */

// Tests.h : header file for QA test cases
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _TESTS_H
#define _TESTS_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "BrowserView.h"
#include "BrowserImpl.h"

/////////////////////////////////////////////////////////////////////////////
// CTESTS class

class CBrowserImpl;
class CBrowserView;

class CTests : public CWnd
{
public:
	CTests(nsIWebBrowser* mWebBrowser,
			   nsIBaseWindow* mBaseWindow,
			   nsIWebNavigation* mWebNav,
			   CBrowserImpl *mpBrowserImpl);
	virtual ~CTests();

	// Some helper methods

	// Mozilla interfaces
	//
	nsCOMPtr<nsIWebBrowser> qaWebBrowser;
	nsCOMPtr<nsIBaseWindow> qaBaseWindow;
	nsCOMPtr<nsIWebNavigation> qaWebNav;	

	CBrowserImpl	*qaBrowserImpl;

	// local test methods

	// local test variables
	nsresult rv;
	CString strMsg;
	char theUrl[200];
	char * uriSpec;
	PRBool exists;
	PRInt32 numEntries;
	PRInt32 theIndex;

	nsCOMPtr<nsIURI> theUri;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTests)
	protected:
	//}}AFX_VIRTUAL


	// Generated message map functions
protected:
	//{{AFX_MSG(CTests)
	afx_msg void OnUpdateNavBack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavForward(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
	afx_msg void OnTestsChangeUrl();
	afx_msg void OnTestsGlobalHistory();
	afx_msg void OnTestsCreateFile();
	afx_msg void OnTestsCreateprofile();
	afx_msg void OnTestsAddWebProgListener();
	afx_msg void OnTestsAddHistoryListener();
	afx_msg void OnInterfacesNsifile();
	afx_msg void OnInterfacesNsishistory();
	afx_msg void OnInterfacesNsiwebnav();
	afx_msg void OnInterfacesNsirequest();
	afx_msg void OnToolsRemoveGHPage();
	afx_msg void OnToolsRemoveAllGH();
	afx_msg void OnToolsTestYourMethod();
	afx_msg void OnToolsTestYourMethod2();
	afx_msg void OnVerifybugs70228();
    afx_msg void OnPasteTest();
    afx_msg void OnCopyTest();
    afx_msg void OnSelectAllTest();
    afx_msg void OnSelectNoneTest();
    afx_msg void OnCutSelectionTest();
    afx_msg void copyLinkLocationTest();
    afx_msg void canCopySelectionTest();
    afx_msg void canCutSelectionTest();
    afx_msg void canPasteTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// individual nsIFile tests
	void InitWithPathTest(nsILocalFile *);
	void AppendRelativePathTest(nsILocalFile *);
	void FileCreateTest(nsILocalFile *);
	void FileExistsTest(nsILocalFile *);
	void FileCopyTest(nsILocalFile *, nsILocalFile *);
	void FileMoveTest(nsILocalFile *, nsILocalFile *);

	// individual nsISHistory tests
	void GetCountTest(nsISHistory *, PRInt32 *);
	void GetIndexTest(nsISHistory *, PRInt32 *);
	void GetMaxLengthTest(nsISHistory *, PRInt32 *);
	void SetMaxLengthTest(nsISHistory *, PRInt32);
	void GetEntryAtIndexTest(nsISHistory *, nsIHistoryEntry *, PRInt32 theIndex);
	void GetURIHistTest(nsIHistoryEntry *);
	void GetTitleHistTest(nsIHistoryEntry *);
	void GetIsSubFrameTest(nsIHistoryEntry *);
	void GetSHEnumTest(nsISHistory*, nsISimpleEnumerator *);
	void SimpleEnumTest(nsISimpleEnumerator *);
	void PurgeHistoryTest(nsISHistory *, PRInt32);

	// individual nsIWebNavigation tests
	void CanGoBackTest();
	void GoBackTest();
	void CanGoForwardTest();
	void GoForwardTest();
	void GoToIndexTest();
	void LoadUriTest(char *, const unsigned long);
	void ReloadTest(const unsigned long);
	void StopUriTest(char *);
	void GetDocumentTest(void);
	void GetCurrentURITest(void);
	void GetSHTest(void);

public:
	// individual nsIRequest tests
	void static IsPendingReqTest(nsIRequest *);
	void static GetStatusReqTest(nsIRequest *);
	void static SuspendReqTest(nsIRequest *);
	void static ResumeReqTest(nsIRequest *);
	void static CancelReqTest(nsIRequest *);
	void static SetLoadGroupTest(nsIRequest *, nsILoadGroup *);
	void static GetLoadGroupTest(nsIRequest *);
};

#endif //_TESTS_H
