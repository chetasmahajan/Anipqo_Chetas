#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>

#include "HashSet.h"
#include "Point.h"

int HashSet_t::nBuckets = 100;

int HashSet_t::Hash(RealCoord_t *array)
{
	assert(array);

	int sum = 0;
//	int size = Nipqo_t::NumParam();		//Chetas- Commented unused.
	for(int i = 0; i < Nipqo_t::NumParam(); i++)
		// Do mod at the end?
		sum = (sum + Nipqo_t::GetPredConst(array[i], i)) % nBuckets;

	assert(sum < nBuckets);
	return sum;
}

int HashSet_t::Hash(IntCoord_t *array)
{
	assert(array);

	int sum = 0;
	for(int i = 0; i < Nipqo_t::NumParam(); i++)
		// Do mod at the end?
		sum = (sum + array[i]) % nBuckets;

	assert(sum < nBuckets);
	return sum;
}

Boolean_t HashSet_t::Put(Point_t *p, bool toCheck)	//true		//Chetas- Added for Correct version.
{
	assert(p);

	Point_t *node;//, *temp;		//Chetas- Commented unused.
	int h = Hash(p->GetIntCoordArray());

	for (node = table[h]; node != NULL; node = node->GetNext()) {
		if(toCheck && node->IsEqual(*p)) {		//Chetas- This may create issue in future when any false positive partitioning vertex is to be added.
			cout<<"\nHashet::Put error\n";
			assert(0);
			return FALSE;
		}
	}

	p->SetNext(table[h]);
	table[h] = p;
	return TRUE;
}

Point_t *HashSet_t::Get(RealCoord_t *realCoord)
{
	assert(realCoord);

	Point_t *node;
	int h = Hash(realCoord);

	for (node = table[h]; node != NULL; node = node->GetNext())
		if(node->IsEqual(realCoord)) return node;

	return NULL;
}

Point_t *HashSet_t::Get(IntCoord_t *intCoord)
{
	assert(intCoord);

	Point_t *node;
	int h = Hash(intCoord);

	for (node = table[h]; node != NULL; node = node->GetNext())
		if (node->IsEqual(intCoord)) return node;

	return NULL;
}

//Chetas- Added for correct version.
void HashSet_t::Remove(RealCoord_t *realCoord)
{
	assert(realCoord);

	Point_t *node, *prev;
	int h = Hash(realCoord);

	node = table[h];
	assert(node != NULL);

	if(node->IsEqual(realCoord)) {
		table[h] = node->GetNext();
		node->SetNext(NULL);
		return;
	}

	prev = node;
	for (node = node->GetNext(); node != NULL; node = node->GetNext()) {
		if(node->IsEqual(realCoord)) {
			prev->SetNext(node->GetNext());
			return;
		}
		prev = node;
	}
	cout<<"\nHashset::Remove error\n";
	assert(0);
}

//Chetas- Added for correct version.
void HashSet_t::Remove(IntCoord_t *intCoord)
{
	assert(intCoord);

	Point_t *node, *prev;
	int h = Hash(intCoord);

	node = table[h];
	assert(node != NULL);

	if(node->IsEqual(intCoord)) {
		table[h] = node->GetNext();
		node->SetNext(NULL);
		return;
	}

	prev = node;
	for (node = node->GetNext(); node != NULL; node = node->GetNext()) {
		if(node->IsEqual(intCoord)) {
			prev->SetNext(node->GetNext());
			return;
		}
		prev = node;
	}
	cout<<"\nHashset::Remove error\n";
	assert(0);
}

HashSet_t::~HashSet_t(void)
{

}

void HashSet_t::freeHashset(void)
{
	for (int i = 0; i < nBuckets; i++) {
		if(table[i] == NULL)
			continue;
		Point_t *pt = table[i];
		while(pt != NULL) {
			Point_t *tmp = pt->GetNext();
			delete pt;
			pt = tmp;
		}
	}
	delete[] table;
}

void HashSet_t::freeHashset(AppendDelList_t<Point_t *> *list) {
	for (int i = 0; i < nBuckets; i++) {
		if(table[i] == NULL)
			continue;
		Point_t *pt = table[i];
		while(pt != NULL) {
			if(!list->Contains(pt)) {
				Remove(pt->GetIntCoordArray());
				delete pt;
				pt = table[i];
			}
			else
				pt = pt->GetNext();
		}
	}
}
