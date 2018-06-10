#ifndef __FILEBROWSER_ENTRY_H__
#define __FILEBROWSER_ENTRY_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_NAME_LEN    128
#define MAX_PATH_LEN    256

#define  SESSION_LEVEL_LIST_IDX  0
#define  VNF_LEVEL_LIST_IDX      1
#define  FILE_LEVEL_LIST_IDX     2
#define  MAX_LEVEL_ENTRY         3

#define INVALID_ID 0xffffffff

#define u32_t unsigned int

typedef struct  FileListEntry {
    u32_t  sessionId;
    u32_t  vnfId;
    char filename[MAX_NAME_LEN];    
    u32_t  size;
    char creationTimestamp[MAX_NAME_LEN];
    struct FileListEntry *next;
} FileListEntry_t;

struct FileListMap {
    FileListEntry_t *table[MAX_LEVEL_ENTRY];
    struct timeval lastRead; 
};

class fileBrowserEntry
{
public:
    fileBrowserEntry() {};
    virtual ~fileBrowserEntry() {};

protected:
    void addListEntry(FileListEntry_t **first, FileListEntry_t * newEntry);
    FileListEntry_t* newEntry(const char *value, int level, u32_t sessionId, u32_t vnfId);

private:
    void setSessionEntryValues(FileListEntry_t *pEntry, u32_t sessionId);
    void setVnfEntryValues(FileListEntry_t *pEntry, u32_t sessionId, u32_t vnfId);
    void setFileEntryValues(FileListEntry_t *pEntry, const char *filename, u32_t sessionId, u32_t vnfId);
    bool isExpectedEntry(bool lsDir, const char *path, const char *name, int level);

protected:
    struct stat fileStat; // set in fileBrowserFileOper::isMode()
};

#endif
