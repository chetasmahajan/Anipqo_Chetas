// $Id: List.h,v 1.3 2003/05/30 16:25:03 arvind Exp arvind $
// list data structure

#ifndef __LIST_H__
#define __LIST_H__

#include <assert.h>
#include <iostream>
#include <stdlib.h>

// #include "Config.h"

// node in the linked list
template <class T>
class ListEntry_t {
public:
	T data;
	ListEntry_t<T> *next;

	ListEntry_t(T& newdata) : data(newdata), next(NULL) { }
	~ListEntry_t(void) { }
};

// iterator for the list
template <class T>
class ListIter_t;

// prepend only ordered list
template <class T>
class List_t {
protected:
	ListEntry_t<T> *first;		// last element in the list
	int size;				// size of the list

	// instantiate a new list entry
	virtual ListEntry_t<T> *NewListEntry(T& newdata)
	{ return new ListEntry_t<T>(newdata); }

public:
	List_t(void) : first(NULL), size(0) { }

	// is the list empty?
	int IsEmpty(void) const
	{ assert(size >= 0); return size == 0; }

	// insert element into the list
	virtual void Insert(T& newdata) = 0;

	// delete an element at the start of the list
	// does not care if an iterator is sitting on the deleted element
	virtual T DeleteTop(void) = 0;

	// make the list empty
	virtual void MakeEmpty(void)
	{
		ListEntry_t<T> *p = NULL;

		while( first != NULL ) {
			p = first->next;
			delete first;
			first = p;
			size--;
		}
		assert(size == 0);
	}

	int Size(void) const
	{ return size; }

	// delete the elements in the list
	virtual ~List_t(void)
	{ MakeEmpty(); }

	friend class ListIter_t<T>;		// associated iterator class

	int Contains(T& newdata)
	{
		ListEntry_t<T> *p = first;

		while(p) {
			if(newdata == p->data) return 1;
			p = p->next;
		}

		return 0;
	}

	void Reverse(ListEntry_t<T> *node, ListEntry_t<T> *prev)
	{
		assert(node);
		ListEntry_t<T> *next = node->next;
		node->next = prev;

		if(NULL == next) {
			first = node;
			return;
		}

		Reverse(next, node);
	}

	void Reverse(void)
	{
		if(NULL == first) return;
		Reverse(first, NULL);
	}
};

// append only ordered list
template<class T>
class AppendList_t : public List_t<T> {
protected:
	ListEntry_t<T> *last;		// last element in the list
	/* Chetas- Defining virtual function from base class. */
	virtual ListEntry_t<T> *NewListEntry(T& newdata)
	{ return new ListEntry_t<T>(newdata); }

public:
	AppendList_t(void) : last(NULL) { }

	// insert an element
	void Insert(T& newdata)
	{
		ListEntry_t<T> *p = NewListEntry(newdata);

		p->next = NULL;
		if( last )
			last->next = p;
		else List_t<T>::first = p;
		last = p;
		List_t<T>::size++;
	}

	T DeleteTop(void)
	{
		assert(List_t<T>::size > 0);

		ListEntry_t<T> *p = List_t<T>::first;
		List_t<T>::first = List_t<T>::first->next;
		if( !List_t<T>::first )
			last = NULL;

		T data = p->data;

		delete p;
		List_t<T>::size--;
		assert(List_t<T>::size >= 0);

		return data;
	}

	void MakeEmpty(void)
	{
		List_t<T>::MakeEmpty();
		last = NULL;
	}

	virtual ~AppendList_t(void) { }
};

// prepend only ordered list
template<class T>
class PrependList_t : public List_t<T> {
public:
	PrependList_t(void) { }

	// insert element at the start of the list
	void Insert(T& newdata)
	{
		ListEntry_t<T> *p = NewListEntry(newdata);

		// prepend
		p->next = List_t<T>::first;
		List_t<T>::first = p;
		List_t<T>::size++;
	}

	T DeleteTop(void)
	{
		assert(List_t<T>::size > 0);
		assert(List_t<T>::first);

		ListEntry_t<T> *p = List_t<T>::first;
		List_t<T>::first = List_t<T>::first->next;

		T data = p->data;

		delete p;
		List_t<T>::size--;
		assert(List_t<T>::size >= 0);

		return data;
	}

	virtual ~PrependList_t(void) { }
};

// append-only list supporting deletion
template<class T>
class AppendDelList_t : public AppendList_t<T> {
	int ID(const T& x) const;
	int IsEqual(const T& x, const T& y) const;

public:
	// delete an arbitrary element of the list --- assumes an
	// IsEqual(T, T) function exists
	void Delete(T& data)
	{
		ListEntry_t<T> *prev = NULL;
		ListEntry_t<T> *cur = List_t<T>::first;

		while( cur != NULL ) {
			if( IsEqual(data, cur->data) )
				break;

			prev = cur;
			cur = cur->next;
		}

		assert(cur);

		ListEntry_t<T> *next = cur->next;
		if( prev )
			prev->next = next;
		else List_t<T>::first = next;
		if( !next )
			AppendList_t<T>::last = prev;

		delete cur;
		AppendList_t<T>::size--;
		assert(List_t<T>::size >= 0);
	}
};

// prepend-only list supporting deletion
template<class T>
class PrependDelList_t : public PrependList_t<T> {
	int ID(const T& x) const;

protected:
	int IsEqual(const T& x, const T& y) const;

public:
	// delete an arbitrary element of the list --- assumes an
	// IsEqual(T, T) function exists
	void Delete(T& data)
	{
		ListEntry_t<T> *prev = NULL;
		ListEntry_t<T> *cur = List_t<T>::first;

		while( cur != NULL ) {
			if( IsEqual(data, cur->data) )
				break;

			prev = cur;
			cur = cur->next;
		}

		assert(cur);

		ListEntry_t<T> *next = cur->next;
		if( prev )
			prev->next = next;
		else List_t<T>::first = next;

		delete cur;
		List_t<T>::size--;
		assert(List_t<T>::size >= 0);
	}
};

// prepend-only list supporting deletion
template<class T>
class ModifyingList_t : public PrependDelList_t<T> {
	int ID(const T& x) const;

public:
	// insert an element in the list --- assumes an
	// IsEqual(T, T) function exists
	void InsertItemAfter(T& newdata, T *insertAfter)
	{
		ListEntry_t<T> *p = NewListEntry(newdata);
		ListEntry_t<T> *cur = List_t<T>::first;

		if(List_t<T>::first == NULL) {
			List_t<T>::first = p;
			p->next = NULL;
			assert(List_t<T>::size == 0);
			List_t<T>::size++;
			return;
		}

		if(insertAfter == NULL) {
			p->next = List_t<T>::first;
			List_t<T>::first = p;
			List_t<T>::size++;
			return;
		}

		while( cur != NULL ) {
			if(PrependDelList_t<T>::IsEqual(*insertAfter, cur->data))
				break;
			cur = cur->next;
		}

		assert(cur);

		p->next = cur->next;
		cur->next = p;

		assert(List_t<T>::size >= 0);
		List_t<T>::size++;
	}

	void InsertItemBefore(T& newdata, T *insertBefore)
	{
		ListEntry_t<T> *p = NewListEntry(newdata);
		ListEntry_t<T> *prev = NULL;
		ListEntry_t<T> *cur = List_t<T>::first;

		if(List_t<T>::first == NULL) {
			List_t<T>::first = p;
			p->next = NULL;
			assert(List_t<T>::size == 0);
			List_t<T>::size++;
			return;
		}

		while( cur != NULL ) {
			if(PrependDelList_t<T>::IsEqual(*insertBefore, cur->data))
				break;

			prev = cur;
			cur = cur->next;
		}

		if(insertBefore == NULL) {
			assert(cur == NULL);
			//	    assert(prev->next = NULL);		--Chetas
			assert(prev->next == NULL);		//--Chetas
			prev->next = p;
			p->next = NULL;
			List_t<T>::size++;
			return;
		}

		assert(cur);
		assert(prev != NULL);

		p->next = prev->next;
		prev->next = p;

		assert(List_t<T>::size >= 0);
		List_t<T>::size++;
	}
};

// iterator for the linked list
template <class T>
class ListIter_t {
	const List_t<T> *list;
	ListEntry_t<T> *cur;

public:
	ListIter_t(void) : list(NULL), cur(NULL)
{ }

	// attach the iterator to a list
	void Attach(const List_t<T> *list_a)
	{
		list = list_a;
		Reset();
	}

	// check for end of list
	int IsEnd(void) const
	{
		assert(list);
		if( !list->first )
			return 1;

		if( cur ) {
			if( !cur->next )
				return 1;
		}

		return 0;
	}

	// next element
	T& Next(void)
	{
		assert(list);
		if( !cur )
			cur = list->first;
		else cur = cur->next;

		assert(cur);
		T& data = cur->data;

		return data;
	}

	// reset to the beginning
	void Reset(void) { cur = NULL; }

	~ListIter_t(void)
	{ }
};

#if 0
int List_t<class T>::Contains(T& newdata)
		{
	int operator== (T &, T &);

	ListIter_t<class T> iter;
	iter.Attach(this);

	while(!iter.IsEnd()) {
		T& data = iter.Next();
		if(data == newdata) return 1;
	}

	return 0;
		}
#endif

#endif    // __LIST_H__
