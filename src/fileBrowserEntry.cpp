#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "fileBrowserEntry.h"

/////////////////////////////////////////////////
/* add an entry, keeping the list ordered */
void fileBrowserEntry::addListEntry(FileListEntry_t **first, FileListEntry_t * newEntry)
{
    FileListEntry_t **prev = first;

    while( *prev != NULL &&
        ((strcmp(newEntry->filename, (*prev)->filename) > 0) || (strcmp(newEntry->filename, (*prev)->filename) == 0 ))) {
        prev = &(*prev)->next;
    }
    newEntry->next = *prev;
    *prev = newEntry;
}

FileListEntry_t* fileBrowserEntry::newEntry(const char *value, int level, u32_t sessionId, u32_t vnfId)
{
    FileListEntry_t *pEntry = (FileListEntry_t *) calloc(1, sizeof(FileListEntry_t));
    if(pEntry == NULL) {
        return NULL;
    }

    if(level == SESSION_LEVEL_LIST_IDX) {
        setSessionEntryValues(pEntry, atoi(value));
    } else if(level == VNF_LEVEL_LIST_IDX) {
        setVnfEntryValues(pEntry, sessionId, atoi(value));
    } else { // FILE_LEVEL_LIST_IDX
        setFileEntryValues(pEntry, value, sessionId, vnfId);
    }

    return pEntry;
}

void fileBrowserEntry::setSessionEntryValues(FileListEntry_t *pEntry, u32_t sessionId)
{
    pEntry->sessionId = sessionId;
}

void fileBrowserEntry::setVnfEntryValues(FileListEntry_t *pEntry, u32_t sessionId, u32_t vnfId)
{
    pEntry->vnfId = vnfId;
    pEntry->sessionId = sessionId;
}

void fileBrowserEntry::setFileEntryValues(FileListEntry_t *pEntry, const char *filename, u32_t sessionId, u32_t vnfId)
{
    pEntry->sessionId = sessionId;
    pEntry->vnfId = vnfId;
    pEntry->size = fileStat.st_size;
    strncpy(pEntry->filename, filename, MAX_NAME_LEN-1);

    strftime(pEntry->creationTimestamp, MAX_NAME_LEN, "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));
    printf("size = %ld, creationTimestamp = %s.\n", fileStat.st_size, pEntry->creationTimestamp);
}
