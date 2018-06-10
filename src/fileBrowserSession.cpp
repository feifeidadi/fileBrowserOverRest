#include <string.h>
#include "fileBrowserConfd.h"

int fileBrowserSession::getAllSessions(struct FileListMap *dp)
{
    return getSubDirs(ROOT_DIR, dp, SESSION_LEVEL_LIST_IDX);
}

bool fileBrowserSession::isSessionTable(const char *tableName)
{
    return (strcmp(tableName, SESSION_LIST_NAME) == 0);
}

int fileBrowserSession::createSessionDir(u32_t sessionId)
{
    int ret = CONFD_ERR;

    if(sessionId >= 0) {
        char id[MAX_PATH_LEN] = {};
        snprintf(id, MAX_PATH_LEN, "%s/%d", ROOT_DIR, sessionId);
        ret = mkDir(id);
    } else {
        ret = CONFD_ERRCODE_APPLICATION;
        printf("Invalid sessiond-id(%d).\n", sessionId);
    }

    return ret;
}

FileListEntry_t* fileBrowserSession::findSessionEntryById(struct FileListMap *dp, u32_t sessionId)
{
    FileListEntry_t *pEntry = findSessionLevelEntry(dp);
    while(pEntry != NULL) {
        if(pEntry->sessionId == sessionId) {
            printf("Session %d found.\n", sessionId);
            break;
        }
        pEntry = pEntry->next;
    }

    if(pEntry == NULL) {
        printf("Session %d not found.\n", sessionId);
    }

    return pEntry;
}

FileListEntry_t* fileBrowserSession::findSessionLevelEntry(struct FileListMap *dp)
{
    FileListEntry_t *curr = NULL;

    printf("first (#1 - Session level) - Next getSessionLists() \n");
    //--- get al available sessions
    if (getSessionLists(dp) != CONFD_ERR) {
        curr = dp->table[SESSION_LEVEL_LIST_IDX];
    }

    return curr;
}

void fileBrowserSession::getSessionAndVnfIds(confd_hkeypath_t *keypath, u32_t &sessionId, int sLoc, u32_t &vnfId, int vLoc)
{
    getSessionIdFromKeypath(keypath, sessionId, sLoc);
    //getVnfIdFromKeypath(keypath, vnfId, vLoc);
    vnfId = CONFD_GET_UINT32(&(keypath->v[vLoc][0]));
    printf("vnf-id(%d)\n", vnfId);
}

void fileBrowserSession::getSessionIdFromKeypath(confd_hkeypath_t *keypath, u32_t &sessionId, int sLoc)
{
    sessionId = CONFD_GET_UINT32(&(keypath->v[sLoc][0]));
    printf("session-id(%d)\n", sessionId);
}

//
//  Get all session IDs and keep it in the map
//  return 1 of ok
//
int fileBrowserSession::getSessionLists(struct FileListMap *dp)
{
    // Get all file lists for this session 

    // Release previous data
    FREE_ONE_LEVEL_LIST(dp, SESSION_LEVEL_LIST_IDX);

#ifdef UNIT_TEST
    char fileName[MAX_NAME_LEN];
    strcpy(fileName, "Session-leve");
    fakeSomeEntries(dp, SESSION_LEVEL_LIST_IDX, fileName, 2);
#else
    getAllSessions(dp);
#endif

    return 1;
}

int fileBrowserSession::removeSession(struct FileListMap *dp, confd_hkeypath_t *keypath)
{
    FileListEntry_t *pEntry = NULL;
    u32_t sessionId = 0;

    // '/file-browser/sessions/session{session-id}'
    printf("Session\n");
    getSessionIdFromKeypath(keypath, sessionId, 0);
    pEntry = findSessionEntryById(dp, sessionId);
    if(pEntry == NULL) {
        printf("Session %d not found.\n", sessionId);
    } else {
        printf("Session %d found. Going to remove it.\n", sessionId);
        rmSessionDir(sessionId);
    }

    return 0;
}



