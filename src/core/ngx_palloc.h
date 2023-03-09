
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);  //* 清理函数类型别名 

//* 待清理内存块类型 
typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t; 

//* 自定义清理回调的内存块的类型 
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;  //* 内存块清理函数 
    void                 *data;     //* 内存数据 
    ngx_pool_cleanup_t   *next;     //* 下一个待清理的内存 
};


//* 大内存块类型 
typedef struct ngx_pool_large_s  ngx_pool_large_t;

//* 大内存块类型 
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;     //* 下一个大内存快地址 
    void                 *alloc;    //* 指向大内存块数据 
};

/* *
 * 小块内存池 
 * 虽然 next 的类型是 ngx_pool_t，但是实际使用的时候，只会使用到其中的 pool->d 也就是小内存的区域。
 * 主要是为了保证结构，pool_data_t 没有保存其中的内存块大小等信息
 */
typedef struct {
    u_char               *last;     //* 已使用内存块末位地址 
    u_char               *end;      //* 内存池末位地址 
    ngx_pool_t           *next;     //* 下一个内存池地址
    ngx_uint_t            failed;   //* 失败次数 
} ngx_pool_data_t;

/* *
 * Nginx内存池数据结构
 * 储存内存池所有信息
 */
struct ngx_pool_s {
    ngx_pool_data_t       d;        //* 内存数据，标记内存块开始、结尾、以及下一个内存池地址 
    size_t                max;      //* 小内存池最大大小
    ngx_pool_t           *current;  //* 自引用结构，指向自身地址 
    ngx_chain_t          *chain;    // ? 缓冲区链表，不知道干嘛的
    ngx_pool_large_t     *large;    //* 大内存块链表 
    ngx_pool_cleanup_t   *cleanup;  //* 自定义清理回调的数据类型链表 
    ngx_log_t            *log;      //* 日志文件
};

//* 自定义清理回调文件类型 
typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
