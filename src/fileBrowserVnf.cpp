#include <stdio.h>
#include "fileBrowser.h"
#include "fileBrowserFile.h"
#include "fileBrowserDiskOper.h"
#include "fileBrowserVnf.h"

int fileBrowserVnf::getVnfListFromSession(int sessionId, struct FileListMap *dp)
{
    char sessionPath[256] = {};

    snprintf(sessionPath, sizeof(sessionPath), "%s/%d", ROOT_DIR, sessionId);

    return getSubDirs(sessionPath, dp, VNF_LEVEL_LIST_IDX, sessionId);
}

bool fileBrowserVnf::isVnfTable(const char *tableName)
{
    return (strcmp(tableName, VNF_LIST_NAME) == 0);
}

FileListEntry_t* fileBrowserVnf::findVnfEntryByIds(struct FileListMap *dp, u32_t sessionId, u32_t vnfId)
{
    FileListEntry_t *pEntry = findVnfLevelEntry(dp, sessionId, vnfId);

    while(pEntry != NULL) {
        if(pEntry->vnfId == vnfId) {
            printf("Vnf %d found under session %d.\n", pEntry->vnfId, pEntry->sessionId);
            break;
        }
        pEntry = pEntry->next;
    }

    return pEntry;
}

void fileBrowserVnf::getVnfIdFromKeypath(confd_hkeypath_t *keypath, u32_t &vnfId, int vLoc)
{
    vnfId = CONFD_GET_UINT32(&(keypath->v[vLoc][0]));
    printf("vnf-id(%d)\n", vnfId);
}

//
//  Get all vnf IDs for thh session and keep it in the map
//  return 1 of ok
//
int fileBrowserVnf::getVnfLists(struct FileListMap *dp, u32_t session, unsigned  int vnfID)
{
    // If we already have the file list for the sessionID and VnfID, skip it
    FileListEntry_t *pEntry = dp->table[VNF_LEVEL_LIST_IDX];
    if ((pEntry != NULL) && (pEntry->sessionId == session) && (pEntry->vnfId == vnfID)) {
        printf("getVnfLists() - skip getting file list since already existed\n");
        return 1;
    }

    // Release previous data
    FREE_ONE_LEVEL_LIST(dp, VNF_LEVEL_LIST_IDX);

#ifdef UNIT_TEST
    char fileName[MAX_NAME_LEN];
    strcpy(fileName, "VNF-leve");
    fakeSomeEntries(dp, VNF_LEVEL_LIST_IDX, fileName, 3);
#else
    getVnfListFromSession(session, dp);
#endif

    return 1;
}


FileListEntry_t *fileBrowserVnf::findVnfLevelEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId)
{
    FileListEntry_t *curr = NULL;
    //--- get available vnfs for this session
    printf("first (#2 - vnf level) - Next getVnfLists() \n");
    if (getVnfLists(dp, sessionId, vnfId) != CONFD_ERR) {
        curr = dp->table[VNF_LEVEL_LIST_IDX];
    }

    return curr;
}

int fileBrowserVnf::removeVnf(struct FileListMap *dp, confd_hkeypath_t *keypath)
{
    FileListEntry_t *pEntry = NULL;
    u32_t sessionId = 0, vnfId = 0;

    // '/file-browser/sessions/session{session-id}/vnfs/vnf{vnf-id}'
    printf("Vnf\n");
    sessionId = CONFD_GET_UINT32(&(keypath->v[3][0]));
    getVnfIdFromKeypath(keypath, vnfId, 0);
    pEntry = findVnfEntryByIds(dp, sessionId, vnfId);
    if(pEntry == NULL) {
        printf("Vnf %d has not been found under session %d.\n", vnfId, sessionId);
    } else {
        printf("Vnf %d found under session %d. Going to remove it.\n", vnfId, sessionId);
        rmVnfDir(sessionId, vnfId);
    }

    return 0;
}

