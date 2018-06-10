#include "fileBrowser.h"

fileBrowser* fileBrowser::pInstance = NULL;

fileBrowser::fileBrowser()
{
	pInstance = this;
}

fileBrowser::~fileBrowser()
{
}

fileBrowser* fileBrowser::getInstance()
{
    return (pInstance == NULL ? new fileBrowser() : pInstance);
}

int fileBrowser::createDetachedThread(pthread_t *thread, void *(*start_routine) (void *), void *arg)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(thread, &attr, start_routine, arg);

    return 0;
}

int fileBrowser::createFileBrowserConfdThread()
{
    return createDetachedThread(&ctConfdThreadId, fileBrowserConfdThread, NULL);
}

void fileBrowser::createDir()
{
    if(!dirExisted(ROOT_DIR)) {
        mkDir(ROOT_DIR);
    }
}

int fileBrowser::init()
{
    createDir();

    createFileBrowserConfdThread();

    return 0;
}

