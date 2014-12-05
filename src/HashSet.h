#ifndef __HASHSET_H__
#define __HASHSET_H__

#include <stdio.h>
#include "TypeDefs.h"
#include "other/List.h"

class Point_t;

class HashSet_t {
	static int nBuckets;
	Point_t **table;

	static int Hash(RealCoord_t *array);
	static int Hash(IntCoord_t *array);

public:
	// HashSet_t(int size = 100) : nBuckets(size)
	HashSet_t(void) {
		assert(0 < nBuckets);

		table = new Point_t *[nBuckets];
		assert(table);

		for (int i = 0; i < nBuckets; i++) table[i] = NULL;
	}

	Boolean_t Put(Point_t *p, bool toCheck = true);
	Point_t *Get(RealCoord_t *realCoord);
	Point_t *Get(IntCoord_t *intCoord);
	void Remove(RealCoord_t *realCoord);		//Chetas- Added for correct version.
	void Remove(IntCoord_t *intCoord);			//Chetas- Added for correct version.
	void freeHashset(void);						//Chetas- For freeing memory.
	void freeHashset(AppendDelList_t<Point_t *> *list);		//Chetas- For freeing memory.

	~HashSet_t(void);
};

#endif // __HASHSET_H__
