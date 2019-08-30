#pragma once

#include "nginx_types.h"

typedef struct {
    ngx_flag_t      enable;
    ngx_str_t       configDir;
    ngx_str_t       lyricsDir;
    ngx_str_t       relatedUploadDir;
    ngx_str_t       lyricsHttpLinkBase;
} MLServerNgxConf;

typedef struct {
    uint8_t         isReqBodyReceived;
} MLServerContext;


#ifdef __cplusplus
extern "C" {
#endif

    int mlserver_reload();
    int mlserver_init(ngx_str_t *installed_path, const MLServerNgxConf *conf);
    void mlserver_quit();

#ifdef __cplusplus
}
#endif
        
