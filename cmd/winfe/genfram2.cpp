/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "stdafx.h"

#include "rosetta.h"
#include "ssl.h"
#include "secnav.h"
#include "wfemsg.h"
#include "printpag.h"
#include "prefapi.h"
#include "navfram.h"
#include "confhook.h"
#include "msgcom.h"
#include "prefinfo.h"
#include "prefs.h"
#ifdef EDITOR
#include "edt.h"
#endif // EDITOR
#include "nethelp.h"
#include "navfram.h"
#include "prefapi.h"
#ifdef MOZ_MAIL_NEWS
#include "mailpriv.h" 
#include "offlndlg.h" 
#endif //MOZ_MAIL_NEWS

#ifdef MOZ_MAIL_NEWS
extern "C" char *FE_GetProgramDirectory(char *buffer, int length);
#endif //MOZ_MAIL_NEWS

void CGenericFrame::OnSecurity()
{
  HG73537
  MWContext * pContext = GetMainContext()->GetContext();  
  
  if (pContext != NULL)
  {
	  CAbstractCX * pCX = ABSTRACTCX(pContext);   

	  if (pCX != NULL)
	  {      
		URL_Struct * pURL = pCX->CreateUrlFromHist(TRUE); 
  
		SECNAV_SecurityAdvisor(pContext, pURL);
	  }  
  }  

}

void CGenericFrame::OnUpdateSecurity(CCmdUI *pCmdUI)
{
	HG73537
	int status = XP_GetSecurityStatus(GetMainContext()->GetContext());	

	pCmdUI->Enable(status == SSL_SECURITY_STATUS_ON_LOW || status == SSL_SECURITY_STATUS_ON_HIGH);
}

#ifdef MOZ_OFFLINE
void CGenericFrame::OnUpdateOnlineStatus(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!NET_IsOffline());
}
#endif //MOZ_OFFLINE

void CGenericFrame::OnEnterIdle(UINT nWhy, CWnd* pWho )
{
    CFrameWnd::OnEnterIdle(nWhy, pWho);
}

CFrameWnd *CGenericFrame::GetFrameWnd()	
{
	return((CFrameWnd *)this);
}

void CGenericFrame::UpdateHistoryDialog()	{
}

BOOL CGenericFrame::PreTranslateMessage(MSG* pMsg)
{
	if (m_bDisableHotkeys && pMsg->message == WM_SYSCOMMAND && 
			pMsg->wParam == SC_CLOSE && pMsg->lParam == 0)
	    return TRUE;  // This is a hotkey close message so kill it.

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CGenericFrame::OnClose()
{
	//	See if the document will allow closing.
	//  Do not only query the active document as MFC does.
    //  Instead, query the main context document, and it will check children also.
	CDocument* pDocument = NULL;
//	if(m_NavCenterFrame) {
//		m_NavCenterFrame->DestroyWindow();
//		m_NavCenterFrame = NULL;
//    }

    if(GetMainWinContext())    {
        pDocument = (CDocument *)GetMainWinContext()->GetDocument();
    }

	if (pDocument != NULL && !pDocument->CanCloseFrame(this))
	{
		// document can't close right now -- don't close it
		return;
	}
	m_isClosing = TRUE;
    CFrameWnd::OnClose();
}

void CGenericFrame::OnFishCam()
{
	if(GetMainContext())	{
		GetMainContext()->NormalGetUrl("http://home.netscape.com/fishcam/fishcam.html");
	}
}

void CGenericFrame::OnOpenMailWindow()
{
#ifdef MOZ_MAIL_NEWS
	// Had to put this if condition here because
	//putting it inside of CFolderFrame::Open was causing
	//"Collabra Discussions" to bring up "AltMailMsgCenter"
	if(theApp.m_hPostalLib)
		FEU_AltMail_ShowMessageCenter(); 
	else 
		WFE_MSGOpenFolders();
#endif // MOZ_MAIL_NEWS
}

void CGenericFrame::OnOpenInboxWindow()
{
#ifdef MOZ_MAIL_NEWS
	WFE_MSGOpenInbox();
#endif
}

void CGenericFrame::OnOpenNewsWindow()
{
#ifdef MOZ_MAIL_NEWS
	WFE_MSGOpenNews();
#endif
}

void CGenericFrame::OnShowAddressBookWindow()
{
#ifdef MOZ_MAIL_NEWS
	WFE_MSGOpenAB();
#endif
}

#ifdef MOZ_MAIL_NEWS
void CGenericFrame::OnMigrationTools()
{
	WFE_MSGLaunchMigrationUtility(m_hWnd, FALSE, NULL);
}

void CGenericFrame::OnUpdateMigrationTools(CCmdUI* pCmdUI)
{
	char    executable [_MAX_PATH + 32];
	strcpy (executable, "");
	FE_GetProgramDirectory( executable, _MAX_PATH + 32 );
	if ( *executable )
	{
		strcat( executable, "import32.exe" );
	}
#ifdef XP_WIN32
	DWORD result = GetFileAttributes(executable);

	pCmdUI->Enable(result !=0xFFFFFFFF);
#endif
}
#endif //MOZ_MAIL_NEWS

void CGenericFrame::OnFileMailNew()
{
#ifdef MOZ_MAIL_NEWS
	if (GetMainContext()) 
	{
		if (WFE_MSGCheckWizard(this))
			MSG_Mail (GetMainContext()->GetContext());
	}
#endif
}

// Toggle Go offline, Go online menu item
//
void CGenericFrame::OnGoOffline()
{
#ifdef MOZ_OFFLINE
	BOOL bOnline;
	// This is now just a toggle.
	PREF_GetBoolPref("network.online", &bOnline);
	
	COfflineInfo *info = NULL;
	BOOL bProgress = FALSE;
	if(!bOnline)
	{
#ifdef MOZ_MAIL_NEWS
		//if we're offline then tell it to send mail when we come back online
		info = new COfflineInfo(FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE);
		CProgressDialog *pProgress = 
			  new CProgressDialog(this, NULL,COfflineDlg::ShutDownFrameCallBack,
			  info, szLoadString(IDS_SYNCHRONIZING), NULL,
			  COfflineDlg::ShowOnlineCallBack);	// need correct window title

		if(!pProgress->GetProgressShown())
		{
			PREF_SetBoolPref("network.online", TRUE);
		}
#endif //MOZ_MAIL_NEWS
	}
	else
	{
		//if we have a master then there could be mail/news activity and we
		//  need to call progress pane.
		if(WFE_MSGGetMasterValue() != NULL)
		{
#ifdef MOZ_MAIL_NEWS
			//if we're online then tell backend to do nothing (any synchronization
			//gets done through synchronize menu
			info = new COfflineInfo(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE);
			new CProgressDialog(this, NULL,COfflineDlg::ShutDownFrameCallBack,
				  info, szLoadString(IDS_SYNCHRONIZING));	// need correct window title
#endif // MOZ_MAIL_NEWS
		}
		else
		{
			PREF_SetBoolPref("network.online", FALSE);
		}

	}

	if(info)
	{
		delete info;
	}

#endif //MOZ_OFFLINE  
}		

void CGenericFrame::OnUpdateGoOffline(CCmdUI* pCmdUI)
{
#ifdef MOZ_OFFLINE
	BOOL bOnline;
	CString menuText;

	PREF_GetBoolPref("network.online", &bOnline);
	if (bOnline)
	{	//online now, set to offline
		menuText.LoadString(IDS_GO_OFFLINE);
	}
	else
	{	//offline now, set to online
		menuText.LoadString(IDS_GO_ONLINE);
	}
	pCmdUI->SetText(LPCTSTR(menuText));
	pCmdUI->Enable(!theApp.m_bSynchronizing);
#endif //MOZ_OFFLINE
}

#ifdef MOZ_MAIL_NEWS
void CGenericFrame::OnSynchronize()
{
#ifdef MOZ_OFFLINE
	if(!theApp.m_bSynchronizing)
	{
		WFE_Synchronize(this, FALSE);
	}
#endif //MOZ_OFFLINE
}

void CGenericFrame::OnUpdateSynchronize(CCmdUI *pCmdUI)
{
#ifdef MOZ_OFFLINE
    BOOL bOnline;
    PREF_GetBoolPref("network.online", &bOnline);
	//don't want it enabled if app is currently synchronizing.
    pCmdUI->Enable(bOnline && !theApp.m_bSynchronizing);
#endif //MOZ_OFFLINE
}
#endif //MOZ_MAIL_NEWS

void CGenericFrame::OnDoneGoingOffline()
{
#ifdef MOZ_OFFLINE
	// I'm sure this could happen at the worst possible time
	if (!GetMainContext() || !GetMainContext()->GetContext())
		return;

	MWContext *pContext = GetMainContext()->GetContext();
	if (NET_IsOffline() && !XP_IsContextBusy(pContext)) {
		FE_Progress(pContext, szLoadString(IDS_STATUS_OFFLINE));
	} else {
		FE_Progress(pContext, szLoadString(IDS_DOC_DONE));
	}
#endif // MOZ_OFFLINE
}

void CGenericFrame::OnLDAPSearch()
{

}

//	Set some options for printing.
void CGenericFrame::OnFilePageSetup() 
{
	CPrintPageSetup * cdlg = new CPrintPageSetup ( this );
	if(cdlg != NULL)	{
		cdlg->DoModal ( );
	} 
	delete cdlg;
}

void CGenericFrame::OnGoHistory()
{
    //theApp.CreateNewNavCenter(NULL, TRUE, HT_VIEW_HISTORY);
}

void CGenericFrame::OnPageFromWizard()
{
#ifdef EDITOR
	char * url=NULL;
	int bOK = PREF_CopyConfigString("internal_url.page_from_wizard.url",&url);
	if (bOK == PREF_NOERROR) {
		if(GetMainContext())	{
            if( EDT_IS_EDITOR(GetMainContext()->GetContext()) ){
                // Load into a new Navigator window
                FE_LoadUrl(url, LOAD_URL_NAVIGATOR);
            } else {
			    GetMainContext()->NormalGetUrl(url);
            }
		}
	}
	if (url) XP_FREE(url);
#endif /* EDITOR */
}

void CGenericFrame::OnLiveCall()
{
	LaunchConfEndpoint( NULL, GetSafeHwnd() );
}

void CGenericFrame::OnSysColorChange() 
{
	//	Update our idea of the current system info.
	sysInfo.UpdateInfo();
	prefInfo.SysColorChange();
	
	//	We'll need to do a reload to handle the current
	//		colors internal to the document.
//#ifdef GOLD
    // This fixes critical bug 27239
    //  put in ifdef GOLD to minizize impact on non-gold version - TEMPORARY:
    // TODO: MERGE THIS TO REMOVE IFDEF GOLD
	if(GetMainContext())	{
        CWinCX *pContext = GetMainWinContext();
        if( pContext && pContext->GetContext()){
#ifdef EDITOR
            if( EDT_IS_EDITOR(pContext->GetContext()) ) {
                // Relayout page without having to do NET_GetURL
                EDT_RefreshLayout(pContext->GetContext());
            } else 
#endif // EDITOR
            {
			    pContext->NiceReload();
		    }
        }
	}
//#else
//	if(GetMainContext())	{
//		GetMainContext()->NiceReload();
//	}
//#endif

	//	Hopfully they'll redraw us so that the frame takes
	//		on the new colors.
	CFrameWnd::OnSysColorChange();
}


////////////////////////////////////////////////////////////////////////////
// Pop up the preferences box to let the user modify program configuration
//

void
CGenericFrame::OnDisplayPreferences()
{
	wfe_DisplayPreferences(this);
}

// Pop up the preferences box to let the user modify program configuration
//
void CGenericFrame::OnPrefsMailNews()
{
	wfe_DisplayPreferences(this);
}


void CGenericFrame::OnHelpMenu()
{
	NetHelp( HELP_COMMUNICATOR );
}


LRESULT CGenericFrame::OnHelpMsg(WPARAM wParam, LPARAM lParam) 
{

	//currently we only handle this message for the find replace dialog.
 	CFindReplaceDialog * dlg = ::CFindReplaceDialog::GetNotifier(lParam);
	if (!dlg) 
		return FALSE;

	// if it's the current find replace dialog then call help
	if(dlg->m_hWnd == (HWND)wParam)
	{
		NetHelp("FIND_IN");
		return TRUE;
	}
    return(FALSE);

}

void CGenericFrame::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CNSGenFrame::OnActivate(nState, pWndOther, bMinimized);
}
void CGenericFrame::RefreshNewEncoding(int16 csid, BOOL bSave )
{
     m_iCSID = csid;
     XP_ASSERT(0); // subclass should overload this
}
int16 nIDToCsid(UINT nID)
{
     static int16 nid_to_csid[] = {
       CS_LATIN1,                  // ID_OPTIONS_ENCODING_1
       CS_LATIN2,                  // ID_OPTIONS_ENCODING_2
       CS_SJIS_AUTO,               // ID_OPTIONS_ENCODING_3
       CS_SJIS,                    // ID_OPTIONS_ENCODING_4
       CS_EUCJP,                   // ID_OPTIONS_ENCODING_5
       CS_BIG5,                    // ID_OPTIONS_ENCODING_6
       CS_CNS_8BIT,                // ID_OPTIONS_ENCODING_7
       CS_GB_8BIT,                 // ID_OPTIONS_ENCODING_8
       CS_KSC_8BIT_AUTO,           // ID_OPTIONS_ENCODING_9
       CS_USER_DEFINED_ENCODING,   // ID_OPTIONS_ENCODING_10
       CS_CP_1250,                 // ID_OPTIONS_ENCODING_11
       CS_CP_1251,                 // ID_OPTIONS_ENCODING_12
       CS_8859_5,                  // ID_OPTIONS_ENCODING_13
       CS_KOI8_R,                  // ID_OPTIONS_ENCODING_14
       CS_8859_7,                  // ID_OPTIONS_ENCODING_15
       CS_CP_1253,                 // ID_OPTIONS_ENCODING_16
       CS_8859_9,                  // ID_OPTIONS_ENCODING_17
       CS_UTF8,                    // ID_OPTIONS_ENCODING_18
       CS_UCS2,                    // ID_OPTIONS_ENCODING_19
       CS_UTF7,                    // ID_OPTIONS_ENCODING_20
       CS_ARMSCII8,		   // ID_OPTIONS_ENCODING_21
       CS_TIS620		   // ID_OPTIONS_ENCODING_22
     };

     if( nID >= ID_OPTIONS_ENCODING_1 && nID <= ID_OPTIONS_ENCODING_70)
     {
        int idx = nID - ID_OPTIONS_ENCODING_1;

        XP_ASSERT(idx < (sizeof(nid_to_csid)/ sizeof(int16)));

        int16 csid = nid_to_csid[nID - ID_OPTIONS_ENCODING_1];
        return csid;
     }
     XP_ASSERT(0);
     return 0;
}
void CGenericFrame::OnToggleEncoding(UINT nID)
{
	 int16 csid = nIDToCsid(nID);
     this->RefreshNewEncoding(csid);
}
void CGenericFrame::OnUpdateEncoding(CCmdUI* pCmdUI)
{
     BOOL bCheck;
     bCheck = FALSE;
     int16 csid = nIDToCsid(pCmdUI->m_nID);
     bCheck = (m_iCSID == csid) ? TRUE : FALSE;
     // handle special case here
     if(FALSE ==  bCheck)
     {
        switch(csid)
        {
          case CS_UCS2:
            bCheck = (m_iCSID == CS_UCS2_SWAP) ? TRUE : FALSE;
            break;
          case CS_KSC_8BIT:
            bCheck = (m_iCSID == CS_KSC_8BIT_AUTO) ? TRUE : FALSE;
            break;
          default:
            break;
        }
     }
     pCmdUI->SetCheck(bCheck);
}

BEGIN_MESSAGE_MAP(CNSGenFrame, CFrameWnd)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

void CNSGenFrame::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

CNSNavFrame *CNSGenFrame::GetDockedNavCenter()
{
    CNSNavFrame *pRetval = NULL;
    
    //  Do we have one?
    CWnd *pWnd = GetDescendantWindow(NC_IDW_OUTLINER);
    if(pWnd) 
	{
        CFrameWnd *pFrame = pWnd->GetParentFrame();
        if(pFrame && pFrame->IsKindOf(RUNTIME_CLASS(CNSNavFrame))) {
            pRetval = (CNSNavFrame *)pFrame;
        }
    }
    
    return(pRetval);
}

BOOL CNSGenFrame::AllowDocking()
{
    return(FALSE);
}


