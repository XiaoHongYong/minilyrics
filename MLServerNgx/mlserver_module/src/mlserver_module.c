#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <assert.h>

#include "ngx_utils.h"
#include "../../src/mlserver_4_ngx.h"


static ngx_int_t  mlserver_mod_add_variables(ngx_conf_t *cf);
static void       *mlserver_mod_ngx_create_conf(ngx_conf_t *cf);
static char       *mlserver_mod_ngx_merge_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t  mlserver_mod_ngx_module_init(ngx_cycle_t *cycle);
static void       mlserver_mod_ngx_exit_master(ngx_cycle_t *cycle);
char      *ngx_conf_data_producer_add_tunnel(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char      *ngx_conf_data_producer_remove_tunnel(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
ngx_int_t mlserver_http_module_init(ngx_conf_t *cf);

static ngx_flag_t  run_unittest_on = 0;
static ngx_str_t unittest_options = ngx_null_string;

static char *mlserver_mod_set_unittest_options(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    char *ret = ngx_conf_set_flag_slot(cf, cmd, conf);
    // run_unittest_on = ((MLServerNgxConf *)conf)->runUnittest;

    if (cf->args->nelts == 3) {
        ngx_str_t *value = ((ngx_str_t *)cf->args->elts) + 2;
        unittest_options.data = malloc(value->len);
        unittest_options.len = value->len;
        memcpy(unittest_options.data, value->data, value->len);
    }

    return ret;
}

static ngx_command_t  mlserver_mod_ngx_commands[] = {
    {
        ngx_string("mlserver"),
        NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(MLServerNgxConf, enable),
        NULL
    },

    {
        ngx_string("ml_conf_dir"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(MLServerNgxConf, configDir),
        NULL
    },

    {
        ngx_string("ml_lyrics_dir"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(MLServerNgxConf, lyricsDir),
        NULL
    },

    {
        ngx_string("ml_related_upload_dir"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(MLServerNgxConf, relatedUploadDir),
        NULL
    },

    {
        ngx_string("ml_base_http_link"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(MLServerNgxConf, lyricsHttpLinkBase),
        NULL
    },

    ngx_null_command
};

static ngx_http_module_t  ngx_mlserver_mod_http_module_ctx = {
    mlserver_mod_add_variables,       /* preconfiguration */
    mlserver_http_module_init,    /* postconfiguration */

    NULL,                           /* create main configuration */
    NULL,                           /* init main configuration */

    NULL,                           /* create server configuration */
    NULL,                           /* merge server configuration */

    mlserver_mod_ngx_create_conf,     /* create location configuration */
    mlserver_mod_ngx_merge_conf       /* merge location configuration */
};


ngx_module_t  mlserver_module = {
    NGX_MODULE_V1,
    &ngx_mlserver_mod_http_module_ctx,     /* module context */
    mlserver_mod_ngx_commands,        /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init master */
    mlserver_mod_ngx_module_init,     /* init module */
    NULL,                           /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    NULL,                           /* exit process */
    mlserver_mod_ngx_exit_master,     /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_http_variable_t  mlserver_mod_vars[] = {

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};

static ngx_atomic_t        is_initialized = 0;

ngx_log_t *g_ngx_log = NULL;

#define ngx_http_cycle_get_module_loc_conf(cycle, module)                    \
    (cycle->conf_ctx[ngx_http_module.index] ?                                 \
        ((ngx_http_conf_ctx_t *) cycle->conf_ctx[ngx_http_module.index])      \
            ->loc_conf[module.ctx_index]:                                    \
        NULL)

static ngx_int_t mlserver_mod_ngx_module_init(ngx_cycle_t *cycle) {
    if (ngx_test_config) {
        return NGX_OK;
    }

    g_ngx_log = cycle->log;

    if (is_initialized) {
        int ret = mlserver_reload();
        log_d(cycle->log, "reload configuration %s", ret == 0 ? "OK" : "NG");
        return ret;
    }
    is_initialized = 1;

    // Construct data path to initialize core_frame
    ngx_str_t installed_path, data = ngx_string("../");
    ngx_int_t result = ngx_str_join(cycle->pool, &cycle->conf_prefix, &data, &installed_path);
    if (result != NGX_OK) {
        return result;
    }

    MLServerNgxConf *conf = ngx_http_cycle_get_module_loc_conf(cycle, mlserver_module);
    int ret = mlserver_init(&installed_path, conf);
    if (ret != 0) {
        log_e(cycle->log, "Failed to initialize core_frame, error: %d", ret);
        return NGX_ERROR;
    }

//    if (run_unittest_on) {
//        // Cannot get unittest flag via ngx_http_cycle_get_module_main_conf, use global flag instead.
//        run_mlserver_mod_unittest(unittest_options);
//        free(unittest_options.data);
//        unittest_options.len = 0;
//        return NGX_ERROR;
//    }

    return NGX_OK;
}

static void mlserver_mod_ngx_exit_master(ngx_cycle_t *cycle) {
    mlserver_quit();
}

//
// Macros for conf variable initialation.
//
#define init_as_unset(field, type)      conf->field = (type)-1

static void *mlserver_mod_ngx_create_conf(ngx_conf_t *cf) {
    MLServerNgxConf  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(MLServerNgxConf));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable                    = NGX_CONF_UNSET;

    return conf;
}

#define mlserver_mod_ngx_conf_merge_str_value(field, type)                     \
    if (conf->field.data == NULL) {                                          \
        if (prev->field.data) {                                              \
            conf->field.len = prev->field.len;                               \
            conf->field.data = prev->field.data;                             \
        } else {                                                             \
            conf->field.len = 0;                                             \
            conf->field.data = NULL;                                         \
        }                                                                    \
    }

#define ngx_conf_merge_unset_to_default_0(field, type)                       \
    if ((type)conf->field == (type)-1) {                                     \
        conf->field = ((type)prev->field == (type)-1) ? 0 : prev->field;     \
    }

static char *mlserver_mod_ngx_merge_conf(ngx_conf_t *cf, void *parent, void *child) {
    MLServerNgxConf *prev = parent;
    MLServerNgxConf *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);
    mlserver_mod_ngx_conf_merge_str_value(configDir, MLServerNgxConf);
    mlserver_mod_ngx_conf_merge_str_value(lyricsDir, MLServerNgxConf);
    mlserver_mod_ngx_conf_merge_str_value(relatedUploadDir, MLServerNgxConf);
    mlserver_mod_ngx_conf_merge_str_value(lyricsHttpLinkBase, MLServerNgxConf);

//    ngx_conf_merge_value(conf->mls, prev->enable, 0);
    

    return NGX_CONF_OK;
}

/**
 * This method is used for add core_frame variables to http variable list.
 */
static ngx_int_t  mlserver_mod_add_variables(ngx_conf_t *cf) {
    ngx_http_variable_t  *var, *v;

    for (v = mlserver_mod_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}
