extern "C" {
    
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <assert.h>
    
}

#include "mlserver_4_ngx.h"
#include "StdAfx.h"
#include "MLServer.h"
#include "NgxBuffer.h"


MLServer g_mlserver;

extern "C" ngx_module_t  mlserver_module;


extern "C" int mlserver_reload() {
    return ERR_OK;
}

extern "C" int mlserver_init(ngx_str_t *installed_path, const MLServerNgxConf *conf) {
    InitBaseFrameWork(1, (char **)&(installed_path->data), "Log.txt", "MLServer.ini");
    g_mlserver.Init(conf);

    return ERR_OK;
}

extern "C" void mlserver_quit() {
}

static void finishWithResponse(ngx_http_request_t *r, ngx_chain_t *responseBodyChain) {
    r->headers_out.content_length_n = ngxChainGetBufLen(responseBodyChain);
    r->headers_out.status = NGX_HTTP_OK;

    ngx_int_t rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        // Error occured.
    } else {
        ngxChainSetLastBuf(responseBodyChain);
        ngx_http_output_filter(r, responseBodyChain);
    }
    
    ngx_http_finalize_request(r, NGX_HTTP_OK);
}

#define INVALID_REQUEST "Invalid Request."

static void ngx_http_read_request_body_handler(ngx_http_request_t *r)
{
    MLServerContext *ctx = (MLServerContext *)ngx_pcalloc(r->pool, sizeof(MLServerContext));
    assert(ctx);
    
    ctx->isReqBodyReceived = 1;

    ngx_str_t body;
    if (ngxMergeChainBufs(r->pool, r->request_body->bufs, body) == ERR_OK) {
        ngx_chain_t *outChain = NULL;
        NgxOutputStream stream(r->pool, &outChain);

        g_mlserver.Process(body.data, body.len, stream);
        if (outChain == NULL) {
            stream.write(INVALID_REQUEST, CountOf(INVALID_REQUEST) - 1);
        }

        ngx_http_finalize_request(r, NGX_DONE);
        finishWithResponse(r, outChain);
        return;
    }

    ngx_http_finalize_request(r, NGX_DONE);
}

static void writeStubStatus(ngx_http_request_t *r) {
    ngx_chain_t *outChain = NULL;
    NgxOutputStream stream(r->pool, &outChain);

    char buf[256];

    sprintf(buf, "Active connections: %llu.\n", *ngx_stat_active);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Server states accepts: %llu.\n", *ngx_stat_accepted);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Server states handled: %llu.\n", *ngx_stat_handled);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Server states requests: %llu.\n", *ngx_stat_requests);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Reading: %llu.\n", *ngx_stat_reading);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Writing: %llu.\n", *ngx_stat_writing);
    stream.write(buf, strlen(buf));

    sprintf(buf, "Waiting: %llu.\n", *ngx_stat_waiting);
    stream.write(buf, strlen(buf));

    finishWithResponse(r, outChain);
}

extern "C" ngx_int_t mlserver_http_rewrite_handler(ngx_http_request_t *r) {
    MLServerNgxConf *conf = (MLServerNgxConf *)ngx_http_get_module_loc_conf(r, mlserver_module);
    if (conf->enable == 1 && r->method == NGX_HTTP_POST) {
        MLServerContext *ctx = (MLServerContext *)ngx_http_get_module_ctx(r, mlserver_module);
        if (ctx == NULL) {
            ctx = (MLServerContext *)ngx_pcalloc(r->pool, sizeof(MLServerContext));
            if (ctx == NULL) {
                return NGX_DECLINED;
            }
            ngx_http_set_ctx(r, ctx, mlserver_module);
        }

        ngx_int_t rc = ngx_http_read_client_request_body(r,
            ngx_http_read_request_body_handler);
//        if (rc == NGX_OK || rc == NGX_DECLINED) {
//            return NGX_DECLINED;
//        }
        
        return NGX_DONE;
    }
    
    if (conf->enableStubStatus == 1) {
        writeStubStatus(r);
        return NGX_DONE;
    }
    
//    NgxHttpRequest req(r, ctx);
//
//    coreFrameApp.onRequestHeader(req);
//
//    if (ctx->isContentProvider || req.responseBodyChain) {
//        req.finishWithResponse();
//        return NGX_DONE;
//    }
    
    return NGX_DECLINED;
}

extern "C" ngx_int_t mlserver_http_module_init(ngx_conf_t *cf) {
    ngx_http_core_main_conf_t *cmcf = (ngx_http_core_main_conf_t *)ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    ngx_http_handler_pt *h = (ngx_http_handler_pt *)ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
    *h = mlserver_http_rewrite_handler;

    return NGX_OK;
}
