#ifndef __FILEBROWSER_CONFD_CB_H__
#define __FILEBROWSER_CONFD_CB_H__

#include <confd_lib.h>
#include <confd_dp.h>

int t_init(struct confd_trans_ctx *tctx);
int t_finish(struct confd_trans_ctx *tctx);
int get_next(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next);
int get_elem(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath);
int set_elem(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, confd_value_t *newval);
int create(struct confd_trans_ctx *tctx,   confd_hkeypath_t *keypath);
int doremove(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath);

#endif
