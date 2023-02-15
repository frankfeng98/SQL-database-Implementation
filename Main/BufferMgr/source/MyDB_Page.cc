
#ifndef PAGE_C
#define PAGE_C

#include "MyDB_Page.h"
#include "MyDB_BufferManager.h"

MyDB_Page::MyDB_Page(bool pinned, bool anonymous, MyDB_TablePtr db_table, MyDB_BufferManager* db_bufMgr, long pos) :
    pinned(pinned), anonymous(anonymous), db_table(db_table),  db_bufMgr(db_bufMgr), pos(pos)
    {
    dirty = false;
    referenceCount = 0;
    bytes = nullptr;
    size = 0;
    timestamp = -1;
}

void MyDB_Page::wroteBytes() {
    if (!pinned){
        db_bufMgr->updateLRU(this);
    }
    this->dirty = true;
}

void * MyDB_Page::getBytes() {
    db_bufMgr->accessPage(this);
    return this->bytes;
}

void MyDB_Page::decreaseRefCount(MyDB_PagePtr page) {
    referenceCount--;
    if (referenceCount == 0) {
        removePage(page);
    }
}

void MyDB_Page::increaseRefCount() {
    referenceCount++;
}

bool MyDB_Page::isPinned() {
    return this->pinned;
}

void MyDB_Page::setPinned(bool pinned) {
    this->pinned = pinned;
}

bool MyDB_Page::isAnonymous() {
    return this->anonymous;
}

bool MyDB_Page::isDirty() {
    return this->dirty;
}

long MyDB_Page::getTimeStamp() {
    return this->timestamp;
}

void MyDB_Page::setTimeStamp(long timestamp) {
    this->timestamp = timestamp;
}

MyDB_Page::~MyDB_Page() {

}

void MyDB_Page::removePage(MyDB_PagePtr page) {
    //invoking the kill page method in buffer mgr so that it can be updated globally
    db_bufMgr->removePage(page);
}

#endif