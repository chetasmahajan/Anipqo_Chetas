#ifndef __MYLIST_H__
#define __MYLIST_H__

#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include "TypeDefs.h"

#define EXPAND_FACTOR 10

template <class T>
class MyList_t {
	int numEntries;
	int numEntriesSet;
	T *entries;

public:
	MyList_t(int ne = 0, MyList_t<T> *newList = NULL)
: numEntries(ne), numEntriesSet(0), entries(NULL) {
		assert(0 <= numEntries);

		if(0 == numEntries) {
			assert(NULL == newList);
			return;
		}

		entries = new T[numEntries];
		assert(entries);

		if(NULL != newList) {
			assert(newList->numEntriesSet <= numEntries);
			numEntriesSet = newList->numEntriesSet;
			for(int i = 0 ; i < numEntriesSet; i++) entries[i] = newList->entries[i];
		}
	}

	int NumEntries(void) { return numEntries; }
	int NumEntriesSet(void) { return numEntriesSet; }
	int Size(void) { return numEntriesSet; }
	int IsEmpty(void) { return (0 == numEntriesSet); }
	int MakeEmpty(void) { return numEntriesSet = 0; }

	// Inserts the elements in order; not necessaty; simplify?
	void Merge(MyList_t<T>& newList, int maxDistinctEntries) {
		int i;
		assert(numEntriesSet <= maxDistinctEntries);
		assert(newList.numEntriesSet <= maxDistinctEntries);

		Boolean_t *bitVector = new Boolean_t[maxDistinctEntries+1];
		for(i = 0; i <= maxDistinctEntries; i++) bitVector[i] = FALSE;

		for(i = 0; i < numEntriesSet; i++) {
			assert(entries[i] <= maxDistinctEntries);
			bitVector[entries[i]] = TRUE;
		}

		for(i = 0; i < newList.numEntriesSet; i++) {
			assert(newList.entries[i] <= maxDistinctEntries);
			bitVector[newList.entries[i]] = TRUE;
		}

		int numSetBits = 0;
		for(i = 0; i <= maxDistinctEntries; i++) if(TRUE == bitVector[i]) numSetBits++;

		if(numEntries < numSetBits) {
			int div = numSetBits / EXPAND_FACTOR;
			int mod = numSetBits % EXPAND_FACTOR;
			assert(0 <= div); assert(0 <= mod); assert(mod < EXPAND_FACTOR);
			if(0 < mod) div++;

			ReallocateList(div*EXPAND_FACTOR, FALSE);
		}

		for(i = 0; i < numEntries; i++) entries[i] = -1;

		numEntriesSet = 0;
		for(i = 0; i <= maxDistinctEntries; i++) {
			if(TRUE == bitVector[i]) entries[numEntriesSet++] = i;
		}

		assert(numEntriesSet <= numEntries);
		delete bitVector;
	}

	Boolean_t Contains(T& entry) {
		for(int i = 0; i < numEntriesSet; i++)
			if(entries[i] == entry) return TRUE;
		return FALSE;
	}

	void Append(T& entry) {
		assert(numEntriesSet <= numEntries);
		assert(FALSE == Contains(entry));

		if(numEntriesSet == numEntries) ReallocateList(numEntries+EXPAND_FACTOR);
		assert(numEntriesSet < numEntries);
		entries[numEntriesSet] = entry;
		numEntriesSet++;
	}

	void Insert(T& entry, int pos) {
		assert(numEntriesSet <= numEntries);
		assert(pos <= numEntriesSet);
		assert(FALSE == Contains(entry));

		if(pos == numEntriesSet) {
			Append(entry);
			return;
		}

		assert(pos < numEntriesSet);
		if(numEntriesSet == numEntries)
			ReallocateList(numEntries+EXPAND_FACTOR);
		assert(numEntriesSet < numEntries);

		for(int i = numEntriesSet; pos < i; i--)
			entries[i] = entries[i-1];
		entries[pos] = entry;
		numEntriesSet++;
	}

	T& Entry(int entryNum)
	{
		assert(0 <= entryNum);
		assert(entryNum < numEntriesSet);
		assert(numEntriesSet <= numEntries);
		return entries[entryNum];
	}

	void SetEntry(int entryNum, T& newEntry)
	{
		assert(0 <= entryNum);
		assert(entryNum < numEntriesSet);
		assert(numEntriesSet <= numEntries);
		//	assert(pcp);								//pcp is unknown -Chetas
		assert(FALSE == Contains(newEntry));

		entries[entryNum] = newEntry;
	}

	void DeleteEntryNum(int entryNum)
	{
		assert(0 <= entryNum);
		assert(entryNum < numEntriesSet);
		assert(numEntriesSet <= numEntries);

		for(int i = entryNum; i < numEntriesSet; i++) entries[i] = entries[i+1];
		numEntriesSet--;
	}

	void DeleteEntry(T& entryTBD)
	{
		int entryNum;
		for(entryNum = 0; entryNum < numEntriesSet; entryNum++)
			//	    if(newEntryTBD == entries[entryNum]) break;		//newEntryTBD not found in this context. -Chetas
			if(entryTBD == entries[entryNum])						//Added entryTBD instead of newEntryTBD. -Chetas
				break;

		assert(0 <= entryNum);
		assert(entryNum < numEntriesSet);
		assert(numEntriesSet <= numEntries);

		for(int i = entryNum; i < numEntriesSet; i++) entries[i] = entries[i+1];
		numEntriesSet--;
	}

	void ReallocateList(int newNumEntries, Boolean_t do_not_initilize = FALSE) {
		assert(numEntries < newNumEntries);

		T *oldEntries = entries;

		entries = new T[newNumEntries];
		assert(entries);

		if(do_not_initilize) return;

		if(NULL != oldEntries) {
			for(int i = 0; i < numEntriesSet; i++) entries[i] = oldEntries[i];
			delete oldEntries;
		}
		else {
			assert(0 == numEntries);
			assert(0 == numEntriesSet);
		}

		numEntries = newNumEntries;
	}

	~MyList_t(void)
	{ if(entries) delete entries; }
};

#endif // __MYLIST_H__
