#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "backtrace.h"

#include <assert.h>
#include <alloca.h>
#include <bfd.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BACKTRACE_MAX_DEPTH 1024

/**
 * @brief 地址信息
 */
struct backtrace_addrinfo {
    bfd         *bfd_;       ///< bfd 对象
    asymbol    **syms_;      ///< 符号表
    void        *addr_;      ///< 地址
    const char  *module_;    ///< 模块
    const char  *function_;  ///< 函数
    const char  *file_;      ///< 文件
    unsigned int line_;      ///< 行号
};

struct backtrace_frame_info {
    void         *base_;    ///< 基地址
    unsigned long offset_;  ///< 偏移
    const char   *module_;  ///< 模块路径
};

/**
 * @brief 获取给定地址的信息（函数名、源文件名、行号等）
 *
 * @param path     模块路径
 * @param offset   模块内偏移
 * @param addrinfo 地址信息
 * @return int 0 成功，-1 失败
 */
static int backtrace_get_addrinfo(
    const struct backtrace_frame_info *frame_info, struct backtrace_addrinfo *addrinfo) {
    assert(frame_info);
    assert(addrinfo);

    // 初始化 bfd
    bfd_init();

    // 打开模块
    bfd *bfd = bfd_openr(frame_info->module_, NULL);
    if (!bfd) {
        return -1;
    }

    // 检查格式
    if (!bfd_check_format(bfd, bfd_object)) {
        bfd_close(bfd);
        return -1;
    }

    // 检查是否有符号表
    if ((bfd->flags & HAS_SYMS) == 0) {
        bfd_close(bfd);
        return -1;
    }

    // 读取符号表
    asymbol    **syms;
    unsigned int sym_size;
    long sym_count = bfd_read_minisymbols(bfd, FALSE, (void **)&syms, &sym_size);  // 读取静态符号表
    if (sym_count == 0) {
        sym_count = bfd_read_minisymbols(bfd, TRUE, (void **)&syms, &sym_size);  // 读取动态符号表
    }

    // 检查符号表
    if (sym_count <= 0) {
        bfd_close(bfd);
        return -1;
    }

    // 获取偏移
    long const offset = frame_info->offset_;

    // 查找地址对应的文件、函数、行号
    for (struct bfd_section *section = bfd->sections; section; section = section->next) {
        const char  *filename, *function;
        unsigned int line;
        if (bfd_find_nearest_line(bfd, section, syms, offset, &filename, &function, &line)) {
            if (filename)
                addrinfo->file_ = filename;

            if (function)
                addrinfo->function_ = function;

            addrinfo->line_ = line;
        }
    }

    // 保存地址信息
    addrinfo->module_ = frame_info->module_;
    addrinfo->addr_   = frame_info->base_;
    addrinfo->bfd_    = bfd;
    addrinfo->syms_   = syms;

    // 释放资源
    return 0;
}

/**
 * @brief 释放地址信息中使用的资源
 *
 * @param addrinfo 地址信息
 */
static void backtrace_addrinfo_free(struct backtrace_addrinfo *addrinfo) {
    if (addrinfo) {
        if (addrinfo->syms_)
            free(addrinfo->syms_);

        if (addrinfo->bfd_)
            bfd_close(addrinfo->bfd_);

        addrinfo->bfd_      = NULL;
        addrinfo->syms_     = NULL;
        addrinfo->addr_     = NULL;
        addrinfo->module_   = NULL;
        addrinfo->function_ = NULL;
        addrinfo->file_     = NULL;
        addrinfo->line_     = 0;
    }
}

/**
 * @brief 获取帧信息
 *
 * @param info        帧信息
 * @param frames      帧地址
 * @param module_size 模块缓冲区大小
 */
static void backtrace_fill_frame_info(struct backtrace_frame_info info[],
                                      void                        *const frames[],
                                      size_t                       count) {
    assert(info);
    assert(frames);
    assert(count > 0);

    for (size_t i = 0; i < count; ++i) {
        Dl_info addr_info;
        if (dladdr(frames[i], &addr_info) == 0) {
            memset(&info[i], 0, sizeof(struct backtrace_frame_info));
        }

        info[i].base_   = addr_info.dli_fbase;
        info[i].offset_ = (unsigned long)frames[i] - (unsigned long)addr_info.dli_fbase - 1; // SP 指向下一条指令，所以减 1 来指向当前指令
        info[i].module_ = addr_info.dli_fname;
    }
}

/**
 * @brief 获取调用栈
 *
 * @param frame_info 调用栈
 * @param count      调用栈深度
 * @param callback   栈帧处理回调
 * @param opaque     回调参数
 */
static void backtrace_dump_frames(const struct backtrace_frame_info frame_info[],
                                  int                       count,
                                  backtrace_dump_callback_t callback,
                                  void                     *opaque) {
    assert(frame_info);
    assert(count > 0);
    assert(callback);

    for (int i = 0; i < count; ++i) {
        struct backtrace_addrinfo addrinfo;
        if (backtrace_get_addrinfo(&frame_info[i], &addrinfo) == 0) {
            callback(opaque, i, addrinfo.addr_, addrinfo.function_, addrinfo.file_, addrinfo.line_);
            backtrace_addrinfo_free(&addrinfo);
        }
    }
}

int backtrace_dump(backtrace_dump_callback_t callback, void *opaque) {
    assert(callback);

    // 获取调用栈
    void *frames[BACKTRACE_MAX_DEPTH];
    int   count = backtrace(frames, BACKTRACE_MAX_DEPTH);
    if (count <= 0) {
        return -1;
    }

    // 获取栈帧信息
    struct backtrace_frame_info *frame_info = alloca(count * sizeof(struct backtrace_frame_info));
    backtrace_fill_frame_info(frame_info, frames, count);

    // 处理调用栈
    backtrace_dump_frames(frame_info, count, callback, opaque);
    return 0;
}

int backtrace_dump_save(int fd) {
    // 获取调用栈
    void *frames[BACKTRACE_MAX_DEPTH];
    int   count = backtrace(frames, BACKTRACE_MAX_DEPTH);
    if (count <= 0) {
        return -1;
    }

    // 获取栈帧信息
    struct backtrace_frame_info *frame_info = alloca(count * sizeof(struct backtrace_frame_info));
    backtrace_fill_frame_info(frame_info, frames, count);

    // 将地址写入文件
    FILE *fp = fdopen(fd, "w");
    if (!fp) {
        return -1;
    }

    // 写入栈帧数量
    if (fwrite(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    // 保存栈帧信息
    for (int i = 0; i < count; ++i) {
        // 写基地址
        if (fwrite(&frame_info[i].base_, sizeof(void *), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 写偏移
        if (fwrite(&frame_info[i].offset_, sizeof(unsigned long), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 写模块路径长度，包含 '\0'
        size_t len = strlen(frame_info[i].module_) + 1;
        if (fwrite(&len, sizeof(size_t), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 写模块路径
        if (fwrite(frame_info[i].module_, len, 1, fp) != 1) {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

int backtrace_dump_load(int fd, backtrace_dump_callback_t callback, void *opaque) {
    assert(callback);

    // 从文件中读取地址
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        return -1;
    }

    // 读取栈帧数量
    int count;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count <= 0) {
        fclose(fp);
        return -1;
    }

    // 分配栈帧信息内存
    struct backtrace_frame_info *frames = alloca(count * sizeof(struct backtrace_frame_info));

    // 读取栈帧信息
    for (int i = 0; i < count; ++i) {
        // 读基地址
        if (fread(&frames[i].base_, sizeof(void *), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 读偏移
        if (fread(&frames[i].offset_, sizeof(unsigned long), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 读模块路径长度
        size_t len;
        if (fread(&len, sizeof(size_t), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }

        // 读模块路径
        frames[i].module_ = alloca(len);
        if (fread((void *)frames[i].module_, len, 1, fp) != 1) {
            fclose(fp);
            return -1;
        }
    }

    // 处理调用栈
    backtrace_dump_frames(frames, count, callback, opaque);

    fclose(fp);
    return 0;
}