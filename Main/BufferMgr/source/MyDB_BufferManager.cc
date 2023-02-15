
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr tablePtr, long pos) {
	pair<MyDB_TablePtr, long> pageKey = make_pair(tablePtr, pos);
	MyDB_PageHandle pageHandle = nullptr;

	if(pageMap.find(pageKey) != pageMap.end()) {
		// page exists
		Node* pageNode = pageMap[pageKey];
		pageHandle = make_shared<MyDB_PageHandleBase>(pageNode->pagePtr);
	} else {
		// page not exists
		// MyDB_Page(bool pinned, bool anonymous, MyDB_TablePtr db_table, MyDB_BufferManager* db_bugMgr, size_t pos);
		if (availableMemory.empty()) {
			// check if there is still available Memory and unpinned page available
			if(!evictLRU()){
				// LRU cache is empty, no page is available
				return nullptr;
			}
		}
		MyDB_PagePtr page = new MyDB_Page(false, false, tablePtr, this, pos);
		pageMap[pageKey] = new Node(page, LRUNum++);
		pageHandle = make_shared<MyDB_PageHandleBase>(page);
	}

	return pageHandle;		
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	if (availableMemory.empty()) {
		// check if there is still available Memory and unpinned page available
		if(!evictLRU()){
			// LRU cache is empty, no page is available
			return nullptr;
		}
	}
	MyDB_PagePtr pagePtr = new MyDB_Page(false, true, nullptr, this, tempFilePos++);
	Node* pageNode = new Node(pagePtr, LRUNum++);
	pageMap[make_pair(nullptr, pagePtr->pos)] = pageNode;
	return make_shared<MyDB_PageHandleBase>(pagePtr);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long pos) {
	pair<MyDB_TablePtr, long> pageKey = make_pair(tablePtr, pos);
	MyDB_PagePtr pagePtr = nullptr;
	Node* pageNode = nullptr;

	if (pageMap.find(pageKey) != pageMap.end()) {
		// pinned page exists
		pageNode = pageMap[pageKey];
		pagePtr = pageNode->pagePtr;
		if (!pagePtr->isPinned()) {
			// not pinned, move it out from LRU cache
			removeNode(pageNode);
			pagePtr->pinned = true;
		}
	} else {
		// pinned page not exists
		if (availableMemory.empty()) {
			// check if there is still available Memory and unpinned page available
			if(!evictLRU()){
				// LRU cache is empty, no page is available
				return nullptr;
			}
		}
		// now we have available memory
		if (!availableMemory.empty()) {
			pagePtr = new MyDB_Page(true, false, tablePtr, this, pos);
			pageMap[pageKey] = new Node(pagePtr, LRUNum++);
		}
	}

	return make_shared<MyDB_PageHandleBase>(pagePtr);		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	if (availableMemory.empty()) {
		// check if there is still available Memory and unpinned page available
		if(!evictLRU()){
			// LRU cache is empty, no page is available
			return nullptr;
		}
	}
	
	MyDB_PageHandle pageHandle = getPage();
	pageHandle->pagePtr->pinned = true;
	return pageHandle;
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	pair<MyDB_TablePtr, long> pageKey = make_pair(unpinMe->pagePtr->db_table, unpinMe->pagePtr->pos);
	unpinMe->pagePtr->pinned = false;
	removeNode(pageMap[pageKey]);
	moveToHead(pageMap[pageKey]);
}

void MyDB_BufferManager :: removePage(MyDB_PagePtr page){
	pair<MyDB_TablePtr, long> pageKey = make_pair(page->db_table, page->pos);
	if (pageKey.first == nullptr || (!page->isPinned() && pageKey.first != nullptr)) {
		// anonymous page
		if(page->bytes != nullptr) {
			// has page
			availableMemory.push_back(page->bytes);
		}
		Node* pageNode = pageMap[pageKey];
		if(!page->isPinned()) {
			// in LRU cache
			removeNode(pageNode);
		}
		if (page->isDirty()) {
			writeToFile(page);
		}
	} else {
		if (page->isPinned()) {
			// non-anonymous pinned page
			unpin(make_shared<MyDB_PageHandleBase>(page));
		} 
	}
	page->bytes = nullptr;
}

void MyDB_BufferManager :: accessPage(MyDB_PagePtr page){
	pair<MyDB_TablePtr, long> pageKey = make_pair(page->db_table, page->pos);
	if (!page->isPinned()) {
		removeNode(pageMap[pageKey]);
		moveToHead(pageMap[pageKey]);
	}
	if (page->bytes != nullptr) {
		return;
	}

	if (fds.find(page->db_table) == fds.end()) {
		// file not open
		if (page->db_table == nullptr) {
			// anonymous page
			int fd = open (tempFile.c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
			fds[page->db_table] = fd;
		} else{
			int fd = open (page->db_table->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
			fds[page->db_table] = fd;
		}
	}

	if (availableMemory.empty()) {
		// no LRU cache available
		if (!evictLRU()) {
			// no space even after evict
			return;
		}
	}

	// now we have available LRU cache
	page->bytes = availableMemory.back();
	availableMemory.pop_back();
	
	// read bytes
    lseek(fds[page->db_table], page->pos * pageSize, SEEK_SET);
	read(fds[page->db_table], page->bytes, pageSize);
}

void MyDB_BufferManager :: removeNode(Node* node) {
	// printLRU();
	if (node->next != nullptr && node->prev != nullptr) {
		joinNode(node->prev, node->next);
		node->next = nullptr;
		node->prev = nullptr;
	}
}

void MyDB_BufferManager :: moveToHead(Node* node){
	joinNode(node, head->next);
	joinNode(head, node);
	node->num = LRUNum++;
}

void MyDB_BufferManager :: joinNode(Node* lhs, Node* rhs){
	lhs->next = rhs;
	rhs->prev = lhs;
}

void MyDB_BufferManager :: updateLRU(MyDB_PagePtr page){
	pair<MyDB_TablePtr, long> pageKey = make_pair(page->db_table, page->pos);
	Node* pageNode = pageMap[pageKey];
	removeNode(pageNode);
	moveToHead(pageNode);
}

bool MyDB_BufferManager :: evictLRU(){
	if (head->next == tail && tail->prev == head) {
		return false;
	}
	
	MyDB_PagePtr pagePtr = tail->prev->pagePtr;
	removeNode(tail->prev);
	if (pagePtr->isDirty()) {
		writeToFile(pagePtr);
	}
	availableMemory.push_back(pagePtr->bytes);
	return true;
}

void MyDB_BufferManager :: writeToFile(MyDB_PagePtr page) {
	if(page->bytes != nullptr) {
		// write back to disk
		lseek (fds[page->db_table], page->pos * pageSize, SEEK_SET);
		write (fds[page->db_table], page->bytes, pageSize);
		page->dirty = false;
	}
}

void MyDB_BufferManager :: printLRU() {
	Node* node = head;
	long cnt = 0;
	while(head->next != nullptr && cnt < 30) {
		cout << node->num << " ";
		node = node->next;
		cnt++;
	}
	cout << endl;
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) : pageSize(pageSize), numPages(numPages), tempFile(tempFile) {
	tempFilePos = 0;
	LRUNum = 0;
	head = new Node();
	tail = new Node();
	joinNode(head, tail);

	for (size_t i = 0; i < this->numPages; i++) {
		availableMemory.push_back(malloc(this->pageSize));
	}
	
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	for (auto& pageNode : pageMap) {
		// write back to file
		MyDB_PagePtr page = pageNode.second->pagePtr;
		writeToFile(page);
		// if (page->bytes != nullptr) {
		// 	free(page->bytes);
		// }
		page->bytes = nullptr;
		delete pageNode.second;
		pageNode.second = nullptr;
	}

	// free the available memory
	for (auto& memoryBlock : availableMemory) {
		free (memoryBlock);
	}
	// close files
	for (auto& fd : fds) {
		close (fd.second);
	}

	// delete doubly linked list
	delete this->head;
	this->head = nullptr;
	delete this->tail;
	this->head = nullptr;
}
	
#endif


