/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

#ifndef jsxdrapi_h___
#define jsxdrapi_h___

/*
 * JS external data representation interface API.
 *
 * The XDR system is comprised of three major parts:
 *
 * - the state serialization/deserialization APIs, which allow consumers
 *   of the API to serialize JS runtime state (script bytecodes, atom maps,
 *   object graphs, etc.) for later restoration.  These portions
 *   are implemented in various appropriate files, such as jsscript.c
 *   for the script portions and jsobj.c for object state.
 * - the callback APIs through which the runtime requests an opaque
 *   representation of a native object, and through which the runtime
 *   constructs a live native object from an opaque representation. These
 *   portions are the responsibility of the native object implementor.
 * - utility functions for en/decoding of primitive types, such as
 *   JSStrings.  This portion is implemented in jsxdrapi.c.
 *
 * Spiritually guided by Sun's XDR, where appropriate.
 */

#include "jspubtd.h"
#include "jsprvtd.h"

JS_BEGIN_EXTERN_C

/* We use little-endian byteorder for all encoded data */

#if defined IS_LITTLE_ENDIAN
#define JSXDR_SWAB32(x) x
#define JSXDR_SWAB16(x) x
#elif defined IS_BIG_ENDIAN
#define JSXDR_SWAB32(x) (((x) >> 24) |                                        \
			 (((x) >> 8) & 0xff00) |                              \
			 (((x) << 8) & 0xff0000) |                            \
			 ((x) << 24))
#define JSXDR_SWAB16(x) (((x) >> 8) | ((x) << 8))
#else
#error "unknown byte order"
#endif

#define JSXDR_ALIGN     4

typedef enum JSXDRMode {
    JSXDR_ENCODE,
    JSXDR_DECODE,
    JSXDR_FREE
} JSXDRMode;

typedef enum JSXDRWhence {
    JSXDR_SEEK_SET,
    JSXDR_SEEK_CUR,
    JSXDR_SEEK_END
} JSXDRWhence;

typedef struct JSXDROps {
    JSBool      (*get32)(JSXDRState *, uint32 *);
    JSBool      (*set32)(JSXDRState *, uint32 *);
    JSBool      (*getbytes)(JSXDRState *, char **, uint32);
    JSBool      (*setbytes)(JSXDRState *, char **, uint32);
    void *      (*raw)(JSXDRState *, uint32);
    JSBool      (*seek)(JSXDRState *, int32, JSXDRWhence);
    uint32      (*tell)(JSXDRState *);
    void        (*finalize)(JSXDRState *);
} JSXDROps;

struct JSXDRState {
    JSXDRMode   mode;
    JSXDROps    *ops;
    JSContext   *cx;
    JSClass     **registry;
    uintN       nclasses;
    void        *data;
};

JS_PUBLIC_API(void)
JS_XDRNewBase(JSContext *cx, JSXDRState *xdr, JSXDRMode mode);

JS_PUBLIC_API(JSXDRState *)
JS_XDRNewMem(JSContext *cx, JSXDRMode mode);

JS_PUBLIC_API(void *)
JS_XDRMemGetData(JSXDRState *xdr, uint32 *lp);

JS_PUBLIC_API(void)
JS_XDRMemSetData(JSXDRState *xdr, void *data, uint32 len);

JS_PUBLIC_API(void)
JS_XDRDestroy(JSXDRState *xdr);

JS_PUBLIC_API(JSBool)
JS_XDRUint8(JSXDRState *xdr, uint8 *b);

JS_PUBLIC_API(JSBool)
JS_XDRUint16(JSXDRState *xdr, uint16 *s);

JS_PUBLIC_API(JSBool)
JS_XDRUint32(JSXDRState *xdr, uint32 *lp);

JS_PUBLIC_API(JSBool)
JS_XDRBytes(JSXDRState *xdr, char **bytes, uint32 len);

JS_PUBLIC_API(JSBool)
JS_XDRCString(JSXDRState *xdr, char **sp);

JS_PUBLIC_API(JSBool)
JS_XDRCStringOrNull(JSXDRState *xdr, char **sp);

JS_PUBLIC_API(JSBool)
JS_XDRString(JSXDRState *xdr, JSString **strp);

JS_PUBLIC_API(JSBool)
JS_XDRStringOrNull(JSXDRState *xdr, JSString **strp);

JS_PUBLIC_API(JSBool)
JS_XDRDouble(JSXDRState *xdr, jsdouble **dp);

JS_PUBLIC_API(JSBool)
JS_XDRValue(JSXDRState *xdr, jsval *vp);

JS_PUBLIC_API(JSBool)
JS_RegisterClass(JSXDRState *xdr, JSClass *clasp, uint32 *lp);

JS_PUBLIC_API(uint32)
JS_FindClassIdByName(JSXDRState *xdr, const char *name);

JS_PUBLIC_API(JSClass *)
JS_FindClassById(JSXDRState *xdr, uint32 id);

/* Magic values */

#define JSXDR_MAGIC_SCRIPT_1        0xdead0001
#define JSXDR_MAGIC_SCRIPT_2        0xdead0002
#define JSXDR_MAGIC_SCRIPT_CURRENT  JSXDR_MAGIC_SCRIPT_2

#endif /* ! jsxdrapi_h___ */
