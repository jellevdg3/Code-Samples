#pragma once

#include "LinkedListNode.h"
#include "LinkedListIterator.h"

// LinkedList; usage: LinkedList<SomeClass> list;
// Iterator: 	LinkedListIterator<SomeClass*> it = list.getIterator();
// 				while (it.hasNext())
//				{
//					SomeClass* obj = it.next();
//				}
template<class T>
class LinkedList
{
public:
	// Constructor
	LinkedList();
	
	// Destructor
	~LinkedList();

	// Add an item to the list
	void add(T obj);
	
	// Remove an item from the list
	void remove(T obj);
	
	// Get amount of the elements in the list
	int getLength();

	// Obtain the iterator to iterate the list
	LinkedListIterator<T>& getIterator();

private:
	int count;
	LinkedListNode<T>* firstNode;
	LinkedListNode<T>* lastNode;
	LinkedListIterator<T>* it;
};

// Constructor
template<class T>
LinkedList<T>::LinkedList()
{
	count = 0;
	firstNode = 0;
	lastNode = 0;

	it = new LinkedListIterator<T>();
}

// Destructor
template<class T>
LinkedList<T>::~LinkedList()
{
	delete this->it;

	count = 0;
}

// Add an item to the list
template<class T>
void LinkedList<T>::add(T obj)
{
	LinkedListNode<T>* newNode = new LinkedListNode<T>(obj);
	if (lastNode != 0)
	{
		lastNode->setNext(newNode);
		lastNode = newNode;
	}
	else
	{
		lastNode = newNode;
	}

	if (firstNode == 0)
	{
		firstNode = newNode;
	}

	count++;
}

// Remove an item from the list
template<class T>
void LinkedList<T>::remove(T obj)
{
	LinkedListNode<T>* _rememberNode = 0;

	LinkedListIterator<T> it = getIterator();
	while (it.hasNext())
	{
		LinkedListNode<T>* _curNode = it.getNode();
		T _obj = it.next();
		if (obj == _obj)
		{
			count--;
			LinkedListNode<T>* _nextNode = it.getNode();

			if (_rememberNode != 0)
			{
				_rememberNode->setNext(_nextNode);
			}
			else
			{
				if (_curNode == firstNode)
				{
					firstNode = _nextNode;
				}
			}

			if (_curNode == lastNode)
			{
				lastNode = _rememberNode;
			}
		}
		else
		{
			_rememberNode = _curNode;
		}
	}
}

// Get amount of the elements in the list
template<class T>
int LinkedList<T>::getLength()
{
	return count;
}

// Obtain the iterator to iterate the list
template<class T>
LinkedListIterator<T>& LinkedList<T>::getIterator()
{
	it->setNext(firstNode);
	return *it;
}
