#pragma once

template<class T>
class LinkedListNode
{
public:
	// Constructor
	LinkedListNode(T _data);
	
	// Destructor
	~LinkedListNode();

	// Check if this node has a next node
	bool hasNext();
	
	// Obtain the next node from this node
	LinkedListNode<T>* next();
	
	// Get the containing data from this node
	T getData();

	// Set the next node for this node
	void setNext(LinkedListNode<T>* _nextNode);

private:
	LinkedListNode<T>* _nextNode;
	T _data;
};

// Constructor
template<class T>
LinkedListNode<T>::LinkedListNode(T _data)
{
	_nextNode = 0;
	this->_data = _data;
}

// Destructor
template<class T>
LinkedListNode<T>::~LinkedListNode()
{
	delete _data;
}

// Check if this node has a next node
template<class T>
bool LinkedListNode<T>::hasNext()
{
	return _nextNode != 0;
}

// Obtain the next node from this node
template<class T>
LinkedListNode<T>* LinkedListNode<T>::next()
{
	return _nextNode;
}

// Get the containing data from this node
template<class T>
T LinkedListNode<T>::getData()
{
	return _data;
}

// Set the next node for this node
template<class T>
void LinkedListNode<T>::setNext(LinkedListNode<T>* _nextNode)
{
	this->_nextNode = _nextNode;
}
