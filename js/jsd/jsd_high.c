/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

/*
 * JavaScript Debugging support - 'High Level' functions
 */

#include "jsd.h"

/***************************************************************************/

/* XXX not 'static' because of old Mac CodeWarrior bug */ 
JSCList _jsd_context_list = JS_INIT_STATIC_CLIST(&_jsd_context_list);

/* these are used to connect JSD_SetUserCallbacks() with JSD_DebuggerOn() */
static JSD_UserCallbacks _callbacks;
static void*             _user = NULL; 
static JSRuntime*        _jsrt = NULL;

#ifdef JSD_HAS_DANGEROUS_THREAD
static void* _dangerousThread = NULL;
#endif

#ifdef JSD_THREADSAFE
void* _jsd_global_lock = NULL;
#endif

#ifdef DEBUG
void JSD_ASSERT_VALID_CONTEXT(JSDContext* jsdc)
{
    JS_ASSERT(jsdc->inited);
    JS_ASSERT(jsdc->jsrt);
    JS_ASSERT(jsdc->dumbContext);
    JS_ASSERT(jsdc->glob);
}
#endif

static JSClass global_class = {
    "JSDGlobal", 0,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub
};

static JSBool
_validateUserCallbacks(JSD_UserCallbacks* callbacks)
{
    return !callbacks ||
           (callbacks->size && callbacks->size <= sizeof(JSD_UserCallbacks));
}    

static JSDContext*
_newJSDContext(JSRuntime*         jsrt, 
               JSD_UserCallbacks* callbacks, 
               void*              user)
{
    JSDContext* jsdc = NULL;

    if( ! jsrt )
        return NULL;

    if( ! _validateUserCallbacks(callbacks) )
        return NULL;

    jsdc = (JSDContext*) calloc(1, sizeof(JSDContext));
    if( ! jsdc )
        goto label_newJSDContext_failure;

    if( ! JSD_INIT_LOCKS(jsdc) )
        goto label_newJSDContext_failure;

    JS_INIT_CLIST(&jsdc->links);

    jsdc->jsrt = jsrt;

    if( callbacks )
        memcpy(&jsdc->userCallbacks, callbacks, callbacks->size);
    
    jsdc->user = user;

#ifdef JSD_HAS_DANGEROUS_THREAD
    jsdc->dangerousThread = _dangerousThread;
#endif

    JS_INIT_CLIST(&jsdc->threadsStates);
    JS_INIT_CLIST(&jsdc->scripts);
    JS_INIT_CLIST(&jsdc->sources);
    JS_INIT_CLIST(&jsdc->removedSources);

    jsdc->sourceAlterCount = 1;

    if( ! jsd_CreateAtomTable(jsdc) )
        goto label_newJSDContext_failure;

    if( ! jsd_InitObjectManager(jsdc) )
        goto label_newJSDContext_failure;

    jsdc->dumbContext = JS_NewContext(jsdc->jsrt, 256);
    if( ! jsdc->dumbContext )
        goto label_newJSDContext_failure;

    jsdc->glob = JS_NewObject(jsdc->dumbContext, &global_class, NULL, NULL);
    if( ! jsdc->glob )
        goto label_newJSDContext_failure;

    if( ! JS_InitStandardClasses(jsdc->dumbContext, jsdc->glob) )
        goto label_newJSDContext_failure;

    jsdc->inited = JS_TRUE;

    JSD_LOCK();
    JS_INSERT_LINK(&jsdc->links, &_jsd_context_list);
    JSD_UNLOCK();

    return jsdc;

label_newJSDContext_failure:
    jsd_DestroyObjectManager(jsdc);
    jsd_DestroyAtomTable(jsdc);
    if( jsdc )
        free(jsdc);
    return NULL;
}

static void
_destroyJSDContext(JSDContext* jsdc)
{
    JSD_ASSERT_VALID_CONTEXT(jsdc);

    JSD_LOCK();
    JS_REMOVE_LINK(&jsdc->links);
    JSD_UNLOCK();

    jsd_DestroyObjectManager(jsdc);
    jsd_DestroyAtomTable(jsdc);

    jsdc->inited = JS_FALSE;

    /*
    * We should free jsdc here, but we let it leak in case there are any 
    * asynchronous hooks calling into the system using it as a handle
    *
    * XXX we also leak the locks
    */

}

/***************************************************************************/

JSDContext*
jsd_DebuggerOnForUser(JSRuntime*         jsrt, 
                      JSD_UserCallbacks* callbacks, 
                      void*              user)
{
    JSDContext* jsdc;
    JSContext* iter = NULL;

    jsdc = _newJSDContext(jsrt, callbacks, user);
    if( ! jsdc )
        return NULL;

    /* set hooks here */
    JS_SetNewScriptHookProc(jsdc->jsrt, jsd_NewScriptHookProc, jsdc);
    JS_SetDestroyScriptHookProc(jsdc->jsrt, jsd_DestroyScriptHookProc, jsdc);
    JS_SetDebuggerHandler(jsdc->jsrt, jsd_DebuggerHandler, jsdc);
    JS_SetExecuteHook(jsdc->jsrt, jsd_InterpreterHook, jsdc);
    JS_SetCallHook(jsdc->jsrt, jsd_InterpreterHook, jsdc);
    JS_SetObjectHook(jsdc->jsrt, jsd_ObjectHook, jsdc);
    JS_SetThrowHook(jsdc->jsrt, jsd_ThrowHandler, jsdc);
    JS_SetDebugErrorHook(jsdc->jsrt, jsd_DebugErrorHook, jsdc);
#ifdef LIVEWIRE
    LWDBG_SetNewScriptHookProc(jsd_NewScriptHookProc, jsdc);
#endif
    if( jsdc->userCallbacks.setContext )
        jsdc->userCallbacks.setContext(jsdc, jsdc->user);
    return jsdc;
}

JSDContext*
jsd_DebuggerOn(void)
{
    JS_ASSERT(_jsrt);
    JS_ASSERT(_validateUserCallbacks(&_callbacks));
    return jsd_DebuggerOnForUser(_jsrt, &_callbacks, _user);
}

void
jsd_DebuggerOff(JSDContext* jsdc)
{
    /* clear hooks here */
    JS_SetNewScriptHookProc(jsdc->jsrt, NULL, NULL);
    JS_SetDestroyScriptHookProc(jsdc->jsrt, NULL, NULL);
    JS_SetDebuggerHandler(jsdc->jsrt, NULL, NULL);
    JS_SetExecuteHook(jsdc->jsrt, NULL, NULL);
    JS_SetCallHook(jsdc->jsrt, NULL, NULL);
    JS_SetObjectHook(jsdc->jsrt, NULL, NULL);
    JS_SetThrowHook(jsdc->jsrt, NULL, NULL);
    JS_SetDebugErrorHook(jsdc->jsrt, NULL, NULL);
#ifdef LIVEWIRE
    LWDBG_SetNewScriptHookProc(NULL,NULL);
#endif

    /* clean up */
    JSD_LockScriptSubsystem(jsdc);
    jsd_DestroyAllJSDScripts(jsdc);
    JSD_UnlockScriptSubsystem(jsdc);
    jsd_DestroyAllSources(jsdc);
    
    _destroyJSDContext(jsdc);

    if( jsdc->userCallbacks.setContext )
        jsdc->userCallbacks.setContext(NULL, jsdc->user);
}

void
jsd_SetUserCallbacks(JSRuntime* jsrt, JSD_UserCallbacks* callbacks, void* user)
{
    _jsrt = jsrt;
    _user = user;

#ifdef JSD_HAS_DANGEROUS_THREAD
    _dangerousThread = JSD_CURRENT_THREAD();
#endif

    if( callbacks )
        memcpy(&_callbacks, callbacks, sizeof(JSD_UserCallbacks));
    else
        memset(&_callbacks, 0 , sizeof(JSD_UserCallbacks));
}

JSDContext*
jsd_JSDContextForJSContext(JSContext* context)
{
    JSDContext* iter;
    JSDContext* jsdc = NULL;
    JSRuntime*  runtime = JS_GetRuntime(context);

    JSD_LOCK();
    for( iter = (JSDContext*)_jsd_context_list.next;
         iter != (JSDContext*)&_jsd_context_list;
         iter = (JSDContext*)iter->links.next )
    {
        if( runtime == iter->jsrt )
        {
            jsdc = iter;
            break;
        }
    }
    JSD_UNLOCK();
    return jsdc;
}    

JS_STATIC_DLL_CALLBACK(JSBool)
jsd_DebugErrorHook(JSContext *cx, const char *message,
                   JSErrorReport *report, void *closure)
{
    JSDContext* jsdc = (JSDContext*) closure;
    JSD_ErrorReporter errorReporter;
    void*             errorReporterData;
    
    if( ! jsdc )
    {
        JS_ASSERT(0);
        return JS_TRUE;
    }
    if( JSD_IS_DANGEROUS_THREAD(jsdc) )
        return JS_TRUE;

    /* local in case hook gets cleared on another thread */
    JSD_LOCK();
    errorReporter     = jsdc->errorReporter;
    errorReporterData = jsdc->errorReporterData;
    JSD_UNLOCK();

    if(!errorReporter)
        return JS_TRUE;

    switch(errorReporter(jsdc, cx, message, report, errorReporterData))
    {
        case JSD_ERROR_REPORTER_PASS_ALONG:
            return JS_TRUE;
        case JSD_ERROR_REPORTER_RETURN:
            return JS_FALSE;
        case JSD_ERROR_REPORTER_DEBUG:
        {
            jsval rval;
            JSD_ExecutionHookProc   hook;
            void*                   hookData;

            /* local in case hook gets cleared on another thread */
            JSD_LOCK();
            hook = jsdc->debugBreakHook;
            hookData = jsdc->debugBreakHookData;
            JSD_UNLOCK();

            jsd_CallExecutionHook(jsdc, cx, JSD_HOOK_DEBUG_REQUESTED,
                                  hook, hookData, &rval);
            /* XXX Should make this dependent on ExecutionHook retval */
            return JS_TRUE;
        }
        case JSD_ERROR_REPORTER_CLEAR_RETURN:
            if(report && JSREPORT_IS_EXCEPTION(report->flags))
                JS_ClearPendingException(cx);
            return JS_FALSE;
        default:
            JS_ASSERT(0);
            break;
    }
    return JS_TRUE;
}

JSBool
jsd_SetErrorReporter(JSDContext*       jsdc, 
                     JSD_ErrorReporter reporter, 
                     void*             callerdata)
{
    JSD_LOCK();
    jsdc->errorReporter = reporter;
    jsdc->errorReporterData = callerdata;
    JSD_UNLOCK();
    return JS_TRUE;
}

JSBool
jsd_GetErrorReporter(JSDContext*        jsdc, 
                     JSD_ErrorReporter* reporter, 
                     void**             callerdata)
{
    JSD_LOCK();
    if( reporter )
        *reporter = jsdc->errorReporter;
    if( callerdata )
        *callerdata = jsdc->errorReporterData;
    JSD_UNLOCK();
    return JS_TRUE;
}
