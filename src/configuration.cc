/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010 Couchbase, Inc
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "configuration.h"


fdb_config get_default_config(void) {
    fdb_config fconfig;

    fconfig.chunksize = sizeof(uint64_t);
    // 4KB by default.
    fconfig.blocksize = FDB_BLOCKSIZE;
    // 128MB by default.
    fconfig.buffercache_size = 134217728;
    // 4096 WAL entries by default.
    fconfig.wal_threshold = 4096;
    fconfig.wal_flush_before_commit = false;
    // 0 second by default.
    fconfig.purging_interval = 0;
    // Use a seq btree by default.
    fconfig.seqtree_opt = FDB_SEQTREE_USE;
    // Use a synchronous commit by default.
    fconfig.durability_opt = FDB_DRB_NONE;
    fconfig.flags = FDB_OPEN_FLAG_CREATE;
    // 4MB by default.
    fconfig.compaction_buf_maxsize = FDB_COMP_BUF_MAXSIZE;
    // Clean up cache entries when a file is closed.
    fconfig.cleanup_cache_onclose = true;
    // Compress the body of documents using snappy.
    fconfig.compress_document_body = false;
    fconfig.cmp_fixed = NULL;
    fconfig.cmp_variable = NULL;
    // Auto compaction is disabled by default
    fconfig.compaction_mode = FDB_COMPACTION_MANUAL;
    // Compaction threshold, 30% by default
    fconfig.compaction_threshold = FDB_DEFAULT_COMPACTION_THRESHOLD;
    fconfig.compaction_minimum_filesize = 1048576; // 1MB by default
    // 15 seconds by default
    fconfig.compactor_sleep_duration = FDB_COMPACTOR_SLEEP_DURATION;

    return fconfig;
}

bool validate_fdb_config(fdb_config *fconfig) {
    assert(fconfig);

    if (fconfig->chunksize < 4 || fconfig->chunksize > 64) {
        // Chunk size should be set between 4 and 64 bytes.
        return false;
    }
    if (fconfig->blocksize < 1024 || fconfig->blocksize > 131072) {
        // Block size should be set between 1KB and 128KB
        return false;
    }
    if (fconfig->seqtree_opt != FDB_SEQTREE_NOT_USE &&
        fconfig->seqtree_opt != FDB_SEQTREE_USE) {
        return false;
    }
    if (fconfig->durability_opt != FDB_DRB_NONE &&
        fconfig->durability_opt != FDB_DRB_ODIRECT &&
        fconfig->durability_opt != FDB_DRB_ASYNC &&
        fconfig->durability_opt != FDB_DRB_ODIRECT_ASYNC) {
        return false;
    }
    if ((fconfig->flags & FDB_OPEN_FLAG_CREATE) &&
        (fconfig->flags & FDB_OPEN_FLAG_RDONLY)) {
        return false;
    }
    if (fconfig->compaction_threshold > 100) {
        // Compaction threshold should be equal or less then 100 (%).
        return false;
    }
    if (fconfig->compactor_sleep_duration == 0) {
        // Sleep duration should be larger than zero
        return false;
    }

    return true;
}
