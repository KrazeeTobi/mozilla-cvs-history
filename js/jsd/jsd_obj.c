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
 * JavaScript Debugging support - Object support
 */

#include "jsd.h"

/*
* #define JSD_TRACE 1
*/

#ifdef JSD_TRACE
#define TRACEOBJ(jsdc, jsdobj, which) _traceObj(jsdc, jsdobj, which)

static char *
_describeObj(JSDContext* jsdc, JSDObject *jsdobj)
{
    return
        JS_smprintf("%0x new'd in %s at line %d using ctor %s in %s at line %d",
                    (int)jsdobj,
                    JSD_GetObjectNewURL(jsdc, jsdobj),
                    JSD_GetObjectNewLineNumber(jsdc, jsdobj),
                    JSD_GetObjectConstructorName(jsdc, jsdobj),
                    JSD_GetObjectConstructorURL(jsdc, jsdobj),
                    JSD_GetObjectConstructorLineNumber(jsdc, jsdobj));
}

static void
_traceObj(JSDContext* jsdc, JSDObject* jsdobj, int which)
{
    char* description;

    if( !jsdobj )
        return;

    description = _describeObj(jsdc, jsdobj);

    printf("%s : %s\n",
           which == 0 ? "new  " :
           which == 1 ? "final" :
                        "ctor ",
           description);
    if(description)
        free(description);
}
#else
#define TRACEOBJ(jsdc, jsdobj, which) ((void)0)
#endif /* JSD_TRACE */

#ifdef DEBUG
void JSD_ASSERT_VALID_OBJECT(JSDObject* jsdobj)
{
    JS_ASSERT(jsdobj);
    JS_ASSERT(!JS_CLIST_IS_EMPTY(&jsdobj->links));
    JS_ASSERT(jsdobj->obj);
}
#endif


static void
_destroyJSDObject(JSDContext* jsdc, JSDObject* jsdobj)
{
    JS_ASSERT(JSD_OBJECTS_LOCKED(jsdc));

    JS_REMOVE_LINK(&jsdobj->links);
    JS_HashTableRemove(jsdc->objectsTable, jsdobj->obj);

    if(jsdobj->newURL)
        jsd_DropAtom(jsdc, jsdobj->newURL);
    if(jsdobj->ctorURL)
        jsd_DropAtom(jsdc, jsdobj->ctorURL);
    if(jsdobj->ctorName)
        jsd_DropAtom(jsdc, jsdobj->ctorName);
    free(jsdobj);
}

static JSDObject*
_createJSDObject(JSDContext* jsdc, JSContext *cx, JSObject *obj)
{
    JSDObject* jsdobj;
    JSStackFrame* fp;
    JSStackFrame* iter = NULL;
    const char* newURL;
    jsbytecode* pc;

    JS_ASSERT(JSD_OBJECTS_LOCKED(jsdc));

    jsdobj = (JSDObject*) calloc(1, sizeof(JSDObject));
    if( jsdobj )
    {
        JS_INIT_CLIST(&jsdobj->links);
        JS_APPEND_LINK(&jsdobj->links, &jsdc->objectsList);
        jsdobj->obj = obj;
        JS_HashTableAdd(jsdc->objectsTable, obj, jsdobj);

        /* walk the stack to find js frame (if any) causing creation */
        while( NULL != (fp = JS_FrameIterator(cx, &iter)) )
        {
            if( !JS_IsNativeFrame(cx, fp) )
            {
                JSScript* script = JS_GetFrameScript(cx, fp);
                if( !script )
                    continue;

                newURL = JS_GetScriptFilename(cx, script);
                if( newURL )
                    jsdobj->newURL = jsd_AddAtom(jsdc, newURL);

                pc = JS_GetFramePC(cx, fp);
                if( pc )
                    jsdobj->newLineno = JS_PCToLineNumber(cx, script, pc);

                break;
            }
        }
    }
    return jsdobj;
}

void JS_DLL_CALLBACK
jsd_ObjectHook(JSContext *cx, JSObject *obj, JSBool isNew, void *closure)
{
    JSDObject* jsdobj;
    JSDContext* jsdc = (JSDContext*) closure;

    if( ! jsdc || ! jsdc->inited )
        return;

    JSD_LOCK_OBJECTS(jsdc);
    if(isNew)
    {
        jsdobj = _createJSDObject(jsdc, cx, obj);
        TRACEOBJ(jsdc, jsdobj, 0);
    }
    else
    {
        jsdobj = jsd_GetJSDObjectForJSObject(jsdc, obj);
        if( jsdobj )
        {
            TRACEOBJ(jsdc, jsdobj, 1);
            _destroyJSDObject(jsdc, jsdobj);
        }
    }
    JSD_UNLOCK_OBJECTS(jsdc);
}

void
jsd_Constructing(JSDContext* jsdc, JSContext *cx, JSObject *obj,
                 JSStackFrame *fp)
{
    JSDObject* jsdobj;
    JSScript* script;
    JSDScript* jsdscript;
    const char* ctorURL;
    const char* ctorName;

    JSD_LOCK_OBJECTS(jsdc);
    jsdobj = jsd_GetJSDObjectForJSObject(jsdc, obj);
    if( jsdobj && !jsdobj->ctorURL && !JS_IsNativeFrame(cx, fp) )
    {
        script = JS_GetFrameScript(cx, fp);
        if( script )
        {
            ctorURL = JS_GetScriptFilename(cx, script);
            if( ctorURL )
                jsdobj->ctorURL = jsd_AddAtom(jsdc, ctorURL);

            JSD_LOCK_SCRIPTS(jsdc);
            jsdscript = jsd_FindJSDScript(jsdc, script);
            JSD_UNLOCK_SCRIPTS(jsdc);
            if( jsdscript )
            {
                ctorName = jsd_GetScriptFunctionName(jsdc, jsdscript);
                if( ctorName )
                    jsdobj->ctorName = jsd_AddAtom(jsdc, ctorName);
            }
            jsdobj->ctorLineno = JS_GetScriptBaseLineNumber(cx, script);
        }
    }
    TRACEOBJ(jsdc, jsdobj, 3);
    JSD_UNLOCK_OBJECTS(jsdc);
}

JS_STATIC_DLL_CALLBACK(JSHashNumber)
_hash_root(const void *key)
{
    return ((JSHashNumber) key) >> 2; /* help lame MSVC1.5 on Win16 */
}

JSBool
jsd_InitObjectManager(JSDContext* jsdc)
{
    JS_INIT_CLIST(&jsdc->objectsList);
    jsdc->objectsTable = JS_NewHashTable(256, _hash_root,
                                         JS_CompareValues, JS_CompareValues,
                                         NULL, NULL);
    return (JSBool) jsdc->objectsTable;
}

void
jsd_DestroyObjectManager(JSDContext* jsdc)
{
    JSD_LOCK_OBJECTS(jsdc);
    while( !JS_CLIST_IS_EMPTY(&jsdc->objectsList) )
        _destroyJSDObject(jsdc, (JSDObject*)JS_NEXT_LINK(&jsdc->objectsList));
    JS_HashTableDestroy(jsdc->objectsTable);
    JSD_UNLOCK_OBJECTS(jsdc);
}

JSDObject*
jsd_IterateObjects(JSDContext* jsdc, JSDObject** iterp)
{
    JSDObject *jsdobj = *iterp;

    JS_ASSERT(JSD_OBJECTS_LOCKED(jsdc));

    if( !jsdobj )
        jsdobj = (JSDObject *)jsdc->objectsList.next;
    if( jsdobj == (JSDObject *)&jsdc->objectsList )
        return NULL;
    *iterp = (JSDObject*) jsdobj->links.next;
    return jsdobj;
}

JSObject*
jsd_GetWrappedObject(JSDContext* jsdc, JSDObject* jsdobj)
{
    return jsdobj->obj;
}

const char*
jsd_GetObjectNewURL(JSDContext* jsdc, JSDObject* jsdobj)
{
    if( jsdobj->newURL )
        return JSD_ATOM_TO_STRING(jsdobj->newURL);
    return NULL;
}

uintN
jsd_GetObjectNewLineNumber(JSDContext* jsdc, JSDObject* jsdobj)
{
    return jsdobj->newLineno;
}

const char*
jsd_GetObjectConstructorURL(JSDContext* jsdc, JSDObject* jsdobj)
{
    if( jsdobj->ctorURL )
        return JSD_ATOM_TO_STRING(jsdobj->ctorURL);
    return NULL;
}

uintN
jsd_GetObjectConstructorLineNumber(JSDContext* jsdc, JSDObject* jsdobj)
{
    return jsdobj->ctorLineno;
}

const char*
jsd_GetObjectConstructorName(JSDContext* jsdc, JSDObject* jsdobj)
{
    if( jsdobj->ctorName )
        return JSD_ATOM_TO_STRING(jsdobj->ctorName);
    return NULL;
}

JSDObject*
jsd_GetJSDObjectForJSObject(JSDContext* jsdc, JSObject* jsobj)
{
    JSDObject* jsdobj;

    JSD_LOCK_OBJECTS(jsdc);
    jsdobj = (JSDObject*) JS_HashTableLookup(jsdc->objectsTable, jsobj);
    JSD_UNLOCK_OBJECTS(jsdc);
    return jsdobj;
}

JSDObject*
jsd_GetObjectForValue(JSDContext* jsdc, JSDValue* jsdval)
{
    return jsd_GetJSDObjectForJSObject(jsdc, JSVAL_TO_OBJECT(jsdval->val));
}

JSDValue*
jsd_GetValueForObject(JSDContext* jsdc, JSDObject* jsdobj)
{
    return jsd_NewValue(jsdc, OBJECT_TO_JSVAL(jsdobj->obj));
}


