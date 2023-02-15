#ifndef TABLE_REC_ITER_C
#define TABLE_REC_ITER_C

#include "../headers/MyDB_TableRecIterator.h"
#include "../headers/MyDB_PageReaderWriter.h"

MyDB_TableRecIterator::MyDB_TableRecIterator(MyDB_TablePtr myTablePtr, MyDB_RecordPtr myRecordPtr, 
                                        MyDB_TableReaderWriter &myParent): myTable(myTablePtr), myRec(myRecordPtr), 
                                        myParent(myParent)
{
    this->pageIndex = 0;
    this->myIter = this->myParent[this->pageIndex].getIterator(this->myRec);
}

bool MyDB_TableRecIterator::hasNext() {
    if (myParent[pageIndex].getType() == MyDB_PageType::RegularPage && this->myIter->hasNext()) {
        return true;
    }

    if (pageIndex == this->myTable->lastPage()) {
        return false;
    }

    this->pageIndex++;
    this->myIter = myParent[pageIndex].getIterator(myRec);
    return hasNext();
}

void MyDB_TableRecIterator::getNext() {
    return this->myIter->getNext();
}

MyDB_TableRecIterator :: ~MyDB_TableRecIterator () {}

#endif