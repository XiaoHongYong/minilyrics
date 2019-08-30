extern "C" {

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <assert.h>

}

#include "NgxBuffer.h"
#include "Debug.h"
#include "StdAfx.h"

int ngxChainGetBufCount(ngx_chain_t *chain) {
    int count = 0;
    while (chain) {
        chain = chain->next;
        count++;
    }

    return count;
}

size_t ngxChainGetBufLen(ngx_chain_t *chain) {
    size_t len = 0;
    while (chain != NULL) {
        len += ngx_buf_size(chain->buf);
        chain = chain->next;
    }

    return len;
}

bool ngxChainIsLastBuf(ngx_chain_t *chain) {
    while (chain) {
        if (chain->buf->last_buf) {
            return true;
        }
        chain = chain->next;
    }

    return false;
}

void ngxChainSetLastBuf(ngx_chain_t *chain) {
    if (chain == NULL) {
        return;
    }

    while (chain->next != NULL) {
        chain = chain->next;
    }

    chain->buf->last_buf = 1;
}

int ngxMergeChainBufs(ngx_pool_t *pool, ngx_chain_t *src, ngx_str_t &out) {
    out = {0, NULL};
    
    size_t count = 0, len = 0;
    
    for (ngx_chain_t *c = src; c; c = c->next) {
        size_t size = ngx_buf_size(c->buf);
        if (size) {
            len += size;
            count++;
        }
    }
    
    if (count == 0) {
        return ERR_OK;
    } else if (count <= 1 && ngx_buf_in_memory(src->buf)) {
        // No need to allocate memory.
        out.data = src->buf->pos;
        out.len = len;
        return ERR_OK;
    }
    
    out.data = (uint8_t *)ngx_palloc(pool, len + 1);
    if (out.data == NULL) {
        return ERR_NO_MEM;
    }
    out.len = len;
    
    uint8_t *p = out.data;
    for (ngx_chain_t *c = src; c; c = c->next) {
        ngx_buf_t *buf = c->buf;
        size_t size = ngx_buf_size(buf);
        
        if (ngx_buf_in_memory(buf)) {
            memcpy(p, buf->pos, size);
        } else if (buf->in_file) {
            if (ngx_read_file(buf->file, p, size, buf->file_pos) != (ssize_t)size) {
                printf("Failed to read file.");
                return ERR_READ_FILE;
            }
        }
        p += size;
        buf->pos += size;
    }
    
    return ERR_OK;
}
