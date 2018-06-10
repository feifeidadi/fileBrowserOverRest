#ifndef __FILEBROWSER_ACTION_POINT_H__
#define __FILEBROWSER_ACTION_POINT_H__

#include "fileBrowserDiskOper.h"

typedef struct  actionParams {
    u32_t sessionId;
    u32_t vnfId;
    u32_t timePeriod;
    char filename[NAME_MAX];
} actionParams_t;

class fileBrowserActionPoint.cpp : private fileBrowserDiskOper
{
public:
    fileBrowserActionPoint.cpp();
    virtual ~fileBrowserActionPoint.cpp();
    static fileBrowserActionPoint.cpp* getInstance();
    int initActionPoint();

    // Confd action callback
    int handleConfdAction(struct confd_user_info *aUserInfo,struct xml_tag *aActionName, confd_hkeypath_t *aKeyPath, confd_tag_value_t *aParams,int aNumOfParams);

private:
    int _initActionPoint();
    void initActionPointConfig(ActionPointInitInfo &config);

    unsigned char* loadFile(u32_t sessionId, u32_t vnfId, const char *filename, u32_t &length);
    void setPath(char *path, u32_t len, u32_t sessionId, u32_t vnfId);
    void getParamVales(confd_tag_value_t *aParams, int aNumOfParams, actionParams_t &params);
    void showParamInfo(struct xml_tag *aActionName, confd_tag_value_t *aParams,int aNumOfParams);
    void setConfdReplyValues(confd_tag_value_t *reply, unsigned char *buf, u32_t length);

    s32_t handleGetFileContentAction(struct confd_user_info *aUserInfo,confd_tag_value_t *aParams,int aNumOfParams);
    s32_t handleDeleteFilesAction(struct confd_user_info *aUserInfo,confd_tag_value_t *aParams,int aNumOfParams);
    int doAction(struct confd_user_info *aUserInfo, struct xml_tag *aActionName, confd_tag_value_t *aParams, int aNumOfParams);

public:
    static fileBrowserActionPoint.cpp* pInstance;

private:
    ActionPointInitInfo config;
};

#endif
