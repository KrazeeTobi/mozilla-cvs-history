/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef _xp_observer_h_
#define _xp_observer_h_

#if 0
#include "platform.h"
#else
#ifndef _platform_h_
typedef int NS_Error;
#endif
#endif

#include "prtypes.h"
#include "prlog.h"

typedef struct  OpaqueObserverList  *XP_ObserverList;
typedef void* 	XP_Observable;
typedef long 	XP_ObservableMsg;


typedef void    (*XP_ObserverProc) (
				XP_Observable           	inSource,
                            	XP_ObservableMsg  	inMsg, 
                                   	void*                 		ioMsgData, 
                                   	void*                   		ioClosure		);



PR_BEGIN_EXTERN_C

/* 
        Creates a new XP_ObserverList, with an assoicated XP_Observabe, 
        to which you can add observers, who are notified when 
        XP_NotifyObservers is called.
        
        Observerer notification is enabled by default.
*/
extern NS_Error
XP_NewObserverList(       
	XP_Observable		inObservable,
	XP_ObserverList*	outObserverList );
        

/* 
        Disposes of an XP_ObserverList. Does nothing with
        its observers.
*/
extern void
XP_DisposeObserverList(
        XP_ObserverList inObserverList );
       
       
/*
	Returns the XP_Observable associated with an XP_ObserverList
*/        
extern XP_Observable
XP_GetObserverListObservable(
	XP_ObserverList inObserverList);
	
	
/*
	Change the observable associated with an observerlist
*/	       
extern void
XP_SetObserverListObservable(
	XP_ObserverList	inObserverList,
	XP_Observable	inObservable);
	


/*
        Registers a function pointer and void* closure
        as an "observer", which will be called whenever
        XP_NotifyObservers is called an observer notification
        is enabled.
*/
extern int
XP_AddObserver(
        XP_ObserverList  	inObserverList,
        XP_ObserverProc 	inObserver,
        void*                   		inClosure       );
        
/*
        Removes a registered observer. If there are duplicate
        (XP_ObserverProc/void* closure) pairs registered,
        it is undefined which one will be removed.

        Returns false if the observer is not registered.
*/
extern PRBool
XP_RemoveObserver(      
        XP_ObserverList 		inObserverList,
        XP_ObserverProc 	inObserver,
        void*                   		inClosure       );
        

/*
        If observer notification is enabled for this XP_Observable,
        this will call each registered observer proc, passing it
        the given message and void* ioData, in addition to the 
        observer's closure void*.
        
        There is no defined order in which observers are called.
*/
extern void
XP_NotifyObservers(
        XP_ObserverList         	inObserverList,
        XP_ObservableMsg       inMessage,
        void*                           	ioData  );
        

/*
        When called, subsequent calls to XP_NotifyObservers
        will do nothing until XP_EnableObserverNotification
        is called.
*/
extern void
XP_DisableObserverNotification(
        XP_ObserverList   inObserverList    );
        
        
/*
        Enables calling observers when XP_NotifyObservers
        is invoked. 
*/
extern void
XP_EnableObserverNotification(
        XP_ObserverList   inObserverList    );
        

/*
        Returns true if observer notification is enabled.
*/
extern PRBool
XP_IsObserverNotificationEnabled(
        XP_ObserverList   inObserverList    );              
        
        
PR_END_EXTERN_C

        
#endif /* #ifdef _xp_observer_h_ */
