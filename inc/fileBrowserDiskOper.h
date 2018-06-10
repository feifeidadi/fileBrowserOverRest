#ifndef __FILEBROWSER_DISK_OPER_H__
#define __FILEBROWSER_DISK_OPER_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "baseDef.h"
#include "fileBrowserEntry.h"

#define ROOT_DIR "/data"

class fileBrowserDiskOper : private fileBrowserEntry
{
public:
    fileBrowserDiskOper() {};
    virtual ~fileBrowserDiskOper() {};

protected:
    int mkDir(const char *path);
    int rmDir(const char *path);
    int rmSessionDir(u32_t sessionId);
    int rmVnfDir(u32_t sessionId, u32_t vnfId);
    s32_t removeExpiredFiles(const char *path, u32_t timePeriod);
    int lsFile(const char *path, struct FileListMap *dp, int level, u32_t sessionId=0, u32_t vnfId=0);
    int lsDir(const char *path, struct FileListMap *dp, int level, u32_t sessionId=0, u32_t vnfId=0);
    int getSubDirs(const char *path, struct FileListMap *dp, int level, u32_t sessionId=0, u32_t vnfId=0);
    FILE* openFile(const char *fileName, u32_t &length);
    unsigned char* getFileContent(FILE *f, u32_t &length);
    bool dirExisted(const char *);

private:
    bool isMode(const char *path, const char *name, const u32_t mode);
    bool isDir(const char *path, const char *info);
    bool isDir(const char *path, const char *name, const char *info);
    bool isRegularFile(const char *path, const char *name);
    bool isRegularFile(const char *fullPathFile);
    int readDir(bool lsDir, const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId);
    int getFiles(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId);
    int getDirs(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId);
    bool isExpectedEntry(bool lsDir, const char *path, const char *name, int level);
    int executeCommand(const char *cmd);

private:
    pthread_t cmdThreadId;
};

#endif
