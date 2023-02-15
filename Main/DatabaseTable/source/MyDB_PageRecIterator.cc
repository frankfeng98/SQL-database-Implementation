
#ifndef PAGE_REC_ITER_H
#define PAGE_REC_ITER_H

#include "MyDB_PageRecIterator.h"

using namespace std;

void MyDB_PageRecIterator::getNext(){
    if(!this->hasNext()){
        return;
    }
    PageWrapper *myPageWrapper = (PageWrapper *) this->pageHandler->getBytes();
    this->recPtr->fromBinary(&(myPageWrapper->recs[this->offset]));
    this->offset += this->recPtr->getBinarySize();
}

bool MyDB_PageRecIterator::hasNext(){
    PageWrapper *myPageWrapper = (PageWrapper *) this->pageHandler->getBytes();
    if (myPageWrapper->offset > this->offset) {
        return true;
    }
    return false;
}

MyDB_PageRecIterator::MyDB_PageRecIterator(MyDB_PageHandle pageHandler, MyDB_RecordPtr recPtr){
    this->pageHandler = pageHandler;
    this->recPtr = recPtr;
    offset = 0;
}

#endif