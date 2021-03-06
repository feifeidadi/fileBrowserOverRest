#ifndef __FILEBROWSER_CONFD_H__
#define __FILEBROWSER_CONFD_H__

#include <time.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <confd_lib.h>
#include <confd_dp.h>

#include "fileBrowserConfdCB.h"
#include "fileBrowserSession.h"
#include "fileBrowserVnf.h"
#include "fileBrowserFile.h"
#include "fileBrowser.h"
#include "my-fileBrowser.h" // Generated by confdc

#define ENUM_TYPE(x) x
#define STRING_TYPE(x) #x

#define CONFD_CB_NAMES(T) \
    T(CONFD_CB_UNDEFINED = 0), \
    T(CONFD_TRANS_INIT), \
    T(CONFD_TRANS_FINISH), \
    T(CONFD_DATA_GET_NEXT), \
    T(CONFD_DATA_GET_ELEM), \
    T(CONFD_DATA_SET_ELEM), \
    T(CONFD_DATA_REMOVE), \
    T(CONFD_DATA_CREATE), \
    T(CONFD_CB_MAX)

typedef enum {
    CONFD_CB_NAMES(ENUM_TYPE)
} ConfdCallback_e;

using namespace std;

void* fileBrowserConfdThread(void * arg);

class fileBrowserConfd : private fileBrowserSession, private fileBrowserVnf, private fileBrowserFile
{
public:
    fileBrowserConfd();
    virtual ~fileBrowserConfd();

    static fileBrowserConfd* getInstance();
    void* pollConfdSockFds();
    int init();
    int executeConfCB(ConfdCallback_e cbIndex, struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath=NULL, long next=-1);

private:
    void initCallbacks();
    void execConfdAPIs();
    void initPullFd(struct pollfd *set, int index, int sock_fd);
    void initPullFDs(struct pollfd *set);
    void initDataCallbacks();
    void initTransCallbacks();
    void registerConfdCBs();
    void connectConfdSockets(struct sockaddr_in *addr);
    void createConfdSockets();
    void initSockAddr(struct sockaddr_in &addr);

    void logFatalErrorAndExit(const char *logMsg);
    const char* getConfdCBName(ConfdCallback_e cbIndex);

    int setReplyNextValues(confd_value_t *v, const char *tableName, FileListEntry_t *curr);
    int setNextValuesAndReply(struct confd_trans_ctx *tctx, const char *tableName, FileListEntry_t *curr);
    int setReplyValue(confd_hkeypath_t *keypath, confd_value_t *v, FileListEntry_t *pEntry);
    int setValueAndReply(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, FileListEntry_t *pEntry);
    void getLastKeyPathToken(char *keyPathP, std::string &lastToken);
    void getTableName(const char *keyPathP, std::string &fullTblName, std::string &tableName);
    void freeAllFileListMap(struct FileListMap *dp);
    FileListEntry_t *findCurrentEntry(const char *tableName, struct FileListMap *dp, confd_hkeypath_t *keypath, long next);
    FileListEntry_t* getNextFileEntry(struct FileListMap *dp);
    void showKeypathInfo(confd_hkeypath_t *keypath);
    int getKeypathInfo(char *keyPathP, string &fullTblName, string &tableName, string &lastKpathTokenName);
    int removeFile(struct FileListMap *dp, confd_hkeypath_t *keypath);
    int removeFileFromDisk(u32_t sessionId, u32_t vnfId, const char *filename);

    // Callbacks for confd
    int tInit(struct confd_trans_ctx *tctx);
    int tFinish(struct confd_trans_ctx *tctx);
    int getElem(struct confd_trans_ctx* tctx, confd_hkeypath_t* keypath);
    int setElem(struct confd_trans_ctx* tctx, confd_hkeypath_t* keypath, confd_value_t *newval);
    int getNext(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next);
    int Create(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath);
    int doRemove(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath);

public:
    static fileBrowserConfd* pInstance;
private:
    static const char *confdCBNames[];
    static struct confd_daemon_ctx *dctx;
    static struct confd_trans_cbs trans;
    static struct confd_data_cbs  data;
    static int ctlsock;
    static int workersock;
};

#endif
