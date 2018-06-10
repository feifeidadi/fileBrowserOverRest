#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <list>
#include <fstream>

#include "fileBrowser.h"
#include "my-fileBrowser.h"
#include "fileBrowserDiskOper.h"
#include "fileBrowserActionPoint.h"

ActionPoint* fileBrowserActionPoint::ActionPoint = NULL;
fileBrowserActionPoint* fileBrowserActionPoint::pInstance= NULL;

static
int confdActionCB(struct confd_user_info *aUserInfo,struct xml_tag *aActionName, confd_hkeypath_t *aKeyPath, confd_tag_value_t *aParams,int aNumOfParams)
{
    fileBrowserActionPoint* pActionPointInst = fileBrowserActionPoint::getInstance();
    return pActionPointInst->handleConfdAction(aUserInfo, aActionName, aKeyPath, aParams, aNumOfParams);
}

fileBrowserActionPoint::fileBrowserActionPoint()
{
    initActionPointConfig(config);

    if(ActionPoint == NULL) {
        ActionPoint = new ActionPoint();
    }
}

fileBrowserActionPoint::~fileBrowserActionPoint()
{
    if(ActionPoint) {
        delete ActionPoint;
        ActionPoint = NULL;
    }
}

unsigned char* fileBrowserActionPoint::loadFile(u32_t sessionId, u32_t vnfId, const char *filename, u32_t &length)
{
    char fileName[NAME_MAX];

    snprintf(fileName, NAME_MAX, "%s/%d/%d/%s", _ROOT_DIR, sessionId, vnfId, filename);
    FILE *f = openFile(fileName, length);
    if(f == NULL || length == 0)
        return NULL;

    return getFileContent(f, length);
}

void fileBrowserActionPoint::getParamVales(confd_tag_value_t *aParams, int aNumOfParams, actionParams_t &params)
{
    memset(&params, 0, sizeof(actionParams_t));
    params.sessionId = INVALID_ID;
    params.vnfId = INVALID_ID;

    for(int i=0; i<aNumOfParams; i++) {
        if(CONFD_GET_TAG_TAG(&aParams[i]) == my_fileBrowser_session_id) {
            params.sessionId = aParams[i].v.val.u32;
        }
        else if(CONFD_GET_TAG_TAG(&aParams[i]) == my_fileBrowser_vnf_id) {
            params.vnfId = aParams[i].v.val.u32;
        } else if(CONFD_GET_TAG_TAG(&aParams[i]) == my_fileBrowser_time_period) {
            params.timePeriod = aParams[i].v.val.u32;
        } else if(CONFD_GET_TAG_TAG(&aParams[i]) == my_fileBrowser_filename) {
            memcpy(params.filename, aParams[i].v.val.buf.ptr, NAME_MAX);
        }
    }

    printf("Session-id(%d), vnf-id(%d), time-period(%d), filename(%s).\n",
              params.sessionId, params.vnfId, params.timePeriod, params.filename);
}

void fileBrowserActionPoint::setConfdReplyValues(confd_tag_value_t *reply, unsigned char *buf, u32_t length)
{
    if(buf != NULL && length > 0) {
        CONFD_SET_TAG_BINARY(&reply[0], my_fileBrowser_file_content, buf, length);
        CONFD_SET_TAG_ENUM_VALUE(&reply[1], my_fileBrowser_result, my_fileBrowser_success);
    } else {
        CONFD_SET_TAG_BINARY(&reply[0], my_fileBrowser_file_content, NULL, 0);
        CONFD_SET_TAG_ENUM_VALUE(&reply[1], my_fileBrowser_result, my_fileBrowser_failure);
    }
}

s32_t fileBrowserActionPoint::handleGetFileContentAction(struct confd_user_info *aUserInfo,confd_tag_value_t *aParams,int aNumOfParams)
{
    confd_tag_value_t reply[2];
    u32_t length = 0;

    actionParams_t params;
    getParamVales(aParams, aNumOfParams, params);

    unsigned char *buf = loadFile(params.sessionId, params.vnfId, params.filename, length);
    setConfdReplyValues(reply, buf, length);

    confd_action_reply_values(aUserInfo, reply, 2);
    if (buf != NULL) {
        free(buf);
    }

    return 0;
}

void fileBrowserActionPoint::setPath(char *path, u32_t len, u32_t sessionId, u32_t vnfId)
{
    if(sessionId != INVALID_ID) {
        if(vnfId != INVALID_ID) {
            snprintf(path, len, "%s/%d/%d", _ROOT_DIR, sessionId, vnfId);
        } else {
            snprintf(path, len, "%s/%d", _ROOT_DIR, sessionId);
        }
    } else {
        snprintf(path, len, "%s", _ROOT_DIR);
    }
}

s32_t fileBrowserActionPoint::handleDeleteFilesAction(struct confd_user_info *aUserInfo,confd_tag_value_t *aParams, int aNumOfParams)
{
    actionParams_t params;
    getParamVales(aParams, aNumOfParams, params);

    confd_tag_value_t reply;
    char path[MAX_PATH_LEN] = {};
    setPath(path, MAX_PATH_LEN, params.sessionId, params.vnfId);

    if(!removeExpiredFiles(path, params.timePeriod)) {
        CONFD_SET_TAG_ENUM_VALUE(&reply, my_fileBrowser_result, my_fileBrowser_success);
    } else {
        CONFD_SET_TAG_ENUM_VALUE(&reply, my_fileBrowser_result, my_fileBrowser_failure);
    }

    confd_action_reply_values(aUserInfo, &reply, 1);

    return 0;
}

int fileBrowserActionPoint::doAction(struct confd_user_info *aUserInfo, struct xml_tag *aActionName, confd_tag_value_t *aParams, int aNumOfParams)
{
    switch (aActionName->tag) {
        case my_fileBrowser_download_file:
        {
            printf("received event: downloadFileAP action\n");
            handleGetFileContentAction(aUserInfo, aParams, aNumOfParams);
        }
        break;
        case my_fileBrowser_delete_files:
        {
            printf("received event: deleteFilesAP action\n");
            handleDeleteFilesAction(aUserInfo, aParams, aNumOfParams);
        }
        break;
        default:
        {
            printf("got bad operation tag=%d\n", aActionName->tag);
        }
        break;
    }

    return CONFD_OK;
}

void fileBrowserActionPoint::showParamInfo(struct xml_tag *aActionName, confd_tag_value_t *aParams,int aNumOfParams)
{
    printf("Received action with %d parameters, tag=%d \n", aNumOfParams, aActionName->tag);

    char pBuf[BUFSIZ];
    for (int i = 0; i < aNumOfParams; i++) {
        confd_pp_value(pBuf, sizeof(pBuf), CONFD_GET_TAG_VALUE(&aParams[i]));
        printf("parameter(%2d): Tag(%9u:%-9u), content=%s\n", i, CONFD_GET_TAG_NS(&aParams[i]), CONFD_GET_TAG_TAG(&aParams[i]), pBuf);
    }
}

int fileBrowserActionPoint::handleConfdAction(struct confd_user_info *aUserInfo,struct xml_tag *aActionName, confd_hkeypath_t *aKeyPath, confd_tag_value_t *aParams,int aNumOfParams)
{
    showParamInfo(aActionName, aParams, aNumOfParams);

    return doAction(aUserInfo, aActionName, aParams, aNumOfParams);
}

void fileBrowserActionPoint::initActionPointConfig(ActionPointInitInfo &config)
{
    //TODO:
}

int fileBrowserActionPoint::_initActionPoint()
{
    // TODO:
    // init action point, register action callbacks, and etc.
    return 0;
}

fileBrowserActionPoint* fileBrowserActionPoint::getInstance()
{
    return (pInstance == NULL ? new fileBrowserActionPoint() : pInstance);
}

int fileBrowserActionPoint::initActionPoint()
{
    return _initActionPoint();
}
