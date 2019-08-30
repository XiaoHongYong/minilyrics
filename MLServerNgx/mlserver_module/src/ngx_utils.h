#ifndef __Nginx__ngx_utils__
#define __Nginx__ngx_utils__

// Note: This macro is disabled about the entered string have escape characte.
#define CountOf(v)  (sizeof(v) / sizeof((v)[0]))
#define UNUSED_PARAMETER(v) ((void)(v))

#define HEADER_REFERER          "referer"
#define HEADER_COOKIE           "cookie"
#define HEADER_SET_COOKIE       "set-cookie"
#define HEADER_CONTENT_TYPE     "content-type:"
#define HEADER_VARY             "Vary"
#define HEADER_PRAGMA           "Pragma"
#define HEADER_CACHE_CONTROL    "Cache-Control"
#define HEADER_DATE             "Date"
#define HEADER_P3P              "P3P"
#define HEADER_X_REQUESTED_WITH "X-Requested-With"

#define log_e(log, fmt, ...) \
    ngx_log_error(NGX_LOG_ERR, log, 0, "[TxSafe] "fmt, ##__VA_ARGS__)
#define log_i(log, fmt, ...) \
    ngx_log_error(NGX_LOG_INFO, log, 0, "[TxSafe] "fmt, ##__VA_ARGS__)
#define log_d(log, fmt, ...) \
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, log, 0, "[TxSafe] "fmt, ##__VA_ARGS__)

#define SET_OUT_HEADER_CONTENT_TYPE(r, CONTENT_TYPE)        \
    {   r->headers_out.content_type_len = sizeof(CONTENT_TYPE) - 1;\
        r->headers_out.content_type.len = sizeof(CONTENT_TYPE) - 1;\
        r->headers_out.content_type.data = (u_char *)CONTENT_TYPE; }

#define SET_HEADER_OUT_CHARSET(r, CHARSET)        \
    {   r->headers_out.charset.len = sizeof(CHARSET) - 1;\
        r->headers_out.charset.data = (u_char *)CHARSET; }

#define SET_HEADER_OUT_CONTENT_TYPE_AND_CHARSET(r, CONTENT_TYPE, CHARSET)        \
    {   SET_OUT_HEADER_CONTENT_TYPE(r, CONTENT_TYPE); \
        SET_HEADER_OUT_CHARSET(r, CHARSET); }

// The CONTENT_TYPE here is a c style string.
#define SET_IN_HEADER_CONTENT_TYPE(r, CONTENT_TYPE)    \
    r->headers_in.content_type->value.len = strlen((const char*)(CONTENT_TYPE));    \
    r->headers_in.content_type->value.data = (u_char*)CONTENT_TYPE;

#define REQ_HAS_BODY(r) \
    (r->headers_in.content_length_n > 0 || r->headers_in.chunked)

#define IS_REQUEST_HAS_BODY(r)               \
    (NGX_HTTP_POST == r->method || NGX_HTTP_DELETE == r->method || NGX_HTTP_PUT == r->method || NGX_HTTP_PATCH == r->method)

ngx_int_t ngx_str_join(ngx_pool_t *pool, ngx_str_t *s1, ngx_str_t *s2, ngx_str_t *out);
ngx_int_t ngx_str_join3(ngx_pool_t *pool, ngx_str_t *s1, ngx_str_t *s2, ngx_str_t *s3, ngx_str_t *out);

#endif /* defined(__Nginx__ngx_utils__) */
