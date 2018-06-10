#include "fileBrowserConfdCB.h"
#include "fileBrowserConfd.h"

static
int execute_conf_CB(ConfdCallback_e cb_index, struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath = NULL, long next = -1)
{
    fileBrowserConfd *pFileBrowserConfd = fileBrowserConfd::getInstance();
    return pFileBrowserConfd->executeConfCB(cb_index, tctx, keypath, next);
}

int t_init(struct confd_trans_ctx *tctx)
{
    return execute_conf_CB(CONFD_TRANS_INIT, tctx);
}

int t_finish(struct confd_trans_ctx *tctx)
{
    return execute_conf_CB(CONFD_TRANS_FINISH, tctx);
}

int get_next(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next)
{
    return execute_conf_CB(CONFD_DATA_GET_NEXT, tctx, keypath, next);
}

int get_elem(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath)
{
    return execute_conf_CB(CONFD_DATA_GET_ELEM, tctx, keypath);
}

int set_elem(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, confd_value_t *newval)
{
    // TODO
    return CONFD_OK;
}

int create(struct confd_trans_ctx *tctx,   confd_hkeypath_t *keypath)
{
    return execute_conf_CB(CONFD_DATA_CREATE, tctx, keypath);
}    

int doremove(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath)
{
    return execute_conf_CB(CONFD_DATA_REMOVE, tctx, keypath);
}    

