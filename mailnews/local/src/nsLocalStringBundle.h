/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL. You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 * 
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All Rights
 * Reserved.
 */ 

#ifndef _nsLocalStringBundle_H__
#define _nsLocalStringBundle_H__

NS_BEGIN_EXTERN_C

PRUnichar     *LocalGetStringByID(PRInt32 stringID);

NS_END_EXTERN_C



#define	IMAP_OUT_OF_MEMORY                                 -1000
#define	LOCAL_STATUS_SELECTING_MAILBOX                      4000
#define	LOCAL_STATUS_DOCUMENT_DONE							4001
#define LOCAL_STATUS_RECEIVING_MESSAGE_OF					4002
#define POP3_SERVER_ERROR									4003
#define POP3_USERNAME_FAILURE								4004
#define POP3_PASSWORD_FAILURE								4005
#define POP3_MESSAGE_WRITE_ERROR							4006
#define POP3_CONNECT_HOST_CONTACTED_SENDING_LOGIN_INFORMATION 4007
#define POP3_NO_MESSAGES									4008
#define POP3_DOWNLOAD_COUNT									4009
#define POP3_SERVER_DOES_NOT_SUPPORT_UIDL_ETC				4010
#define POP3_SERVER_DOES_NOT_SUPPORT_THE_TOP_COMMAND		4011
#define POP3_RETR_FAILURE									4012
#define POP3_PASSWORD_UNDEFINED								4013
#define POP3_USERNAME_UNDEFINED								4014
#define POP3_LIST_FAILURE									4015
#define POP3_DELE_FAILURE									4016

#endif /* _nsImapStringBundle_H__ */
