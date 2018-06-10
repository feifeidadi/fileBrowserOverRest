#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <confd_lib.h>
#include <confd_dp.h>

#include "fileBrowser.h"
#include "fileBrowserConfd.h"
#include "fileBrowserDiskOper.h"

static
void *executeCommandThread(void *command)
{
    char *cmd = (char *) command;

    printf("Execute command: %s\n", cmd);
    if(cmd != NULL) {
        system(cmd);
        free(cmd);
    }

    return NULL;
}

bool fileBrowserDiskOper::isMode(const char *path, const char *name, const u32_t mode)
{
    char subdir[256];

    if(name != NULL) {
        snprintf(subdir, sizeof(subdir), "%s/%s", path, name);
    } else {
        snprintf(subdir, sizeof(subdir), "%s", path);
    }
    memset(&fileStat, 0, sizeof(struct stat));
    if (stat(subdir, &fileStat) == 0) {
        if((fileStat.st_mode & S_IFMT) == mode) {
            return true;
        }
    }

    return false;
}

bool fileBrowserDiskOper::isDir(const char *path, const char *info)
{
    return isDir(path, NULL, info);
}

bool fileBrowserDiskOper::isDir(const char *path, const char *name, const char *info)
{
    bool dir = isMode(path, name, S_IFDIR);
    if(dir) {
        printf("[%s: %s]\n", info, (name != NULL ? name : path));
    }

    return dir;
}

bool fileBrowserDiskOper::isRegularFile(const char *fullPathFile)
{
    return isRegularFile(fullPathFile, NULL);
}

bool fileBrowserDiskOper::isRegularFile(const char *path, const char *name)
{
    bool isRegFile = isMode(path, name, S_IFREG);
    if(isRegFile) {
        printf("File %s exists.\n", name);
    }

    return isRegFile;
}

int
fileBrowserDiskOper::getFiles(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    return readDir(false, path, dp, level, sessionId, vnfId);
}

int
fileBrowserDiskOper::getDirs(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    return readDir(true, path, dp, level, sessionId, vnfId);
}

bool fileBrowserDiskOper::isExpectedEntry(bool lsDir, const char *path, const char *name, int level)
{
    bool ret = false;

    if(lsDir) {
        const char *info = (level == VNF_LEVEL_LIST_IDX ? "vnf-id" : "session-id");
        ret = isDir(path, name, info);
    } else {
        ret = isRegularFile(path, name);
    }

    return ret;
}

bool fileBrowserDiskOper::dirExisted(const char *path)
{
    DIR *dir;
    if (!(dir = opendir(path))) {
        return false;
    }

    closedir(dir);
    return true;
}

// lsDir is true for getting dir list, else file list
int fileBrowserDiskOper::readDir(bool lsDir, const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path)))
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if(isExpectedEntry(lsDir, path, entry->d_name, level)) {
            FileListEntry_t *pEntry = newEntry(entry->d_name, level, sessionId, vnfId);
            addListEntry(&dp->table[level], pEntry);
        }
    }
    closedir(dir);

    return CONFD_OK;
}

int fileBrowserDiskOper::lsFile(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    getFiles(path, dp, level, sessionId, vnfId);

    return CONFD_OK;
}

int fileBrowserDiskOper::lsDir(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    getDirs(path, dp, level, sessionId, vnfId);

    return CONFD_OK;
}

int fileBrowserDiskOper::getSubDirs(const char *path, struct FileListMap *dp, int level, u32_t sessionId, u32_t vnfId)
{
    return lsDir(path, dp, level, sessionId, vnfId);
}

int fileBrowserDiskOper::executeCommand(const char *cmd)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&cmdThreadId, &attr, executeCommandThread, (void *)strdup(cmd));

    // TODO: thread return value?
    return 0;
}

int fileBrowserDiskOper::rmDir(const char *path)
{
    return 0;
}

int fileBrowserDiskOper::rmVnfDir(u32_t sessionId, u32_t vnfId)
{
    char command[MAX_PATH_LEN];
    snprintf(command, MAX_PATH_LEN, "rm -rf %s/%d/%d", ROOT_DIR, sessionId, vnfId);

    return executeCommand(command);
}

int fileBrowserDiskOper::rmSessionDir(u32_t sessionId)
{
    char command[MAX_PATH_LEN];
    snprintf(command, MAX_PATH_LEN, "rm -rf %s/%d", ROOT_DIR, sessionId);

    return executeCommand(command);
}

int fileBrowserDiskOper::mkDir(const char *path)
{
    if(mkdir(path, 0755) == 0) {
        printf("Create directory (%s) successfully.\n", path);
        return CONFD_OK;
    } else {
        printf("Create directory (%s) failed. %s.\n", path, strerror(errno));
        if(errno == EEXIST) {
            return CONFD_ERRCODE_IN_USE;
        }
    }

    return CONFD_ERR;
}

s32_t fileBrowserDiskOper::removeExpiredFiles(const char *path, u32_t timePeriod)
{
    if(!isDir(path, "Directory")) {
        printf("Directory (%s) does not exist.\n", path);
        return -1;
    }

    char command[256] = {};
    char rmScript[] = "/usr/IMS/current/bin/remove_expired_files.sh";
    if(!isRegularFile(rmScript)) {
        printf("script (%s) does not exist.\n", rmScript);
        return -1;
    }

    snprintf(command, sizeof(command), "sh %s -t trace -m %d %s", rmScript, timePeriod, path);

    return executeCommand(command);
}

/* 
 * Return: opened file stream or NULL
 * Output: file length
*/
FILE* fileBrowserDiskOper::openFile(const char *fileName, u32_t &length)
{
    FILE *f = fopen(fileName, "rb");
    if (f == NULL) {
        printf("Failed to open file: %s!\n", fileName);
        return NULL;
    } 

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    printf("file size: %d.\n", length);
    if(length == 0) {
        printf("Empty file(%s)!\n", fileName);
    }
    fseek(f, 0, SEEK_SET);

    return f;
}

// It's caller's responsibility to free the returning buffer.
unsigned char* fileBrowserDiskOper::getFileContent(FILE *f, u32_t &length)
{
    unsigned char* buffer = (unsigned char *)malloc(length);
    if(buffer == NULL) {
        printf("Failed to alloc memory!\n");
        fclose(f);
        return NULL;
    }

    if(length != fread(buffer, sizeof(unsigned char), length, f)) { 
        printf("fread() failed!\n");
        free(buffer);
        fclose(f);
        return NULL;
    } 
    fclose(f);

    return buffer;
}

