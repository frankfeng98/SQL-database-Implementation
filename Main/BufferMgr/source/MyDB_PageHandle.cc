
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

void *MyDB_PageHandleBase :: getBytes () {
	return this->pagePtr->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
	this->pagePtr->wroteBytes();
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	this->pagePtr->decreaseRefCount(this->pagePtr);
}

MyDB_PageHandleBase:: MyDB_PageHandleBase(MyDB_PagePtr pageToUse) {
	this->pagePtr = pageToUse;
	this->pagePtr->increaseRefCount();
}

#endif

