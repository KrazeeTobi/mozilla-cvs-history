/* -*- Mode: C; tab-width: 4; -*- */
/*******************************************************************************
 * Source date: 28 Apr 1998 21:44:18 GMT
 * netscape/fonts/winfp module C header file
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#ifndef _Mwinfp_H_
#define _Mwinfp_H_

#include "jmc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * winfp
 ******************************************************************************/

/* The type of the winfp interface. */
struct winfpInterface;

/* The public type of a winfp instance. */
typedef struct winfp {
	const struct winfpInterface*	vtable;
} winfp;

/* The inteface ID of the winfp interface. */
#ifndef JMC_INIT_winfp_ID
extern EXTERN_C_WITHOUT_EXTERN const JMCInterfaceID winfp_ID;
#else
EXTERN_C const JMCInterfaceID winfp_ID = { 0x1827292b, 0x1915644a, 0x282f2339, 0x77347420 };
#endif /* JMC_INIT_winfp_ID */
/*******************************************************************************
 * winfp Operations
 ******************************************************************************/

#define winfp_getInterface(self, a, exception)	\
	(((self)->vtable->getInterface)(self, winfp_getInterface_op, a, exception))

#define winfp_addRef(self, exception)	\
	(((self)->vtable->addRef)(self, winfp_addRef_op, exception))

#define winfp_release(self, exception)	\
	(((self)->vtable->release)(self, winfp_release_op, exception))

#define winfp_hashCode(self, exception)	\
	(((self)->vtable->hashCode)(self, winfp_hashCode_op, exception))

#define winfp_equals(self, obj, exception)	\
	(((self)->vtable->equals)(self, winfp_equals_op, obj, exception))

#define winfp_clone(self, exception)	\
	(((self)->vtable->clone)(self, winfp_clone_op, exception))

#define winfp_toString(self, exception)	\
	(((self)->vtable->toString)(self, winfp_toString_op, exception))

#define winfp_finalize(self, exception)	\
	(((self)->vtable->finalize)(self, winfp_finalize_op, exception))

#define winfp_LookupFont(self, a, b, c, exception)	\
	(((self)->vtable->LookupFont)(self, winfp_LookupFont_op, a, b, c, exception))

#define winfp_CreateFontFromFile(self, a, b, c, d, exception)	\
	(((self)->vtable->CreateFontFromFile)(self, winfp_CreateFontFromFile_op, a, b, c, d, exception))

#define winfp_CreateFontStreamHandler(self, a, b, exception)	\
	(((self)->vtable->CreateFontStreamHandler)(self, winfp_CreateFontStreamHandler_op, a, b, exception))

#define winfp_EnumerateSizes(self, a, b, exception)	\
	(((self)->vtable->EnumerateSizes)(self, winfp_EnumerateSizes_op, a, b, exception))

#define winfp_ReleaseFontHandle(self, a, exception)	\
	(((self)->vtable->ReleaseFontHandle)(self, winfp_ReleaseFontHandle_op, a, exception))

#define winfp_GetMatchInfo(self, a, exception)	\
	(((self)->vtable->GetMatchInfo)(self, winfp_GetMatchInfo_op, a, exception))

#define winfp_GetRenderableFont(self, a, b, c, exception)	\
	(((self)->vtable->GetRenderableFont)(self, winfp_GetRenderableFont_op, a, b, c, exception))

#define winfp_Name(self, exception)	\
	(((self)->vtable->Name)(self, winfp_Name_op, exception))

#define winfp_Description(self, exception)	\
	(((self)->vtable->Description)(self, winfp_Description_op, exception))

#define winfp_EnumerateMimeTypes(self, exception)	\
	(((self)->vtable->EnumerateMimeTypes)(self, winfp_EnumerateMimeTypes_op, exception))

#define winfp_ListFonts(self, a, b, exception)	\
	(((self)->vtable->ListFonts)(self, winfp_ListFonts_op, a, b, exception))

#define winfp_ListSizes(self, a, b, exception)	\
	(((self)->vtable->ListSizes)(self, winfp_ListSizes_op, a, b, exception))

#define winfp_CacheFontInfo(self, exception)	\
	(((self)->vtable->CacheFontInfo)(self, winfp_CacheFontInfo_op, exception))

/*******************************************************************************
 * winfp Interface
 ******************************************************************************/

struct netscape_jmc_JMCInterfaceID;
struct java_lang_Object;
struct java_lang_String;
struct netscape_fonts_nfrc;
struct netscape_fonts_nffmi;
struct netscape_jmc_ConstCString;
struct netscape_fonts_nfstrm;
struct netscape_fonts_nfrf;

struct winfpInterface {
	void*	(*getInterface)(struct winfp* self, jint op, const JMCInterfaceID* a, JMCException* *exception);
	void	(*addRef)(struct winfp* self, jint op, JMCException* *exception);
	void	(*release)(struct winfp* self, jint op, JMCException* *exception);
	jint	(*hashCode)(struct winfp* self, jint op, JMCException* *exception);
	jbool	(*equals)(struct winfp* self, jint op, void* obj, JMCException* *exception);
	void*	(*clone)(struct winfp* self, jint op, JMCException* *exception);
	const char*	(*toString)(struct winfp* self, jint op, JMCException* *exception);
	void	(*finalize)(struct winfp* self, jint op, JMCException* *exception);
	void*	(*LookupFont)(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, const char* c, JMCException* *exception);
	void*	(*CreateFontFromFile)(struct winfp* self, jint op, struct nfrc* a, const char* b, const char* c, const char* d, JMCException* *exception);
	struct nfstrm*	(*CreateFontStreamHandler)(struct winfp* self, jint op, struct nfrc* a, const char* b, JMCException* *exception);
	void*	(*EnumerateSizes)(struct winfp* self, jint op, struct nfrc* a, void* b, JMCException* *exception);
	jint	(*ReleaseFontHandle)(struct winfp* self, jint op, void* a, JMCException* *exception);
	struct nffmi*	(*GetMatchInfo)(struct winfp* self, jint op, void* a, JMCException* *exception);
	struct nfrf*	(*GetRenderableFont)(struct winfp* self, jint op, struct nfrc* a, void* b, jdouble c, JMCException* *exception);
	const char*	(*Name)(struct winfp* self, jint op, JMCException* *exception);
	const char*	(*Description)(struct winfp* self, jint op, JMCException* *exception);
	const char*	(*EnumerateMimeTypes)(struct winfp* self, jint op, JMCException* *exception);
	void*	(*ListFonts)(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, JMCException* *exception);
	void*	(*ListSizes)(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, JMCException* *exception);
	jint	(*CacheFontInfo)(struct winfp* self, jint op, JMCException* *exception);
};

/*******************************************************************************
 * winfp Operation IDs
 ******************************************************************************/

typedef enum winfpOperations {
	winfp_getInterface_op,
	winfp_addRef_op,
	winfp_release_op,
	winfp_hashCode_op,
	winfp_equals_op,
	winfp_clone_op,
	winfp_toString_op,
	winfp_finalize_op,
	winfp_LookupFont_op,
	winfp_CreateFontFromFile_op,
	winfp_CreateFontStreamHandler_op,
	winfp_EnumerateSizes_op,
	winfp_ReleaseFontHandle_op,
	winfp_GetMatchInfo_op,
	winfp_GetRenderableFont_op,
	winfp_Name_op,
	winfp_Description_op,
	winfp_EnumerateMimeTypes_op,
	winfp_ListFonts_op,
	winfp_ListSizes_op,
	winfp_CacheFontInfo_op
} winfpOperations;

/*******************************************************************************
 * Writing your C implementation: "winfp.h"
 * *****************************************************************************
 * You must create a header file named "winfp.h" that implements
 * the struct winfpImpl, including the struct winfpImplHeader
 * as it's first field:
 * 
 * 		#include "Mwinfp.h" // generated header
 * 
 * 		struct winfpImpl {
 * 			winfpImplHeader	header;
 * 			<your instance data>
 * 		};
 * 
 * This header file will get included by the generated module implementation.
 ******************************************************************************/

/* Forward reference to the user-defined instance struct: */
typedef struct winfpImpl	winfpImpl;


/* This struct must be included as the first field of your instance struct: */
typedef struct winfpImplHeader {
	const struct winfpInterface*	vtablewinfp;
	jint		refcount;
} winfpImplHeader;

/*******************************************************************************
 * Instance Casting Macros
 * These macros get your back to the top of your instance, winfp,
 * given a pointer to one of its interfaces.
 ******************************************************************************/

#undef  winfpImpl2nffp
#define winfpImpl2nffp(winfpImplPtr) \
	((nffp*)((char*)(winfpImplPtr) + offsetof(winfpImplHeader, vtablewinfp)))

#undef  nffp2winfpImpl
#define nffp2winfpImpl(nffpPtr) \
	((winfpImpl*)((char*)(nffpPtr) - offsetof(winfpImplHeader, vtablewinfp)))

#undef  winfpImpl2winfp
#define winfpImpl2winfp(winfpImplPtr) \
	((winfp*)((char*)(winfpImplPtr) + offsetof(winfpImplHeader, vtablewinfp)))

#undef  winfp2winfpImpl
#define winfp2winfpImpl(winfpPtr) \
	((winfpImpl*)((char*)(winfpPtr) - offsetof(winfpImplHeader, vtablewinfp)))

/*******************************************************************************
 * Operations you must implement
 ******************************************************************************/


extern JMC_PUBLIC_API(void*)
_winfp_getBackwardCompatibleInterface(struct winfp* self, const JMCInterfaceID* iid,
	JMCException* *exception);

extern JMC_PUBLIC_API(void)
_winfp_init(struct winfp* self, JMCException* *exception, struct nffbp* broker);

extern JMC_PUBLIC_API(void*)
_winfp_getInterface(struct winfp* self, jint op, const JMCInterfaceID* a, JMCException* *exception);

extern JMC_PUBLIC_API(void)
_winfp_addRef(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(void)
_winfp_release(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(jint)
_winfp_hashCode(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(jbool)
_winfp_equals(struct winfp* self, jint op, void* obj, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_clone(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(const char*)
_winfp_toString(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(void)
_winfp_finalize(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_LookupFont(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, const char* c, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_CreateFontFromFile(struct winfp* self, jint op, struct nfrc* a, const char* b, const char* c, const char* d, JMCException* *exception);

extern JMC_PUBLIC_API(struct nfstrm*)
_winfp_CreateFontStreamHandler(struct winfp* self, jint op, struct nfrc* a, const char* b, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_EnumerateSizes(struct winfp* self, jint op, struct nfrc* a, void* b, JMCException* *exception);

extern JMC_PUBLIC_API(jint)
_winfp_ReleaseFontHandle(struct winfp* self, jint op, void* a, JMCException* *exception);

extern JMC_PUBLIC_API(struct nffmi*)
_winfp_GetMatchInfo(struct winfp* self, jint op, void* a, JMCException* *exception);

extern JMC_PUBLIC_API(struct nfrf*)
_winfp_GetRenderableFont(struct winfp* self, jint op, struct nfrc* a, void* b, jdouble c, JMCException* *exception);

extern JMC_PUBLIC_API(const char*)
_winfp_Name(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(const char*)
_winfp_Description(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(const char*)
_winfp_EnumerateMimeTypes(struct winfp* self, jint op, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_ListFonts(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, JMCException* *exception);

extern JMC_PUBLIC_API(void*)
_winfp_ListSizes(struct winfp* self, jint op, struct nfrc* a, struct nffmi* b, JMCException* *exception);

extern JMC_PUBLIC_API(jint)
_winfp_CacheFontInfo(struct winfp* self, jint op, JMCException* *exception);

/*******************************************************************************
 * Factory Operations
 ******************************************************************************/

JMC_PUBLIC_API(winfp*)
winfpFactory_Create(JMCException* *exception, struct nffbp* broker);

/******************************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _Mwinfp_H_ */
