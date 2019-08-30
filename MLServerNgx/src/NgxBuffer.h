#ifndef __NgxBuffer__
#define __NgxBuffer__


#include "nginx_types.h"

int ngxChainGetBufCount(ngx_chain_t *chain);
size_t ngxChainGetBufLen(ngx_chain_t *chain);
bool ngxChainIsLastBuf(ngx_chain_t *chain);

void ngxChainSetLastBuf(ngx_chain_t *chain);

int ngxMergeChainBufs(ngx_pool_t *pool, ngx_chain_t *src, ngx_str_t &out);

#endif /* defined(__NgxBuffer__) */
