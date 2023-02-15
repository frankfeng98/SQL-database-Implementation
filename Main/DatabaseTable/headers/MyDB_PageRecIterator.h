
#ifndef PAGE_Rec_ITER_H
#define PAGE_Rec_ITER_H

#include <memory>

using namespace std;

#include "MyDB_Page.h"
#include "MyDB_PageHandle.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_RecordIterator.h"
#include "MyDB_Record.h"

class MyDB_PageRecIterator : public MyDB_RecordIterator {

public:
	// put the contents of the next record in the file/page into the iterator record
	// this should be called BEFORE the iterator record is first examined
	void getNext ();

	// return true iff there is another record in the file/page
	bool hasNext ();

    MyDB_PageRecIterator(MyDB_PageHandle pageHandler, MyDB_RecordPtr recPtr);

    ~MyDB_PageRecIterator() {};

private:
    MyDB_PageHandle pageHandler;
    MyDB_RecordPtr recPtr;

    size_t offset;
};

typedef shared_ptr<MyDB_PageRecIterator> MyDB_PageRecIteratorPtr;

#endif