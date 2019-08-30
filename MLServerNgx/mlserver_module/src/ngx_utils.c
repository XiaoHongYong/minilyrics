#include <ngx_config.h>
#include <ngx_core.h>
#include <assert.h>
#include <ngx_times.h>

#include "ngx_utils.h"


ngx_int_t ngx_str_join(ngx_pool_t *pool, ngx_str_t *s1, ngx_str_t *s2, ngx_str_t *out) {
    out->len = s1->len + s2->len;
    out->data = ngx_pcalloc(pool, out->len + 1);
    if (out->data == NULL) {
        return NGX_ERROR;
    }

    memcpy(out->data, s1->data, s1->len);
    memcpy(out->data + s1->len, s2->data, s2->len);
    out->data[out->len] = '\0';

    return NGX_OK;
}

ngx_int_t ngx_str_join3(ngx_pool_t *pool, ngx_str_t *s1, ngx_str_t *s2, ngx_str_t *s3, ngx_str_t *out) {
    out->len = s1->len + s2->len + s3->len;
    out->data = ngx_pcalloc(pool, out->len + 1);
    if (out->data == NULL) {
        return NGX_ERROR;
    }

    memcpy(out->data, s1->data, s1->len);
    memcpy(out->data + s1->len, s2->data, s2->len);
    memcpy(out->data + s1->len + s2->len, s3->data, s3->len);
    out->data[out->len] = '\0';

    return NGX_OK;
}
