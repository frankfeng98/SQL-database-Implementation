

#ifndef PAGE_LIST_ITER_ALT_C
#define PAGE_LIST_ITER_ALT_C

#include "../headers/MyDB_PageListIteratorAlt.h"
#include "../headers/MyDB_PageRecIteratorAlt.h"

void MyDB_PageListIteratorAlt :: getCurrent (MyDB_RecordPtr intoMe) {
	myIter->getCurrent (intoMe);
}

bool MyDB_PageListIteratorAlt :: advance () {

	if (myIter->advance ())
		return true;

	if (curPage == forUs.size () - 1)
		return false;

	curPage++;
	myIter = forUs[curPage].getIteratorAlt ();
	return advance ();
}

void *MyDB_PageListIteratorAlt :: getCurrentPointer () {
	return myIter->getCurrentPointer ();
}

MyDB_PageListIteratorAlt :: MyDB_PageListIteratorAlt (vector <MyDB_PageReaderWriter> &forUsIn) {
	forUs = forUsIn;
	curPage = 0;
	myIter = forUsIn[curPage].getIteratorAlt ();
}

MyDB_PageListIteratorAlt :: ~MyDB_PageListIteratorAlt () {}

#endif
