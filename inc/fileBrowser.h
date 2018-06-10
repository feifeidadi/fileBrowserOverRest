#ifndef __FILEBROWSER_MGR_H__
#define __FILEBROWSER_MGR_H__

#include <pthread.h>
#include "baseType.h"
#include "fileBrowserConfd.h"

class fileBrowser : private fileBrowserDiskOper
{
public:
    fileBrowser();
    virtual ~fileBrowser();
    static fileBrowser* getInstance();
    int init();

private:
    int createDetachedThread(pthread_t *thread, void *(*start_routine) (void *), void *arg);
    int createFileBrowserConfdThread();
    void createDir();

private:
    static fileBrowser *pInstance;
    pthread_t ctConfdThreadId;
};

#endif
