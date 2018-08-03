#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   环形缓冲区
 *          生产者与消费者同时访问缓冲区时无须加锁
 *          多个生产者访问同一个缓冲区时，生产者之间需要加锁
 *          多个消费者访问同一个缓冲区时，消费者之间需要加锁
 */
typedef void circular_buffer;

struct buffer_piece
{
    void * data;
    size_t size;
};

#define BIT(n)  (1 << n)

/* 使用读信号量 */
#define CB_READ_SEMA    BIT(0)

/* 使用写信号量 */
#define CB_WRIT_SEMA    BIT(1)

/* 是否通在进程间共享信号量 */
#define CB_SHARED_SEMA  BIT(2)

/**
 * \brief 创建环形缓冲区
 *
 * \param name      共享内存对象名称。如为空指针则创建无法附着的匿名缓冲区。
 *                  匿名缓冲区不创建共享内存对象，只能通过fork跨进程共享，无法通过exec跨进程共享。
 *                  命名缓冲区可以通过fork、exec跨进程共享。
 * \param capacity  缓冲区容量，实际分配的容量会向上对齐到4KB
 * \param flag      创建缓冲区时使用的标志（CB_READ_SEMA, CB_WRIT_SEMA, CB_SHARED_SEMA）。
 *                  跨进程的匿名、命名缓冲区如果需要在进程间共享信号量，需要设置CB_SHARED_SEMA
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer * circular_buffer_create(const char * name, size_t capacity, int flag);

/**
 * \brief 创建环形缓冲区
 *
 * \param shmfd     共享内存对象文件描述符，需要以O_RDWR方式打开；当shmfd值为-1时创建匿名缓冲区
 * \param capacity  缓冲区容量，实际分配的容量会向上对齐到4KB
 * \param flag      创建缓冲区时使用的标志（CB_READ_SEMA, CB_WRIT_SEMA）
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer * circular_buffer_fcreate(int shmfd, size_t capacity, int flag);

/**
 * \brief 附着到一个命名环形缓冲区
 *
 * \param name 环形缓冲区名称，不可为空指针
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer * circular_buffer_attach(const char * name);

/**
 * \brief 附着到环形缓冲区
 *
 * \param shmfd 共享内存对象文件描述符，共享内存上的缓冲必须是已经初始化好的，shmfd必须是有效的值
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer * circular_buffer_fattach(int shmfd);

/**
 * \brief 消费者接口，获取缓冲区对象中可读的buffer
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 *
 * \return 返回buffer_piece结构，其中data字段总是不为空，通过检测size字段长判断可读空间的大小
 */
struct buffer_piece circular_buffer_get_readable(circular_buffer * cb);

/**
 * \brief 当没有可读数据时将等待在读信号量上
 *
 * \param cb   环形缓冲区指针，不可为空指针
 * \param size 请求可读的数据的数量
 *
 * \return 返回0表示至少有size字节的数据可读；
 *         返回-1表示未设置CB_READ_SEMA或者信号量操作有误
 */
int circular_buffer_wait_readable(circular_buffer * cb, size_t size);

/**
 * \brief 生产者接口，获取缓冲区对象中可写的buffer
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 *
 * \return 返回buffer_piece结构，其中data字段总是不为空，通过检测size字段长判断可写空间的大小
 */
struct buffer_piece circular_buffer_get_writable(circular_buffer * cb);

/**
 * \brief 当没有可写空间时将等待在写信号量上
 *
 * \param cb   环形缓冲区指针，不可为空指针
 * \param size 请求可写的空间的大小
 *
 * \return 返回0表示至少有size字节的空间可写；
 *         返回-1表示未设置CB_READ_SEMA或者信号量操作有误
 */
int circular_buffer_wait_writable(circular_buffer * cb, size_t size);

/**
 * \brief 消费者接口，丢弃已读数据，调用此接口来通知缓冲区对象来移动读索引
 *
 * \param cb    环形缓冲区对象指针，不可为空指针
 * \param size  已读数据长度，当size超过可读数据长度时，会丢弃当前可读数据的实际长度
 *
 * \return 返回实际丢弃的长度
 */
size_t circular_buffer_pop_data(circular_buffer * cb, size_t size);

/**
 * \brief 生产者接口，通知已写入数据，调用此接口来通知缓冲区对象来移动写索引
 *
 * \param cb    环形缓冲区对象指针，不可为空指针
 * \param size  已写数据的长度，当size超过可写长度时，会自适应到可写的最大长度
 *
 * \return 返回实际写入的长度
 */
size_t circular_buffer_push_data(circular_buffer * cb, size_t size);

/**
 * \brief 解除附着，释放内存，共享内存对象不会销毁
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 */
void circular_buffer_detach(circular_buffer * cb);

/**
 * \brief 释放内存，删除共享内存对象，变成匿名对象，无法再附着，当引用数为0时会被系统销毁
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 */
void circular_buffer_destroy(circular_buffer * cb);

#ifdef __cplusplus
}
#endif
