#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "circular_buffer.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

struct circular_buffer_header {
    size_t capacity;       ///< 缓冲区容量
    volatile size_t widx;  ///< 写索引
    volatile size_t ridx;  ///< 读索引
    char name[256];        ///< 共享内存对象名称
    int flag;              ///< 标志
};

static inline size_t readable_size(struct circular_buffer_header *cb) {
    return cb->widx - cb->ridx;
}

static inline size_t writable_size(struct circular_buffer_header *cb) {
    return cb->capacity - readable_size(cb);
}

void *map_shared_memory(int const shmfd, size_t const capacity) {
    assert(capacity > 0);

    // 对齐到4KB
    size_t const aligned_head_size =
        ((sizeof(struct circular_buffer_header)) + 4095) & (~4095);

    // 先预订 total + capacity 长度的地址空间
    size_t const total = capacity + aligned_head_size;
    void *addr = mmap(NULL, total + capacity, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!addr) goto ERROR_RETURN;

    // 将共享内存的[0, total)段映射到预订的内存[buffer, buffer + total)段上
    int flags = MAP_SHARED | MAP_FIXED | (shmfd == -1 ? MAP_ANONYMOUS : 0);
    int prot = PROT_READ | PROT_WRITE;
    void *new_addr = mmap(addr, total, prot, flags, shmfd, 0);
    if (new_addr != addr) {
        goto ERROR_RETURN;
    }

    // 创建地址相邻的镜像内存区
    // 将共享内存 [aligned_head_size, total) 段内存再次映射到预订的内存[buffer +
    // total, buffer + total + capacity) 段
    if (mremap((char *)addr + aligned_head_size, 0, capacity,
               MREMAP_MAYMOVE | MREMAP_FIXED,
               (char *)addr + total) != (char *)addr + total) {
        goto ERROR_RETURN;
    }

    return addr;

ERROR_RETURN:
    if (addr != MAP_FAILED) {
        munmap(addr, total + capacity);
    }

    return NULL;
}

circular_buffer *circular_buffer_create(const char *name, size_t capacity,
                                        int const flag) {
    assert(capacity > 0);
    int const anonymous = name ? 0 : 1;
    int shmfd;
    if (anonymous)
        shmfd = -1;
    else {
        // 创建共享内存对象
        shmfd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (shmfd < 0) return NULL;
    }

    struct circular_buffer_header *header =
        circular_buffer_fcreate(shmfd, capacity, flag);
    if (!header) {
        close(shmfd);
        return NULL;
    }

    if (!anonymous) {
        // 保存共享内存对象名称
        strcpy(header->name, name);
        close(shmfd);
    }

    return header;
}

circular_buffer *circular_buffer_fcreate(int const shmfd, size_t capacity,
                                         int const flag) {
    // align to 4K
    capacity = (capacity + 4095) & (~4095);
    size_t const aligned_head_size =
        (sizeof(struct circular_buffer_header) + 4095) & (~4095);

    // 设置共享内存大小
    if (shmfd >= 0 && ftruncate(shmfd, capacity + aligned_head_size) < 0) {
        return NULL;
    }

    // 映射共享内存
    struct circular_buffer_header *header = map_shared_memory(shmfd, capacity);
    if (!header) return NULL;

    // 初始共享内存队列头
    header->capacity = capacity;
    header->ridx = 0;
    header->widx = 0;
    header->name[0] = 0;
    header->flag = flag;
    return header;
}

circular_buffer *circular_buffer_attach(const char *name) {
    assert(name);

    // 打开共享内存对象
    struct circular_buffer_header *header = NULL;
    int const shmfd = shm_open(name, O_RDWR, 0);
    if (shmfd < 0) return header;

    // 映射共享内存
    header = circular_buffer_fattach(shmfd);
    close(shmfd);
    return header;
}

circular_buffer *circular_buffer_fattach(int const shmfd) {
    assert(shmfd >= 0);

    // 从共享内存对象文件中读出容量
    size_t capacity;
    ssize_t const nread = read(shmfd, &capacity, sizeof capacity);
    if (nread == sizeof capacity) {
        lseek(shmfd, 0, SEEK_SET);
        return map_shared_memory(shmfd, capacity);
    }

    return NULL;
}

struct buffer_piece circular_buffer_get_writable(circular_buffer *cb) {
    assert(cb);
    struct buffer_piece ret = {};
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    ret.size = writable_size(header);
    size_t const aligned_head_size =
        (sizeof(struct circular_buffer_header) + 4095) & (~4095);
    ret.data = (char *)header + aligned_head_size +
               (header->widx & (header->capacity - 1));
    return ret;
}

struct buffer_piece circular_buffer_get_readable(circular_buffer *cb) {
    assert(cb);
    struct buffer_piece ret = {};
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    ret.size = readable_size(header);
    size_t const aligned_head_size =
        (sizeof(struct circular_buffer_header) + 4095) & (~4095);
    ret.data = (char *)header + aligned_head_size +
               (header->ridx & (header->capacity - 1));
    return ret;
}

size_t circular_buffer_pop_data(circular_buffer *cb, size_t size) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    size_t const tmp = readable_size(header);
    if (size > tmp) size = tmp;
    header->ridx += size;
    return size;
}

size_t circular_buffer_push_data(circular_buffer *cb, size_t size) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    size_t const tmp = writable_size(header);
    if (size > tmp) size = tmp;
    header->widx += size;
    return size;
}

void circular_buffer_detach(circular_buffer *cb) {
    assert(cb);
    size_t const aligned_head_size =
        (sizeof(struct circular_buffer_header) + 4095) & (~4095);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    munmap(header, aligned_head_size + header->capacity + header->capacity);
}

void circular_buffer_destroy(circular_buffer *cb) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    if (header->name[0]) shm_unlink(header->name);
    size_t const aligned_head_size =
        (sizeof(struct circular_buffer_header) + 4095) & (~4095);
    munmap(header, aligned_head_size + header->capacity + header->capacity);
}
