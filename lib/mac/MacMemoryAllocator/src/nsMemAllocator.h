/* -*- Mode: C++;    tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- 
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape 
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */


#include <stddef.h>

#if STATS_MAC_MEMORY
#include "prio.h"
#endif

class nsMemAllocator;
class nsHeapChunk;


enum {
	kFreeBlockHeaderTag		= 'FREE',
	kFreeBlockTrailerTag	= 'free',
	kUsedBlockHeaderTag		= 'USED',
	kUsedBlockTrailerTag	= 'used',
	kDummyBlockHeaderTag  = 'D\'oh',
	kRefdBlockHeaderTag		= 'REFD',
	kRefdBlockTrailerTag	= 'refd',
	kUsedMemoryFillPattern	= 0xDB,
	kFreeMemoryFillPattern 	= 0xEF		//if you don't want to crash hard, change to 0x04 or 0x05,
};


typedef	UInt32	MemoryBlockTag;


struct MemoryBlockHeader
{
	nsHeapChunk					*owningChunk;

#if STATS_MAC_MEMORY
	// it sucks putting an extra data member in here which affects stats, but there is no other
	// way to store the logical size of each block.
	size_t						logicalBlockSize;
#endif

#if DEBUG_HEAP_INTEGRITY
	//MemoryBlockHeader			*next;
	//MemoryBlockHeader			*prev;
	UInt32						blockID;
	MemoryBlockTag				headerTag;			// put this as the last thing before memory
#endif

	static MemoryBlockHeader*	GetHeaderFromBlock(void *block)	 { return (MemoryBlockHeader*)((char *)block - sizeof(MemoryBlockHeader)); }

#if DEBUG_HEAP_INTEGRITY	
	// inline, so won't crash if this is a bad block
	Boolean				HasHeaderTag(MemoryBlockTag inHeaderTag) { return (headerTag == inHeaderTag); }
	void				SetHeaderTag(MemoryBlockTag inHeaderTag) { headerTag = inHeaderTag; }
#else
	// stubs
	Boolean				HasHeaderTag(MemoryBlockTag inHeaderTag){ return true; }
	void				SetHeaderTag(MemoryBlockTag inHeaderTag){}
#endif
									
};


struct MemoryBlockTrailer {
	MemoryBlockTag		trailerTag;
};

#if DEBUG_HEAP_INTEGRITY
#define MEMORY_BLOCK_TAILER_SIZE sizeof(struct MemoryBlockTrailer)
#else
#define MEMORY_BLOCK_TAILER_SIZE 0
#endif	/* DEBUG_HEAP_INTEGRITY */



//--------------------------------------------------------------------
class nsHeapChunk
{
	public:
		
								nsHeapChunk(nsMemAllocator *inOwningAllocator, Size heapSize);
								~nsHeapChunk();
		
		nsHeapChunk*			GetNextChunk() const	{ return mNextChunk; }
		void					SetNextChunk(nsHeapChunk *inChunk)	{ mNextChunk = inChunk;	}
		
		nsMemAllocator*			GetOwningAllocator() const { return mOwningAllocator; }

		void					IncrementUsedBlocks()	{	mUsedBlocks ++;	}
		void					DecrementUsedBlocks()	{	mUsedBlocks -- ;  MEM_ASSERT(mUsedBlocks >= 0, "Bad chunk block count"); }
		
		Boolean					IsEmpty() const			{ return mUsedBlocks == 0;	}
		
		UInt32					GetChunkSize()			{ return mHeapSize;	}
		
#if DEBUG_HEAP_INTEGRITY	
		Boolean					IsGoodChunk()			{ return mSignature == kChunkSignature; }
#endif
		
	protected:
#if DEBUG_HEAP_INTEGRITY
		enum {
			kChunkSignature = 'Chnk'
		};

		OSType					mSignature;
#endif
		nsMemAllocator			*mOwningAllocator;
		nsHeapChunk				*mNextChunk;
		UInt32					mUsedBlocks;
		UInt32					mHeapSize;		
};




//--------------------------------------------------------------------
class nsMemAllocator
{
	public:

								nsMemAllocator(size_t minBlockSize, size_t maxBlockSize);
		virtual 				~nsMemAllocator() = 0;
		
		static size_t			GetBlockSize(void *thisBlock);
		static nsMemAllocator*	GetAllocatorFromBlock(void *thisBlock);
		
		virtual void *			AllocatorMakeBlock(size_t blockSize) = 0;
		virtual void 			AllocatorFreeBlock(void *freeBlock) = 0;
		// AllocatorResizeBlock can return nil if the block cannot be resized in place.
		// realloc then does a malloc and copy. If this returns a valid block, then it
		// is assumed that no coying needs to be done
		virtual void *			AllocatorResizeBlock(void *block, size_t newSize) = 0;
		virtual size_t 			AllocatorGetBlockSize(void *thisBlock) = 0;
		
		virtual nsHeapChunk*	AllocateChunk(size_t requestedBlockSize) = 0;
		virtual void			FreeChunk(nsHeapChunk *chunkToFree) = 0;
		
		Boolean					IsGoodAllocator() 
#if DEBUG_HEAP_INTEGRITY
									{ return mSignature == kMemAllocatorSignature; }
#else
									{ return true; }
#endif

	protected:
				
		void					AddToChunkList(nsHeapChunk *inNewChunk);
		void					RemoveFromChunkList(nsHeapChunk *inChunk);
		
		enum {
			kMemAllocatorSignature = 'ARSE'				// Allocators R Supremely Efficient
		};
		
#if DEBUG_HEAP_INTEGRITY
		OSType					mSignature;				// signature for debugging
#endif

		nsHeapChunk				*mFirstChunk;			// pointer to first subheap managed by this allocator
		nsHeapChunk				*mLastChunk;			// pointer to last subheap managed by this allocator

		UInt32					mMinBlockSize;			// smallest block normally handled by this allocator (inclusive)
		UInt32					mMaxBlockSize;			// largest block handled by this allocator (inclusive)
		
		UInt32					mNumChunks;				// number of chunks in list

		UInt32					mBaseChunkSize;			// size of subheap allocated at startup
		UInt32					mTempChunkSize;			// size of additional subheaps


#if STATS_MAC_MEMORY
		
	public:
	
		void					AccountForNewBlock(size_t logicalSize);
		void					AccountForFreedBlock(size_t logicalSize);
		void					AccountForResizedBlock(size_t oldLogicalSize, size_t newLogicalSize);
		
		void					DumpMemoryStats(PRFileDesc *statsFile);
		void					DumpHeapUsage(PRFileDesc *statsFile);
		
		UInt32					GetMaxHeapUsage() { return mMaxHeapSpaceUsed; }
		
	private:
	
		UInt32					mCurBlockCount;			// number of malloc blocks allocated now
		UInt32					mMaxBlockCount;			// max number of malloc blocks allocated
		
		UInt32					mCurBlockSpaceUsed;		// sum of logical size of allocated blocks
		UInt32					mMaxBlockSpaceUsed;		// max of sum of logical size of allocated blocks
		
		UInt32					mCurHeapSpaceUsed;		// sum of physical size of allocated chunks
		UInt32					mMaxHeapSpaceUsed;		// max of sum of logical size of allocated chunks
		
		UInt32					mCurSubheapCount;		// current number of subheaps allocated by this allocator
		UInt32					mMaxSubheapCount;		// max number of subheaps allocated by this allocator
		
		UInt32*         mCountHistogram;    // cumulative hist of frequencies
		
		// the difference between mCurBlockSpaceUsed and mCurHeapSpaceUsed is
		// the allocator overhead, which consists of:
		//
		//	1. Block overhead (rounding, headers & trailers)
		//	2. Unused block space in chunks
		//	3. Chunk overhead (rounding, headers & trailers)
		//
		//	This is a reasonable measure of the space efficiency of these allocators
#endif

};


