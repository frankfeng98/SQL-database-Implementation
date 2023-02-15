
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include <sstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer): 
	myTable(forMe), myBuffer(myBuffer) 
{
	//Nothing has been written to the table yet
	if (this->myTable->lastPage() == -1) {
		this->myTable->setLastPage(0);
		this->lastPage = make_shared <MyDB_PageReaderWriter> (this->myTable, this->myBuffer, this->myTable->lastPage ());
		this->lastPage->clear();
	} else { //table has contents to it so just point to the last page
		this->lastPage = make_shared <MyDB_PageReaderWriter> (this->myTable, this->myBuffer, this->myTable->lastPage ());
	}
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t i) {
	while (this->myTable->lastPage() < i) {
        this->myTable->setLastPage(this->myTable->lastPage() + 1);
        this->lastPage = make_shared<MyDB_PageReaderWriter>(this->myTable, this->myBuffer, this->myTable->lastPage());
        this->lastPage->clear();
    }

	MyDB_PageReaderWriter accessedPage(this->myTable, this->myBuffer, i); 
	return accessedPage;
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	return make_shared<MyDB_Record>(this->myTable->getSchema());
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	MyDB_PageReaderWriter last (this->myTable, this->myBuffer, this->myTable->lastPage());
	return last; 
}


void MyDB_TableReaderWriter :: append (MyDB_RecordPtr toBeAppend) {
	while (!this->lastPage->append(toBeAppend)) {
		//no available space on the last page, create a new last page
		this->myTable->setLastPage(this->myTable->lastPage() + 1);
		this->lastPage = make_shared <MyDB_PageReaderWriter> (this->myTable, this->myBuffer, this->myTable->lastPage ());
		this->lastPage->clear();
	}
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fileName) {
	//clear the database contents
	this->myTable->setLastPage(0);
	this->lastPage = make_shared<MyDB_PageReaderWriter> (this->myTable, this->myBuffer, this->myTable->lastPage());
	this->lastPage->clear();

	//load contents from file to the table
	ifstream file(fileName);
    if (file.is_open()) {
		MyDB_RecordPtr recordPtr = getEmptyRecord();
        string line;
        while (getline(file, line)) {
            recordPtr->fromString(line);
            append(recordPtr);
        }
        file.close();
    } else {
        fprintf(stderr, "Cannot open file \'%s\'", fileName.c_str());
    }
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {
	return make_shared<MyDB_TableRecIterator>(this->myTable, iterateIntoMe, *this);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string fileName) {
	ofstream output(fileName);
	if (output.is_open()) {
        MyDB_RecordPtr recordPtr = getEmptyRecord();
        MyDB_RecordIteratorPtr recIt = getIterator(recordPtr);
        while (recIt->hasNext()) {
            recIt->getNext();
            ostringstream stream;
            stream << recordPtr;
            output << stream.str() << endl;
        }
        output.close();
    } else {
        fprintf(stderr, "Cannot open file \'%s\'", fileName.c_str());
    }
}

#endif

