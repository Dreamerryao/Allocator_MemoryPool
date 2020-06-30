#pragma GCC optimize(3,"Ofast","inline")
#include "stdafx.h"
#include "new_allocator.h"
#include "my_allocator.h"
#include <ctime>
#include <vector>
#include <iostream>
#include <string>


#define INT 1
#define FLOAT 2
#define DOUBLE 3
#define CLASS  4

#define NEW_ALLOCATOR 10001
#define MY_ALLOCATOR 10002
#define STD_ALLOCATOR 10003
class vecWrapper
{
public:
	vecWrapper() {
		m_pVec = NULL;
		m_type = INT;
	}
	virtual ~vecWrapper()	{
	}
public:
	void setPointer(int type, void *pVec) { m_type = type; m_pVec = pVec; }
	virtual void visit(int index) = 0;
	virtual int size() = 0;
	virtual void resize(int size) = 0;
	virtual bool checkElement(int index, void *value) = 0;
	virtual void setElement(int idex, void *value) = 0;
protected:
	int m_type;
	void *m_pVec;
};

template <typename T, template<class> class anyAllocator>
class vecWrapperT : public vecWrapper
{
public:
	vecWrapperT(int type, std::vector<T, anyAllocator<T> > *pVec)
	{
		m_type = type;
		m_pVec = pVec;//type and Vector using template T
	}
	virtual ~vecWrapperT() {
		if (m_pVec)
			delete ((std::vector<T, anyAllocator<T> > *)m_pVec);
	}
public:
	virtual void visit(int index)
	{
		T temp = (*(std::vector<T, anyAllocator<T> > *)m_pVec)[index];
	}
	virtual int size()
	{
		return ((std::vector<T, anyAllocator<T> > *)m_pVec)->size();
	}
	virtual void resize(int size)
	{
		((std::vector<T, anyAllocator<T> > *)m_pVec)->resize(size);
	}
	virtual bool checkElement(int index, void *pValue)
	{
		T temp = (*(std::vector<T, anyAllocator<T> > *)m_pVec)[index];
		if (temp == (*((T *)pValue)))
			return true;
		else
			return false;
	}

	virtual void setElement(int index, void *value)
	{
		(*(std::vector<T, anyAllocator<T> > *)m_pVec)[index] = *((T *)value);
	}
};

class myObject
{
public:
	myObject() : m_X(0), m_Y(0) {}
	myObject(int t1, int t2) :m_X(t1), m_Y(t2) {}
	myObject(const myObject &rhs) { m_X = rhs.m_X; m_Y = rhs.m_Y;}
	~myObject() { /*std::cout << "my object destructor called" << std::endl;*/ }
	bool operator == (const myObject &rhs)
	{
		if ((rhs.m_X == m_X) && (rhs.m_Y == m_Y))
			return true;
		else
			return false;
	}
protected:
	int m_X;
	int m_Y;
};

#define TESTSIZE 100000

template <template<class> class anyAllocator>
class test {
   public:

    void testMain(int mode) {
		// clock_t start;

		std::string mm = mode==10001?"Malloc_Allocator":(mode==10002?"My_Allocator":"std_Allocator");
		// start = clock();
		vecWrapper **testVec;
	testVec = new vecWrapper*[TESTSIZE];

	int tIndex, tSize;
	//test allocator
	for (int i = 0; i < TESTSIZE - 4; i++)
	{
		tSize = (int)((float)rand()/(float)RAND_MAX * 10000);
		// tSize = 64;
		vecWrapperT<int,anyAllocator> *pNewVec = new vecWrapperT<int,anyAllocator>(INT, new std::vector<int, anyAllocator<int>>(tSize));
		testVec[i] = (vecWrapper *)pNewVec;
	}

	for (int i = 0; i < 4; i++)
	{
		tSize = (int)((float)rand() / (float)RAND_MAX * 10000);
		// tSize = 100;
		vecWrapperT<myObject,anyAllocator> *pNewVec = new vecWrapperT<myObject,anyAllocator>(CLASS, new std::vector<myObject, anyAllocator<myObject>>(tSize));
		testVec[TESTSIZE - 4 + i] = (vecWrapper *)pNewVec;
	}
	// std::cout <<mm <<" allocator: "<< (clock() - start) * 1.0 / CLOCKS_PER_SEC << " seconds"<< std::endl;
	// start = clock();
	//test resize
	for (int i = 0; i < 100; i++)
	{
		tIndex = (int)((float)rand() / (float)RAND_MAX * 10000);
		// tIndex = 10;
		tSize = (int)((float)rand() / (float)RAND_MAX * 10000);
		testVec[tIndex]->resize(tSize);
	}
	// std::cout <<mm <<" resize: "<< (clock() - start) * 1.0 / CLOCKS_PER_SEC << " seconds"<< std::endl;
	// start = clock();
	// test assignment
	tIndex = (int)((float)rand() / (float)RAND_MAX * (TESTSIZE - 4 - 1));
	int tIntValue = 10;
	testVec[tIndex]->setElement(testVec[tIndex]->size()/2, &tIntValue);
	if (!testVec[tIndex]->checkElement(testVec[tIndex]->size()/ 2, &tIntValue))
		std::cout<< "incorrect assignment in vector %d\n" << tIndex << std::endl;

	tIndex = TESTSIZE - 4 + 3;
	myObject tObj(11,15);
	testVec[tIndex]->setElement(testVec[tIndex]->size()/2, &tObj);
	if (!testVec[tIndex]->checkElement(testVec[tIndex]->size()/2, &tObj))
		std::cout << "incorrect assignment in vector %d\n" << tIndex << std::endl;

	myObject tObj1(13, 20);
	testVec[tIndex]->setElement(testVec[tIndex]->size()/2, &tObj1);
	if (!testVec[tIndex]->checkElement(testVec[tIndex]->size() / 2-1, &tObj1))
		std::cout << "incorrect assignment in vector " << tIndex << " for object (13,20)" << std::endl;
 	// std::cout <<mm <<" assignment: "<< (clock() - start) * 1.0 / CLOCKS_PER_SEC << " seconds"<< std::endl;

	for (int i = 0; i < TESTSIZE; i++)
		delete testVec[i];

	delete []testVec;

	}
};

int main()
{	
	//test correct
	test<std::allocator> test1;
	test1.testMain(STD_ALLOCATOR);
	test<Malloc_Allocator>test2;
	test2.testMain(NEW_ALLOCATOR);
	test<MyAllocator> test3;
	test3.testMain(MY_ALLOCATOR);
}

