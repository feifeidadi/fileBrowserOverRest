#ifndef __FILEBROWSER_FILE_H__
#define __FILEBROWSER_FILE_H__

#include <stdlib.h>
#include <string.h>
#include <confd_lib.h>
#include <confd_dp.h>
#include "mxLog.h"
#include "fileBrowserDiskOper.h"

#define FREE_ONE_LEVEL_LIST(dp, level) \
do { \
    FileListEntry_t *pEntry = dp->table[level]; \
    while (pEntry) { \
        FileListEntry_t *next = pEntry->next; \
        free(pEntry); \
        pEntry = next; \
    } \
    dp->table[level] = NULL; \
} while(0)

class fileBrowserFile : private fileBrowserDiskOper
{
public:
    fileBrowserFile() {};
    virtual ~fileBrowserFile() {};

protected:
    bool fileListExisted(int headIdx, struct FileListMap *dp, u32_t session, u32_t vnfID);
    void freeOneLevelList(struct FileListMap *dp, int level);
    FileListEntry_t* findFileEntry(confd_hkeypath_t *keypath, struct FileListMap *dp);
    FileListEntry_t* findFileEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId, const char *filename);
    FileListEntry_t* findFileLevelEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId);
    int getFileLists(struct FileListMap *dp, u32_t session, u32_t vnfID);
    int getFileListFromVnf(int sessionId, int vnfId, struct FileListMap *dp);
};

#endif
