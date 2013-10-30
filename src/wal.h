/*
 * Copyright 2013 Jung-Sang Ahn <jungsang.ahn@gmail.com>.
 * All Rights Reserved.
 */

#ifndef _JSAHN_WAL_H
#define _JSAHN_WAL_H

#include <stdint.h>
#include "forestdb.h"
#include "hash.h"
#include "list.h"
#include "forestdb.h"

typedef enum {
    WAL_RESULT_SUCCESS,
    WAL_RESULT_FAIL
} wal_result;

typedef enum {
    WAL_ACT_INSERT,
    WAL_ACT_REMOVE
} wal_item_action;

struct wal_item{
    void *key;
    uint16_t keylen;
    wal_item_action action;
    uint64_t offset;
    uint32_t doc_size;
    struct hash_elem he_key;
#ifdef __FDB_SEQTREE
    fdb_seqnum_t seqnum;
    struct hash_elem he_seq;
#endif
    struct list_elem list_elem;
};

//typedef void wal_flush_func(void *dbhandle, void *key, int keylen, uint64_t offset, wal_item_action action);
typedef void wal_flush_func(void *dbhandle, struct wal_item *item);

#define WAL_FLAG_INITIALIZED 0x1

struct wal {
    uint8_t flag;
    size_t size;
    struct hash hash_bykey;
#ifdef __FDB_SEQTREE
    struct hash hash_byseq;
#endif
    struct list list;
    struct list_elem *last_commit;
    spin_t lock;
};

//typedef struct fdb_doc_struct fdb_doc;

wal_result wal_init(struct filemgr *file, int nbucket);
int wal_is_initialized(struct filemgr *file);
wal_result wal_insert(struct filemgr *file, fdb_doc *doc, uint64_t offset);
wal_result wal_find(struct filemgr *file, fdb_doc *doc, uint64_t *offset);
wal_result wal_remove(struct filemgr *file, fdb_doc *doc);
wal_result wal_commit(struct filemgr *file);
wal_result wal_flush(struct filemgr *file, void *dbhandle, wal_flush_func *func);
wal_result wal_close(struct filemgr *file);
wal_result wal_shutdown(struct filemgr *file);
size_t wal_get_size(struct filemgr *file);

#endif
