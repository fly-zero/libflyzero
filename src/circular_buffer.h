#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief 环形缓冲区
 *        生产者与消费者同时访问缓冲区时无须加锁
 *        多个生产者访问同一个缓冲区时，生产者之间需要加锁
 *        多个消费者访问同一个缓冲区时，消费者之间需要加锁
 */
typedef void circular_buffer;

struct buffer_piece {
    void* data;
    size_t size;
};

/**
 * \brief 创建环形缓冲区
 *
 * \param name         共享内存对象名称。如为空指针则创建无法附着的匿名缓冲区。
 *                     匿名缓冲区不创建共享内存对象，只能通过 fork 跨进程共享，无法通过 exec 跨进程共享。
 *                     命名缓冲区可以通过 fork、exec 跨进程共享。
 * \param capacity     缓冲区容量，实际分配的容量会向上对齐到 4KB
 * \param private_size 私有数据区大小，用于存储私有数据
 * \param flag         创建缓冲区时使用的标志，暂时未使用
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer* circular_buffer_create(
    const char* name, size_t capacity, size_t private_size, int flag);

/**
 * \brief 创建环形缓冲区
 *
 * \param shmfd        共享内存对象文件描述符，需要以 O_RDWR 方式打开；当 shmfd 值为 -1 时创建匿名缓冲区
 * \param capacity     缓冲区容量，实际分配的容量会向上对齐到 4KB
 * \param private_size 私有数据区大小，用于存储私有数据
 * \param flag         创建缓冲区时使用的标志，暂时未使用
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer* circular_buffer_fcreate(
    int shmfd, size_t capacity, size_t private_size, int flag);

/**
 * \brief 附着到一个命名环形缓冲区
 *
 * \param name 环形缓冲区名称，不可为空指针
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer* circular_buffer_attach(const char* name);

/**
 * \brief 附着到环形缓冲区
 *
 * \param shmfd 共享内存对象文件描述符，共享内存上的缓冲必须是已经初始化好的，shmfd 必须是有效的值
 *
 * \return 成功时返回环形缓冲区对象指针，失败时返回空指针
 */
circular_buffer* circular_buffer_fattach(int shmfd);

/**
 * @brief 获取缓冲区对象中的私有数据
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 *
 * \return 返回私有数据区指针
 */
void * circular_buffer_get_private_data(circular_buffer* cb);

/**
 * \brief 消费者接口，获取缓冲区对象中可读的 buffer
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 *
 * \return 返回 buffer_piece 结构，其中 data 字段总是不为空，通过检测 size 字段长判断可读空间的大小
 */
struct buffer_piece circular_buffer_get_readable(circular_buffer* cb);

/**
 * \brief 生产者接口，获取缓冲区对象中可写的 buffer
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 *
 * \return 返回 buffer_piece 结构，其中 data 字段总是不为空，通过检测 size 字段长判断可写空间的大小
 */
struct buffer_piece circular_buffer_get_writable(circular_buffer* cb);

/**
 * \brief 消费者接口，丢弃已读数据，调用此接口来通知缓冲区对象来移动读索引
 *
 * \param cb   环形缓冲区对象指针，不可为空指针
 * \param size 已读数据长度，当 size 超过可读数据长度时，会丢弃当前可读数据的实际长度
 *
 * \return 返回实际丢弃的长度
 */
size_t circular_buffer_pop_data(circular_buffer* cb, size_t size);

/**
 * \brief 生产者接口，通知已写入数据，调用此接口来通知缓冲区对象来移动写索引
 *
 * \param cb   环形缓冲区对象指针，不可为空指针
 * \param size 已写数据的长度，当 size 超过可写长度时，会自适应到可写的最大长度
 *
 * \return 返回实际写入的长度
 */
size_t circular_buffer_push_data(circular_buffer* cb, size_t size);

/**
 * \brief 解除附着，释放内存，共享内存对象不会销毁
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 */
void circular_buffer_detach(circular_buffer* cb);

/**
 * \brief 释放内存，删除共享内存对象，变成匿名对象，无法再附着，当引用数为0时会被系统销毁
 *
 * \param cb 环形缓冲区对象指针，不可为空指针
 */
void circular_buffer_destroy(circular_buffer* cb);

#ifdef __cplusplus
}
#endif
