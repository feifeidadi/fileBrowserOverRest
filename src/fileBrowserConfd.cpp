#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <string.h>

#include "fileBrowserConfd.h"

using namespace std;

fileBrowserConfd* fileBrowserConfd::pInstance = NULL;
struct confd_daemon_ctx* fileBrowserConfd::dctx = NULL;
struct confd_trans_cbs fileBrowserConfd::trans;
struct confd_data_cbs  fileBrowserConfd::data;
int fileBrowserConfd::ctlsock = 0;
int fileBrowserConfd::workersock = 0;
const char *fileBrowserConfd::confdCBNames[] = { CONFD_CB_NAMES(STRING_TYPE) };

fileBrowserConfd::fileBrowserConfd()
{
}

fileBrowserConfd::~fileBrowserConfd()
{
}

int fileBrowserConfd::init()
{
    initCallbacks();
    execConfdAPIs();

    return 0;
}

fileBrowserConfd* fileBrowserConfd::getInstance()
{
    return (pInstance == NULL ? new fileBrowserConfd() : pInstance);
}

void fileBrowserConfd::freeAllFileListMap(struct FileListMap *dp)
{
    for (int i=0; i <  MAX_LEVEL_ENTRY; i++) {
        freeOneLevelList(dp, i);
    }
}

void fileBrowserConfd::getTableName(const char *keyPathP, string & fullTblName, string &tableName)
{
    // /file-browser/session{0}/vnf
    string keyPathStr = keyPathP;
    u32_t keyPos = keyPathStr.find("{");
    
    if (keyPos != string::npos) {
        fullTblName.assign(keyPathStr, 0, keyPos);
        string tempStr = fullTblName;
        u32_t lastSlashPos = tempStr.find_last_of("/");
        tableName = tempStr.assign(tempStr, lastSlashPos+1, keyPos);        
    } else {
        fullTblName="";
        tableName="";
    }
}

void fileBrowserConfd::getLastKeyPathToken(char *keyPathP, string & lastToken) 
{
    string keyPathStr = keyPathP;

    u32_t lastSlashPos = keyPathStr.find_last_of("/");

    char * pLastToken = keyPathP + lastSlashPos +1;

    lastToken = pLastToken;
}

int fileBrowserConfd::setReplyNextValues(confd_value_t *v, const char *tableName, FileListEntry_t *curr)
{

    if (isSessionTable(tableName)) {
        CONFD_SET_UINT32(v, curr->sessionId);
        printf("CONFD_SET_UINT32() sessionId=%d \n", curr->sessionId);
    }
    else if (isVnfTable(tableName)) {
        CONFD_SET_UINT32(v, curr->vnfId);
        printf("CONFD_SET_UINT32(), vnf=%d \n", curr->vnfId);
    } else {
        CONFD_SET_STR(v, curr->filename);
        printf("CONFD_SET_STR(), fileName='%s' \n", curr->filename);
    }

    return 0;
}

int fileBrowserConfd::setNextValuesAndReply(struct confd_trans_ctx *tctx, const char *tableName, FileListEntry_t *curr)
{
    confd_value_t v[3];
    int keys = 1;

    if (isSessionTable(tableName)) {
        CONFD_SET_UINT32(&v[0], curr->sessionId);
        printf("CONFD_SET_UINT32() sessionId=%d \n", curr->sessionId);
    }
    else if (isVnfTable(tableName)) {
        CONFD_SET_UINT32(&v[0], curr->vnfId);
        printf("CONFD_SET_UINT32(), vnf=%d \n", curr->vnfId);
    } else {
        CONFD_SET_STR(&v[0], curr->filename);
        printf("fileName='%s', size=%d.\n", curr->filename, curr->size);
    }

    confd_data_reply_next_key(tctx, v, keys, (long)curr->next);

    return 0;
}

FileListEntry_t *fileBrowserConfd::findCurrentEntry(const char *tableName, struct FileListMap *dp, confd_hkeypath_t *keypath, long next)
{
    FileListEntry_t *pEntry = NULL;

    if (next == -1) {  /* first call */
        u32_t sessionId = 0, vnfId = 0;

        if (isSessionTable(tableName)) {
            pEntry = findSessionLevelEntry(dp);
        } else if (isVnfTable(tableName)) {
            getSessionIdFromKeypath(keypath, sessionId, 2);
            pEntry = findVnfLevelEntry(dp, sessionId, vnfId);
         } else {
            getSessionAndVnfIds(keypath, sessionId, 5, vnfId, 2);
            pEntry = findFileLevelEntry(dp, sessionId, vnfId);
        }
    } else {
        pEntry = (FileListEntry_t *)next;
        printf("Return next pointer %p \n", pEntry);
    }

    return pEntry;
}

int fileBrowserConfd::getNext(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next)
{
    struct confd_cs_node *csp = confd_find_cs_node(keypath, keypath->len);
    const char *tableName=NULL;

    if (csp)
        tableName = confd_hash2str(csp->tag);  //--- get just table name

    char keyPathP[MAX_NAME_LEN];
    confd_pp_kpath(keyPathP, MAX_NAME_LEN, keypath);
    printf("\nget_next(), next=%ld, table name path ='%s', table name='%s' \n", next, keyPathP, tableName);

    struct FileListMap *dp = (struct FileListMap *) tctx->t_opaque;
    FileListEntry_t *curr = findCurrentEntry(tableName, dp, keypath, next);

    if (curr == NULL) {
        printf("curr is NULL, return empty\n");
        confd_data_reply_next_key(tctx, NULL, -1, -1);
        return CONFD_OK;
    }

    setNextValuesAndReply(tctx, tableName, curr);

    return CONFD_OK;
}

int fileBrowserConfd::tFinish(struct confd_trans_ctx *tctx)
{
    struct FileListMap *dp = (struct FileListMap *) tctx->t_opaque;
    printf("t_finish() \n");

    if (dp != NULL) {
        freeAllFileListMap(dp);
        free(dp);
        dp = NULL;
    }

    return CONFD_OK; 
}

int fileBrowserConfd::tInit(struct confd_trans_ctx *tctx)
{
    struct FileListMap *pMap = new FileListMap;
    if (pMap == NULL)
        return CONFD_ERR;

    memset(pMap, 0, sizeof(struct FileListMap));
    tctx->t_opaque = pMap;
    
    char buf[INET6_ADDRSTRLEN];
    memset(buf, 0, sizeof(buf));
    tctx->t_opaque = pMap;
    inet_ntop(tctx->uinfo->af, &tctx->uinfo->ip, buf, sizeof(buf));
    printf("--- t_init() for %s from %s\n", tctx->uinfo->username, buf);

    confd_trans_set_fd(tctx, workersock);

    return CONFD_OK;    
}

FileListEntry_t* fileBrowserConfd::getNextFileEntry(struct FileListMap *dp)
{
    static int index = 0;
    FileListEntry_t *pEntry = dp->table[FILE_LEVEL_LIST_IDX];
    for (int i = 0; i < index && pEntry != NULL; i++) {
        pEntry = pEntry->next;
    }

    index = (pEntry == NULL ? 0 : (index+1));

    return pEntry;
}


int fileBrowserConfd::setReplyValue(confd_hkeypath_t *keypath, confd_value_t *v, FileListEntry_t *pEntry)
{
    int ret = CONFD_OK;

    switch (CONFD_GET_XMLTAG(&(keypath->v[0][0]))) {
    case my_fileBrowser_size:
        printf("set size '%d' \n", pEntry->size);
        CONFD_SET_UINT32(v, pEntry->size);
        break;
    case my_fileBrowser_creation_timestamp:
        printf("set timestamp'%s' \n", pEntry->creationTimestamp);
        CONFD_SET_STR(v, pEntry->creationTimestamp);
    break;
    case my_fileBrowser_filename:
        printf("set filename '%s' \n", pEntry->filename);
        CONFD_SET_STR(v, pEntry->filename);
        break;

    case my_fileBrowser_session_id:
        printf("set sessionId '%d' \n", pEntry->sessionId);
        CONFD_SET_UINT32(v, pEntry->sessionId);
        break;

    case my_fileBrowser_vnf_id:
        printf("set sessionId '%d' \n", pEntry->vnfId);
        CONFD_SET_UINT32(v, pEntry->vnfId);
        break;

    default:
        printf("Unknow xml tag '%d' \n", CONFD_GET_XMLTAG(&(keypath->v[0][0])));
        ret = CONFD_ERR;
    }

    return ret;
}

int fileBrowserConfd::setValueAndReply(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, FileListEntry_t *pEntry)
{
    if (pEntry == NULL) {
        confd_data_reply_not_found(tctx);
        printf("Not found the fileEntry\n");
        return CONFD_OK;
    }
    printf("Found the entry. fileName='%s', timeStamp='%s', size=%d, sessionId=%d, vnfId=%d.\n", pEntry->filename, pEntry->creationTimestamp, pEntry->size, pEntry->sessionId, pEntry->vnfId);

    confd_value_t v;
    setReplyValue(keypath, &v, pEntry);

    confd_data_reply_value(tctx, &v);

    return CONFD_OK;
}

int fileBrowserConfd::Create(struct confd_trans_ctx *tctx,   confd_hkeypath_t *keypath)
{
    struct confd_cs_node *csp = confd_find_cs_node(keypath, keypath->len);
    const char *tableName=NULL;

    if (csp)
        tableName = confd_hash2str(csp->tag);  //--- get just table name

    char keyPathP[MAX_NAME_LEN];
    confd_pp_kpath(keyPathP, MAX_NAME_LEN, keypath);
    printf("\nTable name path ='%s', table name='%s' \n", keyPathP, tableName);

    u32_t sessionId = CONFD_GET_UINT32(&(keypath->v[0][0]));
    printf("Going to create session-id(%d) directory.\n", sessionId);
    return createSessionDir(sessionId);
}

int fileBrowserConfd::removeFileFromDisk(u32_t sessionId, u32_t vnfId, const char *filename)
{
    char command[MAX_PATH_LEN];
    snprintf(command, MAX_PATH_LEN, "rm -f %s/%d/%d/%s", 
            ROOT_DIR, sessionId, vnfId, filename);
    printf("Execute command: %s\n", command);

    return system(command);
}

int fileBrowserConfd::getKeypathInfo(char *keyPathP, string &fullTblName, string &tableName, string &lastKpathTokenName)
{
    getTableName(keyPathP, fullTblName, tableName);
    getLastKeyPathToken(keyPathP, lastKpathTokenName);
    printf("\nget_elem(), table name path ='%s', fullTblName ='%s', table = '%s', lastKpathTokenName = '%s' \n", keyPathP, fullTblName.c_str(), tableName.c_str(), lastKpathTokenName.c_str());
    return 0;
}

void fileBrowserConfd::showKeypathInfo(confd_hkeypath_t *keypath)
{
    char keyPathP[MAX_NAME_LEN];
    confd_pp_kpath(keyPathP, MAX_NAME_LEN, keypath);

    string fullTblName, tableName, lastKpathTokenName;
    getKeypathInfo(keyPathP, fullTblName, tableName, lastKpathTokenName);
}

int fileBrowserConfd::removeFile(struct FileListMap *dp, confd_hkeypath_t *keypath)
{
    FileListEntry_t *pEntry = NULL;
    u32_t sessionId = 0, vnfId = 0;

    // e.g. '/file-browser/sessions/session{100}/vnfs/vnf{1}/files/file{1.pcap}'
    char *filename = (char*)CONFD_GET_BUFPTR(&keypath->v[0][0]);
    getSessionAndVnfIds(keypath, sessionId, 6, vnfId, 3);
    printf("File: %s/%d/%d/%s\n", ROOT_DIR, sessionId, vnfId, filename);
    pEntry = findFileEntry(dp, sessionId, vnfId, filename);
    if(pEntry == NULL) {
        printf("file %s has not been found under session %d vnf %d.\n",
             filename, vnfId, sessionId);
    } else {
        printf("Vnf %d found under session %d. Going to remove it.\n", vnfId, sessionId);
        removeFileFromDisk(sessionId, vnfId, filename);
    }

    return 0;
}

int fileBrowserConfd::doRemove(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath)
{
    struct FileListMap *dp = (struct FileListMap *) tctx->t_opaque;
    showKeypathInfo(keypath);

    confd_value_t *list = &keypath->v[1][0];
    if (CONFD_GET_XMLTAG(list) == my_fileBrowser_session) {
        removeSession(dp, keypath);
    } else if (CONFD_GET_XMLTAG(list) == my_fileBrowser_vnf) {
        removeVnf(dp, keypath);
    } else if(CONFD_GET_XMLTAG(list) == my_fileBrowser_file) {
        removeFile(dp, keypath);
    }

    return CONFD_OK;
}

int fileBrowserConfd::setElem(struct confd_trans_ctx* tctx, confd_hkeypath_t* keypath, confd_value_t *newval)
{
    //--- fill in your code here

    return CONFD_OK;
}

int fileBrowserConfd::getElem(struct confd_trans_ctx* tctx, confd_hkeypath_t* keypath)
{
    char keyPathP[MAX_NAME_LEN];
    string fullTblName, tableName, lastKpathTokenName;
    struct FileListMap *dp = (struct FileListMap *) tctx->t_opaque;

    confd_pp_kpath(keyPathP, MAX_NAME_LEN, keypath);
    getKeypathInfo(keyPathP, fullTblName, tableName, lastKpathTokenName);

    FileListEntry_t *pEntry = NULL;
    u32_t sessionId = 0, vnfId = 0;
    if (lastKpathTokenName.compare(VNF_ID_NAME) == 0) {
        // '/file-browser/sessions/session{session-id}/vnfs/vnf{vnf-id}/vnf-id'
        getSessionAndVnfIds(keypath, sessionId, 4, vnfId, 1);
        pEntry = findVnfEntryByIds(dp, sessionId, vnfId);
    } else if (lastKpathTokenName.compare(SESSION_ID_NAME) == 0) {
        // '/file-browser/sessions/session{session-id}/session-id'
        getSessionIdFromKeypath(keypath, sessionId, 1);
        pEntry = findSessionEntryById(dp, sessionId);
    } else {
        // e.g. '/file-browser/sessions/session{session-id}/vnfs/vnf{vnf-id}/files/file{filename}/creation-timestamp'
        getSessionAndVnfIds(keypath, sessionId, 7, vnfId, 4);

        getFileLists(dp, sessionId, vnfId);
        pEntry = findFileEntry(keypath, dp);
    }

    return setValueAndReply(tctx, keypath, pEntry);;
}

void fileBrowserConfd::logFatalErrorAndExit(const char *logMsg)
{
    printf("ConfD Fatal Error: %s", logMsg);
    confd_fatal(logMsg);    
}

void fileBrowserConfd::initTransCallbacks()
{
    /* Transaction callbacks */
    trans.init = t_init;
    trans.finish = t_finish;
}

void fileBrowserConfd::initDataCallbacks()
{
    /* And finallly these are our read/write callbacks for  */
    /* the servers database */
    data.get_elem = get_elem;
    data.get_next = get_next;
    data.set_elem = set_elem;
    data.create   = create;
    data.remove   = doremove;
    strcpy(data.callpoint, my_fileBrowser__callpointid_fileBrowserCP);  //--- this is the callpoint 
}

void fileBrowserConfd::initCallbacks()
{
    initDataCallbacks();
    initTransCallbacks();
}

const char* fileBrowserConfd::getConfdCBName(ConfdCallback_e cbIndex)
{
    u32_t index = cbIndex < CONFD_CB_MAX ? cbIndex : CONFD_CB_UNDEFINED;
    return confdCBNames[index];
}

int fileBrowserConfd::executeConfCB(ConfdCallback_e cbIndex, struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next)
{
    int ret = CONFD_ERR;
    printf("Confd callback -- %s.\n", getConfdCBName(cbIndex));

    if (cbIndex == CONFD_TRANS_INIT) {
        ret = tInit(tctx);
    } else if (cbIndex == CONFD_TRANS_FINISH) {
        ret = tFinish(tctx);
    } else if (cbIndex == CONFD_DATA_GET_NEXT) {
        ret = getNext(tctx, keypath, next);
    } else if (cbIndex == CONFD_DATA_GET_ELEM) {
        ret = getElem(tctx, keypath);
    } else if (cbIndex == CONFD_DATA_SET_ELEM) {
        printf("get_elem() callback is not implemented yet.\n");
        //ret = setElem(tctx, keypath, newval);
    } else if (cbIndex == CONFD_DATA_REMOVE) {
        ret = doRemove(tctx, keypath);
    } else if (cbIndex == CONFD_DATA_CREATE) {
        ret = Create(tctx, keypath);
    } else {
        printf("Unexpected confd callback index (%d).\n", cbIndex);
    }

    return ret;
}

void fileBrowserConfd::initSockAddr(struct sockaddr_in &addr)
{
    struct in_addr in;
    inet_aton("127.0.0.1", &in);
    addr.sin_addr.s_addr = in.s_addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFD_PORT);
}

void fileBrowserConfd::createConfdSockets()
{
    if ((ctlsock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open ctlsocket\n");

    if ((workersock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open workersocket\n");
}

void fileBrowserConfd::connectConfdSockets(struct sockaddr_in *addr)
{
    if (confd_connect(dctx, ctlsock, CONTROL_SOCKET, (struct sockaddr*)addr, sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");

    if (confd_connect(dctx, workersock, WORKER_SOCKET,(struct sockaddr*)addr, sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");
}

void fileBrowserConfd::registerConfdCBs()
{
    confd_register_trans_cb(dctx, &trans);

    if (confd_register_data_cb(dctx, &data) == CONFD_ERR) {
        close(ctlsock);
        close(workersock);
        logFatalErrorAndExit("Failed to register data cb \n");
    }

    if (confd_register_done(dctx) != CONFD_OK) {
        close(ctlsock);
        close(workersock);
        logFatalErrorAndExit("Failed to complete registration \n");
    }
}

void fileBrowserConfd::execConfdAPIs()
{

    confd_init("fileBrowser", stderr, CONFD_TRACE);

    if ((dctx = confd_init_daemon("fileBrowser")) == NULL)   // confd is looking for this process for call back
        confd_fatal("Failed to initialize confd\n");

    struct sockaddr_in addr;
    initSockAddr(addr);
    confd_load_schemas((struct sockaddr*)&addr,sizeof (struct sockaddr_in));

    createConfdSockets();
    connectConfdSockets(&addr);

    registerConfdCBs();
}

void fileBrowserConfd::initPullFd(struct pollfd *set, int index, int sock_fd)
{
    set[index].fd = sock_fd;
    set[index].events = POLLIN;
    set[index].revents = 0;
}

void fileBrowserConfd::initPullFDs(struct pollfd *set)
{
    initPullFd(set, 0, ctlsock);
    initPullFd(set, 1, workersock);
}

void* fileBrowserConfd::pollConfdSockFds()
{
    while (1) {
        struct pollfd set[2];
        initPullFDs(set);

        if (poll(&set[0], sizeof(set)/sizeof(*set), -1) < 0) {
            perror("Poll failed:");
            continue;
        }

        int ret;
        /* Check for I/O */
        if (set[0].revents & POLLIN) {
            if ((ret = confd_fd_ready(dctx, ctlsock)) == CONFD_EOF) {
                confd_fatal("Control socket closed\n");
            } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                confd_fatal("Error on control socket request: %s (%d): %s\n",
                     confd_strerror(confd_errno), confd_errno, confd_lasterr());
            }
        }
        if (set[1].revents & POLLIN) {
            if ((ret = confd_fd_ready(dctx, workersock)) == CONFD_EOF) {
                confd_fatal("Worker socket closed\n");
            } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                confd_fatal("Error on worker socket request: %s (%d): %s\n",
                     confd_strerror(confd_errno), confd_errno, confd_lasterr());
            }
        }

    }

    return NULL;
}

//////////////////////////////////////////////
///
///
///
//////////////////////////////////////////////

void* fileBrowserConfdThread(void * arg)
{
    fileBrowserConfd *pFileBrowserConfd = fileBrowserConfd::getInstance();
    pFileBrowserConfd->init();

    return pFileBrowserConfd->pollConfdSockFds();
}
