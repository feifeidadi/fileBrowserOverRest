#ifndef __FILEBROWSER_SESSION_H__
#define __FILEBROWSER_SESSION_H__

#include "fileBrowserDiskOper.h"

#define SESSION_LIST_NAME "session"
#define SESSION_ID_NAME "session-id"

class fileBrowserSession : private fileBrowserDiskOper
{
public:
    fileBrowserSession() {};
    virtual ~fileBrowserSession() {};

protected:
    int  getAllSessions(struct FileListMap *dp);
    void getSessionIdFromKeypath(confd_hkeypath_t *keypath, u32_t &sessionId, int sLoc);
    void getSessionAndVnfIds(confd_hkeypath_t *keypath, u32_t &sessionId, int sLoc, u32_t &vnfId, int vLoc);
    bool isSessionTable(const char *tableName);
    int createSessionDir(u32_t sessionId);
    FileListEntry_t *findSessionEntryById(struct FileListMap *dp, u32_t sessionId);
    FileListEntry_t *findSessionLevelEntry(struct FileListMap *dp);
    int getSessionLists(struct FileListMap *dp);
    int removeSession(struct FileListMap *dp, confd_hkeypath_t *keypath);
};

#endif
