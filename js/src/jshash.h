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

#ifndef jshash_h___
#define jshash_h___
/*
 * API to portable hash table code.
 */
#include <stddef.h>
#include <stdio.h>
#include "jstypes.h"
#include "jscompat.h"

JS_BEGIN_EXTERN_C

typedef uint32 JSHashNumber;
typedef struct JSHashEntry JSHashEntry;
typedef struct JSHashTable JSHashTable;

#define JS_HASH_BITS 32
#define JS_GOLDEN_RATIO 0x9E3779B9U

typedef JSHashNumber (*JSHashFunction)(const void *key);
typedef intN (*JSHashComparator)(const void *v1, const void *v2);
typedef intN (*JSHashEnumerator)(JSHashEntry *he, intN i, void *arg);

/* Flag bits in JSHashEnumerator's return value */
#define HT_ENUMERATE_NEXT       0       /* continue enumerating entries */
#define HT_ENUMERATE_STOP       1       /* stop enumerating entries */
#define HT_ENUMERATE_REMOVE     2       /* remove and free the current entry */
#define HT_ENUMERATE_UNHASH     4       /* just unhash the current entry */

typedef struct JSHashAllocOps {
    void *              (*allocTable)(void *pool, size_t size);
    void                (*freeTable)(void *pool, void *item);
    JSHashEntry *       (*allocEntry)(void *pool, const void *key);
    void                (*freeEntry)(void *pool, JSHashEntry *he, uintN flag);
} JSHashAllocOps;

#define HT_FREE_VALUE   0               /* just free the entry's value */
#define HT_FREE_ENTRY   1               /* free value and entire entry */

struct JSHashEntry {
    JSHashEntry         *next;          /* hash chain linkage */
    JSHashNumber          keyHash;        /* key hash function result */
    const void          *key;           /* ptr to opaque key */
    void                *value;         /* ptr to opaque value */
};

struct JSHashTable {
    JSHashEntry         **buckets;      /* vector of hash buckets */
    uint32              nentries;       /* number of entries in table */
    uint32              shift;          /* multiplicative hash shift */
    JSHashFunction      keyHash;        /* key hash function */
    JSHashComparator    keyCompare;     /* key comparison function */
    JSHashComparator    valueCompare;   /* value comparison function */
    JSHashAllocOps      *allocOps;      /* allocation operations */
    void                *allocPriv;     /* allocation private data */
#ifdef HASHMETER
    uint32              nlookups;       /* total number of lookups */
    uint32              nsteps;         /* number of hash chains traversed */
    uint32              ngrows;         /* number of table expansions */
    uint32              nshrinks;       /* number of table contractions */
#endif
};

/*
 * Create a new hash table.
 * If allocOps is null, use default allocator ops built on top of malloc().
 */
extern JS_PUBLIC_API(JSHashTable *)
JS_NewHashTable(uint32 n, JSHashFunction keyHash,
                JSHashComparator keyCompare, JSHashComparator valueCompare,
                JSHashAllocOps *allocOps, void *allocPriv);

extern JS_PUBLIC_API(void)
JS_HashTableDestroy(JSHashTable *ht);

/* Low level access methods */
extern JS_PUBLIC_API(JSHashEntry **)
JS_HashTableRawLookup(JSHashTable *ht, JSHashNumber keyHash, const void *key);

extern JS_PUBLIC_API(JSHashEntry *)
JS_HashTableRawAdd(JSHashTable *ht, JSHashEntry **hep, JSHashNumber keyHash,
                   const void *key, void *value);

extern JS_PUBLIC_API(void)
JS_HashTableRawRemove(JSHashTable *ht, JSHashEntry **hep, JSHashEntry *he);

/* Higher level access methods */
extern JS_PUBLIC_API(JSHashEntry *)
JS_HashTableAdd(JSHashTable *ht, const void *key, void *value);

extern JS_PUBLIC_API(JSBool)
JS_HashTableRemove(JSHashTable *ht, const void *key);

extern JS_PUBLIC_API(intN)
JS_HashTableEnumerateEntries(JSHashTable *ht, JSHashEnumerator f, void *arg);

extern JS_PUBLIC_API(void *)
JS_HashTableLookup(JSHashTable *ht, const void *key);

extern JS_PUBLIC_API(intN)
JS_HashTableDump(JSHashTable *ht, JSHashEnumerator dump, FILE *fp);

/* General-purpose C string hash function. */
extern JS_PUBLIC_API(JSHashNumber)
JS_HashString(const void *key);

/* Stub function just returns v1 == v2 */
extern JS_PUBLIC_API(intN)
JS_CompareValues(const void *v1, const void *v2);

JS_END_EXTERN_C

#endif /* jshash_h___ */
