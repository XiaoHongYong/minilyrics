#ifndef __NgxOutputStream__
#define __NgxOutputStream__

#include "nginx_types.h"

#include <string.h>
#include <string>

/**
 * NgxOutputStream is used to write secured/encrypted HTML/JS data back to Nginx.
 */
class NgxOutputStream {
public:
    NgxOutputStream(ngx_pool_t *pool, ngx_chain_t **chain);

    void write(const char *str, size_t len) { write((const uint8_t *)str, len); }
    void write(std::string &str) { write(str.c_str(), str.size()); }
    void write(const uint8_t *str, size_t len);
    void write(const ngx_str_t &str) { write((const char *)str.data, str.len); }

    /**
     * Write buffer direct to ngx_buf_t, whithout memory allocation and copy.
     *
     * NOTE: input @str must NOT been changed, and should always be valid.
     */
    void writeWithoutCopy(const ngx_str_t &str);

protected:
    ngx_chain_t         **_chain;
    ngx_pool_t          *_pool;
    ngx_buf_t           *_buf;

};

#endif /* defined(__NgxOutputStream__) */
