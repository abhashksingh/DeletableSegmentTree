#include <iostream>
#include <vector>
#include <string>
#include <cassert>
using namespace std;

template <typename T>
class Operator
{
public:
	virtual T apply(T leftValue, T rightValue) = 0;
};

template <typename T>
class MinOperator : public Operator<T>
{
public:
	T apply(T leftValue, T rightValue)
	{
		if (leftValue > rightValue)
		{
			return rightValue;
		}
		else
		{
			return leftValue;
		}
	}
};

template <typename T>
class AddOperator : public Operator<T>
{
public:
	T apply(T leftValue, T rightValue)
	{
		return leftValue + rightValue;
	}
};

template <typename T>
class SegmentTree
{
public:
	SegmentTree(vector<T>& initialValues, Operator<T>* op, T identity);
	T get(int index);
	void set(int index, T value);
	T queryRange(int start, int end);
	void buildTree(int rangeStart, int rangeEnd, int currentNode, vector<T>& initialValues);
	int size();
	int getCapacity() { return capacity; }
	Operator<T>* getOperator() { return oper; }
	T getIdentity() { return identity; }
	void add(T value);//add a new element to the end
	void display();
	~SegmentTree();

private:
	int length;
	int capacity;
	Operator<T>* oper;
	T identity;
	T* tree;
	int getMid(int start, int end)
	{
		return (start + ((end - start) / 2));
	}

	int leftChild(int node)
	{
		return node + 1;
	}

	int rightChild(int node, int start, int mid)
	{
		int numberOfLeavesInTheLeftSubtree = mid - start + 1;
		int numberOfNodesInTheLeftSubtree = 2 * numberOfLeavesInTheLeftSubtree - 1;
		return node + numberOfNodesInTheLeftSubtree + 1;
	}

	T queryRange(int queryStart, int queryEnd, int rangeStart, int rangeEnd, int currentNode);
	void set(int index, int rangeStart, int rangeEnd, int currentNode, T value);
	void resize(int newCapacity);
};

template <typename T>
SegmentTree<T>::SegmentTree(vector<T>& initialValues, Operator<T>* op, T identity)
{
	this->oper = op;
	this->identity = identity;
	this->length = this->capacity = initialValues.size();
	this->tree = new T[(2 * this->capacity) - 1];
	buildTree(0, capacity - 1, 0, initialValues);
}

template<typename T>
T SegmentTree<T>::get(int index)
{
	if (index < 0 || index >= length) throw ("index out of bounds: " + to_string(index));
	return queryRange(index, index);
}

template<typename T>
void SegmentTree<T>::set(int index, T value)
{
	return set(index, 0, capacity - 1, 0, value);
}

template<typename T>
void SegmentTree<T>::set(int index, int rangeStart, int rangeEnd, int currentNode, T value)
{
	if (rangeStart == rangeEnd && rangeEnd == index)
	{
		// the currentNode represents a leaf. and the range contains only the element at the desried index
		tree[currentNode] = value;
		return;
	}
	int mid = getMid(rangeStart, rangeEnd);
	if (index <= mid)
		set(index, rangeStart, mid, leftChild(currentNode), value);
	else
		set(index, mid + 1, rangeEnd, rightChild(currentNode, rangeStart, mid), value);
	// because my children have changed, I gotta update my value as well
	tree[currentNode] = oper->apply(tree[leftChild(currentNode)], tree[rightChild(currentNode, rangeStart, mid)]);
}

template<typename T>
void SegmentTree<T>::resize(int newCapacity)
{
	// take all the previous values
	// append a bunch of identities to increase the size
	// rebuild the tree with these values
	vector<T> values;
	for (int i = 0; i < length; ++i)
		values.push_back(get(i));
	for (int i = length; i < newCapacity; ++i)
		values.push_back(identity);
	
	delete[] tree;
	tree = new T[(2*newCapacity)-1];
	capacity = newCapacity;
	buildTree(0, capacity - 1, 0, values);

		
}

template<typename T>
void SegmentTree<T>::buildTree(int rangeStart, int rangeEnd, int currentNode, vector<T>& initialValues)
{
	if (rangeStart == rangeEnd)
	{
		//the current node represents a leaf,
		//which has range of a single element
		tree[currentNode] = initialValues[rangeStart];
		return;
	}
	else
	{
		int mid = getMid(rangeStart, rangeEnd);
		// left subtree
		buildTree(rangeStart, mid, leftChild(currentNode), initialValues);
		// right subtree
		buildTree(mid + 1, rangeEnd, rightChild(currentNode, rangeStart, mid), initialValues);
		// combine
		tree[currentNode] = oper->apply(tree[leftChild(currentNode)]
			, tree[rightChild(currentNode, rangeStart, mid)]);
	}
}

template<typename T>
int SegmentTree<T>::size()
{
	return this->length;
}

template<typename T>
void SegmentTree<T>::add(T value)
{
	if (length == capacity)
	{
		resize((2 * capacity)+1);
	}
	++length;
	set(length-1, value);
}

template<typename T>
void SegmentTree<T>::display()
{
	cout<<"[ ";
	for (int i = 0; i < this->length; i++)
	{ 
		cout<<this->get(i)<<" ";
	}
	for (int i = length; i < capacity; i++) 
	{
		cout << "__";
	}
	cout << " ]";
}

template <typename T>SegmentTree<T>::~SegmentTree()
{
	delete[] tree;
	tree = nullptr;
}

template<typename T>
T SegmentTree<T>::queryRange(int start, int end)
{
	return queryRange(start, end, 0, capacity - 1, 0);
}

template<typename T>
T SegmentTree<T>::queryRange(int queryStart, int queryEnd, int rangeStart, int rangeEnd, int currentNode)
{
	if (queryStart <= rangeStart && rangeEnd <= queryEnd)
	{
		// the currentNode represents a range that is fully contained within the query
		// so we need tha value of this node completely
		return tree[currentNode];
	}
	else if (queryStart > rangeEnd || rangeStart > queryEnd)
	{
		//the query has no overlap wiith the range of currentNode
		return this->identity;
	}
	else
	{
		//partial overlap
		int mid = getMid(rangeStart, rangeEnd);
		T leftValue = queryRange(queryStart, queryEnd, rangeStart, mid, leftChild(currentNode));
		T rightValue = queryRange(queryStart, queryEnd, mid + 1, rangeEnd,
			rightChild(currentNode, rangeStart, mid));
		return oper->apply(leftValue, rightValue);
	}
}

template <typename T>
class SegmentTreeDeletable
{
private:
	SegmentTree<T>* tree = nullptr;
	SegmentTree<int>* deletedIndices = nullptr;
	int length;
	AddOperator<int> addOp;
	int getTrueIndex(int index);
	T identity;
public:
	SegmentTreeDeletable(vector<T>& initialValues, Operator<T>* op, T identity);
	~SegmentTreeDeletable();
	int size() { return tree->size(); }
	T get(int index);
	void set(int index, T value);
	T queryRange(int start, int end);
	void add(T value);
	void remove(int index);
	int getCapacity();
	Operator<T>* getOperator() { return tree->getOperator; }
	T getIdentity() { return tree->getIdentity; }
	void display();
};

template<typename T>
SegmentTreeDeletable<T>::SegmentTreeDeletable(vector<T>& initialValues, Operator<T>* op, T identity)
{
	assert(op!=nullptr);
	this->identity = identity;
	this->length = 0;
	if (tree == nullptr)
	{
		tree = new SegmentTree<T>(initialValues, op, identity);
		length = initialValues.size();
	}

	if (deletedIndices == nullptr)
	{
		vector<int> deleted;
		for (uint32_t i = 0; i < initialValues.size(); ++i)
		{
			deleted.push_back(0);
		}
		deletedIndices = new SegmentTree<int>(deleted,&addOp,0);
	}
}

template<typename T>
SegmentTreeDeletable<T>::~SegmentTreeDeletable()
{
	if (tree != nullptr)
	{
		delete tree;
		tree = nullptr;
	}
	if (deletedIndices !=nullptr)
	{
		delete deletedIndices;
		deletedIndices = nullptr;
	}
}

template<typename T>
T SegmentTreeDeletable<T>::get(int index)
{
	index = getTrueIndex(index);
	return tree->get(index);
}

template<typename T>
void SegmentTreeDeletable<T>::set(int index, T value)
{
	index = getTrueIndex(index);
	tree->set(index, value);
}

template<typename T>
void SegmentTreeDeletable<T>::add(T value)
{
	tree->add(value);
	deletedIndices->add(0);
	length++;
}

template<typename T>
void SegmentTreeDeletable<T>::remove(int index)
{
	index = getTrueIndex(index);
	deletedIndices->set(index, 1);
	tree->set(index,identity);
	this->length--;
}

template<typename T>
T SegmentTreeDeletable<T>::queryRange(int start, int end)
{
	start = getTrueIndex(start);
	end = getTrueIndex(end);

	return tree->queryRange(start,end);
}

template<typename T>
int SegmentTreeDeletable<T>::getTrueIndex(int index)
{
	int low = index;
	int high = tree->size() - 1;
	while (low <= high) 
	{
		int mid = low + (high - low) / 2;
		int deletedCount = deletedIndices->queryRange(0, mid);
		if (mid - index < deletedCount) low = mid + 1;
		else high = mid - 1;
	}
	return low;
}

template<typename T>
int SegmentTreeDeletable<T>::getCapacity()
{
	int capacity = tree->capacity;
	int countDeleted = deletedIndices->queryRange(0, capacity);
	return capacity - countDeleted;
}

template<typename T>
void SegmentTreeDeletable<T>::display()
{
	tree->display();
	cout << endl;
	deletedIndices->display();
	cout << endl;
	cout<<"Apparent array [ ";
	for (int i = 0; i < length; i++) 
	{
		cout << this->get(i) << " ";
	}
	cout << " ]" << endl;
}


int main()
{
	AddOperator<int> intAddOp;
	vector<int> arr = {3,4,5};
	SegmentTreeDeletable<int>* segTree =
		new SegmentTreeDeletable<int>(arr, &intAddOp, 0);

	segTree->display();

	for (int i = 0; i < 10; ++i)
	{
		segTree->display();
		segTree->add(i*10);
		if (i % 3 == 0)
		{
			segTree->display();
			segTree->remove(0);
		}
	}

}