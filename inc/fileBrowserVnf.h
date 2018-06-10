#ifndef __FILEBROWSER_VNF_H__
#define __FILEBROWSER_VNF_H__

#include <confd_lib.h>
#include <confd_dp.h>
#include "mxLog.h"
#include "fileBrowserFile.h"

#define VNF_ID_NAME   "vnf-id"
#define VNF_LIST_NAME "vnf"

class fileBrowserVnf : private fileBrowserDiskOper
{
public:
    fileBrowserVnf() {};
    virtual ~fileBrowserVnf() {};

protected:
    int getVnfListFromSession(int sessionId, struct FileListMap *dp);
    void getVnfIdFromKeypath(confd_hkeypath_t *keypath, u32_t &vnfId, int vLoc);
    bool isVnfTable(const char *tableName);
    FileListEntry_t* findVnfEntryByIds(struct FileListMap *dp, u32_t sessionId, u32_t vnfId);
    int getVnfLists(struct FileListMap *dp, u32_t session, unsigned  int vnfID);
    FileListEntry_t* findVnfLevelEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId);
    int removeVnf(struct FileListMap *dp, confd_hkeypath_t *keypath);
};

#endif
