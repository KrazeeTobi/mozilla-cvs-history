/*
 * @(#)jni.h	1.28 97/05/19
 *
 * Copyright (c) 1993-1996 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies.
 *
 * The Java source code is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You shall
 * not disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

/* 
 * We used part of Netscape's Java Runtime Interface (JRI) as the starting
 * point of our design and implementation.
 */

/******************************************************************************
 * Java Runtime Interface
 * Copyright (c) 1996 Netscape Communications Corporation. All rights reserved.
 *****************************************************************************/

#ifndef JNI_H
#define JNI_H

#include <stdio.h>
#include <stdarg.h>

/* jni_md.h contains the machine-dependent typedefs for jbyte, jint 
   and jlong */ 

#include "jni_md.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * JNI Types
 */

typedef unsigned char	jboolean;
typedef unsigned short	jchar;
typedef short		jshort;
typedef float		jfloat;
typedef double		jdouble;

typedef jint            jsize;

#ifdef __cplusplus

class _jobject {};
class _jclass : public _jobject {};
class _jthrowable : public _jobject {};
class _jstring : public _jobject {};
class _jarray : public _jobject {};
class _jbooleanArray : public _jarray {};
class _jbyteArray : public _jarray {};
class _jcharArray : public _jarray {};
class _jshortArray : public _jarray {};
class _jintArray : public _jarray {};
class _jlongArray : public _jarray {};
class _jfloatArray : public _jarray {};
class _jdoubleArray : public _jarray {};
class _jobjectArray : public _jarray {};

typedef _jobject *jobject;
typedef _jclass *jclass;
typedef _jthrowable *jthrowable;
typedef _jstring *jstring;
typedef _jarray *jarray;
typedef _jbooleanArray *jbooleanArray;
typedef _jbyteArray *jbyteArray;
typedef _jcharArray *jcharArray;
typedef _jshortArray *jshortArray;
typedef _jintArray *jintArray;
typedef _jlongArray *jlongArray;
typedef _jfloatArray *jfloatArray;
typedef _jdoubleArray *jdoubleArray;
typedef _jobjectArray *jobjectArray;

#else

struct _jobject;

typedef struct _jobject *jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jarray jobjectArray;

#endif

typedef jobject jref; /* For transition---not meant to be part of public 
			 API anymore.*/
typedef jobject jweak;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

struct _jfieldID;
typedef struct _jfieldID *jfieldID;

struct _jmethodID;
typedef struct _jmethodID *jmethodID;

/*
 * jboolean constants
 */

#define JNI_FALSE 0
#define JNI_TRUE 1

/*
 * possible return values for JNI functions.
 */

#define JNI_OK 0
#define JNI_ERR (-1)

/*
 * used in ReleaseScalarArrayElements
 */
  
#define JNI_COMMIT 1
#define JNI_ABORT 2

/*
 * used in RegisterNatives to describe native method name, signature,
 * and function pointer.
 */

typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod;

/*
 * JNI Native Method Interface.
 */

struct JNINativeInterface_;

struct JNIEnv_;

#ifdef __cplusplus
typedef JNIEnv_ JNIEnv;
#else
typedef const struct JNINativeInterface_ *JNIEnv;
#endif

/*
 * JNI Invocation Interface.
 */

struct JNIInvokeInterface_;

struct JavaVM_;

#ifdef __cplusplus
typedef JavaVM_ JavaVM;
#else
typedef const struct JNIInvokeInterface_ *JavaVM;
#endif

struct JNINativeInterface_ {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    void *reserved3;
    JNIFUNCPTR(jint, GetVersion)(JNIEnv *env);

    JNIFUNCPTR(jclass, DefineClass)
      (JNIEnv *env, const char *name, jobject loader, const jbyte *buf, 
       jsize len);
    JNIFUNCPTR(jclass, FindClass)
      (JNIEnv *env, const char *name);

    JNIFUNCPTR(jmethodID, FromReflectedMethod)
       (JNIEnv *env, jobject method);
    JNIFUNCPTR(jfieldID, FromReflectedField)
       (JNIEnv *env, jobject field);

    JNIFUNCPTR(jobject, ToReflectedMethod)
      (JNIEnv *env, jclass cls, jmethodID methodID);

      /*void *reserved4;
      void *reserved5;
      void *reserved6;
      */

    JNIFUNCPTR(jclass, GetSuperclass)
      (JNIEnv *env, jclass sub);
    JNIFUNCPTR(jboolean, IsAssignableFrom)
      (JNIEnv *env, jclass sub, jclass sup);

    JNIFUNCPTR(jobject, ToReflectedField)
      (JNIEnv *env, jclass cls, jfieldID fieldID);

      /*     void *reserved7;*/

    JNIFUNCPTR(jint, Throw)
      (JNIEnv *env, jthrowable obj);
    JNIFUNCPTR(jint, ThrowNew)
      (JNIEnv *env, jclass clazz, const char *msg);
    JNIFUNCPTR(jthrowable, ExceptionOccurred)
      (JNIEnv *env);
    JNIFUNCPTR(void, ExceptionDescribe)
      (JNIEnv *env);
    JNIFUNCPTR(void, ExceptionClear)
      (JNIEnv *env);
    JNIFUNCPTR(void, FatalError)
      (JNIEnv *env, const char *msg);

    JNIFUNCPTR(jint, PushLocalFrame)
      (JNIEnv *env, jint capacity);
    JNIFUNCPTR (jobject, PopLocalFrame)
      (JNIEnv *env, jobject result);

      /* void *reserved8;
	 void *reserved9;
	 */

    JNIFUNCPTR(jobject, NewGlobalRef)
      (JNIEnv *env, jobject lobj);
    JNIFUNCPTR(void, DeleteGlobalRef)
      (JNIEnv *env, jobject gref);
    JNIFUNCPTR(void, DeleteLocalRef)
      (JNIEnv *env, jobject obj);
    JNIFUNCPTR(jboolean, IsSameObject)
      (JNIEnv *env, jobject obj1, jobject obj2);

    JNIFUNCPTR(jobject, NewLocalRef)
      (JNIEnv *env, jobject ref);
    JNIFUNCPTR(jint, EnsureLocalCapacity)
      (JNIEnv *env, jint capacity);

      /*
      void *reserved10;
      void *reserved11;
      */

    JNIFUNCPTR(jobject, AllocObject)
      (JNIEnv *env, jclass clazz);
    JNIFUNCPTR(jobject, NewObject)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jobject, NewObjectV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jobject, NewObjectA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jclass, GetObjectClass)
      (JNIEnv *env, jobject obj);
    JNIFUNCPTR(jboolean, IsInstanceOf)
      (JNIEnv *env, jobject obj, jclass clazz);

    JNIFUNCPTR(jmethodID, GetMethodID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    JNIFUNCPTR(jobject, CallObjectMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jobject, CallObjectMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jobject, CallObjectMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    JNIFUNCPTR(jboolean, CallBooleanMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jboolean, CallBooleanMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jboolean, CallBooleanMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    JNIFUNCPTR(jbyte, CallByteMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jbyte, CallByteMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jbyte, CallByteMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jchar, CallCharMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jchar, CallCharMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jchar, CallCharMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jshort, CallShortMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jshort, CallShortMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jshort, CallShortMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jint, CallIntMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jint, CallIntMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jint, CallIntMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jlong, CallLongMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jlong, CallLongMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jlong, CallLongMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jfloat, CallFloatMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jfloat, CallFloatMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jfloat, CallFloatMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jdouble, CallDoubleMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(jdouble, CallDoubleMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(jdouble, CallDoubleMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(void, CallVoidMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    JNIFUNCPTR(void, CallVoidMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    JNIFUNCPTR(void, CallVoidMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    JNIFUNCPTR(jobject, CallNonvirtualObjectMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jobject, CallNonvirtualObjectMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       va_list args);
    JNIFUNCPTR(jobject, CallNonvirtualObjectMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue * args);

    JNIFUNCPTR(jboolean, CallNonvirtualBooleanMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jboolean, CallNonvirtualBooleanMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jboolean, CallNonvirtualBooleanMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue * args);

    JNIFUNCPTR(jbyte, CallNonvirtualByteMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jbyte, CallNonvirtualByteMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jbyte, CallNonvirtualByteMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue *args);

    JNIFUNCPTR(jchar, CallNonvirtualCharMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jchar, CallNonvirtualCharMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jchar, CallNonvirtualCharMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    JNIFUNCPTR(jshort, CallNonvirtualShortMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jshort, CallNonvirtualShortMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jshort, CallNonvirtualShortMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    JNIFUNCPTR(jint, CallNonvirtualIntMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jint, CallNonvirtualIntMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jint, CallNonvirtualIntMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    JNIFUNCPTR(jlong, CallNonvirtualLongMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jlong, CallNonvirtualLongMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jlong, CallNonvirtualLongMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue *args);

    JNIFUNCPTR(jfloat, CallNonvirtualFloatMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jfloat, CallNonvirtualFloatMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jfloat, CallNonvirtualFloatMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    JNIFUNCPTR(jdouble, CallNonvirtualDoubleMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jdouble, CallNonvirtualDoubleMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(jdouble, CallNonvirtualDoubleMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    JNIFUNCPTR(void, CallNonvirtualVoidMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(void, CallNonvirtualVoidMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    JNIFUNCPTR(void, CallNonvirtualVoidMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue * args);

    JNIFUNCPTR(jfieldID, GetFieldID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    JNIFUNCPTR(jobject, GetObjectField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jboolean, GetBooleanField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jbyte, GetByteField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jchar, GetCharField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jshort, GetShortField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jint, GetIntField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jlong, GetLongField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jfloat, GetFloatField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    JNIFUNCPTR(jdouble, GetDoubleField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);

    JNIFUNCPTR(void, SetObjectField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jobject val);
    JNIFUNCPTR(void, SetBooleanField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean val);
    JNIFUNCPTR(void, SetByteField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jbyte val);
    JNIFUNCPTR(void, SetCharField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jchar val);
    JNIFUNCPTR(void, SetShortField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jshort val);
    JNIFUNCPTR(void, SetIntField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jint val);
    JNIFUNCPTR(void, SetLongField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jlong val);
    JNIFUNCPTR(void, SetFloatField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jfloat val);
    JNIFUNCPTR(void, SetDoubleField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jdouble val);

    JNIFUNCPTR(jmethodID, GetStaticMethodID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    JNIFUNCPTR(jobject, CallStaticObjectMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jobject, CallStaticObjectMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jobject, CallStaticObjectMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jboolean, CallStaticBooleanMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jboolean, CallStaticBooleanMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jboolean, CallStaticBooleanMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jbyte, CallStaticByteMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jbyte, CallStaticByteMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jbyte, CallStaticByteMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jchar, CallStaticCharMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jchar, CallStaticCharMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jchar, CallStaticCharMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jshort, CallStaticShortMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jshort, CallStaticShortMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jshort, CallStaticShortMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jint, CallStaticIntMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jint, CallStaticIntMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jint, CallStaticIntMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jlong, CallStaticLongMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jlong, CallStaticLongMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jlong, CallStaticLongMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jfloat, CallStaticFloatMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jfloat, CallStaticFloatMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jfloat, CallStaticFloatMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(jdouble, CallStaticDoubleMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    JNIFUNCPTR(jdouble, CallStaticDoubleMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    JNIFUNCPTR(jdouble, CallStaticDoubleMethodA)       
      (JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    JNIFUNCPTR(void, CallStaticVoidMethod)
      (JNIEnv *env, jclass cls, jmethodID methodID, ...);
    JNIFUNCPTR(void, CallStaticVoidMethodV)
      (JNIEnv *env, jclass cls, jmethodID methodID, va_list args);
    JNIFUNCPTR(void, CallStaticVoidMethodA)
      (JNIEnv *env, jclass cls, jmethodID methodID, jvalue * args);

    JNIFUNCPTR(jfieldID, GetStaticFieldID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);
    JNIFUNCPTR(jobject, GetStaticObjectField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jboolean, GetStaticBooleanField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jbyte, GetStaticByteField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jchar, GetStaticCharField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jshort, GetStaticShortField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jint, GetStaticIntField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jlong, GetStaticLongField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jfloat, GetStaticFloatField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    JNIFUNCPTR(jdouble, GetStaticDoubleField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);

    JNIFUNCPTR(void, SetStaticObjectField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value);
    JNIFUNCPTR(void, SetStaticBooleanField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value);
    JNIFUNCPTR(void, SetStaticByteField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value);
    JNIFUNCPTR(void, SetStaticCharField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value);
    JNIFUNCPTR(void, SetStaticShortField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value);
    JNIFUNCPTR(void, SetStaticIntField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jint value);
    JNIFUNCPTR(void, SetStaticLongField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value);
    JNIFUNCPTR(void, SetStaticFloatField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value);
    JNIFUNCPTR(void, SetStaticDoubleField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value);

    JNIFUNCPTR(jstring, NewString)
      (JNIEnv *env, const jchar *unicode, jsize len);
    JNIFUNCPTR(jsize, GetStringLength)
      (JNIEnv *env, jstring str);
    JNIFUNCPTR(const jchar*, GetStringChars)
      (JNIEnv *env, jstring str, jboolean *isCopy);
    JNIFUNCPTR(void, ReleaseStringChars)
      (JNIEnv *env, jstring str, const jchar *chars);
  
    JNIFUNCPTR(jstring, NewStringUTF)
      (JNIEnv *env, const char *utf);
    JNIFUNCPTR(jsize, GetStringUTFLength)
      (JNIEnv *env, jstring str);
    JNIFUNCPTR(const char*, GetStringUTFChars)
      (JNIEnv *env, jstring str, jboolean *isCopy);
    JNIFUNCPTR(void, ReleaseStringUTFChars)
      (JNIEnv *env, jstring str, const char* chars);
  

    JNIFUNCPTR(jsize, GetArrayLength)
      (JNIEnv *env, jarray array);

    JNIFUNCPTR(jobjectArray, NewObjectArray)
      (JNIEnv *env, jsize len, jclass clazz, jobject init);
    JNIFUNCPTR(jobject, GetObjectArrayElement)
      (JNIEnv *env, jobjectArray array, jsize index);
    JNIFUNCPTR(void, SetObjectArrayElement)
      (JNIEnv *env, jobjectArray array, jsize index, jobject val);

    JNIFUNCPTR(jbooleanArray, NewBooleanArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jbyteArray, NewByteArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jcharArray, NewCharArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jshortArray, NewShortArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jintArray, NewIntArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jlongArray, NewLongArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jfloatArray, NewFloatArray)
      (JNIEnv *env, jsize len);
    JNIFUNCPTR(jdoubleArray, NewDoubleArray)
      (JNIEnv *env, jsize len);

    JNIFUNCPTR(jboolean *, GetBooleanArrayElements)
      (JNIEnv *env, jbooleanArray array, jboolean *isCopy);
    JNIFUNCPTR(jbyte *, GetByteArrayElements)
      (JNIEnv *env, jbyteArray array, jboolean *isCopy);
    JNIFUNCPTR(jchar *, GetCharArrayElements)
      (JNIEnv *env, jcharArray array, jboolean *isCopy);
    JNIFUNCPTR(jshort *, GetShortArrayElements)
      (JNIEnv *env, jshortArray array, jboolean *isCopy);
    JNIFUNCPTR(jint *, GetIntArrayElements)
      (JNIEnv *env, jintArray array, jboolean *isCopy);
    JNIFUNCPTR(jlong *, GetLongArrayElements)
      (JNIEnv *env, jlongArray array, jboolean *isCopy);
    JNIFUNCPTR(jfloat *, GetFloatArrayElements)
      (JNIEnv *env, jfloatArray array, jboolean *isCopy);
    JNIFUNCPTR(jdouble *, GetDoubleArrayElements)
      (JNIEnv *env, jdoubleArray array, jboolean *isCopy);

    JNIFUNCPTR(void, ReleaseBooleanArrayElements)
      (JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode);
    JNIFUNCPTR(void, ReleaseByteArrayElements)
      (JNIEnv *env, jbyteArray array, jbyte *elems, jint mode);
    JNIFUNCPTR(void, ReleaseCharArrayElements)
      (JNIEnv *env, jcharArray array, jchar *elems, jint mode);
    JNIFUNCPTR(void, ReleaseShortArrayElements)
      (JNIEnv *env, jshortArray array, jshort *elems, jint mode);
    JNIFUNCPTR(void, ReleaseIntArrayElements)
      (JNIEnv *env, jintArray array, jint *elems, jint mode);
    JNIFUNCPTR(void, ReleaseLongArrayElements)
      (JNIEnv *env, jlongArray array, jlong *elems, jint mode);
    JNIFUNCPTR(void, ReleaseFloatArrayElements)
      (JNIEnv *env, jfloatArray array, jfloat *elems, jint mode);
    JNIFUNCPTR(void, ReleaseDoubleArrayElements)
      (JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode);

    JNIFUNCPTR(void, GetBooleanArrayRegion)
      (JNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    JNIFUNCPTR(void, GetByteArrayRegion)
      (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
    JNIFUNCPTR(void, GetCharArrayRegion)
      (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
    JNIFUNCPTR(void, GetShortArrayRegion)
      (JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf);
    JNIFUNCPTR(void, GetIntArrayRegion)
      (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
    JNIFUNCPTR(void, GetLongArrayRegion)
      (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
    JNIFUNCPTR(void, GetFloatArrayRegion)
      (JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf);
    JNIFUNCPTR(void, GetDoubleArrayRegion)
      (JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf);

    JNIFUNCPTR(void, SetBooleanArrayRegion)
      (JNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    JNIFUNCPTR(void, SetByteArrayRegion)
      (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
    JNIFUNCPTR(void, SetCharArrayRegion)
      (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
    JNIFUNCPTR(void, SetShortArrayRegion)
      (JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf);
    JNIFUNCPTR(void, SetIntArrayRegion)
      (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
    JNIFUNCPTR(void, SetLongArrayRegion)
      (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
    JNIFUNCPTR(void, SetFloatArrayRegion)
      (JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf);
    JNIFUNCPTR(void, SetDoubleArrayRegion)
      (JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf);

    JNIFUNCPTR(jint, RegisterNatives)
      (JNIEnv *env, jclass clazz, const JNINativeMethod *methods, 
       jint nMethods);
    JNIFUNCPTR(jint, UnregisterNatives)
      (JNIEnv *env, jclass clazz);

    JNIFUNCPTR(jint, MonitorEnter)
      (JNIEnv *env, jobject obj);
    JNIFUNCPTR(jint, MonitorExit)
      (JNIEnv *env, jobject obj);
 
    JNIFUNCPTR(jint, GetJavaVM)
      (JNIEnv *env, JavaVM **vm);

     JNIFUNCPTR(void, GetStringRegion)
      (JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf);
    JNIFUNCPTR(void, GetStringUTFRegion)
      (JNIEnv *env, jstring str, jsize start, jsize len, char *buf);

    JNIFUNCPTR(void *, GetPrimitiveArrayCritical)
      (JNIEnv *env, jarray array, jboolean *isCopy);
    JNIFUNCPTR(void, ReleasePrimitiveArrayCritical)
      (JNIEnv *env, jarray array, void *carray, jint mode);

    JNIFUNCPTR(const jchar *, GetStringCritical)
      (JNIEnv *env, jstring string, jboolean *isCopy);
    JNIFUNCPTR(void, ReleaseStringCritical)
      (JNIEnv *env, jstring string, const jchar *cstring);

    JNIFUNCPTR(jweak, NewWeakGlobalRef)
       (JNIEnv *env, jobject obj);
    JNIFUNCPTR(void, DeleteWeakGlobalRef)
       (JNIEnv *env, jweak ref);

    JNIFUNCPTR(jboolean, ExceptionCheck)
       (JNIEnv *env);

};

/*
 * We use inlined functions for C++ so that programmers can write:
 * 
 *    env->FindClass("java/lang/String")
 *
 * in C++ rather than:
 *
 *    (*env)->FindClass(env, "java/lang/String")
 *
 * in C.
 */

struct JNIEnv_ {
    const struct JNINativeInterface_ *functions;
    void *reserved0;
    void *reserved1[6];
#ifdef __cplusplus

    jint GetVersion() {
        return functions->GetVersion(this);
    }
    jclass DefineClass(const char *name, jobject loader, const jbyte *buf,
		       jsize len) {
        return functions->DefineClass(this, name, loader, buf, len);
    }
    jclass FindClass(const char *name) {
        return functions->FindClass(this, name);
    }
    jclass GetSuperclass(jclass sub) {
        return functions->GetSuperclass(this, sub);
    }
    jboolean IsAssignableFrom(jclass sub, jclass sup) {
        return functions->IsAssignableFrom(this, sub, sup);
    }

    jint Throw(jthrowable obj) {
        return functions->Throw(this, obj);
    }    
    jint ThrowNew(jclass clazz, const char *msg) {
        return functions->ThrowNew(this, clazz, msg);
    }
    jthrowable ExceptionOccurred() {
        return functions->ExceptionOccurred(this);
    }
    void ExceptionDescribe() {
        functions->ExceptionDescribe(this);
    }
    void ExceptionClear() {
        functions->ExceptionClear(this);
    }
    void FatalError(const char *msg) {
        functions->FatalError(this, msg);
    }

    jobject NewGlobalRef(jobject lobj) {
        return functions->NewGlobalRef(this,lobj);
    }
    void DeleteGlobalRef(jobject gref) {
        functions->DeleteGlobalRef(this,gref);
    }
    void DeleteLocalRef(jobject obj) {
        functions->DeleteLocalRef(this, obj);
    }

    jboolean IsSameObject(jobject obj1, jobject obj2) {
        return functions->IsSameObject(this,obj1,obj2);
    }

    jobject AllocObject(jclass clazz) {
        return functions->AllocObject(this,clazz);
    }
    jobject NewObject(jclass clazz, jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args, methodID);
        result = functions->NewObjectV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jobject NewObjectV(jclass clazz, jmethodID methodID, 
		       va_list args) {
        return functions->NewObjectV(this,clazz,methodID,args);
    }
    jobject NewObjectA(jclass clazz, jmethodID methodID, 
		       jvalue *args) {
        return functions->NewObjectA(this,clazz,methodID,args);
    }

    jclass GetObjectClass(jobject obj) {
        return functions->GetObjectClass(this,obj);
    }
    jboolean IsInstanceOf(jobject obj, jclass clazz) {
        return functions->IsInstanceOf(this,obj,clazz);
    }

    jmethodID GetMethodID(jclass clazz, const char *name, 
			  const char *sig) {
        return functions->GetMethodID(this,clazz,name,sig);
    }

    jobject CallObjectMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallObjectMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jobject CallObjectMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallObjectMethodV(this,obj,methodID,args);
    }
    jobject CallObjectMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallObjectMethodA(this,obj,methodID,args);
    }

    jboolean CallBooleanMethod(jobject obj, 
			       jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallBooleanMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallBooleanMethodV(jobject obj, jmethodID methodID, 
				va_list args) {
        return functions->CallBooleanMethodV(this,obj,methodID,args);
    }
    jboolean CallBooleanMethodA(jobject obj, jmethodID methodID, 
				jvalue * args) {
        return functions->CallBooleanMethodA(this,obj,methodID, args);
    }

    jbyte CallByteMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallByteMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallByteMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallByteMethodV(this,obj,methodID,args);
    }
    jbyte CallByteMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallByteMethodA(this,obj,methodID,args);
    }

    jchar CallCharMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallCharMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jchar CallCharMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallCharMethodV(this,obj,methodID,args);
    }
    jchar CallCharMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallCharMethodA(this,obj,methodID,args);
    }

    jshort CallShortMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallShortMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jshort CallShortMethodV(jobject obj, jmethodID methodID, 
			    va_list args) {
        return functions->CallShortMethodV(this,obj,methodID,args);
    }
    jshort CallShortMethodA(jobject obj, jmethodID methodID, 
			    jvalue * args) {
        return functions->CallShortMethodA(this,obj,methodID,args);
    }

    jint CallIntMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallIntMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jint CallIntMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallIntMethodV(this,obj,methodID,args);
    }
    jint CallIntMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallIntMethodA(this,obj,methodID,args);
    }

    jlong CallLongMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallLongMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jlong CallLongMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallLongMethodV(this,obj,methodID,args);
    }
    jlong CallLongMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallLongMethodA(this,obj,methodID,args);
    }

    jfloat CallFloatMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallFloatMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallFloatMethodV(jobject obj, jmethodID methodID, 
			    va_list args) {
        return functions->CallFloatMethodV(this,obj,methodID,args);
    }
    jfloat CallFloatMethodA(jobject obj, jmethodID methodID, 
			    jvalue * args) {
        return functions->CallFloatMethodA(this,obj,methodID,args);
    }

    jdouble CallDoubleMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallDoubleMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallDoubleMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallDoubleMethodV(this,obj,methodID,args);
    }
    jdouble CallDoubleMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallDoubleMethodA(this,obj,methodID,args);
    }

    void CallVoidMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallVoidMethodV(this,obj,methodID,args);
	va_end(args);
    }
    void CallVoidMethodV(jobject obj, jmethodID methodID, 
			 va_list args) {
        functions->CallVoidMethodV(this,obj,methodID,args);
    }
    void CallVoidMethodA(jobject obj, jmethodID methodID, 
			 jvalue * args) {
        functions->CallVoidMethodA(this,obj,methodID,args);
    }

    jobject CallNonvirtualObjectMethod(jobject obj, jclass clazz, 
				       jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallNonvirtualObjectMethodV(this,obj,clazz,
							methodID,args);
	va_end(args);
	return result;
    }
    jobject CallNonvirtualObjectMethodV(jobject obj, jclass clazz, 
					jmethodID methodID, va_list args) {
        return functions->CallNonvirtualObjectMethodV(this,obj,clazz,
						      methodID,args);
    }
    jobject CallNonvirtualObjectMethodA(jobject obj, jclass clazz, 
					jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualObjectMethodA(this,obj,clazz,
						      methodID,args);
    }

    jboolean CallNonvirtualBooleanMethod(jobject obj, jclass clazz, 
					 jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallNonvirtualBooleanMethodV(this,obj,clazz,
							 methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallNonvirtualBooleanMethodV(jobject obj, jclass clazz, 
					  jmethodID methodID, va_list args) {
        return functions->CallNonvirtualBooleanMethodV(this,obj,clazz,
						       methodID,args);
    }
    jboolean CallNonvirtualBooleanMethodA(jobject obj, jclass clazz, 
					  jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualBooleanMethodA(this,obj,clazz,
						       methodID, args);
    }

    jbyte CallNonvirtualByteMethod(jobject obj, jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallNonvirtualByteMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallNonvirtualByteMethodV(jobject obj, jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualByteMethodV(this,obj,clazz,
						    methodID,args);
    }
    jbyte CallNonvirtualByteMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualByteMethodA(this,obj,clazz,
						    methodID,args);
    }

    jchar CallNonvirtualCharMethod(jobject obj, jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallNonvirtualCharMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jchar CallNonvirtualCharMethodV(jobject obj, jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualCharMethodV(this,obj,clazz,
						    methodID,args);
    }
    jchar CallNonvirtualCharMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualCharMethodA(this,obj,clazz,
						    methodID,args);
    }

    jshort CallNonvirtualShortMethod(jobject obj, jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallNonvirtualShortMethodV(this,obj,clazz,
						       methodID,args);
	va_end(args);
	return result;
    }
    jshort CallNonvirtualShortMethodV(jobject obj, jclass clazz, 
				      jmethodID methodID, va_list args) {
        return functions->CallNonvirtualShortMethodV(this,obj,clazz,
						     methodID,args);
    }
    jshort CallNonvirtualShortMethodA(jobject obj, jclass clazz,
				      jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualShortMethodA(this,obj,clazz,
						     methodID,args);
    }

    jint CallNonvirtualIntMethod(jobject obj, jclass clazz, 
				 jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallNonvirtualIntMethodV(this,obj,clazz,
						     methodID,args);
	va_end(args);
	return result;
    }
    jint CallNonvirtualIntMethodV(jobject obj, jclass clazz, 
				  jmethodID methodID, va_list args) {
        return functions->CallNonvirtualIntMethodV(this,obj,clazz,
						   methodID,args);
    }
    jint CallNonvirtualIntMethodA(jobject obj, jclass clazz, 
				  jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualIntMethodA(this,obj,clazz,
						   methodID,args);
    }

    jlong CallNonvirtualLongMethod(jobject obj, jclass clazz,
				   jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallNonvirtualLongMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jlong CallNonvirtualLongMethodV(jobject obj, jclass clazz,
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualLongMethodV(this,obj,clazz,
						    methodID,args);
    }
    jlong CallNonvirtualLongMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualLongMethodA(this,obj,clazz,
						    methodID,args);
    }

    jfloat CallNonvirtualFloatMethod(jobject obj, jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallNonvirtualFloatMethodV(this,obj,clazz,
						       methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallNonvirtualFloatMethodV(jobject obj, jclass clazz,
				      jmethodID methodID, 
				      va_list args) {
        return functions->CallNonvirtualFloatMethodV(this,obj,clazz,
						     methodID,args);
    }
    jfloat CallNonvirtualFloatMethodA(jobject obj, jclass clazz, 
				      jmethodID methodID, 
				      jvalue * args) {
        return functions->CallNonvirtualFloatMethodA(this,obj,clazz,
						     methodID,args);
    }

    jdouble CallNonvirtualDoubleMethod(jobject obj, jclass clazz,
				       jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallNonvirtualDoubleMethodV(this,obj,clazz,
							methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallNonvirtualDoubleMethodV(jobject obj, jclass clazz,
					jmethodID methodID, 
					va_list args) {
        return functions->CallNonvirtualDoubleMethodV(this,obj,clazz,
						      methodID,args);
    }
    jdouble CallNonvirtualDoubleMethodA(jobject obj, jclass clazz, 
					jmethodID methodID, 
					jvalue * args) {
        return functions->CallNonvirtualDoubleMethodA(this,obj,clazz,
						      methodID,args);
    }

    void CallNonvirtualVoidMethod(jobject obj, jclass clazz,
				  jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallNonvirtualVoidMethodV(this,obj,clazz,methodID,args);
	va_end(args);
    }
    void CallNonvirtualVoidMethodV(jobject obj, jclass clazz,
				   jmethodID methodID, 
				   va_list args) {
        functions->CallNonvirtualVoidMethodV(this,obj,clazz,methodID,args);
    }
    void CallNonvirtualVoidMethodA(jobject obj, jclass clazz,
				   jmethodID methodID, 
				   jvalue * args) {
        functions->CallNonvirtualVoidMethodA(this,obj,clazz,methodID,args);
    }

    jfieldID GetFieldID(jclass clazz, const char *name, 
			const char *sig) {
        return functions->GetFieldID(this,clazz,name,sig);
    }

    jobject GetObjectField(jobject obj, jfieldID fieldID) {
        return functions->GetObjectField(this,obj,fieldID);
    }
    jboolean GetBooleanField(jobject obj, jfieldID fieldID) {
        return functions->GetBooleanField(this,obj,fieldID);
    }
    jbyte GetByteField(jobject obj, jfieldID fieldID) {
        return functions->GetByteField(this,obj,fieldID);
    }
    jchar GetCharField(jobject obj, jfieldID fieldID) {
        return functions->GetCharField(this,obj,fieldID);
    }
    jshort GetShortField(jobject obj, jfieldID fieldID) {
        return functions->GetShortField(this,obj,fieldID);
    }
    jint GetIntField(jobject obj, jfieldID fieldID) {
        return functions->GetIntField(this,obj,fieldID);
    }
    jlong GetLongField(jobject obj, jfieldID fieldID) {
        return functions->GetLongField(this,obj,fieldID);
    }
    jfloat GetFloatField(jobject obj, jfieldID fieldID) {
        return functions->GetFloatField(this,obj,fieldID);
    }
    jdouble GetDoubleField(jobject obj, jfieldID fieldID) {
        return functions->GetDoubleField(this,obj,fieldID);
    }

    void SetObjectField(jobject obj, jfieldID fieldID, jobject val) {
        functions->SetObjectField(this,obj,fieldID,val);
    }
    void SetBooleanField(jobject obj, jfieldID fieldID, 
			 jboolean val) {
        functions->SetBooleanField(this,obj,fieldID,val);
    }
    void SetByteField(jobject obj, jfieldID fieldID, 
		      jbyte val) {
        functions->SetByteField(this,obj,fieldID,val);
    }
    void SetCharField(jobject obj, jfieldID fieldID, 
		      jchar val) {
        functions->SetCharField(this,obj,fieldID,val);
    }
    void SetShortField(jobject obj, jfieldID fieldID,
		       jshort val) {
        functions->SetShortField(this,obj,fieldID,val);
    }
    void SetIntField(jobject obj, jfieldID fieldID, 
		     jint val) {
        functions->SetIntField(this,obj,fieldID,val);
    }
    void SetLongField(jobject obj, jfieldID fieldID, 
		      jlong val) {
        functions->SetLongField(this,obj,fieldID,val);
    }
    void SetFloatField(jobject obj, jfieldID fieldID, 
		       jfloat val) {
        functions->SetFloatField(this,obj,fieldID,val);
    }
    void SetDoubleField(jobject obj, jfieldID fieldID, 
			jdouble val) {
        functions->SetDoubleField(this,obj,fieldID,val);
    }

    jmethodID GetStaticMethodID(jclass clazz, const char *name, 
				const char *sig) {
        return functions->GetStaticMethodID(this,clazz,name,sig);
    }

    jobject CallStaticObjectMethod(jclass clazz, jmethodID methodID, 
			     ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallStaticObjectMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jobject CallStaticObjectMethodV(jclass clazz, jmethodID methodID, 
			      va_list args) {
        return functions->CallStaticObjectMethodV(this,clazz,methodID,args);
    }
    jobject CallStaticObjectMethodA(jclass clazz, jmethodID methodID, 
			      jvalue *args) {
        return functions->CallStaticObjectMethodA(this,clazz,methodID,args);
    }

    jboolean CallStaticBooleanMethod(jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallStaticBooleanMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallStaticBooleanMethodV(jclass clazz,
				      jmethodID methodID, va_list args) {
        return functions->CallStaticBooleanMethodV(this,clazz,methodID,args);
    }
    jboolean CallStaticBooleanMethodA(jclass clazz,
				      jmethodID methodID, jvalue *args) {
        return functions->CallStaticBooleanMethodA(this,clazz,methodID,args);
    }

    jbyte CallStaticByteMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallStaticByteMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallStaticByteMethodV(jclass clazz,
				jmethodID methodID, va_list args) {
        return functions->CallStaticByteMethodV(this,clazz,methodID,args);
    }
    jbyte CallStaticByteMethodA(jclass clazz, 
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticByteMethodA(this,clazz,methodID,args);
    }

    jchar CallStaticCharMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallStaticCharMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jchar CallStaticCharMethodV(jclass clazz,
				jmethodID methodID, va_list args) {
        return functions->CallStaticCharMethodV(this,clazz,methodID,args);
    }
    jchar CallStaticCharMethodA(jclass clazz,
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticCharMethodA(this,clazz,methodID,args);
    }

    jshort CallStaticShortMethod(jclass clazz,
				 jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallStaticShortMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jshort CallStaticShortMethodV(jclass clazz,
				  jmethodID methodID, va_list args) {
        return functions->CallStaticShortMethodV(this,clazz,methodID,args);
    }
    jshort CallStaticShortMethodA(jclass clazz,
				  jmethodID methodID, jvalue *args) {
        return functions->CallStaticShortMethodA(this,clazz,methodID,args);
    }

    jint CallStaticIntMethod(jclass clazz,
			     jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallStaticIntMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jint CallStaticIntMethodV(jclass clazz,
			      jmethodID methodID, va_list args) {
        return functions->CallStaticIntMethodV(this,clazz,methodID,args);
    }
    jint CallStaticIntMethodA(jclass clazz, 
			      jmethodID methodID, jvalue *args) {
        return functions->CallStaticIntMethodA(this,clazz,methodID,args);
    }

    jlong CallStaticLongMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallStaticLongMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jlong CallStaticLongMethodV(jclass clazz, 
				jmethodID methodID, va_list args) {
        return functions->CallStaticLongMethodV(this,clazz,methodID,args);
    }
    jlong CallStaticLongMethodA(jclass clazz, 
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticLongMethodA(this,clazz,methodID,args);
    }

    jfloat CallStaticFloatMethod(jclass clazz, 
				 jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallStaticFloatMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallStaticFloatMethodV(jclass clazz, 
				  jmethodID methodID, va_list args) {
        return functions->CallStaticFloatMethodV(this,clazz,methodID,args);
    }
    jfloat CallStaticFloatMethodA(jclass clazz, 
				  jmethodID methodID, jvalue *args) {
        return functions->CallStaticFloatMethodA(this,clazz,methodID,args);
    }

    jdouble CallStaticDoubleMethod(jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallStaticDoubleMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallStaticDoubleMethodV(jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallStaticDoubleMethodV(this,clazz,methodID,args);
    }
    jdouble CallStaticDoubleMethodA(jclass clazz, 
				    jmethodID methodID, jvalue *args) {
        return functions->CallStaticDoubleMethodA(this,clazz,methodID,args);
    }

    void CallStaticVoidMethod(jclass cls, jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallStaticVoidMethodV(this,cls,methodID,args);
	va_end(args);
    }
    void CallStaticVoidMethodV(jclass cls, jmethodID methodID, 
			       va_list args) {
        functions->CallStaticVoidMethodV(this,cls,methodID,args);
    }
    void CallStaticVoidMethodA(jclass cls, jmethodID methodID, 
			       jvalue * args) {
        functions->CallStaticVoidMethodA(this,cls,methodID,args);
    }

    jfieldID GetStaticFieldID(jclass clazz, const char *name, 
			      const char *sig) {
        return functions->GetStaticFieldID(this,clazz,name,sig);
    }
    jobject GetStaticObjectField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticObjectField(this,clazz,fieldID);
    }
    jboolean GetStaticBooleanField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticBooleanField(this,clazz,fieldID);
    }
    jbyte GetStaticByteField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticByteField(this,clazz,fieldID);
    }
    jchar GetStaticCharField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticCharField(this,clazz,fieldID);
    }
    jshort GetStaticShortField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticShortField(this,clazz,fieldID);
    }
    jint GetStaticIntField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticIntField(this,clazz,fieldID);
    }
    jlong GetStaticLongField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticLongField(this,clazz,fieldID);
    }
    jfloat GetStaticFloatField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticFloatField(this,clazz,fieldID);
    }
    jdouble GetStaticDoubleField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticDoubleField(this,clazz,fieldID);
    }

    void SetStaticObjectField(jclass clazz, jfieldID fieldID,
			jobject value) {
      functions->SetStaticObjectField(this,clazz,fieldID,value);
    }
    void SetStaticBooleanField(jclass clazz, jfieldID fieldID,
			jboolean value) {
      functions->SetStaticBooleanField(this,clazz,fieldID,value);
    }
    void SetStaticByteField(jclass clazz, jfieldID fieldID,
			jbyte value) {
      functions->SetStaticByteField(this,clazz,fieldID,value);
    }
    void SetStaticCharField(jclass clazz, jfieldID fieldID,
			jchar value) {
      functions->SetStaticCharField(this,clazz,fieldID,value);
    }
    void SetStaticShortField(jclass clazz, jfieldID fieldID,
			jshort value) {
      functions->SetStaticShortField(this,clazz,fieldID,value);
    }
    void SetStaticIntField(jclass clazz, jfieldID fieldID,
			jint value) {
      functions->SetStaticIntField(this,clazz,fieldID,value);
    }
    void SetStaticLongField(jclass clazz, jfieldID fieldID,
			jlong value) {
      functions->SetStaticLongField(this,clazz,fieldID,value);
    }
    void SetStaticFloatField(jclass clazz, jfieldID fieldID,
			jfloat value) {
      functions->SetStaticFloatField(this,clazz,fieldID,value);
    }
    void SetStaticDoubleField(jclass clazz, jfieldID fieldID,
			jdouble value) {
      functions->SetStaticDoubleField(this,clazz,fieldID,value);
    }

    jstring NewString(const jchar *unicode, jsize len) {
        return functions->NewString(this,unicode,len);
    }
    jsize GetStringLength(jstring str) {
        return functions->GetStringLength(this,str);
    }
    const jchar *GetStringChars(jstring str, jboolean *isCopy) {
        return functions->GetStringChars(this,str,isCopy);
    }
    void ReleaseStringChars(jstring str, const jchar *chars) {
        functions->ReleaseStringChars(this,str,chars);
    }
  
    jstring NewStringUTF(const char *utf) {
        return functions->NewStringUTF(this,utf);
    }
    jsize GetStringUTFLength(jstring str) {
        return functions->GetStringUTFLength(this,str);
    }
    const char* GetStringUTFChars(jstring str, jboolean *isCopy) {
        return functions->GetStringUTFChars(this,str,isCopy);
    }
    void ReleaseStringUTFChars(jstring str, const char* chars) {
        functions->ReleaseStringUTFChars(this,str,chars);
    }

    jsize GetArrayLength(jarray array) {
        return functions->GetArrayLength(this,array);
    }

    jobjectArray NewObjectArray(jsize len, jclass clazz, 
				jobject init) {
        return functions->NewObjectArray(this,len,clazz,init);
    }
    jobject GetObjectArrayElement(jobjectArray array, jsize index) {
        return functions->GetObjectArrayElement(this,array,index);
    }
    void SetObjectArrayElement(jobjectArray array, jsize index, 
			       jobject val) {
        functions->SetObjectArrayElement(this,array,index,val);
    }

    jbooleanArray NewBooleanArray(jsize len) {
        return functions->NewBooleanArray(this,len);
    }
    jbyteArray NewByteArray(jsize len) {
        return functions->NewByteArray(this,len);
    }
    jcharArray NewCharArray(jsize len) {
        return functions->NewCharArray(this,len);
    }
    jshortArray NewShortArray(jsize len) {
        return functions->NewShortArray(this,len);
    }
    jintArray NewIntArray(jsize len) {
        return functions->NewIntArray(this,len);
    }
    jlongArray NewLongArray(jsize len) {
        return functions->NewLongArray(this,len);
    }
    jfloatArray NewFloatArray(jsize len) {
        return functions->NewFloatArray(this,len);
    }
    jdoubleArray NewDoubleArray(jsize len) {
        return functions->NewDoubleArray(this,len);
    }

    jboolean * GetBooleanArrayElements(jbooleanArray array, jboolean *isCopy) {
        return functions->GetBooleanArrayElements(this,array,isCopy);
    }
    jbyte * GetByteArrayElements(jbyteArray array, jboolean *isCopy) {
        return functions->GetByteArrayElements(this,array,isCopy);
    }
    jchar * GetCharArrayElements(jcharArray array, jboolean *isCopy) {
        return functions->GetCharArrayElements(this,array,isCopy);
    }
    jshort * GetShortArrayElements(jshortArray array, jboolean *isCopy) {
        return functions->GetShortArrayElements(this,array,isCopy);
    }
    jint * GetIntArrayElements(jintArray array, jboolean *isCopy) {
        return functions->GetIntArrayElements(this,array,isCopy);
    }
    jlong * GetLongArrayElements(jlongArray array, jboolean *isCopy) {
        return functions->GetLongArrayElements(this,array,isCopy);
    }
    jfloat * GetFloatArrayElements(jfloatArray array, jboolean *isCopy) {
        return functions->GetFloatArrayElements(this,array,isCopy);
    }
    jdouble * GetDoubleArrayElements(jdoubleArray array, jboolean *isCopy) {
        return functions->GetDoubleArrayElements(this,array,isCopy);
    }

    void ReleaseBooleanArrayElements(jbooleanArray array, 
				     jboolean *elems,
				     jint mode) {
        functions->ReleaseBooleanArrayElements(this,array,elems,mode);
    }
    void ReleaseByteArrayElements(jbyteArray array, 
				  jbyte *elems,
				  jint mode) {
        functions->ReleaseByteArrayElements(this,array,elems,mode);
    }
    void ReleaseCharArrayElements(jcharArray array, 
				  jchar *elems,
				  jint mode) {
        functions->ReleaseCharArrayElements(this,array,elems,mode);
    }
    void ReleaseShortArrayElements(jshortArray array, 
				   jshort *elems,
				   jint mode) {
        functions->ReleaseShortArrayElements(this,array,elems,mode);
    }
    void ReleaseIntArrayElements(jintArray array, 
				 jint *elems,
				 jint mode) {
        functions->ReleaseIntArrayElements(this,array,elems,mode);
    }
    void ReleaseLongArrayElements(jlongArray array, 
				  jlong *elems,
				  jint mode) {
        functions->ReleaseLongArrayElements(this,array,elems,mode);
    }
    void ReleaseFloatArrayElements(jfloatArray array, 
				   jfloat *elems,
				   jint mode) {
        functions->ReleaseFloatArrayElements(this,array,elems,mode);
    }
    void ReleaseDoubleArrayElements(jdoubleArray array, 
				    jdouble *elems,
				    jint mode) {
        functions->ReleaseDoubleArrayElements(this,array,elems,mode);
    }

    void GetBooleanArrayRegion(jbooleanArray array, 
			       jsize start, jsize len, jboolean *buf) {
        functions->GetBooleanArrayRegion(this,array,start,len,buf);
    }
    void GetByteArrayRegion(jbyteArray array, 
			    jsize start, jsize len, jbyte *buf) {
        functions->GetByteArrayRegion(this,array,start,len,buf);
    }
    void GetCharArrayRegion(jcharArray array, 
			    jsize start, jsize len, jchar *buf) {
        functions->GetCharArrayRegion(this,array,start,len,buf);
    }
    void GetShortArrayRegion(jshortArray array, 
			     jsize start, jsize len, jshort *buf) {
        functions->GetShortArrayRegion(this,array,start,len,buf);
    }
    void GetIntArrayRegion(jintArray array, 
			   jsize start, jsize len, jint *buf) {
        functions->GetIntArrayRegion(this,array,start,len,buf);
    }
    void GetLongArrayRegion(jlongArray array, 
			    jsize start, jsize len, jlong *buf) {
        functions->GetLongArrayRegion(this,array,start,len,buf);
    }
    void GetFloatArrayRegion(jfloatArray array, 
			     jsize start, jsize len, jfloat *buf) {
        functions->GetFloatArrayRegion(this,array,start,len,buf);
    }
    void GetDoubleArrayRegion(jdoubleArray array, 
			      jsize start, jsize len, jdouble *buf) {
        functions->GetDoubleArrayRegion(this,array,start,len,buf);
    }

    void SetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, 
			       jboolean *buf) {
        functions->SetBooleanArrayRegion(this,array,start,len,buf);
    }
    void SetByteArrayRegion(jbyteArray array, jsize start, jsize len,
			    jbyte *buf) {
        functions->SetByteArrayRegion(this,array,start,len,buf);
    }
    void SetCharArrayRegion(jcharArray array, jsize start, jsize len, 
			    jchar *buf) {
        functions->SetCharArrayRegion(this,array,start,len,buf);
    }
    void SetShortArrayRegion(jshortArray array, jsize start, jsize len, 
			     jshort *buf) {
        functions->SetShortArrayRegion(this,array,start,len,buf);
    }
    void SetIntArrayRegion(jintArray array, jsize start, jsize len,
			   jint *buf) {
        functions->SetIntArrayRegion(this,array,start,len,buf);
    }
    void SetLongArrayRegion(jlongArray array, jsize start, jsize len,
			    jlong *buf) {
        functions->SetLongArrayRegion(this,array,start,len,buf);
    }
    void SetFloatArrayRegion(jfloatArray array, jsize start, jsize len, 
			     jfloat *buf) {
        functions->SetFloatArrayRegion(this,array,start,len,buf);
    }
    void SetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len,
			      jdouble *buf) {
        functions->SetDoubleArrayRegion(this,array,start,len,buf);
    }

    jint RegisterNatives(jclass clazz, const JNINativeMethod *methods,
			 jint nMethods) {
        return functions->RegisterNatives(this,clazz,methods,nMethods);
    }
    jint UnregisterNatives(jclass clazz) {
        return functions->UnregisterNatives(this,clazz);
    }  
   
    jint MonitorEnter(jobject obj) {
        return functions->MonitorEnter(this,obj);
    }
    jint MonitorExit(jobject obj) {
        return functions->MonitorExit(this,obj);
    }

    jint GetJavaVM(JavaVM **vm) {
        return functions->GetJavaVM(this,vm);
    }
  
#endif /* __cplusplus */
};

/* These structures will be VM-specific. */

typedef struct JDK1_1InitArgs {
    jint version;

    char **properties;
    jint checkSource; 
    jint nativeStackSize;
    jint javaStackSize;
    jint minHeapSize;
    jint maxHeapSize;
    jint verifyMode;
    char *classpath;

    JNIFUNCPTR(jint, vfprintf)(FILE *fp, const char *format, va_list args);
    JNIFUNCPTR(void, exit)(jint code);
    JNIFUNCPTR(void, abort)();
    
    jint enableClassGC;
    jint enableVerboseGC;
    jint disableAsyncGC;
    jint verbose;
    jboolean debugging;
    jint debugPort;
} JDK1_1InitArgs;

typedef struct JDK1_1AttachArgs {
    void * __padding; /* C compilers don't allow empty structures. */
} JDK1_1AttachArgs;

/* End VM-specific. */

struct JNIInvokeInterface_ {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    JNIFUNCPTR(jint, DestroyJavaVM)(JavaVM *vm);

    JNIFUNCPTR(jint, AttachCurrentThread)
      (JavaVM *vm, JNIEnv **penv, void *args);

    JNIFUNCPTR(jint, DetachCurrentThread)(JavaVM *vm);
};

struct JavaVM_ {
    const struct JNIInvokeInterface_ *functions;
    void *reserved0;
    void *reserved1;
    void *reserved2;
#ifdef __cplusplus

    jint DestroyJavaVM() {
        return functions->DestroyJavaVM(this);
    }
    jint AttachCurrentThread(JNIEnv **penv, void *args) {
        return functions->AttachCurrentThread(this, penv, args);
    }
    jint DetachCurrentThread() {
        return functions->DetachCurrentThread(this);
    }

#endif
};

JNICALL(jint) JNI_GetDefaultJavaVMInitArgs(void *);

JNICALL(jint) JNI_CreateJavaVM(JavaVM **, JNIEnv **, void *);

JNICALL(jint) JNI_GetCreatedJavaVMs(JavaVM **, jsize, jsize *);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* JNI_H */


