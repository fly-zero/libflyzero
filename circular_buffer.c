#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "circular_buffer.h"

struct circular_buffer_header
{
    size_t capacity;            // 缓冲区容量
    volatile size_t widx;       // 写索引
    volatile size_t ridx;       // 读索引
    char name[256];             // 共享内存对象名称
    sem_t rsema;                // 读信号量
    size_t rreq;                // 读请求数量
    sem_t wsema;                // 写信号量
    size_t wreq;                // 写请求数量
    int flag;                   // 标志
};

static inline size_t readable_size(struct circular_buffer_header * cb)
{
    return cb->widx - cb->ridx;
}

static inline size_t writable_size(struct circular_buffer_header * cb)
{
    return cb->capacity - readable_size(cb);
}

void * map_shared_memaroy(int const shmfd, size_t const capacity)
{
    assert(capacity > 0);

    // 对齐到4KB
    size_t const aligned_head_size = ((sizeof (struct circular_buffer_header)) + 4095) & (~4095);

    // 计算总长度
    size_t const total = capacity + aligned_head_size;

    // 先预订total + capacity长度的地址空间
    void * buffer = mmap(NULL, total + capacity, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (!buffer)
        goto ERROR_RETURN;

    int flag = MAP_SHARED | MAP_FIXED;

    // 如果未设置文件描述符，则设置匿名标志
    if (shmfd == -1)
        flag |= MAP_ANONYMOUS;

    // 将共享内存的[0, total)段映射到预订的内存[buffer, buffer + total)段上
    if (mmap(buffer, total, PROT_READ | PROT_WRITE, flag, shmfd, 0) != buffer)
        goto ERROR_RETURN;

    // 创建地址相邻的镜像内存区
    // 将共享内存[aligned_head_size, total)段内存再次映射到预订的内存[buffer + total, buffer + total + capacity)段
    if (mremap((char *)buffer + aligned_head_size, 0, capacity, MREMAP_MAYMOVE | MREMAP_FIXED, (char *)buffer + total) != (char *)buffer + total)
        goto ERROR_RETURN;

    return buffer;

ERROR_RETURN:
    if (buffer != MAP_FAILED)
        munmap(buffer, total + capacity);

    return NULL;
}

circular_buffer * circular_buffer_create(const char * name, size_t capacity, int const flag)
{
    assert(capacity > 0);

    int const anonymous = name ? 0 : 1;

    int shmfd;

    if (anonymous)
        shmfd = -1;
    else {
        // 创建共享内存对象
        shmfd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (shmfd < 0)
            return NULL;
    }

    struct circular_buffer_header * header = circular_buffer_fcreate(shmfd, capacity, flag);

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

circular_buffer * circular_buffer_fcreate(int const shmfd, size_t capacity, int const flag)
{
    // align to 4K
    capacity = (capacity + 4095) & (~4095);

    size_t const aligned_head_size = (sizeof (struct circular_buffer_header) + 4095) & (~4095);

    // 设置共享内存大小
    if (shmfd >= 0 && ftruncate(shmfd, capacity + aligned_head_size) < 0)
        return NULL;

    // 映射共享内存
    struct circular_buffer_header * header = map_shared_memaroy(shmfd, capacity);

    if (!header)
        return NULL;

    // 创建读写信号量
    int const shared = flag & CB_SHARED_SEMA;

    if (flag & CB_READ_SEMA)
        if (sem_init(&header->rsema, shared, 0) < 0)
            goto read_sema_error;

    if (flag & CB_WRIT_SEMA)
        if (sem_init(&header->wsema, shared, 0) < 0)
            goto write_sema_error;

    // 初始共享内存队列头
    header->capacity = capacity;
    header->ridx = 0;
    header->widx = 0;
    header->name[0] = 0;
    header->flag = flag;
    header->rreq = 0;
    header->wreq = 0;

    return header;

write_sema_error:   // 初始化写信号量失败
    sem_destroy(&header->rsema);

read_sema_error:    // 初始化读信号量失败
    munmap(header, aligned_head_size + header->capacity + header->capacity);

    return NULL;
}

circular_buffer * circular_buffer_attach(const char * name)
{
    assert(name);

    struct circular_buffer_header * header = NULL;

    int const shmfd = shm_open(name, O_RDWR, 0);

    if (shmfd < 0)
        return header;

    header = circular_buffer_fattach(shmfd);

    close(shmfd);

    return header;
}

circular_buffer * circular_buffer_fattach(int const shmfd)
{
    assert(shmfd >= 0);

    size_t capacity;

    // 从共享内存对象文件中读出容量
    ssize_t const nread = read(shmfd, &capacity, sizeof capacity);

    if (nread == sizeof capacity) {
        // 重置文件读写偏移
        lseek(shmfd, 0, SEEK_SET);

        // 映射共享内存
        return map_shared_memaroy(shmfd, capacity);
    }

    return NULL;
}

struct buffer_piece circular_buffer_get_writable(circular_buffer * cb)
{
    assert(cb);

    struct buffer_piece ret = { };

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    ret.size = writable_size(header);

    size_t const aligned_head_size = (sizeof (struct circular_buffer_header) + 4095) & (~4095);

    ret.data = (char*)header + aligned_head_size + (header->widx & (header->capacity - 1));

    return ret;
}

int circular_buffer_wait_writable(circular_buffer * cb, size_t const size)
{
    assert(cb);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    // 检查写信号里是否已经设置
    if (!(header->flag & CB_WRIT_SEMA)) return -1;

    // 请求数量不可为0
    if (size == 0) return -1;

    // 检查是否有可写空间
    if (writable_size(header) >= size) goto out;

    // 保存请求数量
    // 当另一个线程的post发生在设置rreq之前，即wait线程的上一个wait被异常中断
    header->wreq = size;

    // 当另一个线程的post发生在设置rreq之后，信号量会触发，但不会被sem_wait被捕获

    while (writable_size(header) >= size) {

        // 阻塞在信号量上
        int const ret = sem_wait(&header->wsema);

        // 发生错误
        if (ret < 0) return -1;
    }

out:
    sem_trywait(&header->wsema);

    return 0;
}

struct buffer_piece circular_buffer_get_readable(circular_buffer * cb)
{
    assert(cb);

    struct buffer_piece ret = { };

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    ret.size = readable_size(header);

    size_t const aligned_head_size = (sizeof (struct circular_buffer_header) + 4095) & (~4095);

    ret.data = (char*)header + aligned_head_size + (header->ridx & (header->capacity - 1));

    return ret;
}

int circular_buffer_wait_readable(circular_buffer * cb, size_t const size)
{
    assert(cb);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    // 检查是否启用了读信号量
    if (!(header->flag & CB_READ_SEMA)) return -1;

    // 请求数量不可为0
    if (size == 0) return -1;

    // 检查是否有可读数据
    if (readable_size(header) >= size) goto out;

    // 保存请求数量
    // 当另一个线程的post发生在设置rreq之前，即wait线程的上一个wait被异常中断
    header->rreq = size;

    // 当另一个线程的post发生在设置rreq之后，信号量会触发，但不会被sem_wait被捕获

    while (readable_size(header) < size) {

        // 阻塞在信号量上
        int const ret = sem_wait(&header->rsema);

        // 发生错误
        if (ret < 0) return -1;
    }

out:
    sem_trywait(&header->rsema);

    return 0;
}

size_t circular_buffer_pop_data(circular_buffer * cb, size_t size)
{
    assert(cb);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    size_t const tmp = readable_size(header);

    if (size > tmp)
        size = tmp;

    header->ridx += size;

    if (header->flag & CB_WRIT_SEMA) {
        if (header->wreq > 0 && writable_size(header) > header->wreq) {
            header->wreq = 0;
            sem_post(&header->wsema);
        }
    }

    return size;
}

size_t circular_buffer_push_data(circular_buffer * cb, size_t size)
{
    assert(cb);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    size_t const tmp = writable_size(header);

    if (size > tmp)
        size = tmp;

    header->widx += size;

    if (header->flag & CB_READ_SEMA) {
        if (header->rreq > 0 && readable_size(header) >= header->rreq) {
            header->rreq = 0;
            sem_post(&header->rsema);
        }
    }

    return size;
}

void circular_buffer_detach(circular_buffer * cb)
{
    assert(cb);

    size_t const aligned_head_size = (sizeof (struct circular_buffer_header) + 4095) & (~4095);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    munmap(header, aligned_head_size + header->capacity + header->capacity);
}

void circular_buffer_destroy(circular_buffer * cb)
{
    assert(cb);

    struct circular_buffer_header * header = (struct circular_buffer_header*)cb;

    if (header->name[0])
        shm_unlink(header->name);

    size_t const aligned_head_size = (sizeof (struct circular_buffer_header) + 4095) & (~4095);

    munmap(header, aligned_head_size + header->capacity + header->capacity);
}
