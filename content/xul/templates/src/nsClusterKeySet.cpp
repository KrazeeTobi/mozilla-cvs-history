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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *   Chris Waterson <waterson@netscape.com>
 */

#include "nsClusterKeySet.h"

PLHashAllocOps nsClusterKeySet::gAllocOps = {
    AllocTable, FreeTable, AllocEntry, FreeEntry };


nsClusterKeySet::nsClusterKeySet()
    : mTable(nsnull)
{
    mHead.mPrev = mHead.mNext = &mHead;

    static const size_t kBucketSizes[] = { sizeof(Entry) };
    static const PRInt32 kNumBuckets = sizeof(kBucketSizes) / sizeof(size_t);
    static const PRInt32 kInitialEntries = 8;

    // Per news://news.mozilla.org/39BEC105.5090206%40netscape.com
    static const PRInt32 kInitialPoolSize = 256;

    mPool.Init("nsClusterKeySet", kBucketSizes, kNumBuckets, kInitialPoolSize);

    mTable = PL_NewHashTable(kInitialEntries,
                             nsClusterKey::HashClusterKey,
                             nsClusterKey::CompareClusterKeys,
                             PL_CompareValues, &gAllocOps, &mPool);

    MOZ_COUNT_CTOR(nsClusterKeySet);
}


nsClusterKeySet::~nsClusterKeySet()
{
    PL_HashTableDestroy(mTable);
    MOZ_COUNT_DTOR(nsClusterKeySet);
}

PRBool
nsClusterKeySet::Contains(const nsClusterKey& aKey)
{
    return nsnull != PL_HashTableLookup(mTable, &aKey);
}

nsresult
nsClusterKeySet::Add(const nsClusterKey& aKey)
{
    PLHashNumber hash = aKey.Hash();
    PLHashEntry** hep = PL_HashTableRawLookup(mTable, hash, &aKey);

    if (hep && *hep)
        return NS_OK; // already had it.

    PLHashEntry* he = PL_HashTableRawAdd(mTable, hep, hash, &aKey, nsnull);
    if (! he)
        return NS_ERROR_OUT_OF_MEMORY;

    Entry* entry = NS_REINTERPRET_CAST(Entry*, he);

    // XXX yes, I am evil. Fixup the key in the hashentry to point to
    // the value it contains, rather than the one on the stack.
    entry->mHashEntry.key = &entry->mKey;

    // thread
    mHead.mPrev->mNext = entry;
    entry->mPrev = mHead.mPrev;
    entry->mNext = &mHead;
    mHead.mPrev = entry;
    
    return NS_OK;
}
