
#ifndef PAGE_H
#define PAGE_H

#include <memory>
#include "MyDB_Table.h"

using namespace std;
class MyDB_Page;
typedef MyDB_Page* MyDB_PagePtr;
class MyDB_BufferManager;

class MyDB_Page {

public:
    MyDB_Page(bool pinned, bool anonymous, MyDB_TablePtr db_table, MyDB_BufferManager* db_bugMgr, long pos);

    void * getBytes();

    void wroteBytes();

    void decreaseRefCount(MyDB_PagePtr page);

    void increaseRefCount();

    bool isPinned();

    void setPinned(bool pinned);

    bool isAnonymous();

    bool isDirty();

    long getTimeStamp();

    void setTimeStamp(long timestamp);

    ~MyDB_Page();

private:
    friend class MyDB_BufferManager;
    bool pinned;
    bool anonymous;
    bool dirty;
    MyDB_TablePtr db_table;
    MyDB_BufferManager* db_bufMgr; 
    long pos;
    size_t referenceCount;
    long timestamp;
    void* bytes;
    size_t size;

    void removePage(MyDB_PagePtr page);
};

#endif