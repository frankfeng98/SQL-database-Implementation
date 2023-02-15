
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageRecIterator.h"

MyDB_PageReaderWriter::MyDB_PageReaderWriter(MyDB_TablePtr myTable, MyDB_BufferManagerPtr myBuffer, int whichPage){
	this->bufferManager = myBuffer;
	this->pageHandle = bufferManager->getPage(myTable, whichPage);
}

void MyDB_PageReaderWriter :: clear () {
	setType(MyDB_PageType::RegularPage);
	PageWrapper *mypageWrapper = (PageWrapper*) this->pageHandle->getBytes();
	mypageWrapper->offset = 0;
	pageHandle->wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	return ((PageWrapper*)this->pageHandle->getBytes())->pageType;
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr recordPtr) {
	return make_shared<MyDB_PageRecIterator>(pageHandle, recordPtr);
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
	PageWrapper *mypageWrapper = (PageWrapper*) this->pageHandle->getBytes();
	mypageWrapper->pageType = pageType;
	pageHandle->wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr recordPtr) {
	PageWrapper *mypageWrapper = (PageWrapper*) this->pageHandle->getBytes();
	size_t recSize = recordPtr->getBinarySize();
	size_t pageSize = bufferManager->getPageSize();
	size_t byteLeft = pageSize - mypageWrapper->offset - sizeof(PageWrapper);
	if (byteLeft < recSize) {
		// not enough space
		return false;
	}

	recordPtr->toBinary(&(mypageWrapper->recs[mypageWrapper->offset]));
	mypageWrapper->offset += recSize;
	pageHandle->wroteBytes();
	return true;
}

#endif
