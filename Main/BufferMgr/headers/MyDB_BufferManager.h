
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "MyDB_Page.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

using namespace std;

struct pair_equal {
    bool operator()(const std::pair<MyDB_TablePtr, long> &lhs, const std::pair<MyDB_TablePtr, long> &rhs) const {
		if(lhs.first->getName() == rhs.first->getName() && lhs.first->getStorageLoc() == rhs.first->getStorageLoc() && lhs.second == rhs.second){
			return true;
		}
        return false;
    }
};

struct TableComp{
	bool operator() (const MyDB_TablePtr lhs, const MyDB_TablePtr rhs) {
		return lhs->getName() < rhs->getName();
	}
};

struct Node{
	Node* next;
	Node* prev;
	MyDB_PagePtr pagePtr;
	// LRU num
	long num;

	Node(MyDB_PagePtr pagePtr, long num): next(nullptr), prev(nullptr), pagePtr(pagePtr), num(num) {
	}

	Node():next(nullptr), prev(nullptr), pagePtr(nullptr) {
		num = -1;
	}

	~Node(){
		next = nullptr;
		prev = nullptr;
		pagePtr = nullptr;
	}
};

class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS 

private:
	friend class MyDB_Page;
	friend class MyDB_PageHandleBase;
	// YOUR STUFF HERE
	size_t pageSize;
	size_t numPages;
	string tempFile;
	long tempFilePos;
	long LRUNum;

	Node* head;
	Node* tail;

	vector<void*> availableMemory;
	map<pair<MyDB_TablePtr, long>, Node*> pageMap;
	map<MyDB_TablePtr, int> fds;
	
	void removeNode(Node* node);
	void moveToHead(Node* node);
	void joinNode(Node* lhs, Node* rhs);
	bool evictLRU();
	void updateLRU(MyDB_PagePtr page);
	void removePage(MyDB_PagePtr page);
	void accessPage(MyDB_PagePtr page);
	void writeToFile(MyDB_PagePtr page);
	void printLRU();
};

#endif


