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
 * Copyright (C) 1998, 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
#ifndef nsImapMailFolder_h__
#define nsImapMailFolder_h__

#include "nsImapCore.h"
#include "nsMsgDBFolder.h"
#include "nsIMessage.h"
#include "nsIImapMailFolderSink.h"
#include "nsIImapMessageSink.h"
#include "nsIImapExtensionSink.h"
#include "nsIImapMiscellaneousSink.h"
#include "nsICopyMessageListener.h"
#include "nsIImapService.h"
#include "nsIUrlListener.h"
#include "nsIImapIncomingServer.h" // we need this for its IID
#include "nsIMsgParseMailMsgState.h"
#include "nsITransactionManager.h"
#include "nsMsgTxn.h"
#include "nsIMsgMessageService.h"
#include "nsIMsgFilterHitNotify.h"
#include "nsIMsgFilterList.h"
#include "prmon.h"
#include "nsIEventQueue.h"
#include "nsIMsgImapMailFolder.h"

class nsImapMoveCoalescer;


#define FOUR_K 4096

/* b64534f0-3d53-11d3-ac2a-00805f8ac968 */

#define NS_IMAPMAILCOPYSTATE_IID \
{ 0xb64534f0, 0x3d53, 0x11d3, \
    { 0xac, 0x2a, 0x00, 0x80, 0x5f, 0x8a, 0xc9, 0x68 } }

class nsImapMailCopyState: public nsISupports
{
public:
    static const nsIID& GetIID()
    {
        static nsIID iid = NS_IMAPMAILCOPYSTATE_IID;
        return iid;
    }
    
    NS_DECL_ISUPPORTS

    nsImapMailCopyState();
    virtual ~nsImapMailCopyState();

    nsCOMPtr<nsISupports> m_srcSupport; // source file spec or folder
    nsCOMPtr<nsISupportsArray> m_messages; // array of source messages
    nsCOMPtr<nsMsgTxn> m_undoMsgTxn; // undo object with this copy operation
    nsCOMPtr<nsIMessage> m_message; // current message to be copied
    nsCOMPtr<nsIMsgCopyServiceListener> m_listener; // listener of this copy
                                                    // operation 
    nsCOMPtr<nsIFileSpec> m_tmpFileSpec; // temp file spec for copy operation

    nsIMsgMessageService* m_msgService; // source folder message service; can
                                        // be Nntp, Mailbox, or Imap
    PRBool m_isMove;             // is a move
    PRBool m_selectedState;      // needs to be in selected state; append msg
    PRUint32 m_curIndex; // message index to the message array which we are
                         // copying 
    PRUint32 m_totalCount;// total count of messages we have to do
    PRBool m_streamCopy;
    char *m_dataBuffer; // temporary buffer for this copy operation
    PRUint32 m_leftOver;
};

class nsImapMailFolder : public nsMsgDBFolder, 
                         public nsIMsgImapMailFolder,
                         public nsIImapMailFolderSink,
                         public nsIImapMessageSink,
                         public nsIImapExtensionSink,
                         public nsIImapMiscellaneousSink,
                         public nsICopyMessageListener,
						 public nsIMsgFilterHitNotify
{
public:
	nsImapMailFolder();
	virtual ~nsImapMailFolder();

	NS_DECL_ISUPPORTS_INHERITED

    // nsICollection methods
	NS_IMETHOD Enumerate(nsIEnumerator **result);

    // nsIFolder methods:
    NS_IMETHOD GetSubFolders(nsIEnumerator* *result);
    
    // nsIMsgFolder methods:
    NS_IMETHOD AddUnique(nsISupports* element);
    NS_IMETHOD ReplaceElement(nsISupports* element, nsISupports* newElement);
    NS_IMETHOD GetMessages(nsISimpleEnumerator* *result);
	NS_IMETHOD UpdateFolder(nsIMsgWindow *aWindow);
    
	NS_IMETHOD CreateSubfolder(const char *folderName);
	NS_IMETHOD AddSubfolder(nsAutoString *name, nsIMsgFolder **child);
    
    NS_IMETHOD Compact();
    NS_IMETHOD EmptyTrash();
	NS_IMETHOD Delete ();
	NS_IMETHOD Rename (const char *newName);
	NS_IMETHOD Adopt(nsIMsgFolder *srcFolder, PRUint32 *outPos);

	NS_IMETHOD GetPrettyName(PRUnichar ** prettyName);	// Override of the base, for top-level mail folder
    
    NS_IMETHOD GetFolderURL(char **url);
    
	NS_IMETHOD UpdateSummaryTotals(PRBool force) ;
    
	NS_IMETHOD GetExpungedBytesCount(PRUint32 *count);
	NS_IMETHOD GetDeletable (PRBool *deletable); 
	NS_IMETHOD GetRequiresCleanup(PRBool *requiresCleanup);
    
	NS_IMETHOD GetSizeOnDisk(PRUint32 * size);
        
        NS_IMETHOD GetCanCreateSubfolders(PRBool *aResult);

	NS_IMETHOD UserNeedsToAuthenticateForFolder(PRBool displayOnly, PRBool *authenticate);
	NS_IMETHOD RememberPassword(const char *password);
	NS_IMETHOD GetRememberedPassword(char ** password);

	NS_IMETHOD MarkMessagesRead(nsISupportsArray *messages, PRBool markRead);
	NS_IMETHOD MarkAllMessagesRead(void);
	NS_IMETHOD MarkMessagesFlagged(nsISupportsArray *messages, PRBool markFlagged);

    NS_IMETHOD DeleteSubFolders(nsISupportsArray *folders);
	NS_IMETHOD ReadFromFolderCacheElem(nsIMsgFolderCacheElement *element);
	NS_IMETHOD WriteToFolderCacheElem(nsIMsgFolderCacheElement *element);
    
    virtual nsresult GetDBFolderInfoAndDB(nsIDBFolderInfo **folderInfo,
                                          nsIMsgDatabase **db);
 	NS_IMETHOD DeleteMessages(nsISupportsArray *messages,
                              nsIMsgWindow *msgWindow, PRBool
                              deleteStorage);
    NS_IMETHOD CopyMessages(nsIMsgFolder *srcFolder, 
                            nsISupportsArray* messages,
                            PRBool isMove, nsIMsgWindow *msgWindow,
                            nsIMsgCopyServiceListener* listener);
    NS_IMETHOD CopyFileMessage(nsIFileSpec* fileSpec, 
                               nsIMessage* msgToReplace,
                               PRBool isDraftOrTemplate,
                               nsIMsgWindow *msgWindow,
                               nsIMsgCopyServiceListener* listener);
	NS_IMETHOD CreateMessageFromMsgDBHdr(nsIMsgDBHdr *msgHdr, nsIMessage
                                         **message);
    NS_IMETHOD GetNewMessages(nsIMsgWindow *aWindow);

    NS_IMETHOD GetPath(nsIFileSpec** aPathName);
	NS_IMETHOD SetPath(nsIFileSpec * aPath);

    // nsIMsgImapMailFolder methods
	NS_DECL_NSIMSGIMAPMAILFOLDER

    // nsIImapMailFolderSink methods
	NS_DECL_NSIIMAPMAILFOLDERSINK

    // nsIImapMessageSink methods
	NS_DECL_NSIIMAPMESSAGESINK

    //nsICopyMessageListener
	NS_DECL_NSICOPYMESSAGELISTENER

    // nsIUrlListener methods
	NS_IMETHOD OnStartRunningUrl(nsIURI * aUrl);
	NS_IMETHOD OnStopRunningUrl(nsIURI * aUrl, nsresult aExitCode);

    // nsIImapExtensionSink methods
    NS_IMETHOD ClearFolderRights(nsIImapProtocol* aProtocol,
                                 nsIMAPACLRightsInfo* aclRights);
    NS_IMETHOD AddFolderRights(nsIImapProtocol* aProtocol,
                               nsIMAPACLRightsInfo* aclRights);
    NS_IMETHOD RefreshFolderRights(nsIImapProtocol* aProtocol,
                                   nsIMAPACLRightsInfo* aclRights);
    NS_IMETHOD FolderNeedsACLInitialized(nsIImapProtocol* aProtocol,
                                         nsIMAPACLRightsInfo* aclRights);
    NS_IMETHOD SetCopyResponseUid(nsIImapProtocol* aProtocol,
                                  nsMsgKeyArray* keyArray,
                                  const char* msgIdString,
                                  nsISupports* copyState);
    NS_IMETHOD SetAppendMsgUid(nsIImapProtocol* aProtocol,
                               nsMsgKey aKey,
                               nsISupports* copyState);
    NS_IMETHOD GetMessageId(nsIImapProtocol* aProtocol,
                            nsCString* messageId,
                            nsISupports* copyState);
    
    // nsIImapMiscellaneousSink methods
	NS_IMETHOD AddSearchResult(nsIImapProtocol* aProtocol, 
                               const char* searchHitLine);
	NS_IMETHOD GetArbitraryHeaders(nsIImapProtocol* aProtocol,
                                   GenericInfo* aInfo);
	NS_IMETHOD GetShouldDownloadArbitraryHeaders(nsIImapProtocol* aProtocol,
                                                 GenericInfo* aInfo);
    NS_IMETHOD GetShowAttachmentsInline(nsIImapProtocol* aProtocol,
                                        PRBool* aBool);
	NS_IMETHOD HeaderFetchCompleted(nsIImapProtocol* aProtocol);
	NS_IMETHOD UpdateSecurityStatus(nsIImapProtocol* aProtocol);
	// ****
	NS_IMETHOD SetBiffStateAndUpdate(nsIImapProtocol* aProtocol,
                                     nsMsgBiffState biffState);
	NS_IMETHOD GetStoredUIDValidity(nsIImapProtocol* aProtocol,
                                    uid_validity_info* aInfo);
	NS_IMETHOD LiteSelectUIDValidity(nsIImapProtocol* aProtocol,
                                     PRUint32 uidValidity);
	NS_IMETHOD ProgressStatus(nsIImapProtocol* aProtocol,
                              PRUint32 aMsgId, const char *extraInfo);
	NS_IMETHOD PercentProgress(nsIImapProtocol* aProtocol,
                               ProgressInfo* aInfo);
	NS_IMETHOD TunnelOutStream(nsIImapProtocol* aProtocol,
                               msg_line_info* aInfo);
	NS_IMETHOD ProcessTunnel(nsIImapProtocol* aProtocol,
                             TunnelInfo *aInfo);
    NS_IMETHOD CopyNextStreamMessage(nsIImapProtocol* aProtocol,
                                     nsISupports* copyState);
    NS_IMETHOD SetUrlState(nsIImapProtocol* aProtocol,
                           nsIMsgMailNewsUrl* aUrl,
                           PRBool isRunning,
                           nsresult statusCode);

	NS_IMETHOD MatchName(nsString *name, PRBool *matches);
	// nsIMsgFilterHitNotification method(s)
	NS_IMETHOD ApplyFilterHit(nsIMsgFilter *filter, PRBool *applyMore);

    // overriding nsMsgDBFolder::GetMsgDatabase()
    NS_IMETHOD GetMsgDatabase(nsIMsgDatabase** aMsgDatabase);

	nsresult GetMessageHeader(nsMsgKey key, nsIMsgDBHdr ** aMsgHdr);

	nsresult MoveIncorporatedMessage(nsIMsgDBHdr *mailHdr, 
									   nsIMsgDatabase *sourceDB, 
									   char *destFolder,
									   nsIMsgFilter *filter);
	nsresult StoreImapFlags(imapMessageFlagsType flags, PRBool addFlags, nsMsgKeyArray &msgKeys);
	static nsresult AllocateUidStringFromKeyArray(nsMsgKeyArray &keyArray, nsCString &msgIds);
protected:
    // Helper methods
	void FindKeysToAdd(const nsMsgKeyArray &existingKeys, nsMsgKeyArray
                       &keysToFetch, nsIImapFlagAndUidState *flagState);
	void FindKeysToDelete(const nsMsgKeyArray &existingKeys, nsMsgKeyArray
                          &keysToFetch, nsIImapFlagAndUidState *flagState);
	void PrepareToAddHeadersToMailDB(nsIImapProtocol* aProtocol, const
                                     nsMsgKeyArray &keysToFetch, 
                                     nsIMailboxSpec *boxSpec);
	void TweakHeaderFlags(nsIImapProtocol* aProtocol, nsIMsgDBHdr *tweakMe);

	nsresult SyncFlags(nsIImapFlagAndUidState *flagState);

	void UpdatePendingCounts(PRBool countUnread, PRBool missingAreRead);
	void SetIMAPDeletedFlag(nsIMsgDatabase *mailDB, const nsMsgKeyArray &msgids, PRBool markDeleted);
	virtual PRBool ShowDeletedMessages();
	virtual PRBool DeleteIsMoveToTrash();
	void ParseUidString(char *uidString, nsMsgKeyArray &keys);
    nsresult GetFolder(const char *name, nsIMsgFolder **pFolder);
	nsresult GetTrashFolder(nsIMsgFolder **pTrashFolder);
    PRBool InTrash(nsIMsgFolder* folder);
	nsresult GetServerKey(char **serverKey);

    nsresult AddDirectorySeparator(nsFileSpec &path);
    nsresult CreateDirectoryForFolder(nsFileSpec &path);
	nsresult CreateSubFolders(nsFileSpec &path);
	nsresult GetDatabase();
	virtual const char *GetIncomingServerType() {return "imap";}

    // Uber message copy service
    nsresult CopyMessagesWithStream(nsIMsgFolder* srcFolder,
                           nsISupportsArray* messages,
                           PRBool isMove,
                           nsIMsgWindow *msgWindow,
                           nsIMsgCopyServiceListener* listener);
    nsresult CopyStreamMessage(nsIMessage* message, nsIMsgFolder* dstFolder,
                               PRBool isMove);
    nsresult InitCopyState(nsISupports* srcSupport, 
                           nsISupportsArray* messages,
                           PRBool isMove,
                           PRBool selectedState,
                           nsIMsgCopyServiceListener* listener);
    void ClearCopyState(nsresult exitCode);
    nsresult SetTransactionManager(nsITransactionManager* txnMgr);
    nsresult BuildIdsAndKeyArray(nsISupportsArray* messages,
                                 nsCString& msgIds, nsMsgKeyArray& keyArray);

    PRBool m_initialized;
    PRBool m_haveDiscoveredAllFolders;
    PRBool m_haveReadNameFromDB;
	nsCOMPtr<nsIMsgParseMailMsgState> m_msgParser;
	nsCOMPtr<nsIMsgFilterList> m_filterList;
	PRBool				m_msgMovedByFilter;
	nsImapMoveCoalescer *m_moveCoalescer;
	nsMsgKey			m_curMsgUid;
	PRInt32			m_nextMessageByteLength;
    nsCOMPtr<nsIEventQueue> m_eventQueue;
    PRBool m_urlRunning;

	// part of temporary libmime converstion trick......these should go away once MIME uses a new stream
	// converter interface...
	nsCOMPtr<nsIOutputStream> m_tempMessageStream;

    // *** jt - undo move/copy trasaction support
    nsCOMPtr<nsITransactionManager> m_transactionManager;
    nsCOMPtr<nsMsgTxn> m_pendingUndoTxn;
    nsCOMPtr<nsImapMailCopyState> m_copyState;
    PRMonitor *m_appendMsgMonitor;
    PRBool	m_verifiedAsOnlineFolder;
	PRBool	m_explicitlyVerify; // whether or not we need to explicitly verify this through LIST
	PRUnichar m_hierarchyDelimiter;
	PRInt32 m_boxFlags;
	nsCString m_onlineFolderName;
	nsFileSpec *m_pathName;
};

#endif
