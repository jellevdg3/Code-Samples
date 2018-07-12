#pragma once

#include "LinkedListNode.h"

template<class T>
class LinkedListIterator
{
public:
	// Constructor
	LinkedListIterator();
	
	// Destructor
	~LinkedListIterator();

	// Check if this iterator has a next node
	bool hasNext();
	
	// Goto the next node in this iterator
	T next();

	// Get the current node for this iterator
	LinkedListNode<T>* getNode();
	
	// Set the next node for this iterator
	void setNext(LinkedListNode<T>* _nextNode);

private:
	LinkedListNode<T>* _nextNode;
};

// Constructor
template<class T>
LinkedListIterator<T>::LinkedListIterator()
{
}

// Destructor
template<class T>
LinkedListIterator<T>::~LinkedListIterator()
{
}

// Check if this iterator has a next node
template<class T>
bool LinkedListIterator<T>::hasNext()
{
	return _nextNode != 0;
}

// Goto the next node in this iterator
template<class T>
T LinkedListIterator<T>::next()
{
	T _data = _nextNode->getData();
	_nextNode = _nextNode->next();
	return _data;
}

// Get the current node for this iterator
template<class T>
LinkedListNode<T>* LinkedListIterator<T>::getNode()
{
	return _nextNode;
}

// Set the next node for this iterator
template<class T>
void LinkedListIterator<T>::setNext(LinkedListNode<T>* _nextNode)
{
	this->_nextNode = _nextNode;
}
