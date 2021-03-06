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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "filemgr.h"
#include "filemgr_ops.h"

#if !defined(WIN32) && !defined(_WIN32)

int _filemgr_linux_open(const char *pathname, int flags, mode_t mode)
{
    int fd;
    do {
        fd = open(pathname, flags | O_LARGEFILE, mode);
    } while (fd == -1 && errno == EINTR);

    if (fd < 0) {
        if (errno == ENOENT) {
            return (int) FDB_RESULT_NO_SUCH_FILE;
        } else {
            return (int) FDB_RESULT_OPEN_FAIL;
        }
    }
    return fd;
}

ssize_t _filemgr_linux_pwrite(int fd, void *buf, size_t count, cs_off_t offset)
{
    ssize_t rv;
    do {
        rv = pwrite(fd, buf, count, offset);
    } while (rv == -1 && errno == EINTR);

    if (rv < 0) {
        return (ssize_t) FDB_RESULT_WRITE_FAIL;
    }
    return rv;
}

ssize_t _filemgr_linux_pread(int fd, void *buf, size_t count, cs_off_t offset)
{
    ssize_t rv;
    do {
        rv = pread(fd, buf, count, offset);
    } while (rv == -1 && errno == EINTR);

    if (rv < 0) {
        return (ssize_t) FDB_RESULT_READ_FAIL;
    }
    return rv;
}

int _filemgr_linux_close(int fd)
{
    int rv = 0;
    if (fd != -1) {
        do {
            rv = close(fd);
        } while (rv == -1 && errno == EINTR);
    }

    if (rv < 0) {
        return FDB_RESULT_CLOSE_FAIL;
    }

    return FDB_RESULT_SUCCESS;
}

cs_off_t _filemgr_linux_goto_eof(int fd)
{
    cs_off_t rv = lseek(fd, 0, SEEK_END);
    if (rv < 0) {
        return (cs_off_t) FDB_RESULT_SEEK_FAIL;
    }
    return rv;
}

cs_off_t _filemgr_linux_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == -1) {
        return (cs_off_t) FDB_RESULT_READ_FAIL;
    }
    return st.st_size;
}

int _filemgr_linux_fsync(int fd)
{
    int rv;
    do {
        rv = fsync(fd);
    } while (rv == -1 && errno == EINTR);

    if (rv == -1) {
        return FDB_RESULT_FSYNC_FAIL;
    }

    return FDB_RESULT_SUCCESS;
}

int _filemgr_linux_fdatasync(int fd)
{
#ifdef __linux__
    int rv;
    do {
        rv = fdatasync(fd);
    } while (rv == -1 && errno == EINTR);

    if (rv == -1) {
        return FDB_RESULT_FSYNC_FAIL;
    }

    return FDB_RESULT_SUCCESS;
#else
    return _filemgr_linux_fsync(fd);
#endif
}

void _filemgr_linux_get_errno_str(char *buf, size_t size) {
    if (!buf) {
        return;
    }
    snprintf(buf, size, "errno = %d: '%s'", errno, strerror(errno));
}

struct filemgr_ops linux_ops = {
    _filemgr_linux_open,
    _filemgr_linux_pwrite,
    _filemgr_linux_pread,
    _filemgr_linux_close,
    _filemgr_linux_goto_eof,
    _filemgr_linux_file_size,
    _filemgr_linux_fdatasync,
    _filemgr_linux_fsync,
    _filemgr_linux_get_errno_str
};

struct filemgr_ops * get_linux_filemgr_ops()
{
    return &linux_ops;
}

#endif
