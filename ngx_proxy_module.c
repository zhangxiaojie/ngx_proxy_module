#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct{
	ngx_str_t port;	
} ngx_proxy_loc_conf_t;

static char *ngx_proxy(ngx_conf_t *cf,ngx_command_t *cmd,void *conf);

static void *ngx_proxy_create_loc_conf(ngx_conf_t *cf);

static char *ngx_proxy_merge_loc_conf(ngx_conf_t *cf,void *parent,void *child);

static ngx_command_t ngx_proxy_commands[] = {
	{
	ngx_string("proxy"),
	NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
	ngx_proxy,
	NGX_HTTP_LOC_CONF_OFFSET,
	offsetof(ngx_proxy_loc_conf_t,port),
	NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_proxy_ctx = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ngx_proxy_create_loc_conf,
	ngx_proxy_merge_loc_conf
};

ngx_module_t ngx_proxy_module={
	NGX_MODULE_V1,
	&ngx_proxy_ctx,
	ngx_proxy_commands,
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

static ngx_int_t ngx_proxy_handler(ngx_http_request_t *r){
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_proxy_loc_conf_t *hlcf;
	hlcf=ngx_http_get_module_loc_conf(r,ngx_proxy_module);
	r->headers_out.content_type.len=sizeof("text/plain")-1;
	r->headers_out.content_type.data=(u_char*)"text/plain";
	b=ngx_pcalloc(r->pool,sizeof(ngx_buf_t));
    ngx_str_t out_str;
    out_str.data=(u_char*)"proxy on port:";
    out_str.len=sizeof("proxy on port:")-1;

    u_char *content=(u_char*)ngx_pcalloc(r->pool,out_str.len+hlcf->port.len);
    ngx_memcpy(content,out_str.data,out_str.len);
    ngx_memcpy(content+out_str.len,hlcf->port.data,hlcf->port.len);
    out_str.data=content;
    out_str.len=out_str.len+hlcf->port.len;
    b->pos=out_str.data;
	b->last=b->pos+out_str.len;
	b->memory = 1;
	b->last_buf = 1;
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n= out_str.len;
	rc = ngx_http_send_header(r);
	if(rc==NGX_ERROR||rc>NGX_OK||r->header_only) return rc;
	ngx_chain_t out;
    out.buf=b;
    out.next=NULL;
	return ngx_http_output_filter(r,&out);
	
}

static void *ngx_proxy_create_loc_conf(ngx_conf_t *cf){
	ngx_proxy_loc_conf_t *conf;
	conf=ngx_palloc(cf->pool,sizeof(ngx_proxy_loc_conf_t));
	conf->port.data = NULL;
    conf->port.len =  0;
	return conf;
}

static char *ngx_proxy_merge_loc_conf(ngx_conf_t *cf,void *parent,void *child){
	ngx_proxy_loc_conf_t *prev = parent;
	ngx_proxy_loc_conf_t *conf = child;
	ngx_conf_merge_str_value(conf->port,prev->port,"8001");
	return NGX_CONF_OK;
}

static char *ngx_proxy(ngx_conf_t *cf,ngx_command_t *cmd,void *conf){
	ngx_http_core_loc_conf_t *clcf;
	clcf=ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler=ngx_proxy_handler;
	ngx_conf_set_str_slot(cf,cmd,conf);
	return NGX_CONF_OK;
}


