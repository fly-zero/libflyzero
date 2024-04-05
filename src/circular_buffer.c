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

#define CACHE_LINE_SIZE 64

#define __cache_aligned __attribute__((aligned(CACHE_LINE_SIZE)))

struct circular_buffer_index {
    size_t value;  ///< 索引
} __cache_aligned;

struct circular_buffer_header {
    struct circular_buffer_index r;     ///< 写缓冲区上下文
    struct circular_buffer_index w;     ///< 读缓冲区上下文
    size_t capacity;                    ///< 缓冲区容量
    size_t private_size;                ///< 私有数据大小
    long page_mask;                     ///< 页掩码
    int flag;                           ///< 标志
    char name[64];                      ///< 共享内存对象名称
    char cache_line[0] __cache_aligned; ///< 缓存行对齐填充
    char private_data[0];               ///< 私有数据
};

static inline size_t readable_size(struct circular_buffer_header *cb) {
    return cb->w.value - cb->r.value;
}

static inline size_t writable_size(struct circular_buffer_header *cb) {
    return cb->capacity - readable_size(cb);
}

static inline size_t aligned_header_size(size_t private_size, long page_mask) {
    return (sizeof(struct circular_buffer_header) + private_size + page_mask) & (~page_mask);
}

static void *map_shared_memory(int const shmfd, size_t head_size, size_t const capacity) {
    // head_size 和 capacity 必须是系统页大小的整数倍
    assert(head_size > 0);
    assert(capacity > 0);
    assert((head_size & (sysconf(_SC_PAGESIZE) - 1)) == 0);
    assert((capacity & (sysconf(_SC_PAGESIZE) - 1)) == 0);

    // 先预订 total + capacity 长度的地址空间
    size_t const total = capacity + head_size;
    void *addr = mmap(NULL, total + capacity, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!addr) goto ERROR_RETURN;

    // 将共享内存的 [0, total) 段映射到预订的内存 [addr, addr + total) 段上
    int flags = MAP_SHARED | MAP_FIXED | (shmfd == -1 ? MAP_ANONYMOUS : 0);
    int prot = PROT_READ | PROT_WRITE;
    void *new_addr = mmap(addr, total, prot, flags, shmfd, 0);
    if (new_addr != addr) {
        goto ERROR_RETURN;
    }

    // 创建地址相邻的镜像内存区
    // 将共享内存 [head_size, total) 段内存再次映射到预订的内存 [addr + total, addr + total + capacity) 段上
    if (mremap((char *)addr + head_size, 0, capacity,
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

circular_buffer *circular_buffer_create(
    const char *name, size_t capacity, size_t private_size, int const flag) {
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
        circular_buffer_fcreate(shmfd, capacity, private_size, flag);
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

circular_buffer *circular_buffer_fcreate(
    int const shmfd, size_t capacity, size_t private_size, int const flag) {
    // 获取系统页大小
    long const page_mask = sysconf(_SC_PAGESIZE) - 1;

    // 计算对齐到系统页大小的容量
    capacity = (capacity + page_mask) & (~page_mask);

    // 计算对齐到系统页大小的头部大小
    size_t const aligned_head_size = aligned_header_size(private_size, page_mask);

    // 设置共享内存大小
    if (shmfd >= 0 && ftruncate(shmfd, capacity + aligned_head_size) < 0) {
        return NULL;
    }

    // 映射共享内存
    struct circular_buffer_header *header = map_shared_memory(shmfd, aligned_head_size, capacity);
    if (!header) return NULL;

    // 初始共享内存队列头
    header->r.value = 0;
    header->w.value = 0;
    header->name[0] = 0;
    header->capacity = capacity;
    header->private_size = private_size;
    header->page_mask = page_mask;
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

    // 从共享内存对象文件中读出 header 的副本
    struct circular_buffer_header header;
    ssize_t const nread = read(shmfd, &header, sizeof header);
    if (nread == sizeof header) {
        lseek(shmfd, 0, SEEK_SET);
        size_t head_size = aligned_header_size(header.private_size, header.page_mask);
        return map_shared_memory(shmfd, head_size, header.capacity);
    }

    return NULL;
}

void *circular_buffer_get_private_data(circular_buffer *cb) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    return header->private_data;
}

struct buffer_piece circular_buffer_get_writable(circular_buffer *cb) {
    assert(cb);
    struct buffer_piece ret = {};
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    ret.size = writable_size(header);
    size_t const aligned_head_size =
        aligned_header_size(header->private_size, header->page_mask);
    ret.data = (char *)header + aligned_head_size +
               (header->w.value & (header->capacity - 1));
    return ret;
}

struct buffer_piece circular_buffer_get_readable(circular_buffer *cb) {
    assert(cb);
    struct buffer_piece ret = {};
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    ret.size = readable_size(header);
    size_t const aligned_head_size =
        aligned_header_size(header->private_size, header->page_mask);
    ret.data = (char *)header + aligned_head_size +
               (header->r.value & (header->capacity - 1));
    return ret;
}

size_t circular_buffer_pop_data(circular_buffer *cb, size_t size) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    size_t const tmp = readable_size(header);
    if (size > tmp) size = tmp;
    header->r.value += size;
    return size;
}

size_t circular_buffer_push_data(circular_buffer *cb, size_t size) {
    assert(cb);
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    size_t const tmp = writable_size(header);
    if (size > tmp) size = tmp;
    header->w.value += size;
    return size;
}

void circular_buffer_detach(circular_buffer *cb) {
    assert(cb);
    // 计算共享内存大小
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    size_t const aligned_head_size =
        aligned_header_size(header->private_size, header->page_mask);

    // 解除映射
    munmap(header, aligned_head_size + header->capacity + header->capacity);
}

void circular_buffer_destroy(circular_buffer *cb) {
    assert(cb);
    // 删除共享内存对象
    struct circular_buffer_header *header = (struct circular_buffer_header *)cb;
    if (header->name[0]) {
        shm_unlink(header->name);
    }

    // 计算共享内存大小
    size_t const aligned_head_size =
        aligned_header_size(header->private_size, header->page_mask);

    // 解除映射
    munmap(header, aligned_head_size + header->capacity + header->capacity);
}
