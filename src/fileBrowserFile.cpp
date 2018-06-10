#include "fileBrowserFile.h"

bool fileBrowserFile::fileListExisted(int headIdx, struct FileListMap *dp, u32_t session, u32_t vnfID)
{
    FileListEntry_t *pEntry = dp->table[headIdx];
    if ((pEntry != NULL) && (pEntry->sessionId == session) && (pEntry->vnfId == vnfID))
        return true;

    return false;
}

void fileBrowserFile::freeOneLevelList(struct FileListMap *dp, int level)
{
    FREE_ONE_LEVEL_LIST(dp, level);
}

FileListEntry_t* fileBrowserFile::findFileEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId, const char *filename)
{
    FileListEntry_t *pEntry = findFileLevelEntry(dp, sessionId, vnfId);

    while(pEntry != NULL) {
        if(!strcmp(pEntry->filename, filename)) {
            printf("File %s found under session %d vnf %d.\n", 
                pEntry->filename, pEntry->sessionId, pEntry->vnfId);
            break;
        }
        pEntry = pEntry->next;
    }

    return pEntry;
}

FileListEntry_t* fileBrowserFile::findFileEntry(confd_hkeypath_t *keypath, struct FileListMap *dp)
{

    char *pFileName = (char *) CONFD_GET_BUFPTR(&keypath->v[1][0]);
    printf("findFileEntry() return keypath fileName = '%s'\n", pFileName);

    FileListEntry_t *pEntry = dp->table[FILE_LEVEL_LIST_IDX];

    while (pEntry != NULL) {
        if (strcmp(pFileName, pEntry->filename) == 0) {
            printf("findFileEntry() found the entry for filename '%s'\n", pFileName);
            return pEntry;
        }
        pEntry = pEntry->next;
    }

    printf("findFileEntry() cannot find  the entry for filename '%s'\n", pFileName);
    return NULL;
}

FileListEntry_t* fileBrowserFile::findFileLevelEntry(struct FileListMap *dp, u32_t sessionId, u32_t vnfId)
{
    FileListEntry_t *curr = NULL;
    printf("first (#3 - file Level, get all file list for session %d, vnf %d'\n", sessionId, vnfId);

    if (getFileLists(dp, sessionId, vnfId) != CONFD_ERR) {
        curr = dp->table[FILE_LEVEL_LIST_IDX];
    }

    return curr;
}

int fileBrowserFile::getFileLists(struct FileListMap *dp, u32_t session, u32_t vnfID)
{
    // If we already have the file list for the sessionID and VnfID, skip it
    if (fileListExisted(FILE_LEVEL_LIST_IDX , dp, session, vnfID) ) {
        printf("getFileLists() - skip getting file list since already existed\n");
        return 1;
    }
    // Release previous data
    freeOneLevelList(dp, FILE_LEVEL_LIST_IDX);

    getFileListFromVnf(session, vnfID, dp);

    return 1;
}

int fileBrowserFile::getFileListFromVnf(int sessionId, int vnfId, struct FileListMap *dp)
{
    char vnfPath[256] = {};

    snprintf(vnfPath, sizeof(vnfPath), "%s/%d/%d", ROOT_DIR, sessionId, vnfId);

    return lsFile(vnfPath, dp, FILE_LEVEL_LIST_IDX, sessionId, vnfId);
}

