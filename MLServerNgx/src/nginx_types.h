
#ifndef __NGINX_TYPES_H__
#define __NGINX_TYPES_H__

#include <stddef.h>
#include <stdint.h>


// structs and functions predefined in nginx
#ifdef __cplusplus
extern "C" {

typedef uintptr_t ngx_uint_t;
typedef intptr_t ngx_flag_t;
typedef intptr_t ngx_int_t;

typedef struct ngx_pool_s ngx_pool_t;
typedef struct ngx_buf_s ngx_buf_t;
typedef struct ngx_chain_s ngx_chain_t;
typedef struct ngx_http_request_s ngx_http_request_t;
typedef struct ngx_file_s ngx_file_t;
typedef struct ngx_log_s ngx_log_t;

#ifndef _NGX_STRING_H_INCLUDED_
typedef struct {
    size_t len;
    unsigned char *data;
} ngx_str_t;

typedef struct ngx_table_elt_s ngx_table_elt_t;
#endif

}  // extern "C"
#endif  // __cplusplus

#endif /* defined(__NGINX_TYPES_H__) */
