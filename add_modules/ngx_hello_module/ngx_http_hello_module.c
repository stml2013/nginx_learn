#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_core.h>
#include <ngx_log.h>

typedef struct {
    ngx_str_t hello_string;
    ngx_int_t hello_counter;
} ngx_http_hello_loc_conf_t;

static char *ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_hello_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf);
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_hello_commands[] = {
    {
        ngx_string("hello_string"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS | NGX_CONF_TAKE1,
        ngx_http_hello_string,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hello_loc_conf_t, hello_string),
        NULL },
    {
        ngx_string("hello_counter"),
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
        ngx_http_hello_counter,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hello_loc_conf_t, hello_counter),
        NULL },
    ngx_null_command
};

static ngx_http_module_t ngx_http_hello_module_ctx = {
    NULL,
    ngx_http_hello_init,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_hello_create_loc_conf,
    NULL
};

ngx_module_t ngx_http_hello_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_module_ctx,
    ngx_http_hello_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_hello_visited_times = 0;

ngx_int_t ngx_http_hello_init(ngx_conf_t *cf) {
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_hello_handler;
    ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "handler is ngx_http_hello_handler...");

    return NGX_OK;
}

void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_hello_loc_conf_t *locf = NULL; 

    locf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    if (locf == NULL) {
        return NULL;
    }

//    ngx_str_null(&locf->hello_string);
    locf->hello_string.data = NULL;
    locf->hello_counter = NGX_CONF_UNSET;

    return locf;
}

char *ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_hello_loc_conf_t *locf;
    ngx_str_t *field, *value;

    //下面来按需挂在handler
//    ngx_http_core_loc_conf_t *clcf;  
//    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);  
//    clcf->handler = ngx_http_hello_handler;

    locf = conf;
    field = (ngx_str_t*)(locf + cmd->offset);
    ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "field is: %d offset: %d ", field, cmd->offset);
    if (field->data) {
        return "is duplicate";
    }

    value = cf->args->elts;
    *field = value[1];

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_string:%s", locf->hello_string.data);

    return NGX_CONF_OK;
}

char *ngx_http_hello_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_hello_loc_conf_t *locf;
    ngx_str_t *value;
    ngx_flag_t *field;
    locf = conf;

    //char* rv = NULL;
    //rv = ngx_conf_set_flag_slot(cf, cmd, locf);
    //ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_counter:%d", locf->hello_counter);

    field = (ngx_flag_t*)((char*)locf + cmd->offset); // 这个地方不加(char*)就会出错
   // ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "field is: %d offset: %d ", field, cmd->offset);
    if (*field != NGX_CONF_UNSET) {
        ngx_log_error(NGX_LOG_EMERG, cf->log, 0, "hello_counter is: %d %d", *field, NGX_CONF_UNSET);
        return "is duplicate";
    }

    value = cf->args->elts;
    if (ngx_strcasecmp(value[1].data, (u_char*)"on") == 0) {
        *field = 1;
    } else if (ngx_strcasecmp(value[1].data, (u_char*)"off") == 0) {
        *field = 0;
    } else {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                     "invalid value \"%s\" in \"%s\" directive, "
                     "it must be \"on\" or \"off\"",
                     value[1].data, cmd->name.data);
        return NGX_CONF_ERROR;
    }

    //ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_counter:%d", locf->hello_counter);
    return NGX_CONF_OK;
}

ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r) {
    ngx_int_t rc;
    ngx_buf_t *b;
    ngx_chain_t out;
    ngx_http_hello_loc_conf_t *locf;
    u_char ngx_hello_string[1024] = {0};
    ngx_int_t content_length = 0;

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_hello_handler is called!");

    locf = ngx_http_get_module_loc_conf(r, ngx_http_hello_module);
    if (locf->hello_string.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello string is empty!");
        return NGX_DECLINED;
    }

    if (locf->hello_counter == NGX_CONF_UNSET 
            || locf->hello_counter == 0) {
        ngx_snprintf(ngx_hello_string, 1024, "%s", locf->hello_string.data);
    } else {
        ngx_snprintf(ngx_hello_string, 1024, "%s visited times:%d", locf->hello_string.data, ++ngx_hello_visited_times);
    }
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello_string:%s", ngx_hello_string);
    content_length = ngx_strlen(ngx_hello_string);

    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    r->headers_out.content_type.len = sizeof("text/html") -1;
    r->headers_out.content_type.data = (u_char*)"text/html";

    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;
        return ngx_http_send_header(r);
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    b->pos = ngx_hello_string;
    b->last = ngx_hello_string + content_length;
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}
