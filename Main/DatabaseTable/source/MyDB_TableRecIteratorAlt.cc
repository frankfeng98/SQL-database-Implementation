

#ifndef TABLE_REC_ITER_ALT_C
#define TABLE_REC_ITER_ALT_C

#include "../headers/MyDB_PageReaderWriter.h"
#include "../headers/MyDB_TableRecIteratorAlt.h"

void MyDB_TableRecIteratorAlt :: getCurrent (MyDB_RecordPtr intoMe) {
	myIter->getCurrent (intoMe);
}

void *MyDB_TableRecIteratorAlt :: getCurrentPointer () {
	return myIter->getCurrentPointer ();
}

bool MyDB_TableRecIteratorAlt :: advance () {

	if (myParent[curPage].getType () == MyDB_PageType :: RegularPage && myIter->advance ())
		return true;

	if (curPage == myTable->lastPage () || curPage == highPage)
		return false;

	curPage++;
	myIter = myParent[curPage].getIteratorAlt ();
	return advance ();
}

MyDB_TableRecIteratorAlt :: MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn,
	int lowPage, int highPageIn) :
	myParent (myParent) {
	myTable = myTableIn;
	curPage = lowPage;
	highPage = highPageIn;
	myIter = myParent[curPage].getIteratorAlt ();
}

MyDB_TableRecIteratorAlt :: MyDB_TableRecIteratorAlt (MyDB_TableReaderWriter &myParent, MyDB_TablePtr myTableIn) :
	myParent (myParent) {
	myTable = myTableIn;
	curPage = 0;
	highPage = 1999999999;
	myIter = myParent[curPage].getIteratorAlt ();
}

MyDB_TableRecIteratorAlt :: ~MyDB_TableRecIteratorAlt () {}

#endif
