

#ifndef PAGE_REC_ITER_ALT_C
#define PAGE_REC_ITER_ALT_C

#include "../headers/MyDB_PageRecIteratorAlt.h"
#include "../../Catalog/headers/MyDB_PageType.h"

#define NUM_BYTES_USED *((size_t *) (((char *) myPage->getBytes ()) + sizeof (size_t)))

void MyDB_PageRecIteratorAlt :: getCurrent (MyDB_RecordPtr intoMe) {
	void *pos = bytesConsumed + (char *) myPage->getBytes ();
 	void *nextPos = intoMe->fromBinary (pos);
	nextRecSize = ((char *) nextPos) - ((char *) pos);
}

void *MyDB_PageRecIteratorAlt :: getCurrentPointer () {
	return bytesConsumed + (char *) myPage->getBytes ();
}

bool MyDB_PageRecIteratorAlt :: advance () {
	if (nextRecSize == -1) {
		cout << "You can't call advance without calling getCurrent!!\n";
		exit (1);
	}
	bytesConsumed += nextRecSize;
	nextRecSize = -1;
	return bytesConsumed != NUM_BYTES_USED;
}

MyDB_PageRecIteratorAlt :: MyDB_PageRecIteratorAlt (MyDB_PageHandle myPageIn) {
	bytesConsumed = sizeof (size_t) * 2;
	myPage = myPageIn;
	nextRecSize = 0;
}

MyDB_PageRecIteratorAlt :: ~MyDB_PageRecIteratorAlt () {}

#endif
