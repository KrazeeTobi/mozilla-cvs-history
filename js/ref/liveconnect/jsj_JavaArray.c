/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

/*
 * This file is part of the Java-vendor-neutral implementation of LiveConnect
 *
 * It contains the definition of the JavaScript JavaArray class.
 * Instances of JavaArray are used to reflect Java arrays.
 */

#include <stdlib.h>
#include <string.h>
#include "prassert.h"

#include "jsj_private.h"      /* LiveConnect internals */

static JSBool
access_java_array_element(JSContext *cx,
                          JNIEnv *jEnv,
                          JSObject *obj,
                          jsid id,
                          jsval *vp,
                          JSBool do_assignment)
{
    jsval idval;
    jarray java_array;
    JavaClassDescriptor *class_descriptor;
    JavaObjectWrapper *java_wrapper;
    jsize array_length, index;
    JavaSignature *array_component_signature;
    
    /* printf("In JavaArray_getProperty\n"); */
    
    java_wrapper = JS_GetPrivate(cx, obj);
    if (!java_wrapper) {
        const char *property_name;
        if (JS_IdToValue(cx, id, &idval) && JSVAL_IS_STRING(idval) &&
            (property_name = JS_GetStringBytes(JSVAL_TO_STRING(idval)))) {
            if (!strcmp(property_name, "constructor")) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
        }
        JS_ReportError(cx, "illegal operation on JavaArray prototype object");
        return JS_FALSE;
    }
    class_descriptor = java_wrapper->class_descriptor;
    java_array = java_wrapper->java_obj;
    
    PR_ASSERT(class_descriptor->type == JAVA_SIGNATURE_ARRAY);

    JS_IdToValue(cx, id, &idval);

    if (!JSVAL_IS_INT(idval)) {
        /*
         * Usually, properties of JavaArray objects are indexed by integers, but
         * Java arrays also inherit all the methods of java.lang.Object, so a
         * string-valued property is also possible.
         */
        if (JSVAL_IS_STRING(idval)) {
            const char *member_name;
            
            member_name = JS_GetStringBytes(JSVAL_TO_STRING(idval));
            
            if (do_assignment) {
                JS_ReportError(cx, "Attempt to write to invalid Java array "
                    "element \"%s\"", member_name);
                return JS_FALSE;
            } else {
                if (!strcmp(member_name, "length")) {
                    array_length = jsj_GetJavaArrayLength(cx, jEnv, java_array);
                    if (array_length < 0)
                        return JS_FALSE;
                    if (vp)
                        *vp = INT_TO_JSVAL(array_length);
                    return JS_TRUE;
                }

                /* Check to see if we're reflecting a Java array method */
                return JavaObject_getPropertyById(cx, obj, id, vp);
            }
        }

        JS_ReportError(cx, "invalid Java array index expression");
        return JS_FALSE;
    }
    
    index = JSVAL_TO_INT(idval);

#if 0
    array_length = jsj_GetJavaArrayLength(cx, jEnv, java_array);
    if (array_length < 0)
        return JS_FALSE;

    /* Just let Java throw an exception instead of checking array bounds here */
    if (index < 0 || index >= array_length) {
        JS_ReportError(cx, "Java array index %d out of range", index);
        return JS_FALSE;
    }
#endif

    array_component_signature = class_descriptor->array_component_signature;

    if (!vp)
        return JS_TRUE;

    if (do_assignment) {
        return jsj_SetJavaArrayElement(cx, jEnv, java_array, index,
                                       array_component_signature, *vp);
    } else {
        return jsj_GetJavaArrayElement(cx, jEnv, java_array, index,
                                       array_component_signature, vp);
    }
}

PR_STATIC_CALLBACK(JSBool)
JavaArray_getPropertyById(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JNIEnv *jEnv;
    jsj_MapJSContextToJSJThread(cx, &jEnv);
    if (!jEnv)
        return JS_FALSE;
    return access_java_array_element(cx, jEnv, obj, id, vp, JS_FALSE);
}

PR_STATIC_CALLBACK(JSBool)
JavaArray_setPropertyById(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JNIEnv *jEnv;
    jsj_MapJSContextToJSJThread(cx, &jEnv);
    if (!jEnv)
        return JS_FALSE;
    return access_java_array_element(cx, jEnv, obj, id, vp, JS_TRUE);
}

#ifdef NO_JSOBJECTOPS

PR_STATIC_CALLBACK(JSBool)
JavaArray_enumerate(JSContext *cx, JSObject *obj)
{
    jarray java_array;
    JavaClassDescriptor *class_descriptor;
    JavaObjectWrapper *java_wrapper;
    jsize array_length, index;
    
    /* printf("In JavaArray_enumerate\n"); */
    
    java_wrapper = JS_GetPrivate(cx, obj);

    /* Check if this is the prototype object */
    if (!java_wrapper)
        return JS_TRUE;

    class_descriptor = java_wrapper->class_descriptor;
    java_array = java_wrapper->java_obj;
    
    PR_ASSERT(class_descriptor->type == JAVA_SIGNATURE_ARRAY);

    array_length = (*jEnv)->GetArrayLength(jEnv, java_array);
    for (index = 0; index < array_length; index++) {
        if (!JS_DefineElement(cx, obj, index, JSVAL_VOID,
                              0, 0, JSPROP_ENUMERATE))
            return JS_FALSE;
    }
    return JavaObject_enumerate(cx, obj);
}

JSBool
JavaArray_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
    jsid id;
    JS_ValueToId(cx, idval, &id);
    return JavaArray_getPropertyById(cx, obj, id, vp);
}

PR_STATIC_CALLBACK(JSBool)
JavaArray_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
    jsid id;
    JS_ValueToId(cx, idval, &id);
    return JavaArray_setPropertyById(cx, obj, id, vp);
}

/*
 * Resolve a component name to be either the name of a static field or a static method
 */
PR_STATIC_CALLBACK(JSBool)
JavaArray_resolve(JSContext *cx, JSObject *obj, jsval id)
{
    jarray java_array;
    const char *member_name;
    JavaClassDescriptor *class_descriptor;
    JavaObjectWrapper *java_wrapper;
    jsize array_length, index;
    
    /* printf("In JavaArray_resolve\n"); */
    
    java_wrapper = JS_GetPrivate(cx, obj);

    /* Check for prototype object */
    if (!java_wrapper)
        return JS_FALSE;

    class_descriptor = java_wrapper->class_descriptor;
    java_array = java_wrapper->java_obj;
    
    PR_ASSERT(class_descriptor->type == JAVA_SIGNATURE_ARRAY);

    array_length = (*jEnv)->GetArrayLength(jEnv, java_array);

    if (JSVAL_IS_STRING(id)) {
        member_name = JS_GetStringBytes(JSVAL_TO_STRING(id));
        if (!strcmp(member_name, "length")) {
            JS_DefineProperty(cx, obj, "length", INT_TO_JSVAL(array_length),
                              0, 0, JSPROP_READONLY);
            return JS_TRUE;
        }
        return JavaObject_resolve(cx, obj, id);
    }

    if (!JSVAL_IS_INT(id)) {
        JS_ReportError(cx, "invalid Java array index expression");
        return JS_FALSE;
    }

    index = JSVAL_TO_INT(id);
    return JS_DefineElement(cx, obj, index, JSVAL_VOID,
                            0, 0, JSPROP_ENUMERATE);
}

/* The definition of the JS class JavaArray */
JSClass JavaArray_class = {
    "JavaArray", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JavaArray_getProperty, JavaArray_setProperty,
    JavaArray_enumerate, JavaArray_resolve,
    JavaObject_convert, JavaObject_finalize
};
#else
static JSBool
JavaArray_lookupProperty(JSContext *cx, JSObject *obj, jsid id,
                         JSObject **objp, JSProperty **propp
#if defined JS_THREADSAFE && defined DEBUG
                            , const char *file, uintN line
#endif
                            )
{
    JNIEnv *jEnv;
    jsj_MapJSContextToJSJThread(cx, &jEnv);
    if (!jEnv)
        return JS_FALSE;

    return access_java_array_element(cx, jEnv, obj, id, NULL, JS_FALSE);
}

static JSBool
JavaArray_defineProperty(JSContext *cx, JSObject *obj, jsid id, jsval value,
                         JSPropertyOp getter, JSPropertyOp setter,
                         uintN attrs, JSProperty **propp)
{
    JS_ReportError(cx, "Elements of JavaArray objects may not be deleted");
    return JS_FALSE;
}

static JSBool
JavaArray_getAttributes(JSContext *cx, JSObject *obj, jsid id,
                        JSProperty *prop, uintN *attrsp)
{
    /* We don't maintain JS property attributes for Java class members */
    *attrsp = JSPROP_PERMANENT|JSPROP_ENUMERATE;
    return JS_FALSE;
}

static JSBool
JavaArray_setAttributes(JSContext *cx, JSObject *obj, jsid id,
                        JSProperty *prop, uintN *attrsp)
{
    /* We don't maintain JS property attributes for Java class members */
    if (*attrsp != JSPROP_PERMANENT|JSPROP_ENUMERATE) {
        PR_ASSERT(0);
        return JS_FALSE;
    }

    /* Silently ignore all setAttribute attempts */
    return JS_TRUE;
}

static JSBool
JavaArray_deleteProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
    JS_ReportError(cx, "Elements of JavaArray objects may not be deleted");
    return JS_FALSE;
}

static JSBool
JavaArray_defaultValue(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
    printf("In JavaArray_defaultValue()\n");
    return JavaObject_convert(cx, obj, JSTYPE_STRING, vp);
}

static JSBool
JavaArray_newEnumerate(JSContext *cx, JSObject *obj, JSIterateOp enum_op,
                       jsval *statep, jsid *idp)
{
    JavaObjectWrapper *java_wrapper;
    JNIEnv *jEnv;
    jsize array_length, index;

    java_wrapper = JS_GetPrivate(cx, obj);
    /* Check for prototype object */
    if (!java_wrapper) {
        *statep = JSVAL_NULL;
        if (idp)
            *idp = INT_TO_JSVAL(0);
        return JS_TRUE;
    }
        
    /* Get the Java per-thread environment pointer for this JSContext */
    jsj_MapJSContextToJSJThread(cx, &jEnv);
    if (!jEnv)
        return JS_FALSE;

    array_length = jsj_GetJavaArrayLength(cx, jEnv, java_wrapper->java_obj);
    if (array_length < 0)
        return JS_FALSE;

    switch(enum_op) {
    case JSENUMERATE_INIT:
        *statep = INT_TO_JSVAL(0);

        if (idp)
            *idp = INT_TO_JSVAL(array_length);
        return JS_TRUE;
        
    case JSENUMERATE_NEXT:
        index = JSVAL_TO_INT(*statep);
        if (index < array_length) {
            JS_ValueToId(cx, INT_TO_JSVAL(index), idp);
            index++;
            *statep = INT_TO_JSVAL(index);
            return JS_TRUE;
        }

        /* Fall through ... */

    case JSENUMERATE_DESTROY:
        *statep = JSVAL_NULL;
        return JS_TRUE;

    default:
        PR_ASSERT(0);
        return JS_FALSE;
    }
}

static JSBool
JavaArray_checkAccess(JSContext *cx, JSObject *obj, jsid id,
                      JSAccessMode mode, jsval *vp, uintN *attrsp)
{
    switch (mode) {
    case JSACC_WATCH:
        JS_ReportError(cx, "Cannot place watchpoints on JavaArray object properties");
        return JS_FALSE;

    case JSACC_IMPORT:
        JS_ReportError(cx, "Cannot export a JavaArray object's properties");
        return JS_FALSE;

    default:
        return JS_TRUE;
    }
}

JSObjectOps JavaArray_ops = {
    /* Mandatory non-null function pointer members. */
    NULL,                       /* newObjectMap */
    NULL,                       /* destroyObjectMap */
    JavaArray_lookupProperty,
    JavaArray_defineProperty,
    JavaArray_getPropertyById,  /* getProperty */
    JavaArray_setPropertyById,  /* setProperty */
    JavaArray_getAttributes,
    JavaArray_setAttributes,
    JavaArray_deleteProperty,
    JavaArray_defaultValue,
    JavaArray_newEnumerate,
    JavaArray_checkAccess,

    /* Optionally non-null members start here. */
    NULL,                       /* thisObject */
    NULL,                       /* dropProperty */
    NULL,                       /* call */
    NULL,                       /* construct */
    NULL,                       /* xdrObject */
    NULL                        /* hasInstance */
};

JSObjectOps *
JavaArray_getObjectOps(JSContext *cx, JSClass *clazz)
{
    return &JavaArray_ops;
}

JSClass JavaArray_class = {
    "JavaArray", JSCLASS_HAS_PRIVATE,
    NULL, NULL, NULL, NULL,
    NULL, NULL, JavaObject_convert, JavaObject_finalize,
    JavaArray_getObjectOps,
};

extern PR_IMPORT_DATA(JSObjectOps) js_ObjectOps;

#endif  /* NO_JSOBJECTOPS */

/* Initialize the JS JavaArray class */
JSBool
jsj_init_JavaArray(JSContext *cx, JSObject *global_obj)
{
#ifndef NO_JSOBJECTOPS
    JavaArray_ops.newObjectMap = js_ObjectOps.newObjectMap;
    JavaArray_ops.destroyObjectMap = js_ObjectOps.destroyObjectMap;
#endif

    if (!JS_InitClass(cx, global_obj, 
        0, &JavaArray_class, 0, 0,
        0, 0, 0, 0))
        return JS_FALSE;
    
    return JS_TRUE;
}

