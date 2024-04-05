#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief backtrace_dump 回调函数
 *
 * @param opaque    用户数据
 * @param frame_no  帧编号
 * @param addr      地址
 * @param function  函数名
 * @param file      文件名
 * @param line_no   行号
 */
typedef void (*backtrace_dump_callback_t)(void        *opaque,
                                          unsigned int frame_no,
                                          void        *addr,
                                          const char  *function,
                                          const char  *file,
                                          unsigned int line_no);

/**
 * @brief 导出当前线程的回溯信息
 *
 * @param callback 回调函数
 * @param opaque   用户数据
 * @return int 0 成功；-1 失败
 */
int backtrace_dump(backtrace_dump_callback_t callback, void *opaque);

/**
 * @brief 保存当前线程的回溯信息
 *
 * @param fd 文件描述符
 * @return int 0 成功；-1 失败
 */
int backtrace_dump_save(int fd);

/**
 * @brief 加载回溯信息
 *
 * @param fd       文件描述符
 * @param callback 回调函数
 * @param opaque   用户数据
 * @return int 0 成功；-1 失败
 */
int backtrace_dump_load(int fd, backtrace_dump_callback_t callback, void *opaque);

#ifdef __cplusplus
}
#endif