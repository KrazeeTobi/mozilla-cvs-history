// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// The contents of this file are subject to the Netscape Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy of
// the License at http://www.mozilla.org/NPL/
//
// Software distributed under the License is distributed on an "AS
// IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
// implied. See the License for the specific language governing
// rights and limitations under the License.
//
// The Original Code is the JavaScript 2 Prototype.
//
// The Initial Developer of the Original Code is Netscape
// Communications Corporation.  Portions created by Netscape are
// Copyright (C) 1998 Netscape Communications Corporation. All
// Rights Reserved.

#ifndef hash_h
#define hash_h

#include "utilities.h"

namespace JavaScript {


//
// Hash Codes
//

	typedef uint32 HashNumber;

	HashNumber hashString(const char *s);
	HashNumber hashString(const String &s);


	template<class Key>
	struct Hash {
		HashNumber operator()(Key key) const;
	};

	template<class Key>
	inline HashNumber Hash<Key>::operator()(Key key) const
	{
		return hashString(key);
	}


	const HashNumber goldenRatio = 0x9E3779B9U;


//
// Private
//

	// Base class for user-defined hash entries.
	// private
	class GenericHashEntry {
	  public:
	    GenericHashEntry *next;         // Link to next entry in the same bucket
	    const HashNumber keyHash;       // This entry's hash value

	  protected:
		explicit GenericHashEntry(HashNumber keyHash): next(0), keyHash(keyHash) {}

	    friend class GenericHashTable;
	};


	// private
	class GenericHashTableIterator;
	class GenericHashTable {
	  protected:
	    GenericHashEntry **buckets;		// Vector of hash buckets
	    GenericHashEntry **bucketsEnd;	// Pointer past end of vector of hash buckets
	    uint defaultLgNBuckets;			// lg2 of minimum number of buckets for which to size the table
	    uint32 nEntries;				// Number of entries in table
	    uint32 minNEntries;				// Minimum number of entries without rehashing
	    uint32 maxNEntries;				// Maximum number of entries without rehashing
	    uint32 shift;					// 32 - lg2(number of buckets)
	  #ifdef DEBUG
	  public:
	    uint32 nReferences;				// Number of iterators and references currently pointing to this hash table
	  #endif

	  public:
		explicit GenericHashTable(uint32 nEntriesDefault);
		~GenericHashTable() {ASSERT(nReferences == 0); delete buckets;}

		void recomputeMinMaxNEntries(uint lgNBuckets);
		void rehash();
		void maybeGrow() {if (nEntries > maxNEntries) rehash();}
		void maybeShrink() {if (nEntries < minNEntries) rehash();}
		
		friend class GenericHashTableIterator;

		typedef GenericHashTableIterator Iterator;
	};

	// This ought to be GenericHashTable::Iterator, but this doesn't work due to a
	// Microsoft VC6 bug.
	class GenericHashTableIterator {
	  protected:
		GenericHashTable &ht;			// Hash table being iterated
		GenericHashEntry *entry;		// Current entry; nil if done
		GenericHashEntry **backpointer;	// Pointer to pointer to current entry
		GenericHashEntry **nextBucket;	// Next bucket; pointer past end of vector of hash buckets if done
	  public:
		explicit GenericHashTableIterator(GenericHashTable &ht);
		~GenericHashTableIterator() {ht.maybeShrink(); DEBUG_ONLY(--ht.nReferences);}

		operator bool() const {return entry != 0;}	// Return true if there are entries left.
		GenericHashTableIterator &operator++();
	};


//
// Hash Tables
//

	template<class Data, class Key, class H = Hash<Key> >
	class HashTable: private GenericHashTable {
		H hasher;						// Hash function

		struct Entry: public GenericHashEntry {
			Data data;
			
			Entry(HashNumber keyHash, Key key): GenericHashEntry(keyHash), data(key) {}
			template<class Value>
			Entry(HashNumber keyHash, Key key, Value value): GenericHashEntry(keyHash), data(key, value) {}
		};

	  public:
		class Reference {
#ifdef _WIN32 // Microsoft VC6 bug: friend declarations to inner classes don't work
		  public:
#endif
			Entry *entry;						// Current entry; nil if done
			GenericHashEntry **backpointer;		// Pointer to pointer to current entry
			const HashNumber keyHash;			// This entry's key's hash value
		  #ifdef DEBUG
			GenericHashTable *ht;				// Hash table to which this Reference points
		  #endif

		  public:
#ifndef _WIN32
			Reference(HashTable &ht, Key key);	// Search for an entry with the given key.
#else // Microsoft VC6 bug: VC6 doesn't allow this to be defined outside the class
			Reference(HashTable &ht, Key key): keyHash(ht.hasher(key)) {
			#ifdef DEBUG
				Reference::ht = &ht;
				++ht.nReferences;
			#endif
				HashNumber kh = keyHash;
				HashNumber h = kh*goldenRatio >> ht.shift;
				GenericHashEntry **bp = ht.buckets + h;
				Entry *e;

				while ((e = static_cast<Entry *>(*bp)) != 0 && !(e->keyHash == kh && e->data == key))
					bp = &e->next;
				entry = e;
				backpointer = bp;
			}
#endif
		  private:
		    Reference(const Reference&);		// No copy constructor
		    void operator=(const Reference&);	// No assignment operator
		  public:
		  #ifdef DEBUG
			~Reference() {if (ht) --ht->nReferences;}
		  #endif
			
			operator bool() const {return entry != 0;}	// Return true if an entry was found.
			Data &operator*() const {ASSERT(entry); return entry->data;}	// Return the data of the entry that was found.
			
			friend class HashTable;
		};

		class Iterator: public GenericHashTableIterator {
		  public:
			explicit Iterator(HashTable &ht): GenericHashTableIterator(ht) {}
		  private:
		    Iterator(const Iterator&);			// No copy constructor
		    void operator=(const Iterator&);	// No assignment operator
		  public:

			// Go to next entry.
			Iterator &operator++() {return *static_cast<Iterator*>(&GenericHashTableIterator::operator++());}
			Data &operator*() const {ASSERT(entry); return static_cast<Entry *>(entry)->data;}	// Return current entry's data.
			void erase();
		};

		HashTable(uint32 nEntriesDefault = 0, const H &hasher = H()): GenericHashTable(nEntriesDefault), hasher(hasher) {}
		~HashTable();

		template<class Value> Data &insert(Reference &r, Key key, Value value);
		Data &insert(Reference &r, Key key);
		Data &insert(Key key);
		void erase(Reference &r);
		void erase(Key key);
		Data *operator[](Key key);
		
		friend class Reference;
		friend class Iterator;

#ifndef _WIN32
		template<class Value> Data &insert(Key key, Value value);
#else // Microsoft VC6 bug: VC6 doesn't allow this to be defined outside the class
		template<class Value> Data &insert(Key key, Value value) {
			Reference r(*this, key);
			if (r)
				return *r = value;
			else
				return insert(r, key, value);
		}
#endif
	};


//
// Implementation
//

	template<class Data, class Key, class H>
	HashTable<Data, Key, H>::~HashTable()
	{
		GenericHashEntry **be = bucketsEnd;
		for (GenericHashEntry **b = buckets; b != be; b++) {
			Entry *e = static_cast<Entry *>(*b);
			while (e) {
				Entry *next = static_cast<Entry *>(e->next);
				delete e;
				e = next;
			}
		}
	}


#ifndef _WIN32
	template<class Data, class Key, class H>
	HashTable<Data, Key, H>::Reference::Reference(HashTable &ht, Key key):
		keyHash(ht.hasher(key))
	{
	#ifdef DEBUG
		Reference::ht = &ht;
		++ht.nReferences;
	#endif
		HashNumber kh = keyHash;
		HashNumber h = kh*goldenRatio >> ht.shift;
		GenericHashEntry **bp = ht.buckets + h;
		Entry *e;

		while ((e = static_cast<Entry *>(*bp)) != 0 && !(e->keyHash == kh && e->data == key))
			bp = &e->next;
		entry = e;
		backpointer = bp;
	}


	// Insert the given key/value pair into the hash table.  Reference must
	// be the result of an unsuccessful search for that key in the table.
	// The reference is not valid after this method is called.
	// Return a reference to the new entry's value.
	template<class Data, class Key, class H> template<class Value>
	inline Data &HashTable<Data, Key, H>::insert(Reference &r, Key key, Value value)
	{
		ASSERT(r.ht == this && !r.entry);
		Entry *e = new Entry(r.keyHash, key, value);
		*r.backpointer = e;
		++nEntries;
		maybeGrow();
	#ifdef DEBUG
		--r.ht->nReferences;
		r.ht = 0;
	#endif
		return e->data;
	}
#endif

	// Same as above but without a Value argument.
	template<class Data, class Key, class H>
	inline Data &HashTable<Data, Key, H>::insert(Reference &r, Key key)
	{
		ASSERT(r.ht == this && !r.entry);
		Entry *e = new Entry(r.keyHash, key);
		*r.backpointer = e;
		++nEntries;
		maybeGrow();
	#ifdef DEBUG
		--r.ht->nReferences;
		r.ht = 0;
	#endif
		return e->data;
	}



	// Insert the given key/value pair into the hash table.  If an entry with a
	// matching key already exists, replace that entry's value.
	// Return a reference to the new entry's value.
#ifndef _WIN32 // Microsoft VC6 bug: VC6 doesn't allow this to be defined outside the class
	template<class Data, class Key, class H> template<class Value>
	Data &HashTable<Data, Key, H>::insert(Key key, Value value)
	{
		Reference r(*this, key);
		if (r)
			return *r = value;
		else
			return insert(r, key, value);
	}
#endif

	// Same as above but without a Value argument.
	template<class Data, class Key, class H>
	Data &HashTable<Data, Key, H>::insert(Key key)
	{
		Reference r(*this, key);
		if (r)
			return *r;
		else
			return insert(r, key);
	}


	// Reference r must point to an existing entry.  Delete that entry.
	// The reference is not valid after this method is called.
	template<class Data, class Key, class H>
	inline void HashTable<Data, Key, H>::erase(Reference &r)
	{
		Entry *e = r.entry;
		ASSERT(r.ht == this && e);
		*r.backpointer = e->next;
		--nEntries;
		delete e;
	#ifdef DEBUG
		--r.ht->nReferences;
		r.ht = 0;
	#endif
		maybeShrink();
	}


	// Remove the hash table entry, if any, matching the given key.
	template<class Data, class Key, class H>
	void HashTable<Data, Key, H>::erase(Key key)
	{
		Reference r(*this, key);
		if (r)
			erase(r);
	}


	// Return a pointer to the value of the hash table entry matching the given key.
	// Return nil if no entry matches.
	template<class Data, class Key, class H>
	Data *HashTable<Data, Key, H>::operator[](Key key)
	{
		Reference r(*this, key);
		if (r)
			return &*r;
		else
			return 0;
	}

}

#endif
