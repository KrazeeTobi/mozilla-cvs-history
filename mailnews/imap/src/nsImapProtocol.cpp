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

// sorry, this has to be before the pre-compiled header
#define FORCE_PR_LOG /* Allow logging in the release build */
// as does this
#define NS_IMPL_IDS
#include "msgCore.h"  // for pre-compiled headers

#include "nsIServiceManager.h"
#include "nsICharsetConverterManager.h"

#include "nsMsgImapCID.h"
#include "nsIEventQueueService.h"

#include "nsImapCore.h"
#include "nsImapProtocol.h"
#include "nscore.h"
#include "nsImapProxyEvent.h"
#include "nsIMAPHostSessionList.h"
#include "nsIMAPBodyShell.h"
#include "nsImapServerResponseParser.h"
#include "nspr.h"
#include "plbase64.h"
#include "nsIWebShell.h"
#include "nsIImapService.h"
#include "nsISocketTransportService.h"
#include "nsXPIDLString.h"

#include "nsImapStringBundle.h"

PRLogModuleInfo *IMAP;

// netlib required files
#include "nsIStreamListener.h"
#include "nsIMsgIncomingServer.h"
#include "nsIImapIncomingServer.h"

// for temp message hack
#if defined(XP_UNIX) || defined(XP_BEOS)
#define MESSAGE_PATH "/tmp/tempMessage.eml"
#elif defined(XP_PC)
#define MESSAGE_PATH  "c:\\temp\\tempMessage.eml"
#elif defined(XP_MAC)
#define MESSAGE_PATH  "tempMessage.eml"
#endif


#define ONE_SECOND ((PRUint32)1000)    // one second
#define FOUR_K ((PRUint32)4096)

const char *kImapTrashFolderName = "Trash"; // **** needs to be localized ****

static NS_DEFINE_CID(kSocketTransportServiceCID, NS_SOCKETTRANSPORTSERVICE_CID);
static NS_DEFINE_IID(kIWebShell, NS_IWEB_SHELL_IID);
static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);
static NS_DEFINE_CID(kCImapService, NS_IMAPSERVICE_CID);
static NS_DEFINE_CID(kCharsetConverterManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);

#define OUTPUT_BUFFER_SIZE (4096*2) // mscott - i should be able to remove this if I can use nsMsgLineBuffer???

#define IMAP_DB_HEADERS "From To Cc Subject Date Priority X-Priority Message-ID References Newsgroups"

static const PRInt32 kImapSleepTime = 1000000;
static PRInt32 gPromoteNoopToCheckCount = 0;

// **** helper class for downloading line ****
TLineDownloadCache::TLineDownloadCache()
{
    fLineInfo = (msg_line_info *) PR_CALLOC(sizeof( msg_line_info));
    fLineInfo->adoptedMessageLine = fLineCache;
    fLineInfo->uidOfMessage = 0;
    fBytesUsed = 0;
}

TLineDownloadCache::~TLineDownloadCache()
{
    PR_FREEIF( fLineInfo);
}

PRUint32 TLineDownloadCache::CurrentUID()
{
    return fLineInfo->uidOfMessage;
}

PRUint32 TLineDownloadCache::SpaceAvailable()
{
    return kDownLoadCacheSize - fBytesUsed;
}

msg_line_info *TLineDownloadCache::GetCurrentLineInfo()
{
    return fLineInfo;
}
    
void TLineDownloadCache::ResetCache()
{
    fBytesUsed = 0;
}
    
PRBool TLineDownloadCache::CacheEmpty()
{
    return fBytesUsed == 0;
}

void TLineDownloadCache::CacheLine(const char *line, PRUint32 uid)
{
    PRUint32 lineLength = PL_strlen(line);
    NS_ASSERTION((lineLength + 1) <= SpaceAvailable(), 
                 "Oops... line length greater than space available");
    
    fLineInfo->uidOfMessage = uid;
    
    PL_strcpy(fLineInfo->adoptedMessageLine + fBytesUsed, line);
    fBytesUsed += lineLength;
}

/* the following macros actually implement addref, release and query interface for our component. */
NS_IMPL_THREADSAFE_ADDREF(nsImapProtocol)
NS_IMPL_THREADSAFE_RELEASE(nsImapProtocol)

NS_IMETHODIMP nsImapProtocol::QueryInterface(const nsIID &aIID, void** aInstancePtr)
{                                                                        
  if (NULL == aInstancePtr)
    return NS_ERROR_NULL_POINTER;
        
  *aInstancePtr = NULL;
                                                                         
  static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID); 
  static NS_DEFINE_IID(kIsThreadsafeIID, NS_ISTHREADSAFE_IID); 

  if (aIID.Equals(nsIStreamListener::GetIID())) 
  {
	  *aInstancePtr = (nsIStreamListener *) this;                                                   
	  NS_ADDREF_THIS();
	  return NS_OK;
  }
  if (aIID.Equals(nsIImapProtocol::GetIID()))
  {
	  *aInstancePtr = (nsIImapProtocol *) this;
	  NS_ADDREF_THIS();
	  return NS_OK;
  }
  if (aIID.Equals(kISupportsIID)) 
  {
	  *aInstancePtr = (void*) ((nsISupports*)this);
	  NS_ADDREF_THIS();
    return NS_OK;                                                        
  }                                                                      
  
  if (aIID.Equals(kIsThreadsafeIID)) 
  {
    return NS_OK;
  }
  return NS_NOINTERFACE;
}

nsImapProtocol::nsImapProtocol() : 
    m_currentCommand("",eOneByte),m_parser(*this), m_flagState(kImapFlagAndUidStateSize, PR_FALSE)
{
	NS_INIT_REFCNT();
	m_flags = 0;
	m_urlInProgress = PR_FALSE;
	m_socketIsOpen = PR_FALSE;
    m_connectionStatus = 0;
	m_hostSessionList = nsnull;
    
    // ***** Thread support *****
    m_thread = nsnull;
    m_dataAvailableMonitor = nsnull;
	m_urlReadyToRunMonitor = nsnull;
	m_pseudoInterruptMonitor = nsnull;
	m_dataMemberMonitor = nsnull;
	m_threadDeathMonitor = nsnull;
    m_eventCompletionMonitor = nsnull;
	m_waitForBodyIdsMonitor = nsnull;
	m_fetchMsgListMonitor = nsnull;
	m_fetchBodyListMonitor = nsnull;
    m_imapThreadIsRunning = PR_FALSE;
	m_currentServerCommandTagNumber = 0;
	m_active = PR_FALSE;
	m_threadShouldDie = PR_FALSE;
	m_pseudoInterrupted = PR_FALSE;
	m_nextUrlReadyToRun = PR_FALSE;
    m_trackingTime = PR_FALSE;
    LL_I2L(m_startTime, 0);
    LL_I2L(m_endTime, 0);
    LL_I2L(m_lastActiveTime, 0);
    m_tooFastTime = 0;
    m_idealTime = 0;
    m_chunkAddSize = 0;
    m_chunkStartSize = 0;
    m_maxChunkSize = 0;
    m_fetchByChunks = PR_FALSE;
    m_chunkSize = 0;
    m_chunkThreshold = 0;
    m_fromHeaderSeen = PR_FALSE;
    m_closeNeededBeforeSelect = PR_FALSE;
	m_needNoop = PR_FALSE;
	m_noopCount = 0;
	m_promoteNoopToCheckCount = 0;
	m_mailToFetch = PR_FALSE;
	m_fetchMsgListIsNew = PR_FALSE;
	m_fetchBodyListIsNew = PR_FALSE;

	m_checkForNewMailDownloadsHeaders = PR_TRUE;	// this should be on by default
    m_hierarchyNameState = kNoOperationInProgress;
    m_onlineBaseFolderExists = PR_FALSE;
    m_discoveryStatus = eContinue;

	// m_dataOutputBuf is used by Send Data
	m_dataOutputBuf = (char *) PR_CALLOC(sizeof(char) * OUTPUT_BUFFER_SIZE);
	m_allocatedSize = OUTPUT_BUFFER_SIZE;

	// used to buffer incoming data by ReadNextLineFromInput
	m_inputStreamBuffer = new nsMsgLineStreamBuffer(OUTPUT_BUFFER_SIZE, CRLF, PR_TRUE /* allocate new lines */, PR_FALSE /* leave CRLFs on the returned string */);
	m_currentBiffState = nsMsgBiffState_Unknown;

	m_userName = nsnull;
	m_hostName = nsnull;

	// where should we do this? Perhaps in the factory object?
	if (!IMAP)
		IMAP = PR_NewLogModule("IMAP");
}

nsresult nsImapProtocol::Initialize(nsIImapHostSessionList * aHostSessionList, nsIEventQueue * aSinkEventQueue)
{
	NS_PRECONDITION(aSinkEventQueue && aHostSessionList, 
             "oops...trying to initalize with a null sink event queue!");
	if (!aSinkEventQueue || !aHostSessionList)
        return NS_ERROR_NULL_POINTER;

    m_sinkEventQueue = dont_QueryInterface(aSinkEventQueue);
    m_hostSessionList = aHostSessionList; // no ref count...host session list has life time > connection
    m_parser.SetHostSessionList(aHostSessionList);
    m_parser.SetFlagState(&m_flagState);

	// Now initialize the thread for the connection and create appropriate monitors
	if (m_thread == nsnull)
    {
        m_dataAvailableMonitor = PR_NewMonitor();
		m_urlReadyToRunMonitor = PR_NewMonitor();
		m_pseudoInterruptMonitor = PR_NewMonitor();
		m_dataMemberMonitor = PR_NewMonitor();
		m_threadDeathMonitor = PR_NewMonitor();
        m_eventCompletionMonitor = PR_NewMonitor();
		m_waitForBodyIdsMonitor = PR_NewMonitor();
		m_fetchMsgListMonitor = PR_NewMonitor();
		m_fetchBodyListMonitor = PR_NewMonitor();

		SetFlag(IMAP_FIRST_PASS_IN_THREAD);
        m_thread = PR_CreateThread(PR_USER_THREAD, ImapThreadMain, (void*)
                                   this, PR_PRIORITY_NORMAL, PR_LOCAL_THREAD,
                                   PR_JOINABLE_THREAD, 0);
        NS_ASSERTION(m_thread, "Unable to create imap thread.\n");
    }
	return NS_OK;
}

nsImapProtocol::~nsImapProtocol()
{
	PR_FREEIF(m_userName);

	nsCRT::free(m_hostName);

	PR_FREEIF(m_dataOutputBuf);
	if (m_inputStreamBuffer)
		delete m_inputStreamBuffer;

    // **** We must be out of the thread main loop function
    NS_ASSERTION(m_imapThreadIsRunning == PR_FALSE, "Oops, thread is still running.\n");

    if (m_dataAvailableMonitor)
    {
        PR_DestroyMonitor(m_dataAvailableMonitor);
        m_dataAvailableMonitor = nsnull;
    }

	if (m_urlReadyToRunMonitor)
	{
		PR_DestroyMonitor(m_urlReadyToRunMonitor);
		m_urlReadyToRunMonitor = nsnull;
	}
	if (m_pseudoInterruptMonitor)
	{
		PR_DestroyMonitor(m_pseudoInterruptMonitor);
		m_pseudoInterruptMonitor = nsnull;
	}
	if (m_dataMemberMonitor)
	{
		PR_DestroyMonitor(m_dataMemberMonitor);
		m_dataMemberMonitor = nsnull;
	}
	if (m_threadDeathMonitor)
	{
		PR_DestroyMonitor(m_threadDeathMonitor);
		m_threadDeathMonitor = nsnull;
	}
    if (m_eventCompletionMonitor)
    {
        PR_DestroyMonitor(m_eventCompletionMonitor);
        m_eventCompletionMonitor = nsnull;
    }
	if (m_waitForBodyIdsMonitor)
	{
		PR_DestroyMonitor(m_waitForBodyIdsMonitor);
		m_waitForBodyIdsMonitor = nsnull;
	}
	if (m_fetchMsgListMonitor)
	{
		PR_DestroyMonitor(m_fetchMsgListMonitor);
		m_fetchMsgListMonitor = nsnull;
	}
	if (m_fetchBodyListMonitor)
	{
		PR_DestroyMonitor(m_fetchBodyListMonitor);
		m_fetchBodyListMonitor = nsnull;
	}
}

const char*
nsImapProtocol::GetImapHostName()
{
	if (!m_userName && m_runningUrl)
	{
		nsCOMPtr<nsIURI> url = do_QueryInterface(m_runningUrl);
		url->GetHost(&m_hostName);
	}

	return m_hostName;
}

const char*
nsImapProtocol::GetImapUserName()
{
	nsIMsgIncomingServer * server = m_server;
	if (!m_userName && server)
		server->GetUsername(&m_userName);
	return m_userName;
}

void
nsImapProtocol::SetupSinkProxy()
{
    if (m_runningUrl)
    {
        NS_ASSERTION(m_sinkEventQueue && m_thread, 
                     "fatal... null sink event queue or thread");
        nsresult res;

        if (!m_imapLog)
        {
            nsCOMPtr<nsIImapLog> aImapLog;
            res = m_runningUrl->GetImapLog(getter_AddRefs(aImapLog));
            if (NS_SUCCEEDED(res) && aImapLog)
            {
                nsImapLogProxy * alog = new nsImapLogProxy (aImapLog, this,
                                                m_sinkEventQueue, m_thread);
				m_imapLog = do_QueryInterface(alog);
            }
        }
                
        if (!m_imapMailFolderSink)
        {
            nsCOMPtr<nsIImapMailFolderSink> aImapMailFolderSink;
            res = m_runningUrl->GetImapMailFolderSink(getter_AddRefs(aImapMailFolderSink));
            if (NS_SUCCEEDED(res) && aImapMailFolderSink)
            {
                nsImapMailFolderSinkProxy * folderProxy = new nsImapMailFolderSinkProxy(aImapMailFolderSink,
                                                             this,
                                                             m_sinkEventQueue,
                                                             m_thread);
				m_imapMailFolderSink = do_QueryInterface(folderProxy);
            }
        }
        if (!m_imapMessageSink)
        {
            nsCOMPtr<nsIImapMessageSink> aImapMessageSink;
            res = m_runningUrl->GetImapMessageSink(getter_AddRefs(aImapMessageSink));
            if (NS_SUCCEEDED(res) && aImapMessageSink)
            {
                nsImapMessageSinkProxy * messageSink = new nsImapMessageSinkProxy(aImapMessageSink,
                                                       this,
                                                       m_sinkEventQueue,
                                                       m_thread);
				m_imapMessageSink = do_QueryInterface(messageSink);
            }
        }
        if (!m_imapExtensionSink)
        {
            nsCOMPtr<nsIImapExtensionSink> aImapExtensionSink;
            res = m_runningUrl->GetImapExtensionSink(getter_AddRefs(aImapExtensionSink));
            if(NS_SUCCEEDED(res) && aImapExtensionSink)
            {
                nsImapExtensionSinkProxy * extensionSink = new nsImapExtensionSinkProxy(aImapExtensionSink,
                                                           this,
                                                           m_sinkEventQueue,
                                                           m_thread);
				m_imapExtensionSink = do_QueryInterface(extensionSink);
            }
        }
        if (!m_imapMiscellaneousSink)
        {
            nsCOMPtr<nsIImapMiscellaneousSink> aImapMiscellaneousSink;
            res = m_runningUrl->GetImapMiscellaneousSink(getter_AddRefs(aImapMiscellaneousSink));
            if (NS_SUCCEEDED(res) && aImapMiscellaneousSink)
            {
                 nsImapMiscellaneousSinkProxy * miscSink = new
                    nsImapMiscellaneousSinkProxy(aImapMiscellaneousSink,
                                             this,
                                             m_sinkEventQueue,
                                             m_thread);
				m_imapMiscellaneousSink = do_QueryInterface(miscSink);
            }
        }
    }
}

// Setup With Url is intended to set up data which is held on a PER URL basis and not
// a per connection basis. If you have data which is independent of the url we are currently
// running, then you should put it in Initialize(). 
nsresult nsImapProtocol::SetupWithUrl(nsIURI * aURL, nsISupports* aConsumer)
{
    nsresult rv = NS_ERROR_FAILURE;
	NS_PRECONDITION(aURL, "null URL passed into Imap Protocol");
	if (aURL)
	{
        rv = aURL->QueryInterface(nsIImapUrl::GetIID(), 
                                           getter_AddRefs(m_runningUrl));
        if (NS_FAILED(rv)) return rv;

        if (!m_server)
            rv = m_runningUrl->GetServer(getter_AddRefs(m_server));

		if ( m_runningUrl && !m_channel /* and we don't have a transport yet */)
		{
			// extract the file name and create a file transport...
			PRInt32 port = IMAP_PORT;
			nsXPIDLCString hostName;

			NS_WITH_SERVICE(nsISocketTransportService, socketService, kSocketTransportServiceCID, &rv);

			if (NS_SUCCEEDED(rv) && aURL)
			{
				aURL->GetPort(&port);
				aURL->GetHost(getter_Copies(hostName));

				ClearFlag(IMAP_CONNECTION_IS_OPEN); 
				rv = socketService->CreateTransport(hostName, port, getter_AddRefs(m_channel));
				
				if (NS_SUCCEEDED(rv))
					rv = m_channel->OpenOutputStream(0 /* start position */, getter_AddRefs(m_outputStream));
			}
		} // if m_runningUrl
	} // if aUR
    
	return rv;
}


// when the connection is done processing the current state, free any per url state data...
void nsImapProtocol::ReleaseUrlState()
{
	m_runningUrl = null_nsCOMPtr();
	m_imapMailFolderSink = null_nsCOMPtr();
	m_imapMessageSink = null_nsCOMPtr();
	m_imapExtensionSink = null_nsCOMPtr();
	m_imapMiscellaneousSink = null_nsCOMPtr();
	// mscott - do we need to release all of our sinks here? will we re-use them on the next
	// url request? I'm guessing we need to release them even though we'll just re-acquire
	// them on the next request.
	// YES, we should release them - they may not be valid, because we may switch folders.

}

void nsImapProtocol::ImapThreadMain(void *aParm)
{
    nsImapProtocol *me = (nsImapProtocol *) aParm;
    NS_ASSERTION(me, "Yuk, me is null.\n");
    
    PR_CEnterMonitor(aParm);
    NS_ASSERTION(me->m_imapThreadIsRunning == PR_FALSE, 
                 "Oh. oh. thread is already running. What's wrong here?");
    if (me->m_imapThreadIsRunning)
    {
        PR_CExitMonitor(me);
        return;
    }

	// Wrap the PLQueue we're going to make in an nsIEventQueue
	nsresult result = NS_OK;
	NS_WITH_SERVICE(nsIEventQueueService, pEventQService, kEventQueueServiceCID, &result);
	if (NS_FAILED(result))
		return;
		
		PLEventQueue* aPLQueue = PL_CreateEventQueue("ImapProtocolThread",
                                       PR_GetCurrentThread());


	if (NS_FAILED(result = pEventQService->CreateFromPLEventQueue(aPLQueue, getter_AddRefs(me->m_eventQueue))))
		return;

    NS_ASSERTION(me->m_eventQueue, 
                 "Unable to create imap thread event queue.\n");
    if (!me->m_eventQueue)
    {
        PR_CExitMonitor(me);
        return;
    }
    me->m_imapThreadIsRunning = PR_TRUE;
    PR_CExitMonitor(me);

    // call the platform specific main loop ....
    me->ImapThreadMainLoop();

    me->m_eventQueue = null_nsCOMPtr();

    if (me->m_server)
    {
        nsCOMPtr<nsIImapIncomingServer>
            aImapServer(do_QueryInterface(me->m_server, &result));
        if (NS_SUCCEEDED(result))
            aImapServer->RemoveConnection(me);
    }
        
    me->m_runningUrl = null_nsCOMPtr();
    me->m_channel = null_nsCOMPtr();
    me->m_outputStream = null_nsCOMPtr();
    me->m_outputConsumer = null_nsCOMPtr();
    me->m_streamConsumer = null_nsCOMPtr();
    me->m_sinkEventQueue = null_nsCOMPtr();
    me->m_eventQueue = null_nsCOMPtr();
    me->m_server = null_nsCOMPtr();
    me->m_imapLog = null_nsCOMPtr();
    me->m_imapMailFolderSink = null_nsCOMPtr();
    me->m_imapExtensionSink = null_nsCOMPtr();
    me->m_imapMessageSink = null_nsCOMPtr();
    me->m_imapMiscellaneousSink = null_nsCOMPtr();

    NS_RELEASE(me);
}

PRBool
nsImapProtocol::ImapThreadIsRunning()
{
    PRBool retValue = PR_FALSE;
    PR_CEnterMonitor(this);
    retValue = m_imapThreadIsRunning;
    PR_CExitMonitor(this);
    return retValue;
}

NS_IMETHODIMP
nsImapProtocol::GetThreadEventQueue(nsIEventQueue **aEventQueue)
{
    // *** should subclassing PLEventQueue and ref count it ***
    // *** need to find a way to prevent dangling pointer ***
    // *** a callback mechanism or a semaphor control thingy ***
    
	PR_CEnterMonitor(this);
    if (aEventQueue)
	{
        *aEventQueue = m_eventQueue;
		NS_IF_ADDREF(*aEventQueue);
	}
    PR_CExitMonitor(this);
    return NS_OK;
}

NS_IMETHODIMP 
nsImapProtocol::NotifyFEEventCompletion()
{
    PR_EnterMonitor(m_eventCompletionMonitor);
    PR_Notify(m_eventCompletionMonitor);
    PR_ExitMonitor(m_eventCompletionMonitor);
    return NS_OK;
}

void
nsImapProtocol::WaitForFEEventCompletion()
{
    PR_EnterMonitor(m_eventCompletionMonitor);
    PR_Wait(m_eventCompletionMonitor, PR_INTERVAL_NO_TIMEOUT);
    PR_ExitMonitor(m_eventCompletionMonitor);
}

NS_IMETHODIMP
nsImapProtocol::TellThreadToDie(PRBool isSaveToClose)
{
    // **** jt - This routine should only be called by imap service.
    PR_CEnterMonitor(this);

    PRBool closeNeeded = GetServerStateParser().GetIMAPstate() ==
        nsImapServerResponseParser::kFolderSelected && isSaveToClose;
    nsString2 command("", eOneByte);
    nsresult rv;

    if (closeNeeded && GetDeleteIsMoveToTrash())
    {
        IncrementCommandTagNumber();
        command = GetServerCommandTag();
        command.Append(" close" CRLF);
        rv = SendData(command.GetBuffer());
    }
        
	IncrementCommandTagNumber();
	command = GetServerCommandTag();
	command.Append(" logout" CRLF);
	rv = SendData(command.GetBuffer());

    PR_EnterMonitor(m_threadDeathMonitor);
    m_threadShouldDie = PR_TRUE;
    PR_ExitMonitor(m_threadDeathMonitor);

    PR_EnterMonitor(m_eventCompletionMonitor);
    PR_NotifyAll(m_eventCompletionMonitor);
    PR_ExitMonitor(m_eventCompletionMonitor);

    PR_EnterMonitor(m_urlReadyToRunMonitor);
    PR_NotifyAll(m_urlReadyToRunMonitor);
    PR_ExitMonitor(m_urlReadyToRunMonitor);

    PR_EnterMonitor(m_dataAvailableMonitor);
    PR_Notify(m_dataAvailableMonitor);
    PR_ExitMonitor(m_dataAvailableMonitor);

    PR_CExitMonitor(this);

    return NS_OK;
}

NS_IMETHODIMP
nsImapProtocol::GetLastActiveTimeStamp(PRTime* aTimeStamp)
{
    PR_CEnterMonitor(this);
    if (aTimeStamp)
        *aTimeStamp = m_lastActiveTime;
    PR_CExitMonitor(this);
    return NS_OK;
}

void
nsImapProtocol::ImapThreadMainLoop()
{
	PRIntervalTime sleepTime = kImapSleepTime;
    // ****** please implement PR_LOG 'ing ******
    while (ImapThreadIsRunning() && !DeathSignalReceived())
    {
		// if we are making our first pass through this loop and
		// we already have a url to process then jump right in and
		// process the current url. Don't try to wait for the monitor
		// the first time because it may have already been signaled.
		if (TestFlag(IMAP_FIRST_PASS_IN_THREAD) && m_runningUrl)
		{
			ProcessCurrentURL();
			ClearFlag(IMAP_FIRST_PASS_IN_THREAD);
		}

        PR_EnterMonitor(m_urlReadyToRunMonitor);

        PR_Wait(m_urlReadyToRunMonitor, sleepTime);

		if (m_nextUrlReadyToRun && m_runningUrl)
			ProcessCurrentURL();

		m_nextUrlReadyToRun = PR_FALSE;

        PR_ExitMonitor(m_urlReadyToRunMonitor);
    }
    m_imapThreadIsRunning = PR_FALSE;
}

void nsImapProtocol::EstablishServerConnection()
{
	// mscott: I really haven't worried about how much of this establish server connection
	// stuff we *REALLY* need. For now I just want to read out the greeting so I never bother
	// the parser with it...
	char * serverResponse = CreateNewLineFromSocket(); // read in the greeting
	PR_FREEIF(serverResponse); // we don't care about the greeting yet...

	// record the fact that we've received a greeting for this connection so we don't ever
	// try to do it again..
	SetFlag(IMAP_RECEIVED_GREETING);

#ifdef UNREADY_CODE // mscott: I don't think we need to care about this stuff...
    while (!DeathSignalReceived() && (GetConnectionStatus() == MK_WAITING_FOR_CONNECTION))
    {
        TImapFEEvent *feFinishConnectionEvent = 
            new TImapFEEvent(FinishIMAPConnection,  // function to call
                             this,                  // for access to current
                                                    // entry and monitor
                             nil,
							 TRUE);
        
        fFEEventQueue->AdoptEventToEnd(feFinishConnectionEvent);
        IMAP_YIELD(PR_INTERVAL_NO_WAIT);
        
        // wait here for the connection finish io to finish
        WaitForFEEventCompletion();
		if (!DeathSignalReceived() && (GetConnectionStatus() == MK_WAITING_FOR_CONNECTION))
			WaitForIOCompletion();
        
    }       
    
	if (GetConnectionStatus() == MK_CONNECTED)
    {
        // get the one line response from the IMAP server
        char *serverResponse = CreateNewLineFromSocket();
        if (serverResponse)
		{
			if (!XP_STRNCASECMP(serverResponse, "* OK", 4))
			{
				SetConnectionStatus(0);
				preAuthSucceeded = FALSE;
				//if (!XP_STRNCASECMP(serverResponse, "* OK Netscape IMAP4rev1 Service 3.0", 35))
				//	GetServerStateParser().SetServerIsNetscape30Server();
			}
			else if (!XP_STRNCASECMP(serverResponse, "* PREAUTH", 9))
			{
				// we've been pre-authenticated.
				// we can skip the whole password step, right into the
				// kAuthenticated state
				GetServerStateParser().PreauthSetAuthenticatedState();

				// tell the master that we're authenticated
				XP_Bool loginSucceeded = TRUE;
				TImapFEEvent *alertEvent = 
					new TImapFEEvent(msgSetUserAuthenticated,  // function to call
							 this,                // access to current entry
							 (void *)loginSucceeded,
							 TRUE);            
				if (alertEvent)
				{
					fFEEventQueue->AdoptEventToEnd(alertEvent);
				}
				else
					HandleMemoryFailure();

				preAuthSucceeded = TRUE;
           		if (fCurrentBiffState == MSG_BIFF_Unknown)
           		{
           			fCurrentBiffState = MSG_BIFF_NoMail;
               		SendSetBiffIndicatorEvent(fCurrentBiffState);
           		}

				if (GetServerStateParser().GetCapabilityFlag() ==
					kCapabilityUndefined)
					Capability();

				if ( !(GetServerStateParser().GetCapabilityFlag() & 
						(kIMAP4Capability | 
						 kIMAP4rev1Capability | 
						 kIMAP4other) ) )
				{
					AlertUserEvent_UsingId(MK_MSG_IMAP_SERVER_NOT_IMAP4);
					SetCurrentEntryStatus(-1);			
					SetConnectionStatus(-1);        // stop netlib
					preAuthSucceeded = FALSE;
				}
				else
				{
					ProcessAfterAuthenticated();
					// the connection was a success
					SetConnectionStatus(0);
				}
			}
			else
			{
				preAuthSucceeded = FALSE;
				SetConnectionStatus(MK_BAD_CONNECT);
			}

			fNeedGreeting = FALSE;
			FREEIF(serverResponse);
		}
        else
            SetConnectionStatus(MK_BAD_CONNECT);
    }

    if ((GetConnectionStatus() < 0) && !DeathSignalReceived())
	{
		if (!MSG_Biff_Master_NikiCallingGetNewMail())
    		AlertUserEvent_UsingId(MK_BAD_CONNECT);
	}
#endif   
}

void nsImapProtocol::ProcessCurrentURL()
{
	PRBool	logonFailed = FALSE;
    
	// we used to check if the current running url was 
	// Reinitialize the parser
	GetServerStateParser().InitializeState();
	GetServerStateParser().SetConnected(PR_TRUE);

	// mscott - I believe whenever we get here that we will also have
	// a connection. Why? Because when we load the url we open the 
	// connection if it isn't open already....However, if we haven't received
	// the greeting yet, we need to make sure we strip it out of the input
	// before we start to do useful things...
	if (!TestFlag(IMAP_RECEIVED_GREETING))
		EstablishServerConnection();

	// Update the security status for this URL
#ifdef UNREADY_CODE
	UpdateSecurityStatus();
#endif

	// Step 1: If we have not moved into the authenticated state yet then do so
	// by attempting to logon.
	if (!DeathSignalReceived() && (GetConnectionStatus() >= 0) &&
            (GetServerStateParser().GetIMAPstate() == 
			 nsImapServerResponseParser::kNonAuthenticated))
    {
		/* if we got here, the server's greeting should not have been PREAUTH */
		if (GetServerStateParser().GetCapabilityFlag() == kCapabilityUndefined)
				Capability();
			
		if ( !(GetServerStateParser().GetCapabilityFlag() & (kIMAP4Capability | kIMAP4rev1Capability | 
					 kIMAP4other) ) )
		{
#ifdef UNREADY_CODE
			AlertUserEvent_UsingId(MK_MSG_IMAP_SERVER_NOT_IMAP4);

			SetCurrentEntryStatus(-1);			
			SetConnectionStatus(-1);        // stop netlib
#endif
		}
		else
		{
			logonFailed = !TryToLogon();
	    }
    } // if death signal not received

    if (!DeathSignalReceived() && (GetConnectionStatus() >= 0))
    {
		if (m_runningUrl)
			FindMailboxesIfNecessary();
		nsIImapUrl::nsImapState imapState;
		if (m_runningUrl)
			m_runningUrl->GetRequiredImapState(&imapState);
		if (imapState == nsIImapUrl::nsImapAuthenticatedState)
			ProcessAuthenticatedStateURL();
	    else   // must be a url that requires us to be in the selected stae 
			ProcessSelectedStateURL();

#ifdef DEBUG_bienvenu1
		nsresult rv;
		nsCOMPtr<nsIImapIncomingServer>	aImapServer  = do_QueryInterface(m_server, &rv);
		if (NS_SUCCEEDED(rv))
			aImapServer->RemoveConnection(this);
        TellThreadToDie(PR_TRUE);
#endif
		// The URL has now been processed
        if (!logonFailed && GetConnectionStatus() < 0)
            HandleCurrentUrlError();
#ifdef UNREADY_CODE
		if (GetServerStateParser().LastCommandSuccessful())
			SetCurrentEntryStatus(0);
#endif
        if (DeathSignalReceived())
        {
           	HandleCurrentUrlError();
        }
        else
        {
        }
	}
    else if (!logonFailed)
        HandleCurrentUrlError(); 

	nsresult rv = NS_OK;

	nsCOMPtr<nsIMsgMailNewsUrl> mailnewsurl = do_QueryInterface(m_runningUrl, &rv);
    if (NS_SUCCEEDED(rv) && mailnewsurl && m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->SetUrlState(this, mailnewsurl, PR_FALSE,
                                             NS_OK);  // we are done with this
                                                      // url.
    }
    m_lastActiveTime = PR_Now(); // ** jt -- is this the best place for time stamp
	PseudoInterrupt(FALSE);	// clear this, because we must be done interrupting?
    nsCOMPtr<nsISupports> copyState;

	if (m_runningUrl)
		rv = m_runningUrl->GetCopyState(getter_AddRefs(copyState));
    if (NS_SUCCEEDED(rv) && m_imapMiscellaneousSink && copyState)
    {
        m_imapMiscellaneousSink->CopyNextStreamMessage(this, copyState);
        WaitForFEEventCompletion();
    }

	// release this by hand so that we can load the next queued url without thinking
	// this connection is busy running a url.
	m_runningUrl = null_nsCOMPtr();

	// now try queued urls, now that we've released this connection.
	if (m_server && m_imapMiscellaneousSink)
	{
		nsCOMPtr<nsIImapIncomingServer>	aImapServer  = do_QueryInterface(m_server, &rv);
		if (NS_SUCCEEDED(rv))
		{
			rv = m_imapMiscellaneousSink->LoadNextQueuedUrl(this, aImapServer);
		}
	}
	// release the url as we are done with it...
	ReleaseUrlState();
}

void nsImapProtocol::ParseIMAPandCheckForNewMail(const char* commandString)
{
    if (commandString)
        GetServerStateParser().ParseIMAPServerResponse(commandString);
    else
        GetServerStateParser().ParseIMAPServerResponse(
            m_currentCommand.GetBuffer());
    // **** fix me for new mail biff state *****
}


/////////////////////////////////////////////////////////////////////////////////////////////
// we suppport the nsIStreamListener interface 
////////////////////////////////////////////////////////////////////////////////////////////
NS_IMETHODIMP nsImapProtocol::OnDataAvailable(nsIChannel * /* aChannel */, nsISupports *ctxt, nsIInputStream *aIStream, PRUint32 aSourceOffset, PRUint32 aLength)
{
    PR_CEnterMonitor(this);

    nsresult res = NS_OK;


    if(NS_SUCCEEDED(res) && aLength > 0)
    {
		// make sure m_inputStream is set to the right input stream...
		if (!m_inputStream)
			m_inputStream = dont_QueryInterface(aIStream);

        // if we received data, we need to signal the data available monitor...
		// Read next line from input will actually read the data out of the stream
		PR_EnterMonitor(m_dataAvailableMonitor);
        PR_Notify(m_dataAvailableMonitor);
		PR_ExitMonitor(m_dataAvailableMonitor);
	}

    PR_CExitMonitor(this);

	return res;
}

NS_IMETHODIMP nsImapProtocol::OnStartRequest(nsIChannel * /* aChannel */, nsISupports *ctxt)
{
    PR_CEnterMonitor(this);
	nsresult rv = NS_OK;
	nsCOMPtr<nsIMsgMailNewsUrl> mailnewsurl = do_QueryInterface(m_runningUrl, &rv);
    if (NS_SUCCEEDED(rv) && mailnewsurl && m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->SetUrlState(this, mailnewsurl, PR_TRUE,
                                             NS_OK);
    }
    PR_CExitMonitor(this);
	return NS_OK;
}

// stop binding is a "notification" informing us that the stream associated with aURL is going away. 
NS_IMETHODIMP nsImapProtocol::OnStopRequest(nsIChannel * /* aChannel */, nsISupports *ctxt, nsresult aStatus, const PRUnichar* aMsg)
{
    PR_CEnterMonitor(this);
	nsresult rv = NS_OK;
	nsCOMPtr<nsIMsgMailNewsUrl> mailnewsurl = do_QueryInterface(m_runningUrl, &rv);
    if (NS_SUCCEEDED(rv) && mailnewsurl && m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->SetUrlState(this, mailnewsurl, PR_FALSE,
                                             aStatus); // set change in url
    }
    m_channel = null_nsCOMPtr();
    m_outputStream = null_nsCOMPtr();
    m_inputStream = null_nsCOMPtr();
    PR_CExitMonitor(this);
	return NS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End of nsIStreamListenerSupport
//////////////////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP nsImapProtocol::GetStreamConsumer (nsISupports **result)
{
	if (result)
	{
		*result = m_streamConsumer;
		NS_IF_ADDREF(*result);
		return NS_OK;
	}
	return NS_ERROR_NULL_POINTER;
}

NS_IMETHODIMP
nsImapProtocol::GetRunningUrl(nsIURI **result)
{
    if (result && m_runningUrl)
        return m_runningUrl->QueryInterface(nsIURI::GetIID(), (void**)
                                            result);
    else
        return NS_ERROR_NULL_POINTER;
}

/*
 * Writes the data contained in dataBuffer into the current output stream. It also informs
 * the transport layer that this data is now available for transmission.
 * Returns a positive number for success, 0 for failure (not all the bytes were written to the
 * stream, etc). We need to make another pass through this file to install an error system (mscott)
 */

nsresult nsImapProtocol::SendData(const char * dataBuffer)
{
	PRUint32 writeCount = 0; 
    nsresult rv = NS_ERROR_NULL_POINTER;

    if (!m_channel)
        return NS_ERROR_FAILURE;

	if (dataBuffer && m_outputStream)
	{
        m_currentCommand = dataBuffer;
        Log("SendData", nsnull, dataBuffer);
		rv = m_outputStream->Write(dataBuffer, PL_strlen(dataBuffer), &writeCount);
        if (NS_FAILED(rv))
            TellThreadToDie(PR_FALSE);
	}

	return rv;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Begin protocol state machine functions...
//////////////////////////////////////////////////////////////////////////////////////////////

// LoadUrl takes a url, initializes all of our url specific data by calling SetupUrl.
// If we don't have a connection yet, we open the connection. Finally, we signal the 
// url to run monitor to let the imap main thread loop process the current url (it is waiting
// on this monitor). There is a contract that the imap thread has already been started b4 we
// attempt to load a url....
nsresult nsImapProtocol::LoadUrl(nsIURI * aURL, nsISupports * aConsumer)
{
	nsresult rv = NS_OK;
	if (aURL)
	{
		if (aConsumer)
			m_streamConsumer = do_QueryInterface(aConsumer, &rv);

		rv = SetupWithUrl(aURL, aConsumer); 
        if (NS_FAILED(rv)) return rv;
    	SetupSinkProxy(); // generate proxies for all of the event sinks in the url
        m_lastActiveTime = PR_Now();
		if (m_channel && m_runningUrl)
		{
			nsIImapUrl::nsImapAction imapAction;
			m_runningUrl->GetImapAction(&imapAction);

			// if we're running a select or delete all, do a noop first.
			// this should really be in the connection cache code when we know
			// we're pulling out a selected state connection, but maybe we
			// can get away with this.
			m_needNoop = (imapAction == nsIImapUrl::nsImapSelectFolder || imapAction == nsIImapUrl::nsImapDeleteAllMsgs);

			if (!TestFlag(IMAP_CONNECTION_IS_OPEN))
			{
				m_channel->AsyncRead(0, -1, aURL,this /* stream observer */);
				SetFlag(IMAP_CONNECTION_IS_OPEN);
			}

			// We now have a url to run so signal the monitor for url ready to be processed...
			PR_EnterMonitor(m_urlReadyToRunMonitor);
			m_nextUrlReadyToRun = PR_TRUE;
			PR_Notify(m_urlReadyToRunMonitor);
			PR_ExitMonitor(m_urlReadyToRunMonitor);

		} // if we have an imap url and a transport
	} // if we received a url!

	return rv;
}

NS_IMETHODIMP nsImapProtocol::IsBusy(PRBool &aIsConnectionBusy,
                                     PRBool &isInboxConnection)
{
	NS_LOCK_INSTANCE();
    nsresult rv = NS_OK;
	aIsConnectionBusy = PR_FALSE;
    isInboxConnection = PR_FALSE;
    if (!m_channel)
    {
        // ** jt -- something is really wrong kill the thread
        TellThreadToDie(PR_FALSE);
        rv = NS_ERROR_FAILURE;
    }
    else
    {
        if (m_runningUrl) // do we have a url? That means we're working on
                          // it... 
            aIsConnectionBusy = PR_TRUE;
        if (GetServerStateParser().GetSelectedMailboxName() && 
            PL_strcasecmp(GetServerStateParser().GetSelectedMailboxName(),
                          "Inbox") == 0)
            isInboxConnection = PR_TRUE;
        
    }
	NS_UNLOCK_INSTANCE();
	return rv;
}

NS_IMETHODIMP nsImapProtocol::CanHandleUrl(nsIImapUrl * aImapUrl, 
                                           PRBool & aCanRunUrl,
                                           PRBool & hasToWait)
{
	nsresult rv = NS_OK;
	NS_LOCK_INSTANCE();

	aCanRunUrl = PR_FALSE; // assume guilty until proven otherwise...
    hasToWait = PR_FALSE;

    PRBool isBusy = PR_FALSE;
    PRBool isInboxConnection = PR_FALSE;

    if (!m_channel)
    {
        // *** jt -- something is really wrong; it could be the dialer gave up
        // the connection or ip binding has been release by the operating
        // system; tell thread to die and return error failure
        TellThreadToDie(PR_FALSE);
        rv =  NS_ERROR_FAILURE;
    }
    else
    {
        IsBusy(isBusy, isInboxConnection);
        
        PRBool inSelectedState = GetServerStateParser().GetIMAPstate() ==
            nsImapServerResponseParser::kFolderSelected;
        
        nsString2 curUrlFolderName("", eOneByte);
        if (inSelectedState)
        {
            curUrlFolderName =
                GetServerStateParser().GetSelectedMailboxName();
        }
        else if (isBusy)
        {
            nsIImapUrl::nsImapState curUrlImapState;
            m_runningUrl->GetRequiredImapState(&curUrlImapState);
            if (curUrlImapState == nsIImapUrl::nsImapSelectedState)
            {
                curUrlFolderName = OnCreateServerSourceFolderPathString();
                inSelectedState = PR_TRUE;
            }
        }

        nsIImapUrl::nsImapState imapState;
        aImapUrl->GetRequiredImapState(&imapState);
        
        PRBool isSelectedStateUrl = imapState ==
            nsIImapUrl::nsImapSelectedState;
        
        nsCOMPtr<nsIMsgIncomingServer> server;
        rv = aImapUrl->GetServer(getter_AddRefs(server));
        if (NS_SUCCEEDED(rv))
        {
            // compare host/user between url and connection.
            char * urlHostName = nsnull;
            char * urlUserName = nsnull;
            rv = server->GetHostName(&urlHostName);
            if (NS_FAILED(rv)) return rv;
            rv = server->GetUsername(&urlUserName);
            if (NS_FAILED(rv)) return rv;
            if ((!GetImapHostName() || 
                 PL_strcasecmp(urlHostName, GetImapHostName()) == 0) &&
                (!GetImapUserName() || 
                 PL_strcasecmp(urlUserName, GetImapUserName()) == 0))
            {
                if (isSelectedStateUrl)
                {
                    if (inSelectedState)
                    {
                        // *** jt - in selected state can only run url with
                        // matching foldername
                        char *srcFolderName = nsnull;
                        rv = aImapUrl->CreateServerSourceFolderPathString(
                            &srcFolderName);
                        if (NS_SUCCEEDED(rv) && srcFolderName)
                        {
                            char* convertedName =
                                CreateUtf7ConvertedString(srcFolderName,
                                                          PR_TRUE);
                            if (convertedName)
                            {
                                PR_Free(srcFolderName);
                                srcFolderName = convertedName;
                            }
                            PRBool isInbox = 
                                PL_strcasecmp("Inbox", srcFolderName) == 0;
                            if (curUrlFolderName.Length() > 0)
                            {
                                PRBool matched = isInbox ?
                                    PL_strcasecmp(curUrlFolderName.GetBuffer(),
                                                  srcFolderName) == 0 : 
                                    PL_strcmp(curUrlFolderName.GetBuffer(),
                                              srcFolderName) == 0;
                                if (matched)
                                {
                                    if (isBusy)
                                        hasToWait = PR_TRUE;
                                    else
                                        aCanRunUrl = PR_TRUE;
                                }
                            }
                        }
                        PR_FREEIF(srcFolderName);
                    }
                }
                else // *** jt - an authenticated state url can be run in either
                    // authenticated or selected state
                {
                    if (!isBusy)
                        aCanRunUrl = PR_TRUE;
                }
                
                PR_FREEIF(urlHostName);
                PR_FREEIF(urlUserName);
            }
        }
    }
	NS_UNLOCK_INSTANCE();
	return rv;
}


// Command tag handling stuff
void nsImapProtocol::IncrementCommandTagNumber()
{
    sprintf(m_currentServerCommandTag,"%ld", (long) ++m_currentServerCommandTagNumber);
}

char *nsImapProtocol::GetServerCommandTag()
{
    return m_currentServerCommandTag;
}

void nsImapProtocol::ProcessSelectedStateURL()
{
    char *mailboxName = nsnull;
	nsIImapUrl::nsImapAction imapAction; 
	PRBool					bMessageIdsAreUids = PR_TRUE;
	imapMessageFlagsType	msgFlags = 0;
	const char					*hostName = GetImapHostName();
	nsString2				urlHost("",eOneByte);

	// this can't fail, can it?
	nsresult res;
    res = m_runningUrl->GetImapAction(&imapAction);
	m_runningUrl->MessageIdsAreUids(&bMessageIdsAreUids);
	m_runningUrl->GetMsgFlags(&msgFlags);

	mailboxName = OnCreateServerSourceFolderPathString();

    if (mailboxName && !DeathSignalReceived())
    {
		// OK, code here used to check explicitly for multiple connections to the inbox,
		// but the connection pool stuff should handle this now.
    	PRBool selectIssued = FALSE;
        if (GetServerStateParser().GetIMAPstate() == nsImapServerResponseParser::kFolderSelected)
        {
            if (GetServerStateParser().GetSelectedMailboxName() && 
                PL_strcmp(GetServerStateParser().GetSelectedMailboxName(),
                          mailboxName))
            {       // we are selected in another folder
            	if (m_closeNeededBeforeSelect)
                	Close();
                if (GetServerStateParser().LastCommandSuccessful()) 
                {
                	selectIssued = TRUE;
					AutoSubscribeToMailboxIfNecessary(mailboxName);
                    SelectMailbox(mailboxName);
                }
            }
            else if (!GetServerStateParser().GetSelectedMailboxName())
            {       // why are we in the selected state with no box name?
                SelectMailbox(mailboxName);
                selectIssued = TRUE;
            }
            else
            {
            	// get new message counts, if any, from server
            	ProgressEventFunctionUsingId (IMAP_STATUS_SELECTING_MAILBOX);
				if (m_needNoop)
				{
					m_noopCount++;
					if (gPromoteNoopToCheckCount > 0 && (m_noopCount % gPromoteNoopToCheckCount) == 0)
						Check();
					else
            			Noop();	// I think this is needed when we're using a cached connection
					m_needNoop = FALSE;
				}
            }
        }
        else
        {
            // go to selected state
			AutoSubscribeToMailboxIfNecessary(mailboxName);
            SelectMailbox(mailboxName);
            selectIssued = TRUE;
        }

		if (selectIssued)
		{
//			RefreshACLForFolderIfNecessary(mailboxName);
		}
        
        PRBool uidValidityOk = PR_TRUE;
        if (GetServerStateParser().LastCommandSuccessful() && selectIssued && 
           (imapAction != nsIImapUrl::nsImapSelectFolder) && (imapAction != nsIImapUrl::nsImapLiteSelectFolder))
        {
        	uid_validity_info *uidStruct = (uid_validity_info *) PR_Malloc(sizeof(uid_validity_info));
        	if (uidStruct)
        	{
        		uidStruct->returnValidity = kUidUnknown;
				uidStruct->hostName = hostName;
        		m_runningUrl->CreateCanonicalSourceFolderPathString(&uidStruct->canonical_boxname);
				if (m_imapMiscellaneousSink)
				{
					m_imapMiscellaneousSink->GetStoredUIDValidity(this, uidStruct);

			        WaitForFEEventCompletion();
			        
			        // error on the side of caution, if the fe event fails to set uidStruct->returnValidity, then assume that UIDVALIDITY
			        // did not role.  This is a common case event for attachments that are fetched within a browser context.
					if (!DeathSignalReceived())
						uidValidityOk = (uidStruct->returnValidity == kUidUnknown) || (uidStruct->returnValidity == GetServerStateParser().FolderUID());
			    }
			    else
			        HandleMemoryFailure();
			    
			    PR_FREEIF(uidStruct->canonical_boxname);
			    PR_Free(uidStruct);
        	}
        	else
        		HandleMemoryFailure();
        }
            
        if (GetServerStateParser().LastCommandSuccessful() && !DeathSignalReceived() && (uidValidityOk || imapAction == nsIImapUrl::nsImapDeleteAllMsgs))
        {

			switch (imapAction)
			{
			case nsIImapUrl::nsImapLiteSelectFolder:
				if (GetServerStateParser().LastCommandSuccessful() && m_imapMiscellaneousSink)
				{
					m_imapMiscellaneousSink->LiteSelectUIDValidity(this, GetServerStateParser().FolderUID());

					WaitForFEEventCompletion();
					// need to update the mailbox count - is this a good place?
					ProcessMailboxUpdate(FALSE);	// handle uidvalidity change
				}
				break;
            case nsIImapUrl::nsImapMsgFetch:
            {
                nsString2 messageIdString("",eOneByte);

                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
				// we dont want to send the flags back in a group
				// GetServerStateParser().ResetFlagInfo(0);
				if (HandlingMultipleMessages(messageIdString))
				{
					// multiple messages, fetch them all
						m_progressStringId =  IMAP_FOLDER_RECEIVING_MESSAGE_OF;
					
					m_progressIndex = 0;
					m_progressCount = CountMessagesInIdString(messageIdString);
					
	                FetchMessage(messageIdString, 
								 kEveryThingRFC822Peek,
								 bMessageIdsAreUids);
					m_progressStringId = 0;
				}
				else
				{
					// A single message ID

					// First, let's see if we're requesting a specific MIME part
					char *imappart = nsnull;
					m_runningUrl->GetImapPartToFetch(&imappart);
					if (imappart)
					{
						if (bMessageIdsAreUids)
						{
							// We actually want a specific MIME part of the message.
							// The Body Shell will generate it, even though we haven't downloaded it yet.

#ifdef DOING_MPOD
							IMAP_ContentModifiedType modType = GetShowAttachmentsInline() ? 
								IMAP_CONTENT_MODIFIED_VIEW_INLINE :
								IMAP_CONTENT_MODIFIED_VIEW_AS_LINKS;

							nsIMAPBodyShell *foundShell = TIMAPHostInfo::FindShellInCacheForHost(m_runningUrl->GetUrlHost(),
								GetServerStateParser().GetSelectedMailboxName(), messageIdString, modType);
							if (!foundShell)
							{
								// The shell wasn't in the cache.  Deal with this case later.
								Log("SHELL",NULL,"Loading part, shell not found in cache!");
								//PR_LOG(IMAP, out, ("BODYSHELL: Loading part, shell not found in cache!"));
								// The parser will extract the part number from the current URL.
								SetContentModified(modType);
								Bodystructure(messageIdString, bMessageIdsAreUids);
							}
							else
							{
								Log("SHELL", NULL, "Loading Part, using cached shell.");
								//PR_LOG(IMAP, out, ("BODYSHELL: Loading part, using cached shell."));
								SetContentModified(modType);
								foundShell->SetConnection(this);
								GetServerStateParser().UseCachedShell(foundShell);
								foundShell->Generate(imappart);
								GetServerStateParser().UseCachedShell(NULL);
							}
#endif // DOING_MPOD
						}
						else
						{
							// Message IDs are not UIDs.
							NS_ASSERTION(PR_FALSE, "message ids aren't uids");
						}
						PR_Free(imappart);
					}
					else
					{
						// downloading a single message: try to do it by bodystructure, and/or do it by chunks
						PRUint32 messageSize = GetMessageSize(messageIdString,
							bMessageIdsAreUids);
						// We need to check the format_out bits to see if we are allowed to leave out parts,
						// or if we are required to get the whole thing.  Some instances where we are allowed
						// to do it by parts:  when viewing a message, or its source
						// Some times when we're NOT allowed:  when forwarding a message, saving it, moving it, etc.
#ifdef DOING_MPOD
						PRBool allowedToBreakApart = (ce  && !DeathSignalReceived()) ? ce->URL_s->allow_content_change : FALSE;

						if (gMIMEOnDemand &&
							allowedToBreakApart && 
							!GetShouldFetchAllParts() &&
							GetServerStateParser().ServerHasIMAP4Rev1Capability() &&
							(messageSize > (uint32) gMIMEOnDemandThreshold) &&
							!m_runningUrl->MimePartSelectorDetected())	// if a ?part=, don't do BS.
						{
							// OK, we're doing bodystructure

							// Before fetching the bodystructure, let's check our body shell cache to see if
							// we already have it around.
							nsIMAPBodyShell *foundShell = NULL;
							IMAP_ContentModifiedType modType = GetShowAttachmentsInline() ? 
								IMAP_CONTENT_MODIFIED_VIEW_INLINE :
								IMAP_CONTENT_MODIFIED_VIEW_AS_LINKS;

							SetContentModified(modType);  // This will be looked at by the cache
							if (bMessageIdsAreUids)
							{
								foundShell = TIMAPHostInfo::FindShellInCacheForHost(m_runningUrl->GetUrlHost(),
									GetServerStateParser().GetSelectedMailboxName(), messageIdString, modType);
								if (foundShell)
								{
									Log("SHELL",NULL,"Loading message, using cached shell.");
									//PR_LOG(IMAP, out, ("BODYSHELL: Loading message, using cached shell."));
									foundShell->SetConnection(this);
									GetServerStateParser().UseCachedShell(foundShell);
									foundShell->Generate(NULL);
									GetServerStateParser().UseCachedShell(NULL);
								}
							}

							if (!foundShell)
								Bodystructure(messageIdString, bMessageIdsAreUids);
						}
						else
#endif // DOING_MPOD
						{
							// Not doing bodystructure.  Fetch the whole thing, and try to do
							// it in chunks.
							SetContentModified(IMAP_CONTENT_NOT_MODIFIED);
							FetchTryChunking(messageIdString, kEveryThingRFC822,
								bMessageIdsAreUids, NULL, messageSize, TRUE);
						}
					}
				}
            }
			break;
			case nsIImapUrl::nsImapExpungeFolder:
				Expunge();
				// note fall through to next cases.
			case nsIImapUrl::nsImapSelectFolder:
			case nsIImapUrl::nsImapSelectNoopFolder:
            	ProcessMailboxUpdate(TRUE);
				break;
            case nsIImapUrl::nsImapMsgHeader:
            {
                nsString2 messageIds("",eOneByte);
                m_runningUrl->CreateListOfMessageIdsString(&messageIds);

                    // we don't want to send the flags back in a group
            //        GetServerStateParser().ResetFlagInfo(0);
                FetchMessage(messageIds, 
                             kHeadersRFC822andUid,
                             bMessageIdsAreUids);
            }
			break;
            case nsIImapUrl::nsImapSearch:
            {
                nsString2 searchCriteriaString("",eOneByte);
                m_runningUrl->CreateSearchCriteriaString(&searchCriteriaString);
                Search(searchCriteriaString, bMessageIdsAreUids);
                    // drop the results on the floor for now
           }
			break;
            case nsIImapUrl::nsImapDeleteMsg:
            {
                nsString2 messageIdString("",eOneByte);
				
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
				if (HandlingMultipleMessages(messageIdString))
					ProgressEventFunctionUsingId (IMAP_DELETING_MESSAGES);
				else
					ProgressEventFunctionUsingId(IMAP_DELETING_MESSAGE);
                Store(messageIdString, "+FLAGS (\\Deleted)", 
                      bMessageIdsAreUids);
                
                if (GetServerStateParser().LastCommandSuccessful())
                {
                    delete_message_struct *deleteMsg = (delete_message_struct *) PR_Malloc (sizeof(delete_message_struct));

					// convert name back from utf7
					utf_name_struct *nameStruct = (utf_name_struct *) PR_Malloc(sizeof(utf_name_struct));
					char *convertedCanonicalName = NULL;
					if (nameStruct)
					{
						char *convertedName = CreateUtf7ConvertedString(GetServerStateParser().GetSelectedMailboxName(), PR_FALSE);
						if (convertedName)
						{
							m_runningUrl->AllocateCanonicalPath(convertedName, 
											kOnlineHierarchySeparatorUnknown, &convertedCanonicalName);
							PR_Free(convertedName);
						}
					}

					deleteMsg->onlineFolderName = convertedCanonicalName;
					deleteMsg->deleteAllMsgs = FALSE;
                    deleteMsg->msgIdString   = messageIdString.ToNewCString();	// storage adopted, do not delete
					if (m_imapMessageSink)
                    	m_imapMessageSink->NotifyMessageDeleted(this, deleteMsg);
					// notice we don't wait for this to finish...
                }
                else
                    HandleMemoryFailure();
            }
			break;
            case nsIImapUrl::nsImapDeleteAllMsgs:
            {
            	uint32 numberOfMessages = GetServerStateParser().NumberOfMessages();
            	if (numberOfMessages)
				{
					nsString2 messageIdString("1:*", eOneByte);
                    
					Store(messageIdString, "+FLAGS.SILENT (\\Deleted)",
                          PR_FALSE);	// use sequence #'s  
                
					if (GetServerStateParser().LastCommandSuccessful())
						Expunge();      // expunge messages with deleted flag
					if (GetServerStateParser().LastCommandSuccessful())
					{
						delete_message_struct *deleteMsg = (delete_message_struct *) PR_Malloc (sizeof(delete_message_struct));

						// convert name back from utf7
						utf_name_struct *nameStruct = (utf_name_struct *) PR_Malloc(sizeof(utf_name_struct));
						char *convertedCanonicalName = NULL;
						if (nameStruct)
						{
							char *convertedName = CreateUtf7ConvertedString(GetServerStateParser().GetSelectedMailboxName(), PR_FALSE);
							if (convertedName )
							{
								m_runningUrl->AllocateCanonicalPath(convertedName,
									kOnlineHierarchySeparatorUnknown, &convertedCanonicalName);
								PR_Free(convertedName);
							}
						}

						deleteMsg->onlineFolderName = convertedCanonicalName;
						deleteMsg->deleteAllMsgs = PR_TRUE;
						deleteMsg->msgIdString   = nil;

						if (m_imapMessageSink)
                    		m_imapMessageSink->NotifyMessageDeleted(this, deleteMsg);
					}
                
				}
#ifdef HAVE_PORT
                DeleteSubFolders(mailboxName);
#endif
            }
			break;
			case nsIImapUrl::nsImapAppendDraftFromFile:
			{
                OnAppendMsgFromFile();
			}
			break;
            case nsIImapUrl::nsImapAddMsgFlags:
            {
                nsString2 messageIdString("",eOneByte);
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
                        
                ProcessStoreFlags(messageIdString, bMessageIdsAreUids,
                                  msgFlags, PR_TRUE);
                
                    /*
                    if ( !DeathSignalReceived() &&
                    	  GetServerStateParser().Connected() &&
                         !GetServerStateParser().SyntaxError())
                    {
                        //if (msgFlags & kImapMsgDeletedFlag)
                        //    Expunge();      // expunge messages with deleted flag
                        Check();        // flush servers flag state
                    }
                    */
            }
			break;
            case nsIImapUrl::nsImapSubtractMsgFlags:
            {
                nsString2 messageIdString("",eOneByte);
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
                        
                ProcessStoreFlags(messageIdString, bMessageIdsAreUids,
                                  msgFlags, FALSE);
                    
            }
			break;
            case nsIImapUrl::nsImapSetMsgFlags:
            {
                nsString2 messageIdString("",eOneByte);
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
                        
                ProcessStoreFlags(messageIdString, bMessageIdsAreUids,
                                  msgFlags, TRUE);
                ProcessStoreFlags(messageIdString, bMessageIdsAreUids,
                                  ~msgFlags, FALSE);
            }
			break;
            case nsIImapUrl::nsImapBiff:
                PeriodicBiff(); 
			break;
            case nsIImapUrl::nsImapOnlineCopy:
            case nsIImapUrl::nsImapOnlineMove:
            {
                nsString2 messageIdString("",eOneByte);
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
                char *destinationMailbox = 
                    OnCreateServerDestinationFolderPathString();

                if (destinationMailbox)
                {
					if (imapAction == nsIImapUrl::nsImapOnlineMove) 
					{
						if (HandlingMultipleMessages(messageIdString))
							ProgressEventFunctionUsingIdWithString (IMAP_MOVING_MESSAGES_TO, destinationMailbox);
						else
							ProgressEventFunctionUsingIdWithString (IMAP_MOVING_MESSAGE_TO, destinationMailbox); 
					}
					else {
						if (HandlingMultipleMessages(messageIdString))
							ProgressEventFunctionUsingIdWithString (IMAP_COPYING_MESSAGES_TO, destinationMailbox);
						else
							ProgressEventFunctionUsingIdWithString (IMAP_COPYING_MESSAGE_TO, destinationMailbox); 
					}
                    Copy(messageIdString, destinationMailbox, bMessageIdsAreUids);
                    PR_FREEIF( destinationMailbox);
					ImapOnlineCopyState copyState;
					if (DeathSignalReceived())
						copyState = kInterruptedState;
					else
						copyState = GetServerStateParser().LastCommandSuccessful() ? 
                                kSuccessfulCopy : kFailedCopy;
					if (m_imapMessageSink)
						m_imapMessageSink->OnlineCopyReport(this, &copyState);
                    
                    if (GetServerStateParser().LastCommandSuccessful() &&
                        (imapAction == nsIImapUrl::nsImapOnlineMove))
                    {
                        Store(messageIdString, "+FLAGS (\\Deleted)",
                              bMessageIdsAreUids); 
                        
						PRBool storeSuccessful = GetServerStateParser().LastCommandSuccessful();
                        	
						if (m_imapMessageSink)
						{
							copyState = storeSuccessful ? kSuccessfulDelete : kFailedDelete;
							m_imapMessageSink->OnlineCopyReport(this, &copyState);
						}
                    }
                }
                else
                    HandleMemoryFailure();
            }
			break;
            case nsIImapUrl::nsImapOnlineToOfflineCopy:
			case nsIImapUrl::nsImapOnlineToOfflineMove:
            {
#ifdef HAVE_PORT
                char *messageIdString = nsnull;
                m_runningUrl->CreateListOfMessageIdsString(&messageIdString);
                if (messageIdString)
                {
					m_progressStringId =  XP_FOLDER_RECEIVING_MESSAGE_OF;
					m_progressIndex = 0;
					m_progressCount = CountMessagesInIdString(messageIdString);
					
					FetchMessage(messageIdString, 
                                 kEveryThingRFC822Peek,
                                 bMessageIdsAreUids);
                      
                    m_progressStringId = 0;           
                    OnlineCopyCompleted(
                        GetServerStateParser().LastCommandSuccessful() ? 
                                kSuccessfulCopy : kFailedCopy);

                    if (GetServerStateParser().LastCommandSuccessful() &&
                        (m_runningUrl->GetIMAPurlType() == TIMAPUrl::kOnlineToOfflineMove))
                    {
                        Store(messageIdString, "+FLAGS (\\Deleted)",
                              bMessageIdsAreUids); 
						PRBool storeSuccessful = GetServerStateParser().LastCommandSuccessful();

                        OnlineCopyCompleted( storeSuccessful ? kSuccessfulDelete : kFailedDelete);
                    }
                    
                    FREEIF( messageIdString);
                }
                else
                    HandleMemoryFailure();
#endif
            }
			default:
				if (GetServerStateParser().LastCommandSuccessful() && !uidValidityOk)
        			ProcessMailboxUpdate(FALSE);	// handle uidvalidity change
				break;
			}
        }
        PR_FREEIF( mailboxName);
    }
    else if (!DeathSignalReceived())
        HandleMemoryFailure();

}

void nsImapProtocol::AutoSubscribeToMailboxIfNecessary(const char *mailboxName)
{
#ifdef HAVE_PORT
	if (fFolderNeedsSubscribing)	// we don't know about this folder - we need to subscribe to it / list it.
	{
		fHierarchyNameState = kListingForInfoOnly;
		List(mailboxName, FALSE);
		fHierarchyNameState = kNoOperationInProgress;

		// removing and freeing it as we go.
		TIMAPMailboxInfo *mb = NULL;
		int total = XP_ListCount(fListedMailboxList);
		do
		{
			mb = (TIMAPMailboxInfo *) XP_ListRemoveTopObject(fListedMailboxList);
			delete mb;
		} while (mb);

		// if the mailbox exists (it was returned from the LIST response)
		if (total > 0)
		{
			// Subscribe to it, if the pref says so
			if (TIMAPHostInfo::GetHostIsUsingSubscription(fCurrentUrl->GetUrlHost()) && fAutoSubscribeOnOpen)
			{
				XP_Bool lastReportingErrors = GetServerStateParser().GetReportingErrors();
				GetServerStateParser().SetReportingErrors(FALSE);
				Subscribe(mailboxName);
				GetServerStateParser().SetReportingErrors(lastReportingErrors);
			}

			// Always LIST it anyway, to get it into the folder lists,
			// so that we can continue to perform operations on it, at least
			// for this session.
			fHierarchyNameState = kNoOperationInProgress;
			List(mailboxName, FALSE);
		}

		// We should now be subscribed to it, and have it in our folder lists
		// and panes.  Even if something failed, we don't want to try this again.
		fFolderNeedsSubscribing = FALSE;

	}
#endif
}


void nsImapProtocol::BeginMessageDownLoad(
    PRUint32 total_message_size, // for user, headers and body
    const char *content_type)
{
	char *sizeString = PR_smprintf("OPEN Size: %ld", total_message_size);
	Log("STREAM",sizeString,"Begin Message Download Stream");
	PR_FREEIF(sizeString);
    //total_message_size)); 
	StreamInfo *si = (StreamInfo *) PR_CALLOC (sizeof (StreamInfo));		// This will be freed in the event
	if (si)
	{
		si->size = total_message_size;
		si->content_type = PL_strdup(content_type);
		if (si->content_type)
		{

			if (GetServerStateParser().GetDownloadingHeaders())
			{
				if (m_imapMailFolderSink)
				{
					m_imapMailFolderSink->SetupHeaderParseStream(this, si);
					WaitForFEEventCompletion();
				}
			}
			else if (m_imapMessageSink) 
            {
                m_imapMessageSink->SetupMsgWriteStream(this, si);
				WaitForFEEventCompletion();
			}
            PL_strfree(si->content_type);
        }
		else
			HandleMemoryFailure();
        PR_Free(si);
	}
	else
		HandleMemoryFailure();
}

PRBool
nsImapProtocol::GetShouldDownloadArbitraryHeaders()
{
    // *** allocate instead of using local variable to be thread save ***
    GenericInfo *aInfo = (GenericInfo*) PR_CALLOC(sizeof(GenericInfo));
    const char *hostName = GetImapHostName();
    PRBool rv;
    aInfo->rv = PR_TRUE;         // default
    
	aInfo->hostName = PL_strdup (hostName);
    if (m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->GetShouldDownloadArbitraryHeaders(this, aInfo);
        WaitForFEEventCompletion();
    }
    rv = aInfo->rv;
    if (aInfo->hostName)
        PL_strfree(aInfo->hostName);
    if (aInfo->c)
        PL_strfree(aInfo->c);
    PR_Free(aInfo);
    return rv;
}

char*
nsImapProtocol::GetArbitraryHeadersToDownload()
{
    // *** allocate instead of using local variable to be thread save ***
    GenericInfo *aInfo = (GenericInfo*) PR_CALLOC(sizeof(GenericInfo));
    const char *hostName = GetImapHostName();
    char *rv = nsnull;
    aInfo->rv = PR_TRUE;         // default
    aInfo->hostName = PL_strdup (hostName);
    if (m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->GetShouldDownloadArbitraryHeaders(this, aInfo);
        WaitForFEEventCompletion();
    }
    if (aInfo->hostName)
        PL_strfree(aInfo->hostName);
    rv = aInfo->c;
    PR_Free(aInfo);
    return rv;
}

void
nsImapProtocol::AdjustChunkSize()
{
	PRInt32 t32;

	m_endTime = PR_Now();
	m_trackingTime = FALSE;
	PRTime t;
	LL_SUB(t, m_endTime, m_startTime);
	if (! LL_GE_ZERO(t))
		return;							// bogus for some reason
	
	LL_L2I(t32, t);
	if (t32 <= m_tooFastTime) {
		m_chunkSize += m_chunkAddSize;
		m_chunkThreshold = m_chunkSize + (m_chunkSize / 2);
		if (m_chunkSize > m_maxChunkSize)
			m_chunkSize = m_maxChunkSize;
	}
	else if (t32 <= m_idealTime)
		return;
	else {
		if (m_chunkSize > m_chunkStartSize)
			m_chunkSize = m_chunkStartSize;
		else if (m_chunkSize > (m_chunkAddSize * 2))
			m_chunkSize -= m_chunkAddSize;
		m_chunkThreshold = m_chunkSize + (m_chunkSize / 2);
	}
}

// authenticated state commands 
// escape any backslashes or quotes.  Backslashes are used a lot with our NT server
char *nsImapProtocol::CreateEscapedMailboxName(const char *rawName)
{
	nsString2 escapedName(rawName, eOneByte);

	for (PRInt32 strIndex = 0; *rawName; strIndex++)
	{
		char currentChar = *rawName++;
		if ((currentChar == '\\') || (currentChar == '\"'))
		{
			escapedName.Insert('\\', strIndex++);
		}
	}
	return escapedName.ToNewCString();
}

void nsImapProtocol::SelectMailbox(const char *mailboxName)
{
    ProgressEventFunctionUsingId (IMAP_STATUS_SELECTING_MAILBOX);
    IncrementCommandTagNumber();
    
    m_closeNeededBeforeSelect = PR_FALSE;		// initial value
	GetServerStateParser().ResetFlagInfo(0);    
    char *escapedName = CreateEscapedMailboxName(mailboxName);
    nsString2 commandBuffer(GetServerCommandTag(), eOneByte);
	commandBuffer.Append(" select \"");
	commandBuffer.Append(escapedName);
	commandBuffer.Append("\"" CRLF);

    delete []escapedName;
    nsresult res;       
    res = SendData(commandBuffer.GetBuffer());
    if (NS_FAILED(res)) return;
	ParseIMAPandCheckForNewMail();

	PRInt32 numOfMessagesInFlagState = 0;
	nsIImapUrl::nsImapAction imapAction; 
	m_flagState.GetNumberOfMessages(&numOfMessagesInFlagState);
	res = m_runningUrl->GetImapAction(&imapAction);
	// if we've selected a mailbox, and we're not going to do an update because of the
	// url type, but don't have the flags, go get them!
	if (NS_SUCCEEDED(res) &&
		imapAction != nsIImapUrl::nsImapSelectFolder && imapAction != nsIImapUrl::nsImapExpungeFolder 
		&& imapAction != nsIImapUrl::nsImapLiteSelectFolder &&
		imapAction != nsIImapUrl::nsImapDeleteAllMsgs && 
		((GetServerStateParser().NumberOfMessages() != numOfMessagesInFlagState) && (numOfMessagesInFlagState == 0))) 
	{
	   	ProcessMailboxUpdate(PR_FALSE);	
	}
}

// Please call only with a single message ID
void nsImapProtocol::Bodystructure(const char *messageId, PRBool idIsUid)
{
    IncrementCommandTagNumber();
    
    nsString2 commandString(GetServerCommandTag(), eOneByte);
    if (idIsUid)
    	commandString.Append(" UID");
   	commandString.Append(" fetch ");

	commandString.Append(messageId);
	commandString.Append("  (BODYSTRUCTURE)" CRLF);

	            
    nsresult rv = SendData(commandString.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail(commandString.GetBuffer());
}

void nsImapProtocol::PipelinedFetchMessageParts(const char *uid, nsIMAPMessagePartIDArray *parts)
{
	// assumes no chunking

	// build up a string to fetch
	nsString2 stringToFetch("",eOneByte), what("",eOneByte);
	int32 currentPartNum = 0;
	while ((parts->GetNumParts() > currentPartNum) && !DeathSignalReceived())
	{
		nsIMAPMessagePartID *currentPart = parts->GetPart(currentPartNum);
		if (currentPart)
		{
			// Do things here depending on the type of message part
			// Append it to the fetch string
			if (currentPartNum > 0)
				stringToFetch.Append(" ");

			switch (currentPart->GetFields())
			{
			case kMIMEHeader:
				what = "BODY[";
				what.Append(currentPart->GetPartNumberString());
				what.Append(".MIME]");
				stringToFetch.Append(what);
				break;
			case kRFC822HeadersOnly:
				if (currentPart->GetPartNumberString())
				{
					what = "BODY[";
					what.Append(currentPart->GetPartNumberString());
					what.Append(".HEADER]");
					stringToFetch.Append(what);
				}
				else
				{
					// headers for the top-level message
					stringToFetch.Append("BODY[HEADER]");
				}
				break;
			default:
				NS_ASSERTION(FALSE, "we should only be pipelining MIME headers and Message headers");
				break;
			}

		}
		currentPartNum++;
	}

	// Run the single, pipelined fetch command
	if ((parts->GetNumParts() > 0) && !DeathSignalReceived() && !GetPseudoInterrupted() && stringToFetch.GetBuffer())
	{
	    IncrementCommandTagNumber();

		nsString2 commandString(GetServerCommandTag(), eOneByte); 
		commandString.Append(" UID fetch ");
		commandString.Append(uid, 10);
		commandString.Append(" (");
		commandString.Append(stringToFetch.GetBuffer());
		commandString.Append(")" CRLF);
		nsresult rv = SendData(commandString.GetBuffer());
        if (NS_SUCCEEDED(rv))
            ParseIMAPandCheckForNewMail(commandString.GetBuffer());
	}
}




// this routine is used to fetch a message or messages, or headers for a
// message...

void
nsImapProtocol::FetchMessage(nsString2 &messageIds, 
                             nsIMAPeFetchFields whatToFetch,
                             PRBool idIsUid,
                             PRUint32 startByte, PRUint32 endByte,
                             char *part)
{
    IncrementCommandTagNumber();
    
    nsString2 commandString;
    if (idIsUid)
    	commandString = "%s UID fetch";
    else
    	commandString = "%s fetch";
    
    switch (whatToFetch) {
        case kEveryThingRFC822:
			if (m_trackingTime)
				AdjustChunkSize();			// we started another segment
			m_startTime = PR_Now();			// save start of download time
			m_trackingTime = TRUE;
			if (GetServerStateParser().ServerHasIMAP4Rev1Capability())
			{
				if (GetServerStateParser().GetCapabilityFlag() & kHasXSenderCapability)
					commandString.Append(" %s (XSENDER UID RFC822.SIZE BODY[]");
				else
					commandString.Append(" %s (UID RFC822.SIZE BODY[]");
			}
			else
			{
				if (GetServerStateParser().GetCapabilityFlag() & kHasXSenderCapability)
					commandString.Append(" %s (XSENDER UID RFC822.SIZE RFC822");
				else
					commandString.Append(" %s (UID RFC822.SIZE RFC822");
			}
			if (endByte > 0)
			{
				// if we are retrieving chunks
				char *byterangeString = PR_smprintf("<%ld.%ld>",startByte,endByte);
				if (byterangeString)
				{
					commandString.Append(byterangeString);
					PR_Free(byterangeString);
				}
			}
			commandString.Append(")");
            break;

        case kEveryThingRFC822Peek:
        	{
	        	char *formatString = "";
	        	PRUint32 server_capabilityFlags = GetServerStateParser().GetCapabilityFlag();
	        	
	        	if (server_capabilityFlags & kIMAP4rev1Capability)
	        	{
	        		// use body[].peek since rfc822.peek is not in IMAP4rev1
	        		if (server_capabilityFlags & kHasXSenderCapability)
	        			formatString = " %s (XSENDER UID RFC822.SIZE BODY.PEEK[])";
	        		else
	        			formatString = " %s (UID RFC822.SIZE BODY.PEEK[])";
	        	}
	        	else
	        	{
	        		if (server_capabilityFlags & kHasXSenderCapability)
	        			formatString = " %s (XSENDER UID RFC822.SIZE RFC822.peek)";
	        		else
	        			formatString = " %s (UID RFC822.SIZE RFC822.peek)";
	        	}
	        
				commandString.Append(formatString);
			}
            break;
        case kHeadersRFC822andUid:
			if (GetServerStateParser().ServerHasIMAP4Rev1Capability())
			{
				PRBool useArbitraryHeaders = GetShouldDownloadArbitraryHeaders();	// checks filter headers, etc.
				if (/***** Fix me *** gOptimizedHeaders &&	*/// preference -- able to turn it off
					useArbitraryHeaders)	// if it's ok -- no filters on any header, etc.
				{
					char *headersToDL = NULL;
					char *arbitraryHeaders = GetArbitraryHeadersToDownload();
					if (arbitraryHeaders)
					{
						headersToDL = PR_smprintf("%s %s",IMAP_DB_HEADERS,arbitraryHeaders);
						PR_Free(arbitraryHeaders);
					}
					else
					{
						headersToDL = PR_smprintf("%s",IMAP_DB_HEADERS);
					}
					if (headersToDL)
					{
						char *what = PR_smprintf(" BODY.PEEK[HEADER.FIELDS (%s)])", headersToDL);
						if (what)
						{
							commandString.Append(" %s (UID RFC822.SIZE FLAGS");
							commandString.Append(what);
							PR_Free(what);
						}
						else
						{
							commandString.Append(" %s (UID RFC822.SIZE BODY.PEEK[HEADER] FLAGS)");
						}
						PR_Free(headersToDL);
					}
					else
					{
						commandString.Append(" %s (UID RFC822.SIZE BODY.PEEK[HEADER] FLAGS)");
					}
				}
				else
					commandString.Append(" %s (UID RFC822.SIZE BODY.PEEK[HEADER] FLAGS)");
			}
			else
				commandString.Append(" %s (UID RFC822.SIZE RFC822.HEADER FLAGS)");
            break;
        case kUid:
			commandString.Append(" %s (UID)");
            break;
        case kFlags:
			commandString.Append(" %s (FLAGS)");
            break;
        case kRFC822Size:
			commandString.Append(" %s (RFC822.SIZE)");
			break;
		case kRFC822HeadersOnly:
			if (GetServerStateParser().ServerHasIMAP4Rev1Capability())
			{
				if (part)
				{
					commandString.Append(" %s (BODY[");
					char *what = PR_smprintf("%s.HEADER])", part);
					if (what)
					{
						commandString.Append(what);
						PR_Free(what);
					}
					else
						HandleMemoryFailure();
				}
				else
				{
					// headers for the top-level message
					commandString.Append(" %s (BODY[HEADER])");
				}
			}
			else
				commandString.Append(" %s (RFC822.HEADER)");
			break;
		case kMIMEPart:
			commandString.Append(" %s (BODY[%s]");
			if (endByte > 0)
			{
				// if we are retrieving chunks
				char *byterangeString = PR_smprintf("<%ld.%ld>",startByte,endByte);
				if (byterangeString)
				{
					commandString.Append(byterangeString);
					PR_Free(byterangeString);
				}
			}
			commandString.Append(")");
			break;
		case kMIMEHeader:
			commandString.Append(" %s (BODY[%s.MIME])");
    		break;
	};

	commandString.Append(CRLF);

		// since messageIds can be infinitely long, use a dynamic buffer rather than the fixed one
	const char *commandTag = GetServerCommandTag();
	int protocolStringSize = commandString.Length() + messageIds.Length() + PL_strlen(commandTag) + 1 +
		(part ? PL_strlen(part) : 0);
	char *protocolString = (char *) PR_CALLOC( protocolStringSize );
    
    if (protocolString)
    {
		char *cCommandStr = commandString.ToNewCString();
		char *cMessageIdsStr = messageIds.ToNewCString();

		if ((whatToFetch == kMIMEPart) ||
			(whatToFetch == kMIMEHeader))
		{
			PR_snprintf(protocolString,                                      // string to create
					protocolStringSize,                                      // max size
					cCommandStr,                                   // format string
					commandTag,                          // command tag
					cMessageIdsStr,
					part);
		}
		else
		{
			PR_snprintf(protocolString,                                      // string to create
					protocolStringSize,                                      // max size
					cCommandStr,                                   // format string
					commandTag,                          // command tag
					cMessageIdsStr);
		}
	            
	    nsresult rv = SendData(protocolString);
	    
		delete [] cCommandStr;
		delete [] cMessageIdsStr;
        if (NS_SUCCEEDED(rv))
            ParseIMAPandCheckForNewMail(protocolString);
	    PR_Free(protocolString);
   	}
    else
        HandleMemoryFailure();
}

void nsImapProtocol::FetchTryChunking(nsString2 &messageIds,
                                            nsIMAPeFetchFields whatToFetch,
                                            PRBool idIsUid,
											char *part,
											PRUint32 downloadSize,
											PRBool tryChunking)
{
	GetServerStateParser().SetTotalDownloadSize(downloadSize);
	if (m_fetchByChunks && tryChunking &&
        GetServerStateParser().ServerHasIMAP4Rev1Capability() &&
		(downloadSize > (PRUint32) m_chunkThreshold))
	{
		PRUint32 startByte = 0;
		GetServerStateParser().ClearLastFetchChunkReceived();
		while (!DeathSignalReceived() && !GetPseudoInterrupted() && 
			!GetServerStateParser().GetLastFetchChunkReceived() &&
			GetServerStateParser().ContinueParse())
		{
			PRUint32 sizeToFetch = startByte + m_chunkSize > downloadSize ?
				downloadSize - startByte : m_chunkSize;
			FetchMessage(messageIds, 
						 whatToFetch,
						 idIsUid,
						 startByte, sizeToFetch,
						 part);
			startByte += sizeToFetch;
		}

		// Only abort the stream if this is a normal message download
		// Otherwise, let the body shell abort the stream.
		if ((whatToFetch == kEveryThingRFC822)
			&&
			((startByte > 0 && (startByte < downloadSize) &&
			(DeathSignalReceived() || GetPseudoInterrupted())) ||
			!GetServerStateParser().ContinueParse()))
		{
			AbortMessageDownLoad();
			PseudoInterrupt(FALSE);
		}
	}
	else
	{
		// small message, or (we're not chunking and not doing bodystructure),
		// or the server is not rev1.
		// Just fetch the whole thing.
		FetchMessage(messageIds, whatToFetch,idIsUid, 0, 0, part);
	}
}


void nsImapProtocol::PipelinedFetchMessageParts(nsString2 &uid, nsIMAPMessagePartIDArray *parts)
{
	// assumes no chunking

	// build up a string to fetch
	nsString2 stringToFetch("",eOneByte, 0);
	nsString2 what("",eOneByte, 0);

	int32 currentPartNum = 0;
	while ((parts->GetNumParts() > currentPartNum) && !DeathSignalReceived())
	{
		nsIMAPMessagePartID *currentPart = parts->GetPart(currentPartNum);
		if (currentPart)
		{
			// Do things here depending on the type of message part
			// Append it to the fetch string
			if (currentPartNum > 0)
				stringToFetch += " ";

			switch (currentPart->GetFields())
			{
			case kMIMEHeader:
				what = "BODY[";
				what += currentPart->GetPartNumberString();
				what += ".MIME]";
				stringToFetch += what;
				break;
			case kRFC822HeadersOnly:
				if (currentPart->GetPartNumberString())
				{
					what = "BODY[";
					what += currentPart->GetPartNumberString();
					what += ".HEADER]";
					stringToFetch += what;
				}
				else
				{
					// headers for the top-level message
					stringToFetch += "BODY[HEADER]";
				}
				break;
			default:
				NS_ASSERTION(FALSE, "we should only be pipelining MIME headers and Message headers");
				break;
			}

		}
		currentPartNum++;
	}

	// Run the single, pipelined fetch command
	if ((parts->GetNumParts() > 0) && !DeathSignalReceived() && !GetPseudoInterrupted() && stringToFetch.GetBuffer())
	{
	    IncrementCommandTagNumber();

		char *commandString = PR_smprintf("%s UID fetch %s (%s)%s",
                                          GetServerCommandTag(), uid.GetBuffer(),
                                          stringToFetch.GetBuffer(), CRLF);

		if (commandString)
		{
			nsresult rv = SendData(commandString);
            if (NS_SUCCEEDED(rv))
                ParseIMAPandCheckForNewMail(commandString);
			PR_Free(commandString);
		}
		else
			HandleMemoryFailure();
	}
}

void
nsImapProtocol::AddXMozillaStatusLine(uint16 /* flags */) // flags not use now
{
	static char statusLine[] = "X-Mozilla-Status: 0201\r\n";
	HandleMessageDownLoadLine(statusLine, FALSE);
}

void
nsImapProtocol::PostLineDownLoadEvent(msg_line_info *downloadLineDontDelete)
{
    NS_ASSERTION(downloadLineDontDelete, 
                 "Oops... null msg line info not good");

	if (GetServerStateParser().GetDownloadingHeaders())
	{
		if (m_imapMailFolderSink && downloadLineDontDelete)
		{
			m_imapMailFolderSink->ParseAdoptedHeaderLine(this, downloadLineDontDelete);
		}
	}
    else if (m_imapMessageSink && downloadLineDontDelete)
	{
        m_imapMessageSink->ParseAdoptedMsgLine(this, downloadLineDontDelete);
	}

    // ***** We need to handle the psuedo interrupt here *****
}

// well, this is what the old code used to look like to handle a line seen by the parser.
// I'll leave it mostly #ifdef'ed out, but I suspect it will look a lot like this.
// Perhaps we'll send the line to a messageSink...
void nsImapProtocol::HandleMessageDownLoadLine(const char *line, PRBool chunkEnd)
{
    // when we duplicate this line, whack it into the native line
    // termination.  Do not assume that it really ends in CRLF
    // to start with, even though it is supposed to be RFC822

	// If we are fetching by chunks, we can make no assumptions about
	// the end-of-line terminator, and we shouldn't mess with it.
    
    // leave enough room for two more chars. (CR and LF)
    char *localMessageLine = (char *) PR_CALLOC(strlen(line) + 3);
    if (localMessageLine)
        strcpy(localMessageLine,line);
    char *endOfLine = localMessageLine + strlen(localMessageLine);

	if (!chunkEnd)
	{
#if (MSG_LINEBREAK_LEN == 1)
		if ((endOfLine - localMessageLine) >= 2 &&
			 endOfLine[-2] == CR &&
			 endOfLine[-1] == LF)
			{
			  /* CRLF -> CR or LF */
			  endOfLine[-2] = MSG_LINEBREAK[0];
			  endOfLine[-1] = '\0';
			}
		  else if (endOfLine > localMessageLine + 1 &&
				   endOfLine[-1] != MSG_LINEBREAK[0] &&
			   ((endOfLine[-1] == CR) || (endOfLine[-1] == LF)))
			{
			  /* CR -> LF or LF -> CR */
			  endOfLine[-1] = MSG_LINEBREAK[0];
			}
		else // no eol characters at all
		  {
		    endOfLine[0] = MSG_LINEBREAK[0]; // CR or LF
		    endOfLine[1] = '\0';
		  }
#else
		  if (((endOfLine - localMessageLine) >= 2 && endOfLine[-2] != CR) ||
			   ((endOfLine - localMessageLine) >= 1 && endOfLine[-1] != LF))
			{
			  if ((endOfLine[-1] == CR) || (endOfLine[-1] == LF))
			  {
				  /* LF -> CRLF or CR -> CRLF */
				  endOfLine[-1] = MSG_LINEBREAK[0];
				  endOfLine[0]  = MSG_LINEBREAK[1];
				  endOfLine[1]  = '\0';
			  }
			  else	// no eol characters at all
			  {
				  endOfLine[0] = MSG_LINEBREAK[0];	// CR
				  endOfLine[1] = MSG_LINEBREAK[1];	// LF
				  endOfLine[2] = '\0';
			  }
			}
#endif
	}

	const char *xSenderInfo = GetServerStateParser().GetXSenderInfo();

	if (xSenderInfo && *xSenderInfo && !m_fromHeaderSeen)
	{
		if (!PL_strncmp("From: ", localMessageLine, 6))
		{
			m_fromHeaderSeen = TRUE;
			if (PL_strstr(localMessageLine, xSenderInfo) != NULL)
				AddXMozillaStatusLine(0);
			GetServerStateParser().FreeXSenderInfo();
		}
	}
    // if this line is for a different message, or the incoming line is too big
    if (((m_downloadLineCache.CurrentUID() != GetServerStateParser().CurrentResponseUID()) && !m_downloadLineCache.CacheEmpty()) ||
        (m_downloadLineCache.SpaceAvailable() < (PL_strlen(localMessageLine) + 1)) )
    {
		if (!m_downloadLineCache.CacheEmpty())
		{
			msg_line_info *downloadLineDontDelete = m_downloadLineCache.GetCurrentLineInfo();
			PostLineDownLoadEvent(downloadLineDontDelete);
		}
		m_downloadLineCache.ResetCache();
	}
     
    // so now the cache is flushed, but this string might still be to big
    if (m_downloadLineCache.SpaceAvailable() < (PL_strlen(localMessageLine) + 1) )
    {
        // has to be dynamic to pass to other win16 thread
		msg_line_info *downLoadInfo = (msg_line_info *) PR_CALLOC(sizeof(msg_line_info));
        if (downLoadInfo)
        {
          	downLoadInfo->adoptedMessageLine = localMessageLine;
          	downLoadInfo->uidOfMessage = GetServerStateParser().CurrentResponseUID();
          	PostLineDownLoadEvent(downLoadInfo);
          	if (!DeathSignalReceived())
          		PR_Free(downLoadInfo);
          	else
          	{
          		// this is very rare, interrupt while waiting to display a huge single line
          		// Net_InterruptIMAP will read this line so leak the downLoadInfo
          		
          		// set localMessageLine to NULL so the FREEIF( localMessageLine) leaks also
          		localMessageLine = NULL;
          	}
        }
	}
    else
		m_downloadLineCache.CacheLine(localMessageLine, GetServerStateParser().CurrentResponseUID());

	PR_FREEIF( localMessageLine);
}



void nsImapProtocol::NormalMessageEndDownload()
{
	Log("STREAM", "CLOSE", "Normal Message End Download Stream");

	if (m_trackingTime)
		AdjustChunkSize();
	if (!m_downloadLineCache.CacheEmpty())
	{
	    msg_line_info *downloadLineDontDelete = m_downloadLineCache.GetCurrentLineInfo();
	    PostLineDownLoadEvent(downloadLineDontDelete);
	    m_downloadLineCache.ResetCache();
    }

	if (GetServerStateParser().GetDownloadingHeaders())
	{
		if (m_imapMailFolderSink)
			m_imapMailFolderSink->NormalEndHeaderParseStream(this);
	}
	else if (m_imapMessageSink)
    {
        m_imapMessageSink->NormalEndMsgWriteStream(this);
        WaitForFEEventCompletion();
    }

}

void nsImapProtocol::AbortMessageDownLoad()
{
	Log("STREAM", "CLOSE", "Abort Message  Download Stream");

	if (m_trackingTime)
		AdjustChunkSize();
	if (!m_downloadLineCache.CacheEmpty())
	{
	    msg_line_info *downloadLineDontDelete = m_downloadLineCache.GetCurrentLineInfo();
	    PostLineDownLoadEvent(downloadLineDontDelete);
	    m_downloadLineCache.ResetCache();
    }

	if (GetServerStateParser().GetDownloadingHeaders())
	{
		if (m_imapMailFolderSink)
			m_imapMailFolderSink->AbortHeaderParseStream(this);
	}
	else if (m_imapMessageSink)
        m_imapMessageSink->AbortMsgWriteStream(this);

}


void nsImapProtocol::ProcessMailboxUpdate(PRBool handlePossibleUndo)
{
	if (DeathSignalReceived())
		return;
    // fetch the flags and uids of all existing messages or new ones
    if (!DeathSignalReceived() && GetServerStateParser().NumberOfMessages())
    {
    	if (handlePossibleUndo)
    	{
	    	// undo any delete flags we may have asked to
	    	nsString2 undoIds("",eOneByte);
			
			GetCurrentUrl()->CreateListOfMessageIdsString(&undoIds);
			if (undoIds.Length() > 0)
			{
				char firstChar = (char) undoIds.CharAt(0);
				undoIds.Cut(0, 1);	// remove first character
	    		// if this string started with a '-', then this is an undo of a delete
	    		// if its a '+' its a redo
	    		if (firstChar == '-')
					Store(undoIds, "-FLAGS (\\Deleted)", TRUE);	// most servers will fail silently on a failure, deal with it?
	    		else  if (firstChar == '+')
					Store(undoIds, "+FLAGS (\\Deleted)", TRUE);	// most servers will fail silently on a failure, deal with it?
				else 
					NS_ASSERTION(FALSE, "bogus undo Id's");
			}
		}
    	
        // make the parser record these flags
		nsString2 fetchStr;
		PRInt32 added = 0, deleted = 0;

		m_flagState.GetNumberOfMessages(&added);
		deleted = m_flagState.GetNumberOfDeletedMessages();

		if (!added || (added == deleted))
		{
			nsString2 idsToFetch("1:*");
			FetchMessage(idsToFetch, kFlags, TRUE);	// id string shows uids
			// lets see if we should expunge during a full sync of flags.
			if (!DeathSignalReceived())	// only expunge if not reading messages manually and before fetching new
			{
				// ### TODO read gExpungeThreshhold from prefs.
				if ((m_flagState.GetNumberOfDeletedMessages() >= 20/* gExpungeThreshold */)  /*&& 
					GetDeleteIsMoveToTrash() */)
					Expunge();	// might be expensive, test for user cancel
			}

		}
		else 
		{
			fetchStr.Append(GetServerStateParser().HighestRecordedUID() + 1, 10);
			fetchStr.Append(":*");

			// sprintf(fetchStr, "%ld:*", GetServerStateParser().HighestRecordedUID() + 1);
			FetchMessage(fetchStr, kFlags, TRUE);			// only new messages please
		}
    }
    else if (!DeathSignalReceived())
    	GetServerStateParser().ResetFlagInfo(0);
        
    mailbox_spec *new_spec = GetServerStateParser().CreateCurrentMailboxSpec();
	if (new_spec && !DeathSignalReceived())
	{
		if (!DeathSignalReceived())
		{
			nsIImapUrl::nsImapAction imapAction; 
			nsresult res = m_runningUrl->GetImapAction(&imapAction);
			if (NS_SUCCEEDED(res) && imapAction == nsIImapUrl::nsImapExpungeFolder)
				new_spec->box_flags |= kJustExpunged;
			PR_EnterMonitor(m_waitForBodyIdsMonitor);
			UpdatedMailboxSpec(new_spec);
		}
	}
	else if (!new_spec)
		HandleMemoryFailure();
    
    // Block until libmsg decides whether to download headers or not.
    PRUint32 *msgIdList = NULL;
    PRUint32 msgCount = 0;
    
	if (!DeathSignalReceived())
	{
		WaitForPotentialListOfMsgsToFetch(&msgIdList, msgCount);

		if (new_spec)
			PR_ExitMonitor(m_waitForBodyIdsMonitor);

		if (msgIdList && !DeathSignalReceived() && GetServerStateParser().LastCommandSuccessful())
		{
			FolderHeaderDump(msgIdList, msgCount);
			PR_FREEIF( msgIdList);
		}
			// this might be bogus, how are we going to do pane notification and stuff when we fetch bodies without
			// headers!
	}
    // wait for a list of bodies to fetch. 
    if (!DeathSignalReceived() && GetServerStateParser().LastCommandSuccessful())
    {
        WaitForPotentialListOfBodysToFetch(&msgIdList, msgCount);
        if ( msgCount && !DeathSignalReceived() && GetServerStateParser().LastCommandSuccessful())
    	{
    		FolderMsgDump(msgIdList, msgCount, kEveryThingRFC822Peek);
    		PR_FREEIF(msgIdList);
    	}
	}
	if (DeathSignalReceived())
		GetServerStateParser().ResetFlagInfo(0);
	PR_FREEIF(new_spec);
}

void nsImapProtocol::UpdatedMailboxSpec(mailbox_spec *aSpec)
{
	if (m_imapMailFolderSink)
		m_imapMailFolderSink->UpdateImapMailboxInfo(this, aSpec);
}

void nsImapProtocol::FolderHeaderDump(PRUint32 *msgUids, PRUint32 msgCount)
{
	FolderMsgDump(msgUids, msgCount, kHeadersRFC822andUid);
	
    if (GetServerStateParser().NumberOfMessages())
        HeaderFetchCompleted();
}

void nsImapProtocol::FolderMsgDump(PRUint32 *msgUids, PRUint32 msgCount, nsIMAPeFetchFields fields)
{
	// lets worry about this progress stuff later.
	switch (fields) {
	case kHeadersRFC822andUid:
		m_progressStringId =  IMAP_RECEIVING_MESSAGE_HEADERS_OF;
		break;
	case kFlags:
		m_progressStringId =  IMAP_RECEIVING_MESSAGE_FLAGS_OF;
		break;
	default:
		m_progressStringId =  IMAP_FOLDER_RECEIVING_MESSAGE_OF;
		break;
	}
	
	m_progressIndex = 0;
	m_progressCount = msgCount;
	FolderMsgDumpLoop(msgUids, msgCount, fields);
	
	m_progressStringId = 0;
}

void nsImapProtocol::WaitForPotentialListOfMsgsToFetch(PRUint32 **msgIdList, PRUint32 &msgCount)
{
	PRIntervalTime sleepTime = kImapSleepTime;

    PR_EnterMonitor(m_fetchMsgListMonitor);
    while(!m_fetchMsgListIsNew && !DeathSignalReceived())
        PR_Wait(m_fetchMsgListMonitor, sleepTime);
    m_fetchMsgListIsNew = FALSE;

    *msgIdList = m_fetchMsgIdList;
    msgCount   = m_fetchCount;
    
    PR_ExitMonitor(m_fetchMsgListMonitor);
}

void nsImapProtocol::WaitForPotentialListOfBodysToFetch(PRUint32 **msgIdList, PRUint32 &msgCount)
{
	PRIntervalTime sleepTime = kImapSleepTime;

    PR_EnterMonitor(m_fetchBodyListMonitor);
    while(!m_fetchBodyListIsNew && !DeathSignalReceived())
        PR_Wait(m_fetchBodyListMonitor, sleepTime);
    m_fetchBodyListIsNew = FALSE;

    *msgIdList = m_fetchBodyIdList;
    msgCount   = m_fetchBodyCount;
    
    PR_ExitMonitor(m_fetchBodyListMonitor);
}

#if 0

void nsImapProtocol::NotifyKeyList(PRUint32 *keys, PRUint32 keyCount)
#endif

// libmsg uses this to notify a running imap url about the message headers it should
// download while opening a folder. Generally, the imap thread will be blocked 
// in WaitForPotentialListOfMsgsToFetch waiting for this notification.
NS_IMETHODIMP nsImapProtocol::NotifyHdrsToDownload(PRUint32 *keys, PRUint32 keyCount)
{
    PR_EnterMonitor(m_fetchMsgListMonitor);
    m_fetchMsgIdList = keys;
    m_fetchCount  	= keyCount;
    m_fetchMsgListIsNew = TRUE;
    PR_Notify(m_fetchMsgListMonitor);
    PR_ExitMonitor(m_fetchMsgListMonitor);
	return NS_OK;
}

// libmsg uses this to notify a running imap url about message bodies it should download.
// why not just have libmsg explicitly download the message bodies?
NS_IMETHODIMP nsImapProtocol::NotifyBodysToDownload(PRUint32 *keys, PRUint32 keyCount)
{
    PR_EnterMonitor(m_fetchBodyListMonitor);
    m_fetchBodyIdList = keys;
    m_fetchBodyCount  	= keyCount;
    m_fetchBodyListIsNew = TRUE;
    PR_Notify(m_fetchBodyListMonitor);
    PR_ExitMonitor(m_fetchBodyListMonitor);
	return NS_OK;
}

NS_IMETHODIMP nsImapProtocol::GetFlagsForUID(PRUint32 uid, PRBool *foundIt, imapMessageFlagsType *resultFlags)
{
	PRInt32 i;

	imapMessageFlagsType flags = m_flagState.GetMessageFlagsFromUID(uid, foundIt, &i);
	if (*foundIt)
		*resultFlags = flags;
	return NS_OK;
}

NS_IMETHODIMP nsImapProtocol::GetSupportedUserFlags(PRUint16 *supportedFlags)
{
	if (!supportedFlags)
		return NS_ERROR_NULL_POINTER;

	*supportedFlags = m_flagState.GetSupportedUserFlags();
	return NS_OK;
}
void nsImapProtocol::FolderMsgDumpLoop(PRUint32 *msgUids, PRUint32 msgCount, nsIMAPeFetchFields fields)
{
//   	PastPasswordCheckEvent();

	PRInt32 msgCountLeft = msgCount;
	PRUint32 msgsDownloaded = 0;
	do 
	{
		nsString2 idString("",eOneByte);

		PRUint32 msgsToDownload = (msgCountLeft > 200) ? 200 : msgCountLeft;
   		AllocateImapUidString(msgUids + msgsDownloaded, msgsToDownload, idString);	// 20 * 200

		// except I don't think this works ### DB
		FetchMessage(idString,  fields, TRUE);                  // msg ids are uids                 

		msgsDownloaded += msgsToDownload;
		msgCountLeft -= msgsToDownload;

   	}
	while (msgCountLeft > 0);
}   	
   	

void nsImapProtocol::HeaderFetchCompleted()
{
    if (m_imapMiscellaneousSink)
	{
        m_imapMiscellaneousSink->HeaderFetchCompleted(this);
		WaitForFEEventCompletion();
	}
}


// Use the noop to tell the server we are still here, and therefore we are willing to receive
// status updates. The recent or exists response from the server could tell us that there is
// more mail waiting for us, but we need to check the flags of the mail and the high water mark
// to make sure that we do not tell the user that there is new mail when perhaps they have
// already read it in another machine.

void nsImapProtocol::PeriodicBiff()
{
	
	nsMsgBiffState startingState = m_currentBiffState;
	
	if (GetServerStateParser().GetIMAPstate() == nsImapServerResponseParser::kFolderSelected)
    {
		Noop();	// check the latest number of messages
		PRInt32 numMessages = 0;
		m_flagState.GetNumberOfMessages(&numMessages);
        if (GetServerStateParser().NumberOfMessages() != numMessages)
        {
			PRUint32 id = GetServerStateParser().HighestRecordedUID() + 1;
			nsString2 fetchStr("",eOneByte);						// only update flags
			PRUint32 added = 0, deleted = 0;
			
			deleted = m_flagState.GetNumberOfDeletedMessages();
			added = numMessages;
			if (!added || (added == deleted))	// empty keys, get them all
				id = 1;

			//sprintf(fetchStr, "%ld:%ld", id, id + GetServerStateParser().NumberOfMessages() - fFlagState->GetNumberOfMessages());
			fetchStr.Append(id, 10);
			fetchStr.Append(":*"); 
			FetchMessage(fetchStr, kFlags, TRUE);

			if (((PRUint32) m_flagState.GetHighestNonDeletedUID() >= id) && m_flagState.IsLastMessageUnseen())
				m_currentBiffState = nsMsgBiffState_NewMail;
			else
				m_currentBiffState = nsMsgBiffState_NoMail;
        }
        else
            m_currentBiffState = nsMsgBiffState_NoMail;
    }
    else
    	m_currentBiffState = nsMsgBiffState_Unknown;
    
    if (startingState != m_currentBiffState)
    	SendSetBiffIndicatorEvent(m_currentBiffState);
}

void nsImapProtocol::SendSetBiffIndicatorEvent(nsMsgBiffState newState)
{
    m_imapMiscellaneousSink->SetBiffStateAndUpdate(this, newState);

 	if (newState == nsMsgBiffState_NewMail)
		m_mailToFetch = TRUE;
	else
		m_mailToFetch = FALSE;
    WaitForFEEventCompletion();
}



// We get called to see if there is mail waiting for us at the server, even if it may have been
// read elsewhere. We just want to know if we should download headers or not.

PRBool nsImapProtocol::CheckNewMail()
{
	return m_checkForNewMailDownloadsHeaders;
}



void nsImapProtocol::AllocateImapUidString(PRUint32 *msgUids, PRUint32 msgCount, nsString2 &returnString)
{
	PRUint32 startSequence = (msgCount > 0) ? msgUids[0] : 0xFFFFFFFF;
	PRUint32 curSequenceEnd = startSequence;
	PRUint32 total = msgCount;

	for (PRUint32 keyIndex=0; keyIndex < total; keyIndex++)
	{
		PRUint32 curKey = msgUids[keyIndex];
		PRUint32 nextKey = (keyIndex + 1 < total) ? msgUids[keyIndex + 1] : 0xFFFFFFFF;
		PRBool lastKey = (nextKey == 0xFFFFFFFF);

		if (lastKey)
			curSequenceEnd = curKey;
		if (nextKey == curSequenceEnd + 1 && !lastKey)
		{
			curSequenceEnd = nextKey;
			continue;
		}
		else if (curSequenceEnd > startSequence)
		{
			returnString.Append(startSequence, 10);
			returnString += ':';
			returnString.Append(curSequenceEnd, 10);
			if (!lastKey)
				returnString += ',';
//			sprintf(currentidString, "%ld:%ld,", startSequence, curSequenceEnd);
			startSequence = nextKey;
			curSequenceEnd = startSequence;
		}
		else
		{
			startSequence = nextKey;
			curSequenceEnd = startSequence;
			returnString.Append(msgUids[keyIndex], 10);
			if (!lastKey)
				returnString += ',';
//			sprintf(currentidString, "%ld,", msgUids[keyIndex]);
		}
	}
}

// log info including current state...
void nsImapProtocol::Log(const char *logSubName, const char *extraInfo, const char *logData)
{
	static char *nonAuthStateName = "NA";
	static char *authStateName = "A";
	static char *selectedStateName = "S";
    //	static char *waitingStateName = "W";
	char *stateName = NULL;
    const char *hostName = GetImapHostName();  // initilize to empty string
	switch (GetServerStateParser().GetIMAPstate())
	{
	case nsImapServerResponseParser::kFolderSelected:
		if (m_runningUrl)
		{
			if (extraInfo)
				PR_LOG(IMAP, PR_LOG_ALWAYS, ("%s:%s-%s:%s:%s: %s", hostName,selectedStateName, GetServerStateParser().GetSelectedMailboxName(), logSubName, extraInfo, logData));
			else
				PR_LOG(IMAP, PR_LOG_ALWAYS, ("%s:%s-%s:%s: %s", hostName,selectedStateName, GetServerStateParser().GetSelectedMailboxName(), logSubName, logData));
		}
		return;
		break;
	case nsImapServerResponseParser::kNonAuthenticated:
		stateName = nonAuthStateName;
		break;
	case nsImapServerResponseParser::kAuthenticated:
		stateName = authStateName;
		break;
#if 0 // *** this isn't a server state; its a status ***
	case nsImapServerResponseParser::kWaitingForMoreClientInput:
		stateName = waitingStateName;
		break;
#endif 
	}

	if (m_runningUrl)
	{
		if (extraInfo)
			PR_LOG(IMAP, PR_LOG_ALWAYS, ("%s:%s:%s:%s: %s", hostName,stateName,logSubName,extraInfo,logData));
		else
			PR_LOG(IMAP, PR_LOG_ALWAYS, ("%s:%s:%s: %s",hostName,stateName,logSubName,logData));
	}
}

// In 4.5, this posted an event back to libmsg and blocked until it got a response.
// We may still have to do this.It would be nice if we could preflight this value,
// but we may not always know when we'll need it.
PRUint32 nsImapProtocol::GetMessageSize(nsString2 &messageId, 
                                        PRBool idsAreUids)
{
	MessageSizeInfo *sizeInfo = 
        (MessageSizeInfo *)PR_CALLOC(sizeof(MessageSizeInfo));
	if (sizeInfo)
	{
		const char *folderFromParser =
            GetServerStateParser().GetSelectedMailboxName(); 
		if (folderFromParser)
		{
			sizeInfo->id = (char *)PR_CALLOC(messageId.Length() + 1);
			PL_strcpy(sizeInfo->id, messageId.GetBuffer());
			sizeInfo->idIsUid = idsAreUids;

			nsIMAPNamespace *nsForMailbox = nsnull;
            const char *userName = GetImapUserName();
            m_hostSessionList->GetNamespaceForMailboxForHost(
                GetImapHostName(), userName, folderFromParser,
                nsForMailbox);

			char *nonUTF7ConvertedName = CreateUtf7ConvertedString(folderFromParser, FALSE);
			if (nonUTF7ConvertedName)
				folderFromParser = nonUTF7ConvertedName;

			if (nsForMailbox)
                m_runningUrl->AllocateCanonicalPath(
                    folderFromParser, nsForMailbox->GetDelimiter(),
                    &sizeInfo->folderName);
			else
                 m_runningUrl->AllocateCanonicalPath(
                    folderFromParser,kOnlineHierarchySeparatorUnknown,
                    &sizeInfo->folderName);
			PR_FREEIF(nonUTF7ConvertedName);

			if (sizeInfo->id && sizeInfo->folderName)
			{
                if (m_imapMessageSink)
                {
                    m_imapMessageSink->GetMessageSizeFromDB(this, sizeInfo);
                    WaitForFEEventCompletion();
                }

			}
            PR_FREEIF(sizeInfo->id);
            PR_FREEIF(sizeInfo->folderName);

			int32 rv = 0;
			if (!DeathSignalReceived())
				rv = sizeInfo->size;
			PR_Free(sizeInfo);
			return rv;
		}
    }
	else
	{
		HandleMemoryFailure();
	}
    return 0;
}


PRBool	nsImapProtocol::GetShowAttachmentsInline()
{
    PRBool *rv = (PRBool*) new PRBool(PR_FALSE);

    if (m_imapMiscellaneousSink && rv)
    {
        m_imapMiscellaneousSink->GetShowAttachmentsInline(this, rv);
        WaitForFEEventCompletion();
        return *rv;
    }
    return PR_FALSE;
}

// message id string utility functions
/* static */PRBool nsImapProtocol::HandlingMultipleMessages(const char *messageIdString)
{
	return (PL_strchr(messageIdString,',') != nsnull ||
		    PL_strchr(messageIdString,':') != nsnull);
}

/* static */PRBool nsImapProtocol::HandlingMultipleMessages(nsString2 &messageIdString)
{
	return (messageIdString.FindCharInSet(",:") != -1);
}

PRUint32 nsImapProtocol::CountMessagesInIdString(nsString2 &idString)
{
	return CountMessagesInIdString((const char *) idString.GetBuffer());
}

PRUint32 nsImapProtocol::CountMessagesInIdString(const char *idString)
{
	PRUint32 numberOfMessages = 0;
	char *uidString = PL_strdup(idString);

	if (uidString)
	{
		// This is in the form <id>,<id>, or <id1>:<id2>
		char curChar = *uidString;
		PRBool isRange = FALSE;
		PRInt32	curToken;
		PRInt32	saveStartToken=0;

		for (char *curCharPtr = uidString; curChar && *curCharPtr;)
		{
			char *currentKeyToken = curCharPtr;
			curChar = *curCharPtr;
			while (curChar != ':' && curChar != ',' && curChar != '\0')
				curChar = *curCharPtr++;
			*(curCharPtr - 1) = '\0';
			curToken = atol(currentKeyToken);
			if (isRange)
			{
				while (saveStartToken < curToken)
				{
					numberOfMessages++;
					saveStartToken++;
				}
			}

			numberOfMessages++;
			isRange = (curChar == ':');
			if (isRange)
				saveStartToken = curToken + 1;
		}
		PR_Free(uidString);
	}
	return numberOfMessages;
}


PRMonitor *nsImapProtocol::GetDataMemberMonitor()
{
    return m_dataMemberMonitor;
}

// It would be really nice not to have to use this method nearly as much as we did
// in 4.5 - we need to think about this some. Some of it may just go away in the new world order
PRBool nsImapProtocol::DeathSignalReceived()
{
	PRBool returnValue;
	PR_EnterMonitor(m_threadDeathMonitor);
	returnValue = m_threadShouldDie;
	PR_ExitMonitor(m_threadDeathMonitor);
	
	return returnValue;
}


PRBool nsImapProtocol::GetPseudoInterrupted()
{
	PRBool rv = FALSE;
	PR_EnterMonitor(m_pseudoInterruptMonitor);
	rv = m_pseudoInterrupted;
	PR_ExitMonitor(m_pseudoInterruptMonitor);
	return rv;
}

void nsImapProtocol::PseudoInterrupt(PRBool the_interrupt)
{
	PR_EnterMonitor(m_pseudoInterruptMonitor);
	m_pseudoInterrupted = the_interrupt;
	if (the_interrupt)
	{
		Log("CONTROL", NULL, "PSEUDO-Interrupted");
	}
	PR_ExitMonitor(m_pseudoInterruptMonitor);
}

void	nsImapProtocol::SetActive(PRBool active)
{
	PR_EnterMonitor(GetDataMemberMonitor());
	m_active = active;
	PR_ExitMonitor(GetDataMemberMonitor());
}

PRBool	nsImapProtocol::GetActive()
{
	PRBool ret;
	PR_EnterMonitor(GetDataMemberMonitor());
	ret = m_active;
	PR_ExitMonitor(GetDataMemberMonitor());
	return ret;
}

void nsImapProtocol::SetContentModified(PRBool modified)
{
	// ### DMB this used to poke the content_modified member of the url struct...
}


PRInt32 nsImapProtocol::OpenTunnel (PRInt32 maxNumberOfBytesToRead)
{
	return 0;
}

PRInt32 nsImapProtocol::GetTunnellingThreshold()
{
	return 0;
//	return gTunnellingThreshold;
}

PRBool nsImapProtocol::GetIOTunnellingEnabled()
{
	return PR_FALSE;
//	return gIOTunnelling;
}

// Adds a set of rights for a given user on a given mailbox on the current host.
// if userName is NULL, it means "me," or MYRIGHTS.
void nsImapProtocol::AddFolderRightsForUser(const char *mailboxName, const char *userName, const char *rights)
{
    nsIMAPACLRightsInfo *aclRightsInfo = new nsIMAPACLRightsInfo();
	if (aclRightsInfo)
	{
		nsIMAPNamespace *namespaceForFolder = nsnull;
        const char *userName = GetImapUserName();
        NS_ASSERTION (m_hostSessionList, "fatal ... null host session list");
        if (m_hostSessionList)
            m_hostSessionList->GetNamespaceForMailboxForHost(
                GetImapHostName(), userName, mailboxName,
                namespaceForFolder);

		aclRightsInfo->hostName = PL_strdup(GetImapHostName());

		char *nonUTF7ConvertedName = CreateUtf7ConvertedString(mailboxName, FALSE);
		if (nonUTF7ConvertedName)
			mailboxName = nonUTF7ConvertedName;

		if (namespaceForFolder)
            m_runningUrl->AllocateCanonicalPath(
                mailboxName,
                namespaceForFolder->GetDelimiter(), 
                &aclRightsInfo->mailboxName);
		else
            m_runningUrl->AllocateCanonicalPath(mailboxName,
                          kOnlineHierarchySeparatorUnknown, 
                          &aclRightsInfo->mailboxName);

		PR_FREEIF(nonUTF7ConvertedName);
		if (userName)
			aclRightsInfo->userName = PL_strdup(userName);
		else
			aclRightsInfo->userName = NULL;
		aclRightsInfo->rights = PL_strdup(rights);
		

		if (aclRightsInfo->hostName && 
            aclRightsInfo->mailboxName && aclRightsInfo->rights && 
			userName ? (aclRightsInfo->userName != NULL) : TRUE)
		{
            if (m_imapExtensionSink)
            {
                m_imapExtensionSink->AddFolderRights(this, aclRightsInfo);
                WaitForFEEventCompletion();
            }
        }
        PR_FREEIF(aclRightsInfo->hostName);
        PR_FREEIF(aclRightsInfo->mailboxName);
        PR_FREEIF(aclRightsInfo->rights);
        PR_FREEIF(aclRightsInfo->userName);

		delete aclRightsInfo;
	}
	else
		HandleMemoryFailure();
}

void nsImapProtocol::SetCopyResponseUid(nsMsgKeyArray* aKeyArray,
                                        const char *msgIdString)
{
    if (m_imapExtensionSink)
    {
        nsCOMPtr<nsISupports> copyState;
        if (m_runningUrl)
            m_runningUrl->GetCopyState(getter_AddRefs(copyState));
        m_imapExtensionSink->SetCopyResponseUid(this,aKeyArray, msgIdString,
                                                copyState);
        WaitForFEEventCompletion();
    }
}

void nsImapProtocol::CommitNamespacesForHostEvent()
{
    if (m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->CommitNamespaces(this, GetImapHostName());
        WaitForFEEventCompletion();
    }
}

// notifies libmsg that we have new capability data for the current host
void nsImapProtocol::CommitCapabilityForHostEvent()
{
    if (m_imapMiscellaneousSink)
    {
        m_imapMiscellaneousSink->CommitCapabilityForHost(this, GetImapHostName());
        WaitForFEEventCompletion();
    }
}

// rights is a single string of rights, as specified by RFC2086, the IMAP ACL extension.
// Clears all rights for a given folder, for all users.
void nsImapProtocol::ClearAllFolderRights(const char *mailboxName,
                                          nsIMAPNamespace *nsForMailbox)
{
	NS_ASSERTION (nsForMailbox, "Oops ... null name space");
    nsIMAPACLRightsInfo *aclRightsInfo = new nsIMAPACLRightsInfo();
	if (aclRightsInfo)
	{
		char *nonUTF7ConvertedName = CreateUtf7ConvertedString(mailboxName, FALSE);
		if (nonUTF7ConvertedName)
			mailboxName = nonUTF7ConvertedName;

        const char *hostName = GetImapHostName();

		aclRightsInfo->hostName = PL_strdup(hostName);
		if (nsForMailbox)
            m_runningUrl->AllocateCanonicalPath(mailboxName,
                                                nsForMailbox->GetDelimiter(),
                                                &aclRightsInfo->mailboxName); 
		else
            m_runningUrl->AllocateCanonicalPath(
                mailboxName, kOnlineHierarchySeparatorUnknown,
                &aclRightsInfo->mailboxName);

		PR_FREEIF(nonUTF7ConvertedName);

		aclRightsInfo->rights = NULL;
		aclRightsInfo->userName = NULL;

		if (aclRightsInfo->hostName && aclRightsInfo->mailboxName)
		{
            if (m_imapExtensionSink)
            {
                m_imapExtensionSink->ClearFolderRights(this, aclRightsInfo);
		        WaitForFEEventCompletion();
            }
        }
        PR_FREEIF(aclRightsInfo->hostName);
        PR_FREEIF(aclRightsInfo->mailboxName);

		delete aclRightsInfo;
	}
	else
		HandleMemoryFailure();
}


char* nsImapProtocol::CreateNewLineFromSocket()
{
	PRBool needMoreData = PR_FALSE;
	char * newLine = nsnull;
	PRUint32 numBytesInLine = 0;
	do
	{
		newLine = m_inputStreamBuffer->ReadNextLine(m_inputStream, numBytesInLine, needMoreData); 
		if (needMoreData)
		{
			// wait on the data available monitor!!
			PR_EnterMonitor(m_dataAvailableMonitor);
			// wait for data arrival
			PR_Wait(m_dataAvailableMonitor, PR_INTERVAL_NO_TIMEOUT);
			PR_ExitMonitor(m_dataAvailableMonitor);
			// once data has arrived, recursively 

		}
	} while (!newLine && !DeathSignalReceived()); // until we get the next line and haven't been interrupted
	
	Log("CreateNewLineFromSocket", nsnull, newLine);
	SetConnectionStatus(newLine ? PL_strlen(newLine) : 0);
	return newLine;
}

PRInt32
nsImapProtocol::GetConnectionStatus()
{
    // ***?? I am not sure we really to guard with monitor for 5.0 ***
    PRInt32 status;
	// mscott -- do we need these monitors? as i was debuggin this I continually
	// locked when entering this monitor...control would never return from this
	// function...
//    PR_CEnterMonitor(this);
    status = m_connectionStatus;
//    PR_CExitMonitor(this);
    return status;
}

void
nsImapProtocol::SetConnectionStatus(PRInt32 status)
{
//    PR_CEnterMonitor(this);
    m_connectionStatus = status;
//    PR_CExitMonitor(this);
}

void
nsImapProtocol::NotifyMessageFlags(imapMessageFlagsType flags, nsMsgKey key)
{
    FlagsKeyStruct aKeyStruct;
    aKeyStruct.flags = flags;
    aKeyStruct.key = key;
    if (m_imapMessageSink)
        m_imapMessageSink->NotifyMessageFlags(this, &aKeyStruct);
}

void
nsImapProtocol::NotifySearchHit(const char * hitLine)
{
    if (m_imapMiscellaneousSink)
        m_imapMiscellaneousSink->AddSearchResult(this, hitLine);
}

void nsImapProtocol::SetMailboxDiscoveryStatus(EMailboxDiscoverStatus status)
{
    PR_EnterMonitor(GetDataMemberMonitor());
	m_discoveryStatus = status;
    PR_ExitMonitor(GetDataMemberMonitor());
}

EMailboxDiscoverStatus nsImapProtocol::GetMailboxDiscoveryStatus( )
{
	EMailboxDiscoverStatus returnStatus;
    PR_EnterMonitor(GetDataMemberMonitor());
	returnStatus = m_discoveryStatus;
    PR_ExitMonitor(GetDataMemberMonitor());
    
    return returnStatus;
}

PRBool
nsImapProtocol::GetSubscribingNow()
{
    // ***** code me *****
    return PR_FALSE;// ***** for now
}

void
nsImapProtocol::DiscoverMailboxSpec(mailbox_spec * adoptedBoxSpec)  
{
	// IMAP_LoadTrashFolderName(); **** needs to work on localization issues

	nsIMAPNamespace *ns = nsnull;
    const char* hostName = GetImapHostName();
    const char *userName = GetImapUserName();

    NS_ASSERTION (m_hostSessionList, "fatal null host session list");
    if (!m_hostSessionList) return;

    m_hostSessionList->GetDefaultNamespaceOfTypeForHost(
        hostName, userName, kPersonalNamespace, ns);
	const char *nsPrefix = ns ? ns->GetPrefix() : 0;

	nsString2 canonicalSubDir("",eOneByte, 0);
	if (nsPrefix)
	{
        PRUnichar slash = '/';
		canonicalSubDir = nsPrefix;
		if (canonicalSubDir.Length() && canonicalSubDir.Last() == slash)
            canonicalSubDir.SetCharAt((PRUnichar)0, canonicalSubDir.Length());
	}
		
    switch (m_hierarchyNameState)
	{
	case kNoOperationInProgress:
	case kDiscoverTrashFolderInProgress:
	case kListingForInfoAndDiscovery:
		{
            if (canonicalSubDir.Length() &&
                PL_strstr(adoptedBoxSpec->allocatedPathName,
                          canonicalSubDir.GetBuffer()))
                m_onlineBaseFolderExists = TRUE;

            if (ns && nsPrefix)	// if no personal namespace, there can be no Trash folder
			{
                PRBool onlineTrashFolderExists = PR_FALSE;
                if (m_hostSessionList)
                    m_hostSessionList->GetOnlineTrashFolderExistsForHost(
                        hostName, userName, onlineTrashFolderExists);

				if (GetDeleteIsMoveToTrash() &&	// don't set the Trash flag if not using the Trash model
					!onlineTrashFolderExists && 
                    PL_strstr(adoptedBoxSpec->allocatedPathName, 
                              kImapTrashFolderName))
				{
					PRBool trashExists = PR_FALSE;
                    nsString2 trashMatch("",eOneByte,0);
                    trashMatch = nsPrefix;
                    trashMatch += kImapTrashFolderName;
					{
						char *serverTrashName = nsnull;
                        m_runningUrl->AllocateCanonicalPath(
                            trashMatch.GetBuffer(),
                            ns->GetDelimiter(), &serverTrashName); 
						if (serverTrashName)
						{
							if (!PL_strcasecmp(nsPrefix, "INBOX."))	// need to special-case this because it should be case-insensitive
							{
#ifdef DEBUG
								NS_ASSERTION (PL_strlen(serverTrashName) > 6,
                                              "Oops.. less that 6");
#endif
								trashExists = ((PL_strlen(serverTrashName) > 6 /* XP_STRLEN("INBOX.") */) &&
									(PL_strlen(adoptedBoxSpec->allocatedPathName) > 6 /* XP_STRLEN("INBOX.") */) &&
									!PL_strncasecmp(adoptedBoxSpec->allocatedPathName, serverTrashName, 6) &&	/* "INBOX." */
									!PL_strcmp(adoptedBoxSpec->allocatedPathName + 6, serverTrashName + 6));
							}
							else
							{
								trashExists = (PL_strcmp(serverTrashName, adoptedBoxSpec->allocatedPathName) == 0);
							}
                            if (m_hostSessionList)
							m_hostSessionList->
                                SetOnlineTrashFolderExistsForHost(
                                    hostName, userName, trashExists);
							PR_Free(serverTrashName);
						}
					}
					
					if (trashExists)
	                	adoptedBoxSpec->box_flags |= kImapTrash;
				}
			}

			// Discover the folder (shuttle over to libmsg, yay)
			// Do this only if the folder name is not empty (i.e. the root)
			if (adoptedBoxSpec->allocatedPathName&&
                *adoptedBoxSpec->allocatedPathName)
			{
                nsString2 boxNameCopy ("",eOneByte, 0);
                
				boxNameCopy = adoptedBoxSpec->allocatedPathName;

                if (m_imapMailFolderSink)
                {
                    m_imapMailFolderSink->PossibleImapMailbox(this,
                                                          adoptedBoxSpec);
                    WaitForFEEventCompletion();
                
                    PRBool useSubscription = PR_FALSE;

                    if (m_hostSessionList)
                        m_hostSessionList->GetHostIsUsingSubscription(
                            hostName, userName,
                            useSubscription);

					if ((GetMailboxDiscoveryStatus() != eContinue) && 
						(GetMailboxDiscoveryStatus() != eContinueNew) &&
						(GetMailboxDiscoveryStatus() != eListMyChildren))
					{
                    	SetConnectionStatus(-1);
					}
					else if (boxNameCopy.Length() && 
                             (GetMailboxDiscoveryStatus() == 
                              eListMyChildren) &&
                             (!useSubscription || GetSubscribingNow()))
					{
						NS_ASSERTION (FALSE, 
                                      "we should never get here anymore");
                    	SetMailboxDiscoveryStatus(eContinue);
					}
					else if (GetMailboxDiscoveryStatus() == eContinueNew)
					{
						if (m_hierarchyNameState ==
                            kListingForInfoAndDiscovery &&
                            boxNameCopy.Length() && 
                            !adoptedBoxSpec->folderIsNamespace)
						{
							// remember the info here also
							nsIMAPMailboxInfo *mb = new
                                nsIMAPMailboxInfo(boxNameCopy.GetBuffer(),
                                    adoptedBoxSpec->hierarchySeparator); 
                            m_listedMailboxList.AppendElement((void*) mb);
						}
						SetMailboxDiscoveryStatus(eContinue);
					}
				}
			}
        }
        break;
    case kDiscoverBaseFolderInProgress:
        {
            if (canonicalSubDir.Length() &&
                PL_strstr(adoptedBoxSpec->allocatedPathName,
                          canonicalSubDir.GetBuffer()))
                m_onlineBaseFolderExists = TRUE;
        }
        break;
    case kDeleteSubFoldersInProgress:
        {
            m_deletableChildren.AppendElement((void*)
                adoptedBoxSpec->allocatedPathName);
			delete adoptedBoxSpec->flagState;
            PR_FREEIF( adoptedBoxSpec);
        }
        break;
	case kListingForInfoOnly:
		{
			//UpdateProgressWindowForUpgrade(adoptedBoxSpec->allocatedPathName);
			ProgressEventFunctionUsingIdWithString(
                /***** fix me **** MK_MSG_IMAP_DISCOVERING_MAILBOX */ -1,
                adoptedBoxSpec->allocatedPathName);
			nsIMAPMailboxInfo *mb = new
                nsIMAPMailboxInfo(adoptedBoxSpec->allocatedPathName,
                                  adoptedBoxSpec->hierarchySeparator);
			m_listedMailboxList.AppendElement((void*) mb);
            PR_FREEIF(adoptedBoxSpec->allocatedPathName);
            PR_FREEIF(adoptedBoxSpec);
		}
		break;
	case kDiscoveringNamespacesOnly:
		{
            PR_FREEIF(adoptedBoxSpec->allocatedPathName);
            PR_FREEIF(adoptedBoxSpec);
		}
		break;
    default:
        NS_ASSERTION (FALSE, "we aren't supposed to be here");
        break;
	}
}

void
nsImapProtocol::AlertUserEventUsingId(PRUint32 aMessageId)
{
    if (m_imapMiscellaneousSink)
        m_imapMiscellaneousSink->FEAlert(this, 
                          "**** Fix me with real string ****\r\n");
}

void
nsImapProtocol::AlertUserEvent(const char * message)
{
    if (m_imapMiscellaneousSink)
        m_imapMiscellaneousSink->FEAlert(this, message);
}

void
nsImapProtocol::AlertUserEventFromServer(const char * aServerEvent)
{
    if (m_imapMiscellaneousSink)
        m_imapMiscellaneousSink->FEAlertFromServer(this, aServerEvent);
}

void
nsImapProtocol::ShowProgress()
{
    ProgressInfo aProgressInfo;

	if (m_progressStringId)
	{
		PRUnichar *progressString = NULL;
		progressString = IMAPGetStringByID(m_progressStringId);
		const char *mailboxName = GetServerStateParser().GetSelectedMailboxName();
//		progressString = PR_sprintf_append(progressString, XP_GetString(m_progressStringId), (mailboxName) ? mailboxName : "", ++m_progressIndex, m_progressCount);
		if (progressString)
			PercentProgressUpdateEvent(progressString,(100*(++m_progressIndex))/m_progressCount );
		PR_FREEIF(progressString);
		aProgressInfo.message = progressString;
		aProgressInfo.percent = (100*(m_progressIndex))/m_progressCount;

		if (m_imapMiscellaneousSink)
			m_imapMiscellaneousSink->PercentProgress(this, &aProgressInfo);
	}
}

void
nsImapProtocol::ProgressEventFunctionUsingId(PRUint32 aMsgId)
{
	PRUnichar *status = IMAPGetStringByID(aMsgId);
    if (m_imapMiscellaneousSink)
	{
        m_imapMiscellaneousSink->ProgressStatus(this, aMsgId);
		// who's going to free this? Does ProgressStatus complete synchronously?
	}
}

void
nsImapProtocol::ProgressEventFunctionUsingIdWithString(PRUint32 aMsgId, const
                                                       char * aExtraInfo)
{
    if (m_imapMiscellaneousSink)
	{
//		PRUnichar *progressMsg = IMAPGetStringByID(aMsgId);

		// ### FIXME - need to format this string, and pass it status. Or, invent a new interface
        m_imapMiscellaneousSink->ProgressStatus(this, aMsgId);
	}
}

void
nsImapProtocol::PercentProgressUpdateEvent(PRUnichar *message, PRInt32 percent)
{
    ProgressInfo aProgressInfo;
    aProgressInfo.message = message;
    aProgressInfo.percent = percent;
    if (m_imapMiscellaneousSink)
        m_imapMiscellaneousSink->PercentProgress(this, &aProgressInfo);
}

// convert back and forth between imap utf7 and unicode.
char*
nsImapProtocol::CreateUtf7ConvertedString(const char * aSourceString, 
										  PRBool aConvertToUtf7Imap)
{
	nsresult res;
	char *dstPtr = nsnull;
	PRInt32 dstLength = 0;
	char *convertedString = NULL;
	
	NS_WITH_SERVICE(nsICharsetConverterManager, ccm, kCharsetConverterManagerCID, &res); 

	if(NS_SUCCEEDED(res) && (nsnull != ccm))
	{
		nsString aCharset("x-imap4-modified-utf7");
		PRUnichar *unichars = nsnull;
		PRInt32 unicharLength;

		if (!aConvertToUtf7Imap)
		{
			// convert utf7 to unicode
			nsIUnicodeDecoder* decoder = nsnull;

			res = ccm->GetUnicodeDecoder(&aCharset, &decoder);
			if(NS_SUCCEEDED(res) && (nsnull != decoder)) 
			{
				PRInt32 srcLen = PL_strlen(aSourceString);
				res = decoder->Length(aSourceString, 0, srcLen, &unicharLength);
				// temporary buffer to hold unicode string
				unichars = new PRUnichar[unicharLength + 1];
				if (unichars == nsnull) 
				{
					res = NS_ERROR_OUT_OF_MEMORY;
				}
				else 
				{
					res = decoder->Convert(unichars, 0, &unicharLength, aSourceString, 0, &srcLen);
					unichars[unicharLength] = 0;
				}
				NS_IF_RELEASE(decoder);
				// convert the unicode to 8 bit ascii.
				nsString2 unicodeStr(unichars);
				convertedString = (char *) PR_Malloc(unicharLength + 1);
				if (convertedString)
					unicodeStr.ToCString(convertedString, unicharLength + 1, 0);
			}
		}
		else
		{
			// convert from 8 bit ascii string to modified utf7
			nsString2 unicodeStr(aSourceString);
			nsIUnicodeEncoder* encoder = nsnull;
			aCharset.SetString("x-imap4-modified-utf7");
			res = ccm->GetUnicodeEncoder(&aCharset, &encoder);
			if(NS_SUCCEEDED(res) && (nsnull != encoder)) 
			{
				res = encoder->GetMaxLength(unicodeStr.GetUnicode(), unicodeStr.Length(), &dstLength);
				// allocale an output buffer
				dstPtr = (char *) PR_CALLOC(dstLength + 1);
				unicharLength = unicodeStr.Length();
				if (dstPtr == nsnull) 
				{
					res = NS_ERROR_OUT_OF_MEMORY;
				}
				else 
				{
					res = encoder->Convert(unicodeStr.GetUnicode(), &unicharLength, dstPtr, &dstLength);
					dstPtr[dstLength] = 0;
				}
			}
			NS_IF_RELEASE(encoder);
			nsString2 unicodeStr2(dstPtr);
			convertedString = (char *) PR_Malloc(dstLength + 1);
			if (convertedString)
				unicodeStr2.ToCString(convertedString, dstLength + 1, 0);
        }
        delete [] unichars;
      }
    
    return convertedString;
}

PRUnichar * nsImapProtocol::CreatePRUnicharStringFromUTF7(const char * aSourceString)
{
	PRUnichar *convertedString = NULL;
	nsresult res;
	
	NS_WITH_SERVICE(nsICharsetConverterManager, ccm, kCharsetConverterManagerCID, &res); 

	if(NS_SUCCEEDED(res) && (nsnull != ccm))
	{
		nsString aCharset("x-imap4-modified-utf7");
		PRUnichar *unichars = nsnull;
		PRInt32 unicharLength;

		// convert utf7 to unicode
		nsIUnicodeDecoder* decoder = nsnull;

		res = ccm->GetUnicodeDecoder(&aCharset, &decoder);
		if(NS_SUCCEEDED(res) && (nsnull != decoder)) 
		{
			PRInt32 srcLen = PL_strlen(aSourceString);
			res = decoder->Length(aSourceString, 0, srcLen, &unicharLength);
			// temporary buffer to hold unicode string
			unichars = new PRUnichar[unicharLength + 1];
			if (unichars == nsnull) 
			{
				res = NS_ERROR_OUT_OF_MEMORY;
			}
			else 
			{
				res = decoder->Convert(unichars, 0, &unicharLength, aSourceString, 0, &srcLen);
				unichars[unicharLength] = 0;
			}
			NS_IF_RELEASE(decoder);
			// convert the unicode to 8 bit ascii.
			nsString2 unicodeStr(unichars);
			convertedString = unicodeStr.ToNewUnicode();
		}
    }
    return convertedString;
}


	// imap commands issued by the parser
void
nsImapProtocol::Store(nsString2 &messageList, const char * messageData,
                      PRBool idsAreUid)
{
   IncrementCommandTagNumber();
    
    char *formatString;
    if (idsAreUid)
        formatString = "%s uid store %s %s\015\012";
    else
        formatString = "%s store %s %s\015\012";
        
    // we might need to close this mailbox after this
	m_closeNeededBeforeSelect = GetDeleteIsMoveToTrash() &&
        (PL_strcasestr(messageData, "\\Deleted"));

	const char *commandTag = GetServerCommandTag();
	int protocolStringSize = PL_strlen(formatString) +
        PL_strlen(messageList.GetBuffer()) + PL_strlen(messageData) +
        PL_strlen(commandTag) + 1;
	char *protocolString = (char *) PR_CALLOC( protocolStringSize );

	if (protocolString)
	{
	    PR_snprintf(protocolString, // string to create
                    protocolStringSize, // max size
                    formatString, // format string
                    commandTag, // command tag
                    messageList.GetBuffer(),
                    messageData);
        
	    nsresult rv = SendData(protocolString);
	    if (NS_SUCCEEDED(rv))
            ParseIMAPandCheckForNewMail(protocolString);
	    PR_Free(protocolString);
    }
    else
    	HandleMemoryFailure();
}

void
nsImapProtocol::UidExpunge(const char* messageSet)
{
    IncrementCommandTagNumber();
    nsString2 command(GetServerCommandTag(), eOneByte);
    command.Append(" uid expunge ");
    command.Append(messageSet);
    command.Append(CRLF);
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void
nsImapProtocol::Expunge()
{
    ProgressEventFunctionUsingId (/***** fix me **** MK_IMAP_STATUS_EXPUNGING_MAILBOX */ -1);
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
    command.Append(" expunge"CRLF);
    
	nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void
nsImapProtocol::HandleMemoryFailure()
{
    PR_CEnterMonitor(this);
    // **** jefft fix me!!!!!! ******
    // m_imapThreadIsRunning = PR_FALSE;
    // SetConnectionStatus(-1);
    PR_CExitMonitor(this);
}

void nsImapProtocol::HandleCurrentUrlError()
{
#ifdef UNREADY_CODE
	if (fCurrentUrl->GetIMAPurlType() == TIMAPUrl::kSelectFolder)
	{
		// let the front end know the select failed so they
		// don't leave the view without a database.
		mailbox_spec *notSelectedSpec = (mailbox_spec *) XP_CALLOC(1, sizeof( mailbox_spec));
		if (notSelectedSpec)
		{
			notSelectedSpec->allocatedPathName = fCurrentUrl->CreateCanonicalSourceFolderPathString();
			notSelectedSpec->hostName = fCurrentUrl->GetUrlHost();
			notSelectedSpec->folderSelected = FALSE;
			notSelectedSpec->flagState = NULL;
			notSelectedSpec->onlineVerified = FALSE;
			UpdatedMailboxSpec(notSelectedSpec);
		}
	}
	else if (fCurrentUrl->GetIMAPurlType() == TIMAPUrl::kOfflineToOnlineMove)
	{
		OnlineCopyCompleted(kFailedCopy);
	}
#endif
}

void nsImapProtocol::Capability()
{

    ProgressEventFunctionUsingId (IMAP_STATUS_CHECK_COMPAT);
    IncrementCommandTagNumber();
	nsString2 command(GetServerCommandTag(), eOneByte);

    command.Append(" capability" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::InsecureLogin(const char *userName, const char *password)
{

#ifdef UNREADY_CODE
    ProgressEventFunction_UsingId (MK_IMAP_STATUS_SENDING_LOGIN);
#endif
    IncrementCommandTagNumber();
	nsString2 command (GetServerCommandTag(), eOneByte);
	command.Append(" login \"");
	command.Append(userName);
	command.Append("\" \"");
	command.Append(password);
	command.Append("\""CRLF);

	nsresult rv = SendData(command.GetBuffer());
    
//    PR_snprintf(m_dataOutputBuf,  OUTPUT_BUFFER_SIZE, "%s login \"%s\" \"%s\"" CRLF,
//				GetServerCommandTag(), userName,  password);

//	SendData(m_dataOutputBuf);
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::AuthLogin(const char *userName, const char *password, eIMAPCapabilityFlag flag)
{
#ifdef UNREADY_CODE
	ProgressEventFunction_UsingId (MK_IMAP_STATUS_SENDING_AUTH_LOGIN);
#endif
    IncrementCommandTagNumber();

	char * currentCommand=nsnull;
    nsresult rv;
    
	if (flag & kHasAuthPlainCapability)
	{
		PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE, "%s authenticate plain" CRLF, GetServerCommandTag());
		rv = SendData(m_dataOutputBuf);
        if (NS_FAILED(rv)) return;
		currentCommand = PL_strdup(m_dataOutputBuf); /* StrAllocCopy(currentCommand, GetOutputBuffer()); */
		ParseIMAPandCheckForNewMail();
		if (GetServerStateParser().LastCommandSuccessful()) 
		{
			char plainstr[512]; // placeholder for "<NUL>userName<NUL>password"
			int len = 1; // count for first <NUL> char
			nsCRT::memset(plainstr, 0, 512);
			PR_snprintf(&plainstr[1], 510, "%s", userName);
			len += PL_strlen(userName);
			len++;	// count for second <NUL> char
			PR_snprintf(&plainstr[len], 511-len, "%s", password);
			len += PL_strlen(password);
		
			char *base64Str = PL_Base64Encode(plainstr, len, nsnull);
			if (base64Str)
			{
				PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE, "%s" CRLF, base64Str);
				PR_Free(base64Str);
				rv = SendData(m_dataOutputBuf);
                if (NS_SUCCEEDED(rv))
                    ParseIMAPandCheckForNewMail(currentCommand);
				if (GetServerStateParser().LastCommandSuccessful())
				{
					PR_FREEIF(currentCommand);
					return;
				} // if the last command succeeded
			} // if we got a base 64 encoded string
		} // if the last command succeeded
	} // if auth plain capability

	else if (flag & kHasAuthLoginCapability)
	{
		PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE, "%s authenticate login" CRLF, GetServerCommandTag());
		rv = SendData(m_dataOutputBuf);
        if (NS_FAILED(rv)) return;
		currentCommand = PL_strdup(m_dataOutputBuf);
		ParseIMAPandCheckForNewMail();

		if (GetServerStateParser().LastCommandSuccessful()) 
		{
			char *base64Str = PL_Base64Encode(userName, PL_strlen(userName), nsnull);
			if(base64Str)
			{
				PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE, "%s" CRLF, base64Str);
				PR_Free(base64Str);
				rv = SendData(m_dataOutputBuf);
                if (NS_SUCCEEDED(rv))
                    ParseIMAPandCheckForNewMail(currentCommand);
			}
			if (GetServerStateParser().LastCommandSuccessful()) 
			{
				base64Str = PL_Base64Encode((char*)password, PL_strlen(password), nsnull);
				PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE, "%s" CRLF, base64Str);
				PR_FREEIF(base64Str);
				rv = SendData(m_dataOutputBuf);
                if (NS_SUCCEEDED(rv))
                    ParseIMAPandCheckForNewMail(currentCommand);
				if (GetServerStateParser().LastCommandSuccessful())
				{
					PR_FREEIF(currentCommand);
					return;
				}
			} // if last command successful
		} // if last command successful
	} // if has auth login capability

	// fall back to use InsecureLogin()
	InsecureLogin(userName, password);
	PR_FREEIF(currentCommand);
}

void nsImapProtocol::OnLSubFolders()
{
	// **** use to find out whether Drafts, Sent, & Templates folder
	// exists or not even the user didn't subscribe to it
	char *mailboxName = OnCreateServerSourceFolderPathString();
	if (mailboxName)
	{
#ifdef UNREADY_CODE
		ProgressEventFunction_UsingId(MK_IMAP_STATUS_LOOKING_FOR_MAILBOX);
#endif
		IncrementCommandTagNumber();
		PR_snprintf(m_dataOutputBuf, OUTPUT_BUFFER_SIZE,"%s list \"\" \"%s\"" CRLF, GetServerCommandTag(), mailboxName);
		nsresult rv = SendData(m_dataOutputBuf);
#ifdef UNREADY_CODE
		TimeStampListNow();
#endif
        if (NS_SUCCEEDED(rv))
            ParseIMAPandCheckForNewMail();	
		PR_Free(mailboxName);
	}
	else
	{
		HandleMemoryFailure();
	}

}

void nsImapProtocol::OnGetMailAccount()
{
	NS_ASSERTION(0, "unimplemented feature");
#ifdef UNREADY_CODE
	if (GetServerStateParser().GetCapabilityFlag() & kHasXNetscapeCapability) 
	{
		Netscape();
		if (GetServerStateParser().LastCommandSuccessful()) 
		{
			TImapFEEvent *alertEvent = 
				new TImapFEEvent(msgSetMailAccountURL,  // function to call
								 this,                // access to current entry
								 (void *) fCurrentUrl->GetUrlHost(),
								 PR_TRUE);
			if (alertEvent)
			{
				fFEEventQueue->AdoptEventToEnd(alertEvent);
				// WaitForFEEventCompletion();
			}
			else
				HandleMemoryFailure();
		}
	}
#endif
}

void nsImapProtocol::OnOfflineToOnlineMove()
{
	NS_ASSERTION(0, "unimplemented feature");
#ifdef UNREADY_CODE
    char *destinationMailbox = OnCreateServerDestinationFolderPathString();
    
	if (destinationMailbox)
	{
        uint32 appendSize = 0;
        do {
            WaitForNextAppendMessageSize();
            appendSize = GetAppendSize();
            if (!DeathSignalReceived() && appendSize)
            {
                char messageSizeString[100];
                sprintf(messageSizeString, "%ld",(long) appendSize);
                AppendMessage(destinationMailbox, messageSizeString, GetAppendFlags());
            }
        } while (appendSize && GetServerStateParser().LastCommandSuccessful());
        PR_FREEIF( destinationMailbox);
    }
    else
        HandleMemoryFailure();
#endif
}

void nsImapProtocol::OnAppendMsgFromFile()
{
    nsCOMPtr<nsIFileSpec> fileSpec;
    nsresult rv = NS_OK;
    rv = m_runningUrl->GetMsgFileSpec(getter_AddRefs(fileSpec));
    if (NS_SUCCEEDED(rv) && fileSpec)
    {
        char *mailboxName =  
            OnCreateServerSourceFolderPathString();
        if (mailboxName)
        {
            UploadMessageFromFile(fileSpec, mailboxName, kImapMsgSeenFlag);
            PR_Free( mailboxName );
        }
        else
        {
            HandleMemoryFailure();
        }
    }
}

void nsImapProtocol::UploadMessageFromFile (nsIFileSpec* fileSpec,
                                            const char* mailboxName,
                                            imapMessageFlagsType flags)
{
    if (!fileSpec || !mailboxName) return;
    IncrementCommandTagNumber();

    PRUint32 fileSize = 0;
    PRInt32 totalSize, readCount;
    char *dataBuffer = nsnull;
    nsString2 command(GetServerCommandTag(), eOneByte);
    char* escapedName = CreateEscapedMailboxName(mailboxName);
    nsresult rv;
    PRBool isOpen = PR_FALSE;
    PRBool eof = PR_FALSE;
    nsString2 flagString("", eOneByte);
    PRBool hasLiteralPlus = (GetServerStateParser().GetCapabilityFlag() &
                             kLiteralPlusCapability);

    if (escapedName)
    {
        command.Append(" append \"");
        command.Append(escapedName);
        command.Append("\" (");
        
        SetupMessageFlagsString(flagString, flags,
                                GetServerStateParser().SupportsUserFlags());
        command.Append(flagString.GetBuffer());
        command.Append(") {");

        dataBuffer = (char*) PR_CALLOC(FOUR_K+1);
        if (!dataBuffer) goto done;
        rv = fileSpec->GetFileSize(&fileSize);
        if (NS_FAILED(rv)) goto done;
        rv = fileSpec->openStreamForReading();
        if (NS_FAILED(rv)) goto done;
        command.Append((PRInt32)fileSize);
        if (hasLiteralPlus)
            command.Append("+}" CRLF);
        else
            command.Append("}" CRLF);

        rv = SendData(command.GetBuffer());
        if (NS_FAILED(rv)) goto done;

        if (!hasLiteralPlus)
            ParseIMAPandCheckForNewMail();

        totalSize = fileSize;
        readCount = 0;
        while(NS_SUCCEEDED(rv) && !eof && totalSize > 0)
        {
            rv = fileSpec->read(&dataBuffer, FOUR_K, &readCount);
            if (NS_SUCCEEDED(rv))
            {
                dataBuffer[readCount] = 0;
                rv = SendData(dataBuffer);
                totalSize -= readCount;
                rv = fileSpec->eof(&eof);
            }
        }
        if (NS_SUCCEEDED(rv))
        {
            rv = SendData(CRLF); // complete the append
            ParseIMAPandCheckForNewMail(command.GetBuffer());

            nsIImapUrl::nsImapAction imapAction;
            m_runningUrl->GetImapAction(&imapAction);

            if (GetServerStateParser().LastCommandSuccessful() &&
                imapAction == nsIImapUrl::nsImapAppendDraftFromFile)
            {
                nsCOMPtr<nsISupports> copyState;
                if(m_runningUrl)
                    m_runningUrl->GetCopyState(getter_AddRefs(copyState));

                if (GetServerStateParser().GetCapabilityFlag() &
                    kUidplusCapability)
                {
                    nsMsgKey newKey =
                        GetServerStateParser().CurrentResponseUID();
                    if (m_imapExtensionSink)
                    {
                        m_imapExtensionSink->SetAppendMsgUid(this, newKey,
                                                             copyState);
                        WaitForFEEventCompletion();
                    }
                    nsString2 oldMsgId("", eOneByte);
                    rv =
                        m_runningUrl->CreateListOfMessageIdsString(&oldMsgId);
                    if (NS_SUCCEEDED(rv) && oldMsgId.Length() > 0)
                    {
                        PRBool idsAreUids = PR_TRUE;
                        m_runningUrl->MessageIdsAreUids(&idsAreUids);
                        Store(oldMsgId, "+FLAGS (\\Deleted)", idsAreUids);
                        UidExpunge(oldMsgId.GetBuffer());
                    }
                }
                else if (m_imapExtensionSink)
                { // *** code me to search for the newly appended message
                    nsString2 messageId("", eOneByte);
                    rv = m_imapExtensionSink->GetMessageId(this, &messageId,
                                                       copyState);
                    WaitForFEEventCompletion();
                    if (NS_SUCCEEDED(rv) && messageId.Length() > 0 &&
                        GetServerStateParser().LastCommandSuccessful())
                    {
                        command = "search seen header Message-ID ";
                        command.Append(messageId);

                        Search(command, PR_TRUE, PR_FALSE);
                        if (GetServerStateParser().LastCommandSuccessful())
						{
                            nsMsgKey newkey = nsMsgKey_None;
							nsImapSearchResultIterator *searchResult = 
								GetServerStateParser().CreateSearchResultIterator();
							newkey = searchResult->GetNextMessageNumber();
							delete searchResult;
                            if (newkey != nsMsgKey_None)
                            {
                                m_imapExtensionSink->SetAppendMsgUid
                                    (this, newkey, copyState);
                                WaitForFEEventCompletion();
                            }
                        }
                    }
                }
            }
        }
    }
done:
    PR_FREEIF(dataBuffer);
    rv = fileSpec->isStreamOpen(&isOpen);
    if (NS_SUCCEEDED(rv) && isOpen)
        fileSpec->closeStream();
    delete [] escapedName;
}

//caller must free using PR_Free
char * nsImapProtocol::OnCreateServerSourceFolderPathString()
{
	char *sourceMailbox = nsnull;
	m_runningUrl->CreateServerSourceFolderPathString(&sourceMailbox);
	if (sourceMailbox)
	{
		char *convertedName = CreateUtf7ConvertedString(sourceMailbox, PR_TRUE);
		PR_Free(sourceMailbox);
		sourceMailbox = convertedName;
	}

	return sourceMailbox;
}

//caller must free using PR_Free
char * nsImapProtocol::OnCreateServerDestinationFolderPathString()
{
	char *destinationMailbox = nsnull;
	m_runningUrl->CreateServerDestinationFolderPathString(&destinationMailbox);
	if (destinationMailbox)
	{
		char *convertedName = CreateUtf7ConvertedString(destinationMailbox, PR_TRUE);
		PR_Free(destinationMailbox);
		destinationMailbox = convertedName;
	}

	return destinationMailbox;
}

void nsImapProtocol::OnCreateFolder(const char * aSourceMailbox)
{
	NS_ASSERTION(0, "on create folder is not implemented yet");
#ifdef UNREADY_CODE
           PR_Bool created = CreateMailboxRespectingSubscriptions(aSourceMailbox);
            if (created)
			{
				List(aSourceMailbox, PR_FALSE);
			}
			else
				FolderNotCreated(aSourceMailbox);
#endif
}

void nsImapProtocol::OnSubscribe(const char * aSourceMailbox)
{
	NS_ASSERTION(0, "subscribe is not implemented yet");
#ifdef UNREADY_CODE
	Subscribe(sourceMailbox);
#endif
}

void nsImapProtocol::OnUnsubscribe(const char * aSourceMailbox)
{
	// When we try to auto-unsubscribe from \Noselect folders,
	// some servers report errors if we were already unsubscribed
	// from them.
#ifdef UNREADY_CODE
	PRBool lastReportingErrors = GetServerStateParser().GetReportingErrors();
	GetServerStateParser().SetReportingErrors(FALSE);
	Unsubscribe(sourceMailbox);
	GetServerStateParser().SetReportingErrors(lastReportingErrors);
#endif
}

void nsImapProtocol::OnRefreshACLForFolder(const char *mailboxName)
{
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
    char *escapedName = CreateEscapedMailboxName(mailboxName);
    command.Append(" getacl \"");
	command.Append(escapedName);
	command.Append("\"" CRLF);
            
    delete []escapedName;
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::OnRefreshAllACLs()
{
	// mscott - haven't ported this yet
}

// any state commands
void nsImapProtocol::Logout()
{
//	ProgressEventFunction_UsingId (MK_IMAP_STATUS_LOGGING_OUT);

/******************************************************************
 * due to the undo functionality we cannot issule a close when logout; there
 * is no way to do an undo if the message has been permanently expunge
 * jt - 07/12/1999

    PRBool closeNeeded = GetServerStateParser().GetIMAPstate() ==
        nsImapServerResponseParser::kFolderSelected;

    if (closeNeeded && GetDeleteIsMoveToTrash())
        Close();
********************/

	IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);

	command.Append(" logout" CRLF);

	nsresult rv = SendData(command.GetBuffer());
	// the socket may be dead before we read the response, so drop it.
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::Noop()
{
    //ProgressUpdateEvent("noop...");
    IncrementCommandTagNumber();
	nsString2 command(GetServerCommandTag(), eOneByte);
    
	command.Append(" noop" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::XServerInfo()
{

//    ProgressEventFunction_UsingId (MK_IMAP_GETTING_SERVER_INFO);
    IncrementCommandTagNumber();
  	nsString2 command(GetServerCommandTag(), eOneByte);
  
	command.Append(" XSERVERINFO MANAGEACCOUNTURL MANAGELISTSURL MANAGEFILTERSURL" CRLF);
          
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::XMailboxInfo(const char *mailboxName)
{

//    ProgressEventFunction_UsingId (MK_IMAP_GETTING_MAILBOX_INFO);
    IncrementCommandTagNumber();
  	nsString2 command(GetServerCommandTag(), eOneByte);

	command.Append(" XMAILBOXINFO \"");
	command.Append(mailboxName);
	command.Append("\"  MANAGEURL POSTURL" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::Namespace()
{

//    ProgressEventFunction_UsingId (MK_IMAP_STATUS_GETTING_NAMESPACE);
    IncrementCommandTagNumber();
    
	nsString2 command(GetServerCommandTag(), eOneByte);
	command.Append(" namespace" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}


void nsImapProtocol::MailboxData()
{
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
	command.Append(" mailboxdata" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}


void nsImapProtocol::GetMyRightsForFolder(const char *mailboxName)
{
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
    char *escapedName = CreateEscapedMailboxName(mailboxName);
    
	command.Append(" myrights \"");
	command.Append(escapedName);
	command.Append("\"" CRLF);
            
    delete []escapedName;
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::OnStatusForFolder(const char *mailboxName)
{
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
    char *escapedName = CreateEscapedMailboxName(mailboxName);
    
	command.Append(" STATUS \"");
	command.Append(escapedName);
	command.Append("\" (UIDNEXT MESSAGES UNSEEN)" CRLF);
            
    delete []escapedName;
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();

    mailbox_spec *new_spec = GetServerStateParser().CreateCurrentMailboxSpec(mailboxName);
	if (new_spec && m_imapMailFolderSink)
		m_imapMailFolderSink->UpdateImapMailboxStatus(this, new_spec);
}


void nsImapProtocol::OnListFolder(const char * aSourceMailbox, PRBool aBool)
{
#ifdef UNREADY_CODE
	ListFolder();
#endif
}

void nsImapProtocol::OnUpgradeToSubscription()
{
#ifdef UNREADY_CODE
	UpgradeToSubscription();
#endif
}

void nsImapProtocol::OnDeleteFolder(const char * aSourceMailbox)
{
#ifdef UNREADY_CODE
    PRBool deleted = DeleteSubFolders(sourceMailbox);
    if (deleted)
        deleted = DeleteMailboxRespectingSubscriptions(sourceMailbox);
	if (deleted)
		FolderDeleted(sourceMailbox);
#endif
}

void nsImapProtocol::OnRenameFolder(const char * aSourceMailbox)
{
#ifdef UNREADY_CODE
	char *destinationMailbox = OnCreateServerDestinationFolderPathString();

	if (destinationMailbox)
	{
		PRBool renamed = RenameHierarchyByHand(sourceMailbox, destinationMailbox);
		if (renamed)
			FolderRenamed(sourceMailbox, destinationMailbox);
        
		PR_Free( destinationMailbox);
	}
	else
		HandleMemoryFailure();
#endif
}

void nsImapProtocol::OnMoveFolderHierarchy(const char * aSourceMailbox)
{
#ifdef UNREADY_CODE
	char *destinationMailbox = OnCreateServerDestinationFolderPathString();

    if (destinationMailbox)
    {
        // worst case length
        char *newBoxName = (char *) PR_ALLOC(PL_Strlen(destinationMailbox) +
                                             PL_Strlen(sourceMailbox) + 2);
        if (newBoxName)
        {
            char onlineDirSeparator = fCurrentUrl->GetOnlineSubDirSeparator();
            XP_STRCPY(newBoxName, destinationMailbox);
            
            char *leafSeparator = XP_STRRCHR(sourceMailbox, onlineDirSeparator);
            if (!leafSeparator)
                leafSeparator = sourceMailbox;	// this is a root level box
            else
                leafSeparator++;
            
            if ( *newBoxName && ( *(newBoxName + XP_STRLEN(newBoxName) - 1) != onlineDirSeparator))
            {
                char separatorStr[2];
                separatorStr[0] = onlineDirSeparator;
                separatorStr[1] = 0;
                XP_STRCAT(newBoxName, separatorStr);
            }
            XP_STRCAT(newBoxName, leafSeparator);
			XP_Bool renamed = RenameHierarchyByHand(sourceMailbox, newBoxName);
	        if (renamed)
	            FolderRenamed(sourceMailbox, newBoxName);
        }
    }
    else
    	HandleMemoryFailure();
#endif
}

void nsImapProtocol::FindMailboxesIfNecessary()
{
    //PR_EnterMonitor(fFindingMailboxesMonitor);
    // biff should not discover mailboxes
	PRBool foundMailboxesAlready = PR_FALSE;
	nsIImapUrl::nsImapAction imapAction;
	nsresult rv = NS_OK;

	rv = m_runningUrl->GetImapAction(&imapAction);
	rv = m_hostSessionList->GetHaveWeEverDiscoveredFoldersForHost(GetImapHostName(), GetImapUserName(), foundMailboxesAlready);
    if (NS_SUCCEEDED(rv) && !foundMailboxesAlready &&
		(imapAction != nsIImapUrl::nsImapBiff) &&
		(imapAction != nsIImapUrl::nsImapDiscoverAllBoxesUrl) &&
		(imapAction != nsIImapUrl::nsImapUpgradeToSubscription) &&
		!GetSubscribingNow())
    {
		DiscoverMailboxList();

		// If we decide to do it, here is where we should check to see if
		// a namespace exists (personal namespace only?) and possibly
		// create it if it doesn't exist.
	}
    //PR_ExitMonitor(fFindingMailboxesMonitor);
}


// DiscoverMailboxList() is used to actually do the discovery of folders
// for a host.  This is used both when we initially start up (and re-sync)
// and also when the user manually requests a re-sync, by collapsing and
// expanding a host in the folder pane.  This is not used for the subscribe
// pane.
// DiscoverMailboxList() also gets the ACLs for each newly discovered folder
void nsImapProtocol::DiscoverMailboxList()
{
	PRBool usingSubscription = PR_FALSE;
	SetMailboxDiscoveryStatus(eContinue);
	if (GetServerStateParser().ServerHasACLCapability())
		m_hierarchyNameState = kListingForInfoAndDiscovery;
	else
		m_hierarchyNameState = kNoOperationInProgress;

	// Pretend that the Trash folder doesn't exist, so we will rediscover it if we need to.
	m_hostSessionList->SetOnlineTrashFolderExistsForHost(GetImapHostName(), GetImapUserName(), PR_FALSE);
	m_hostSessionList->GetHostIsUsingSubscription(GetImapHostName(), GetImapUserName(), usingSubscription);

	// iterate through all namespaces and LSUB them.
	PRUint32 count = 0;
	m_hostSessionList->GetNumberOfNamespacesForHost(GetImapHostName(),
                                                    GetImapUserName(), count);
	for (PRUint32 i = 0; i < count; i++ )
	{
		nsIMAPNamespace * ns = nsnull;
		m_hostSessionList->GetNamespaceNumberForHost(GetImapHostName(), GetImapUserName(),i,ns);
		if (ns)
		{
			const char *prefix = ns->GetPrefix();
			if (prefix)
			{
				static PRBool gHideUnusedNamespaces = PR_TRUE;
				// mscott -> WARNING!!! i where are we going to get this
                // global variable for unusued name spaces from??? *wince*
				// dmb - we should get this from a per-host preference,
				// I'd say. But for now, just make it TRUE;
				if (!gHideUnusedNamespaces && *prefix &&
                    PL_strcasecmp(prefix, "INBOX."))	// only do it for
                    // non-empty namespace prefixes, and for non-INBOX prefix 
				{
					// Explicitly discover each Namespace, so that we can
                    // create subfolders of them,
					mailbox_spec *boxSpec = (mailbox_spec *)
                        PR_CALLOC(sizeof(mailbox_spec)); 
					if (boxSpec)
					{
						boxSpec->folderSelected = PR_FALSE;
						boxSpec->hostName = nsCRT::strdup(GetImapHostName());
						boxSpec->connection = this;
						boxSpec->flagState = nsnull;
						boxSpec->discoveredFromLsub = PR_TRUE;
						boxSpec->onlineVerified = PR_TRUE;
						boxSpec->box_flags = kNoselect;
						boxSpec->hierarchySeparator = ns->GetDelimiter();
						m_runningUrl->AllocateCanonicalPath(
                            ns->GetPrefix(), ns->GetDelimiter(), 
                            &boxSpec->allocatedPathName);  
						boxSpec->namespaceForFolder = ns;
						boxSpec->folderIsNamespace = PR_TRUE;

						switch (ns->GetType())
						{
						case kPersonalNamespace:
							boxSpec->box_flags |= kPersonalMailbox;
							break;
						case kPublicNamespace:
							boxSpec->box_flags |= kPublicMailbox;
							break;
						case kOtherUsersNamespace:
							boxSpec->box_flags |= kOtherUsersMailbox;
							break;
						default:	// (kUnknownNamespace)
							break;
						}

						DiscoverMailboxSpec(boxSpec);
					}
					else
						HandleMemoryFailure();
				}

				// now do the folders within this namespace
				nsString2 pattern("",eOneByte);
				nsString2 pattern2("",eOneByte);
				if (usingSubscription)
				{
					pattern.Append(prefix);
					pattern.Append("*");
				}
				else
				{
					pattern.Append(prefix);
					pattern.Append("%"); // mscott just need one percent right?
					// pattern = PR_smprintf("%s%%", prefix);
					char delimiter = ns->GetDelimiter();
					if (delimiter)
					{
						// delimiter might be NIL, in which case there's no hierarchy anyway
						pattern2 = prefix;
						pattern2 += "%";
						pattern2 += delimiter;
						pattern2 += "%";
						pattern2 = PR_smprintf("%s%%%c%%", prefix, delimiter);
					}
				}


				if (usingSubscription) // && !GetSubscribingNow())	should never get here from subscribe pane
					Lsub(pattern.GetBuffer(), PR_TRUE);
				else
				{
					List(pattern.GetBuffer(), PR_TRUE);
					List(pattern2.GetBuffer(), TRUE);
				}
			}
		}
	}

	// explicitly LIST the INBOX if (a) we're not using subscription, or (b) we are using subscription and
	// the user wants us to always show the INBOX.
	PRBool listInboxForHost = PR_FALSE;
	m_hostSessionList->GetShouldAlwaysListInboxForHost(GetImapHostName(), GetImapUserName(), listInboxForHost);
	if (!usingSubscription || listInboxForHost) 
		List("INBOX", PR_TRUE);

	m_hierarchyNameState = kNoOperationInProgress;

	MailboxDiscoveryFinished();

	// Get the ACLs for newly discovered folders
	if (GetServerStateParser().ServerHasACLCapability())
	{
		PRInt32 total = m_listedMailboxList.Count(), cnt = 0;
		GetServerStateParser().SetReportingErrors(FALSE);
		if (total)
		{
#ifdef UNREADY_CODE
			ProgressEventFunction_UsingId(MK_IMAP_GETTING_ACL_FOR_FOLDER);
#endif
			nsIMAPMailboxInfo * mb = nsnull;
			do
			{
				mb = (nsIMAPMailboxInfo *) m_listedMailboxList[0]; // get top element
				m_listedMailboxList.RemoveElementAt(0); // XP_ListRemoveTopObject(fListedMailboxList);
				if (mb)
				{
					if (FolderNeedsACLInitialized(mb->GetMailboxName()))
					{
						char *onlineName = nsnull;
						m_runningUrl->AllocateServerPath(mb->GetMailboxName(), mb->GetDelimiter(), &onlineName);
						if (onlineName)
						{
							OnRefreshACLForFolder(onlineName);
							PR_Free(onlineName);
						}
					}
#ifdef UNREADY_CODE
					PercentProgressUpdateEvent(NULL, (cnt*100)/total);
#endif
					delete mb;	// this is the last time we're using the list, so delete the entries here
					cnt++;
				}
			} while (mb && !DeathSignalReceived());
		}
	}
}

PRBool nsImapProtocol::FolderNeedsACLInitialized(const char *folderName)
{
	FolderQueryInfo *value = (FolderQueryInfo *)PR_MALLOC(sizeof(FolderQueryInfo));
	PRBool rv = PR_FALSE;

	if (!value) return PR_FALSE;

	value->name = PL_strdup(folderName);
	if (!value->name)
	{
		PR_Free(value);
		return PR_FALSE;
	}

	value->hostName = PL_strdup(GetImapHostName());
	if (!value->hostName)
	{
		PR_Free(value->name);
		PR_Free(value);
		return PR_FALSE;
	}

	// mscott - big hack...where do we get a IMAPACLRights object from??????
	m_imapExtensionSink->FolderNeedsACLInitialized(this, /* value */ nsnull);
	WaitForFEEventCompletion();
	//TImapFEEvent *folderACLInitEvent = 
	//    new TImapFEEvent(MOZTHREAD_FolderNeedsACLInitialized,               // function to call
	//                    this,                                              // access to current entry/context
	//                    value, FALSE);

	rv = value->rv;
	PR_Free(value->hostName);
	PR_Free(value->name);
	PR_Free(value);
	return rv;
}

void nsImapProtocol::MailboxDiscoveryFinished()
{
    if (!DeathSignalReceived() && !GetSubscribingNow() &&
		((m_hierarchyNameState == kNoOperationInProgress) || 
		 (m_hierarchyNameState == kListingForInfoAndDiscovery)))
    {
		nsIMAPNamespace *ns = nsnull;
		m_hostSessionList->GetDefaultNamespaceOfTypeForHost(GetImapHostName(),GetImapUserName(), kPersonalNamespace, ns);
		const char *personalDir = ns ? ns->GetPrefix() : 0;
		
		PRBool trashFolderExists = PR_FALSE;
		PRBool usingSubscription = PR_FALSE;
		m_hostSessionList->GetOnlineTrashFolderExistsForHost(GetImapHostName(), GetImapUserName(), trashFolderExists);
		m_hostSessionList->GetHostIsUsingSubscription(GetImapHostName(), GetImapUserName(),usingSubscription);
		if (!trashFolderExists && GetDeleteIsMoveToTrash() && usingSubscription)
		{
			// maybe we're not subscribed to the Trash folder
			if (personalDir)
			{
				char *originalTrashName = CreatePossibleTrashName(personalDir);
				m_hierarchyNameState = kDiscoverTrashFolderInProgress;
				List(originalTrashName, PR_TRUE);
				m_hierarchyNameState = kNoOperationInProgress;
			}
		}

		// There is no Trash folder (either LIST'd or LSUB'd), and we're using the
		// Delete-is-move-to-Trash model, and there is a personal namespace
		if (!trashFolderExists && GetDeleteIsMoveToTrash() && ns)
    	{
    		char *trashName = CreatePossibleTrashName(ns->GetPrefix());
    		if (trashName)
			{
				char *onlineTrashName = nsnull;
				m_runningUrl->AllocateServerPath(trashName, ns->GetDelimiter(), &onlineTrashName);
				if (onlineTrashName)
				{
    				GetServerStateParser().SetReportingErrors(FALSE);
    				PRBool created = CreateMailboxRespectingSubscriptions(onlineTrashName);
    				GetServerStateParser().SetReportingErrors(TRUE);
    				
    				// force discovery of new trash folder.
    				if (created)
					{
						m_hierarchyNameState = kDiscoverTrashFolderInProgress;
    					List(onlineTrashName, PR_FALSE);
						m_hierarchyNameState = kNoOperationInProgress;
					}
    				else
						m_hostSessionList->SetOnlineTrashFolderExistsForHost(GetImapHostName(), GetImapUserName(), PR_TRUE);
					PR_Free(onlineTrashName);
				}
   				PR_FREEIF(trashName);
			} // if trash name
		} //if trashg folder doesn't exist
		m_hostSessionList->SetHaveWeEverDiscoveredFoldersForHost(GetImapHostName(), GetImapUserName(), PR_TRUE);

		// notify front end that folder discovery is complete....
		m_imapMailFolderSink->MailboxDiscoveryDone(this);
    }
}

// returns TRUE is the create succeeded (regardless of subscription changes)
PRBool nsImapProtocol::CreateMailboxRespectingSubscriptions(const char *mailboxName)
{
	CreateMailbox(mailboxName);
	PRBool rv = GetServerStateParser().LastCommandSuccessful();
	if (rv)
	{
		// mscott hack alert!!! what are we going to do about auto subscribe??????
		PRBool fAutoSubscribe = PR_TRUE;
		if (fAutoSubscribe) // auto-subscribe is on
		{
			// create succeeded - let's subscribe to it
			PRBool reportingErrors = GetServerStateParser().GetReportingErrors();
			GetServerStateParser().SetReportingErrors(FALSE);
			OnSubscribe(mailboxName);
			GetServerStateParser().SetReportingErrors(reportingErrors);
		}
	}
	return (rv);
}

void nsImapProtocol::CreateMailbox(const char *mailboxName)
{
#ifdef UNREADY_CODE
    ProgressEventFunction_UsingId (MK_IMAP_STATUS_CREATING_MAILBOX);
#endif

    IncrementCommandTagNumber();
    
    char *escapedName = CreateEscapedMailboxName(mailboxName);
	nsString2 command(GetServerCommandTag(), eOneByte);
	command += " create \"";
	command += escapedName;
	command += "\""CRLF;
               
    delete []escapedName;

	nsresult rv = SendData(command.GetBuffer());
    if(NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

char * nsImapProtocol::CreatePossibleTrashName(const char *prefix)
{
	// mscott we used to have a localized global string for the trash name...
	// I haven't don't localization stuff yet so I'm going to do a bad thing and just
	// use a string literal....(only temporary!!!!! =))...

//	IMAP_LoadTrashFolderName();
	nsString2 returnTrash(prefix, eOneByte);

	returnTrash += "Trash";
	return PL_strdup(returnTrash.GetBuffer());
}

void nsImapProtocol::Lsub(const char *mailboxPattern, PRBool addDirectoryIfNecessary)
{
#ifdef UNREADY_CODE
    ProgressEventFunction_UsingId (MK_IMAP_STATUS_LOOKING_FOR_MAILBOX);
#endif

    IncrementCommandTagNumber();

	char *boxnameWithOnlineDirectory = nsnull;
	if (addDirectoryIfNecessary)
		m_runningUrl->AddOnlineDirectoryIfNecessary(mailboxPattern, &boxnameWithOnlineDirectory);

    char *escapedPattern = CreateEscapedMailboxName(boxnameWithOnlineDirectory ? 
													boxnameWithOnlineDirectory :
													mailboxPattern);

	nsString2 command (GetServerCommandTag(), eOneByte);
	command += " lsub \"\" \"";
	command += escapedPattern;
	command += "\""CRLF;

//    PR_snprintf(GetOutputBuffer(),                              // string to create
//            kOutputBufferSize,                              // max size
//            "%s lsub \"\" \"%s\"" CRLF,                   // format string
//            GetServerCommandTag(),                  // command tag
//            escapedPattern);
            
    delete []escapedPattern;
	PR_FREEIF(boxnameWithOnlineDirectory);

	nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::List(const char *mailboxPattern, PRBool addDirectoryIfNecessary)
{
#ifdef UNREADY_CODE
    ProgressEventFunction_UsingId (MK_IMAP_STATUS_LOOKING_FOR_MAILBOX);
#endif

    IncrementCommandTagNumber();

	char *boxnameWithOnlineDirectory = nsnull;
	if (addDirectoryIfNecessary)
		m_runningUrl->AddOnlineDirectoryIfNecessary(mailboxPattern, &boxnameWithOnlineDirectory);

    char *escapedPattern = CreateEscapedMailboxName(boxnameWithOnlineDirectory ? 
													boxnameWithOnlineDirectory :
													mailboxPattern);

	nsString2 command (GetServerCommandTag(), eOneByte);
	command += " list \"\" \"";
	command += escapedPattern;
	command += "\""CRLF;

//    PR_snprintf(GetOutputBuffer(),                              // string to create
//            kOutputBufferSize,                              // max size
//            "%s list \"\" \"%s\"" CRLF,                   // format string
//            GetServerCommandTag(),                  // command tag
//            escapedPattern);
            
    delete []escapedPattern;
	PR_FREEIF(boxnameWithOnlineDirectory);

	nsresult rv = SendData(command.GetBuffer());  
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}


void nsImapProtocol::Search(nsString2 &searchCriteria,
									  PRBool useUID, 
									  PRBool notifyHit /* TRUE */)
{
	m_notifySearchHit = notifyHit;
//    ProgressEventFunction_UsingId (MK_IMAP_STATUS_SEARCH_MAILBOX);
    IncrementCommandTagNumber();
    
    nsString2 protocolString(GetServerCommandTag(), eOneByte);
    // the searchCriteria string contains the 'search ....' string
    if (useUID)
        protocolString.Append(" uid");
	protocolString.Append(" ");
	protocolString.Append(searchCriteria);
	protocolString.Append(CRLF);;

    nsresult rv = SendData(protocolString.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::Copy(nsString2 &messageList,
                                    const char *destinationMailbox, 
                                    PRBool idsAreUid)
{
    IncrementCommandTagNumber();
    
    char *escapedDestination = CreateEscapedMailboxName(destinationMailbox);

    nsString2 protocolString(GetServerCommandTag(), eOneByte);
    if (idsAreUid)
		protocolString.Append(" uid");
	protocolString.Append(" copy ");
	protocolString.Append(messageList);
	protocolString.Append(" \"");
	protocolString.Append(escapedDestination);
	protocolString.Append("\"" CRLF);
	    
	nsresult rv = SendData(protocolString.GetBuffer());
	if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail(protocolString.GetBuffer());
        
    delete [] escapedDestination;
}

void nsImapProtocol::NthLevelChildList(const char* onlineMailboxPrefix,
                                       PRInt32 depth)
{
	NS_ASSERTION (depth >= 0, 
                  "Oops ... depth must be equal or greater than 0");
	if (depth < 0) return;

	nsString2 truncatedPrefix (onlineMailboxPrefix, eOneByte);
    PRUnichar slash = '/';
	if (truncatedPrefix.Last() == slash)
        truncatedPrefix.SetLength(truncatedPrefix.Length()-1);
		
    char *utf7ListArg = 
        CreateUtf7ConvertedString(truncatedPrefix.GetBuffer(),TRUE);
    if (utf7ListArg)
    {
        nsString2 pattern(utf7ListArg, eOneByte);
        nsString2 suffix("",eOneByte);
        int count = 0;
        char separator = 0;
        m_runningUrl->GetOnlineSubDirSeparator(&separator);
        suffix = separator;
        suffix += '%';
        
        while (count < depth)
        {
            pattern += suffix;
            count++;
            List(pattern.GetBuffer(), FALSE);
        }
        PR_Free(utf7ListArg);
    }
}

void nsImapProtocol::ProcessAuthenticatedStateURL()
{
	nsIImapUrl::nsImapAction imapAction;
	char * sourceMailbox = nsnull;
	m_runningUrl->GetImapAction(&imapAction);

	// switch off of the imap url action and take an appropriate action
	switch (imapAction)
	{
		case nsIImapUrl::nsImapLsubFolders:
			OnLSubFolders();
			return;
			break;
		case nsIImapUrl::nsImapGetMailAccountUrl:
			OnGetMailAccount();
			return;
			break;
		default: 
			break;
	}

	// all of the other states require the following extra step...
    // even though we don't have to to be legal protocol, Close any select mailbox
    // so we don't miss any info these urls might change for the selected folder
    // (e.g. msg count after append)
	if (GetServerStateParser().GetIMAPstate() == nsImapServerResponseParser::kFolderSelected)
	{
		// now we should be avoiding an implicit Close because it performs an implicit Expunge
		// authenticated state urls should not use a cached connection that is in the selected state
		// However, append message can happen either in a selected state or authenticated state
		if (imapAction != nsIImapUrl::nsImapAppendMsgFromFile /* kAppendMsgFromFile */)
		{
			PR_ASSERT(FALSE);
		}
	}

	switch (imapAction)
	{
		case nsIImapUrl::nsImapOfflineToOnlineMove:
			OnOfflineToOnlineMove();
			break;
		case nsIImapUrl::nsImapAppendMsgFromFile:
			OnAppendMsgFromFile();
			break;
		case nsIImapUrl::nsImapDiscoverAllBoxesUrl:
			NS_ASSERTION (!GetSubscribingNow(),
                      "Oops ... should not get here from subscribe UI");
			DiscoverMailboxList();
			break;
		case nsIImapUrl::nsImapDiscoverAllAndSubscribedBoxesUrl:
			NS_ASSERTION (GetSubscribingNow(), 
                          "Oops ... should not get here");
#if NOT_YET
			DiscoverAllAndSubscribedBoxes();
#endif
			break;
		case nsIImapUrl::nsImapCreateFolder:
			sourceMailbox = OnCreateServerSourceFolderPathString();
			OnCreateFolder(sourceMailbox);
			break;
		case nsIImapUrl::nsImapDiscoverChildrenUrl:
        {
            char *canonicalParent = nsnull;
            m_runningUrl->CreateServerSourceFolderPathString(&canonicalParent);
            if (canonicalParent)
            {
				NthLevelChildList(canonicalParent, 2);
                PR_Free(canonicalParent);
            }
			break;
        }
		case nsIImapUrl::nsImapDiscoverLevelChildrenUrl:
        {
            char *canonicalParent = nsnull;
            m_runningUrl->CreateServerSourceFolderPathString(&canonicalParent);
			PRInt32 depth = 0;
            m_runningUrl->GetChildDiscoveryDepth(&depth);
   			if (canonicalParent)
			{
				NthLevelChildList(canonicalParent, depth);
				if (GetServerStateParser().LastCommandSuccessful() &&
                    m_imapMailFolderSink)
					m_imapMailFolderSink->ChildDiscoverySucceeded(this);
				PR_Free(canonicalParent);
			}
			break;
        }
		case nsIImapUrl::nsImapSubscribe:
			sourceMailbox = OnCreateServerSourceFolderPathString();
			OnSubscribe(sourceMailbox); // used to be called subscribe
			break;
		case nsIImapUrl::nsImapUnsubscribe:	
			sourceMailbox = OnCreateServerSourceFolderPathString();			
			OnUnsubscribe(sourceMailbox);
			break;
		case nsIImapUrl::nsImapRefreshACL:
			sourceMailbox = OnCreateServerSourceFolderPathString();	
			OnRefreshACLForFolder(sourceMailbox);
			break;
		case nsIImapUrl::nsImapRefreshAllACLs:
			OnRefreshAllACLs();
			break;
		case nsIImapUrl::nsImapListFolder:
			sourceMailbox = OnCreateServerSourceFolderPathString();	
			OnListFolder(sourceMailbox, PR_FALSE);
			break;
		case nsIImapUrl::nsImapUpgradeToSubscription:
			OnUpgradeToSubscription();
			break;
		case nsIImapUrl::nsImapFolderStatus:
			sourceMailbox = OnCreateServerSourceFolderPathString();	
			OnStatusForFolder(sourceMailbox);
			break;
		case nsIImapUrl::nsImapRefreshFolderUrls:
			sourceMailbox = OnCreateServerSourceFolderPathString();
#ifdef UNREADY_CODE
			XMailboxInfo(sourceMailbox);
			if (GetServerStateParser().LastCommandSuccessful()) 
				InitializeFolderUrl(sourceMailbox);
#endif
			break;
		case nsIImapUrl::nsImapDeleteFolder:
			sourceMailbox = OnCreateServerSourceFolderPathString();
			OnDeleteFolder(sourceMailbox);
			break;
		case nsIImapUrl::nsImapRenameFolder:
			sourceMailbox = OnCreateServerSourceFolderPathString();
			OnRenameFolder(sourceMailbox);
			break;
		case nsIImapUrl::nsImapMoveFolderHierarchy:
			sourceMailbox = OnCreateServerSourceFolderPathString();
			OnMoveFolderHierarchy(sourceMailbox);
			break;
		default:
			break;
	}

	PR_FREEIF(sourceMailbox);
}

void nsImapProtocol::ProcessAfterAuthenticated()
{
	// mscott: ignore admin url stuff for now...
#ifdef UNREADY_CODE
	// if we're a netscape server, and we haven't got the admin url, get it
	if (!TIMAPHostInfo::GetHostHasAdminURL(fCurrentUrl->GetUrlHost()))
	{
		if (GetServerStateParser().GetCapabilityFlag() & kXServerInfoCapability)
		{
			XServerInfo();
			if (GetServerStateParser().LastCommandSuccessful()) 
			{
				TImapFEEvent *alertEvent = 
					new TImapFEEvent(msgSetMailServerURLs,  // function to call
									 this,                // access to current entry
									 (void *) fCurrentUrl->GetUrlHost(),
									 TRUE);            
				if (alertEvent)
				{
					fFEEventQueue->AdoptEventToEnd(alertEvent);
					// WaitForFEEventCompletion();
				}
				else
					HandleMemoryFailure();
			}
		}
		else if (GetServerStateParser().GetCapabilityFlag() & kHasXNetscapeCapability)
		{
			Netscape();
			if (GetServerStateParser().LastCommandSuccessful()) 
			{
				TImapFEEvent *alertEvent = 
					new TImapFEEvent(msgSetMailAccountURL,  // function to call
									 this,                // access to current entry
									 (void *) fCurrentUrl->GetUrlHost(),
									 TRUE);
				if (alertEvent)
				{
					fFEEventQueue->AdoptEventToEnd(alertEvent);
					// WaitForFEEventCompletion();
				}
				else
					HandleMemoryFailure();
			}
		}
	}
#endif

	if (GetServerStateParser().ServerHasNamespaceCapability())
	{
		PRBool nameSpacesOverridable = PR_FALSE;
		PRBool haveNameSpacesForHost = PR_FALSE;
		m_hostSessionList->GetNamespacesOverridableForHost(GetImapHostName(), GetImapUserName(), nameSpacesOverridable);
		m_hostSessionList->GetGotNamespacesForHost(GetImapHostName(), GetImapUserName(), haveNameSpacesForHost); 

	// mscott: VERIFY THIS CLAUSE!!!!!!!
		if (nameSpacesOverridable && !haveNameSpacesForHost)
		{
			Namespace();
		}
	}
}

void nsImapProtocol::SetupMessageFlagsString(nsString2& flagString,
                                             imapMessageFlagsType flags,
                                             PRUint16 userFlags)
{
    if (flags & kImapMsgSeenFlag)
        flagString.Append("\\Seen ");
    if (flags & kImapMsgAnsweredFlag)
        flagString.Append("\\Answered ");
    if (flags & kImapMsgFlaggedFlag)
        flagString.Append("\\Flagged ");
    if (flags & kImapMsgDeletedFlag)
        flagString.Append("\\Deleted ");
    if (flags & kImapMsgDraftFlag)
        flagString.Append("\\Draft ");
    if (flags & kImapMsgRecentFlag)
        flagString.Append("\\Recent ");
    if ((flags & kImapMsgForwardedFlag) && 
        (userFlags & kImapMsgSupportForwardedFlag))
        flagString.Append("$Forwarded ");	// Not always available
    if ((flags & kImapMsgMDNSentFlag) && (
        userFlags & kImapMsgSupportMDNSentFlag))
        flagString.Append("$MDNSent ");	// Not always available
    
    // eat the last space
    if (flagString.Length() > 0)
        flagString.SetLength(flagString.Length()-1);
}

void nsImapProtocol::ProcessStoreFlags(nsString2 &messageIdsString,
                                                 PRBool idsAreUids,
                                                 imapMessageFlagsType flags,
                                                 PRBool addFlags)
{
	if (!flags)
		return;
		
    nsString2 flagString("",eOneByte);

	uint16 userFlags = GetServerStateParser().SupportsUserFlags();
	uint16 settableFlags = GetServerStateParser().SettablePermanentFlags();

	if (!(flags & userFlags) && !(flags & settableFlags))
		return;					// if cannot set any of the flags bail out
    
    if (addFlags)
        flagString = "+Flags (";
    else
        flagString = "-Flags (";
    
    if (flags & kImapMsgSeenFlag && kImapMsgSeenFlag & settableFlags)
        flagString .Append("\\Seen ");
    if (flags & kImapMsgAnsweredFlag && kImapMsgAnsweredFlag & settableFlags)
        flagString .Append("\\Answered ");
    if (flags & kImapMsgFlaggedFlag && kImapMsgFlaggedFlag & settableFlags)
        flagString .Append("\\Flagged ");
    if (flags & kImapMsgDeletedFlag && kImapMsgDeletedFlag & settableFlags)
        flagString .Append("\\Deleted ");
    if (flags & kImapMsgDraftFlag && kImapMsgDraftFlag & settableFlags)
        flagString .Append("\\Draft ");
	if (flags & kImapMsgForwardedFlag && kImapMsgSupportForwardedFlag & userFlags)
        flagString .Append("$Forwarded ");	// if supported
	if (flags & kImapMsgMDNSentFlag && kImapMsgSupportMDNSentFlag & userFlags)
        flagString .Append("$MDNSent ");	// if supported

    // replace the final space with ')'
    flagString.SetCharAt(')',flagString.Length() - 1);;
    
    Store(messageIdsString, flagString.GetBuffer(), idsAreUids);
}


void nsImapProtocol::Close()
{
    IncrementCommandTagNumber();

	nsString2 command(GetServerCommandTag(), eOneByte);
	command.Append(" close" CRLF);

//    ProgressEventFunction_UsingId (MK_IMAP_STATUS_CLOSE_MAILBOX);

	GetServerStateParser().ResetFlagInfo(0);
    
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

void nsImapProtocol::Check()
{
    //ProgressUpdateEvent("Checking mailbox...");
    IncrementCommandTagNumber();
    
	nsString2 command(GetServerCommandTag(), eOneByte);
	command.Append(" check" CRLF);
            
    nsresult rv = SendData(command.GetBuffer());
    if (NS_SUCCEEDED(rv))
        ParseIMAPandCheckForNewMail();
}

PRBool nsImapProtocol::TryToLogon()
{

	// mscott - for right now, I'm assuming the user name and password are in the
	// incoming server...no prompt for password dialog yet...

	PRInt32 logonTries = 0;
	PRBool loginSucceeded = PR_FALSE;
	char * password = nsnull;
	char * userName = nsnull;

	// get the password and user name for the current incoming server...
	nsCOMPtr<nsIMsgIncomingServer>  server;
	nsresult rv = m_runningUrl->GetServer(getter_AddRefs(server));
	if (NS_SUCCEEDED(rv) && server)
	{
		rv = server->GetPassword(&password);
		rv = server->GetUsername(&userName);

	}

	do
	{
	    PRBool imapPasswordIsNew = PR_FALSE;

	    if (userName && password)
	    {
			PRBool prefBool = PR_TRUE;

			PRBool lastReportingErrors = GetServerStateParser().GetReportingErrors();
			GetServerStateParser().SetReportingErrors(FALSE);	// turn off errors - we'll put up our own.
#ifdef UNREADY_CODE
			PREF_GetBoolPref("mail.auth_login", &prefBool);
#endif
			if (prefBool) 
			{
				if (GetServerStateParser().GetCapabilityFlag() == kCapabilityUndefined)
					Capability();

				if (GetServerStateParser().GetCapabilityFlag() & kHasAuthPlainCapability)
				{
					AuthLogin (userName, password, kHasAuthPlainCapability);
					logonTries++;
				}
				else if (GetServerStateParser().GetCapabilityFlag() & kHasAuthLoginCapability)
				{
					AuthLogin (userName, password,  kHasAuthLoginCapability); 
					logonTries++;	// I think this counts against most
									// servers as a logon try 
				}
				else
					InsecureLogin(userName, password);
			}
			else
				InsecureLogin(userName, password);

			if (!GetServerStateParser().LastCommandSuccessful())
	        {
	            // login failed!
	            // if we failed because of an interrupt, then do not bother the user
	            if (!DeathSignalReceived())
	            {
#ifdef UNREADY_CODE
					AlertUserEvent_UsingId(XP_MSG_IMAP_LOGIN_FAILED);
#endif
					// if we did get a password then remember so we don't have to prompt
					// the user for it again
		            if (password != nsnull)
		            {
						m_hostSessionList->SetPasswordForHost(GetImapHostName(), 
															  GetImapUserName(), password);
						PR_FREEIF(password);
		            }
#ifdef UNREADY_CODE
		            fCurrentBiffState = MSG_BIFF_Unknown;
		            SendSetBiffIndicatorEvent(fCurrentBiffState);
#endif
				} // if we didn't receive the death signal...
			} // if login failed
	        else	// login succeeded
	        {
				rv = m_hostSessionList->SetPasswordForHost(GetImapHostName(), 
						  								   GetImapUserName(), password);
				rv = m_hostSessionList->GetPasswordVerifiedOnline(GetImapHostName(), GetImapUserName(), imapPasswordIsNew);
				if (NS_SUCCEEDED(rv) && imapPasswordIsNew)
					m_hostSessionList->SetPasswordVerifiedOnline(GetImapHostName(), GetImapUserName());
#ifdef UNREADY_CODE
				NET_SetPopPassword2(passwordForHost); // bug 53380
#endif
				if (imapPasswordIsNew) 
				{
	                if (m_currentBiffState == nsMsgBiffState_Unknown)
	                {
	                	m_currentBiffState = nsMsgBiffState_NoMail;
	                    SendSetBiffIndicatorEvent(m_currentBiffState);
	                }
#ifdef UNREADY_CODE
	                LIBNET_LOCK();
	                if (!DeathSignalReceived())
	                {
	                	setUserAuthenticatedIsSafe = GetActiveEntry()->URL_s->msg_pane != NULL;
						MWContext *context = GetActiveEntry()->window_id;
						if (context)
							FE_RememberPopPassword(context, SECNAV_MungeString(passwordForHost));
					}
					LIBNET_UNLOCK();
#endif
				}

				loginSucceeded = PR_TRUE;
			} // else if login succeeded
			
			GetServerStateParser().SetReportingErrors(lastReportingErrors);  // restore it

			if (loginSucceeded && imapPasswordIsNew)
			{
				// let's record the user as authenticated.
				m_imapExtensionSink->SetUserAuthenticated(this, PR_TRUE);
			}

			if (loginSucceeded)
			{
				ProcessAfterAuthenticated();
			}
	    }
	    else
	    {
			// The user hit "Cancel" on the dialog box
	        //AlertUserEvent("Login cancelled.");
			HandleCurrentUrlError();
#ifdef UNREADY_CODE
			SetCurrentEntryStatus(-1);
	        SetConnectionStatus(-1);        // stop netlib
#endif
			break;
	    }
	}

	while (!loginSucceeded && ++logonTries < 4);

	PR_FREEIF(password);

	if (!loginSucceeded)
	{
#ifdef UNREADY_CODE
		fCurrentBiffState = MSG_BIFF_Unknown;
		SendSetBiffIndicatorEvent(fCurrentBiffState);
#endif
		HandleCurrentUrlError();
		SetConnectionStatus(-1);        // stop netlib
	}

	return loginSucceeded;
}

PRBool
nsImapProtocol::GetDeleteIsMoveToTrash()
{
    PRBool rv = PR_FALSE;
    const char *userName = GetImapUserName();
    NS_ASSERTION (m_hostSessionList, "fatal... null host session list");
    if (m_hostSessionList)
        m_hostSessionList->GetDeleteIsMoveToTrashForHost(GetImapHostName(),
                                                         userName, rv);
    return rv;
}

nsIMAPMailboxInfo::nsIMAPMailboxInfo(const char *name, char delimiter) :
    m_mailboxName("",eOneByte,0)
{
	m_mailboxName = name;
	m_delimiter = delimiter;
	m_childrenListed = FALSE;
}

nsIMAPMailboxInfo::~nsIMAPMailboxInfo()
{
}
