extern "C" {

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <assert.h>

}

#include "NgxOutputStream.h"
#include "Debug.h"


#define BLOCK_SIZE (1024 * 16)

NgxOutputStream::NgxOutputStream(ngx_pool_t *pool, ngx_chain_t **chain) {
    assert(*chain == NULL);
    _pool = pool;
    _chain = chain;
    _buf = NULL;
}

void NgxOutputStream::write(const uint8_t *str, size_t length) {
    while (length > 0) {
        size_t copy_size = _buf ? (size_t)(_buf->end - _buf->last) : 0;
        if (copy_size == 0) {
            copy_size = std::max(length, (size_t)BLOCK_SIZE);

            *_chain = ngx_alloc_chain_link(_pool);
            _buf = ngx_create_temp_buf(_pool, copy_size);
            if (*_chain == NULL || _buf == NULL) {
                *_chain = NULL;
                LOG("Failed to create buffer for NgxOutputStream");
                return;
            }

            (*_chain)->buf = _buf;
            (*_chain)->next = NULL;

            _chain = &((*_chain)->next);
        }

        if (copy_size > length) {
            copy_size = length;
        }

        memcpy(_buf->last, str, copy_size);
        _buf->last += copy_size;
        length -= copy_size;
        str += copy_size;
    }
}

void NgxOutputStream::writeWithoutCopy(const ngx_str_t &str) {
    assert(_buf == NULL && *_chain == NULL);

    *_chain = ngx_alloc_chain_link(_pool);
    _buf = (ngx_buf_t *)ngx_calloc_buf(_pool);
    if (*_chain == NULL || _buf == NULL) {
        *_chain = NULL;
        LOG("Failed to create buffer for NgxOutputStream");
        return;
    }

    _buf->start = _buf->pos = str.data;
    _buf->end = _buf->last = str.data + str.len;
    _buf->memory = 1;

    (*_chain)->buf = _buf;
    (*_chain)->next = NULL;

    _chain = &((*_chain)->next);
    _buf = NULL;
}
