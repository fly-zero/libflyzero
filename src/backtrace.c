#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "backtrace.h"

#include <assert.h>
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

static int backtrace_get_addrinfo(void *addr, struct backtrace_addrinfo *addrinfo) {
    assert(addrinfo);

    // 获取 addr 所在的模块
    Dl_info info;
    if (dladdr(addr, &info) == 0) {
        return -1;
    }

    // 获取模块的基地址
    const char *path = info.dli_fname;
    const char *base = info.dli_fbase;
    if (!path || !base) {
        return -1;
    }

    // 初始化 bfd
    bfd_init();

    // 打开模块
    bfd *bfd = bfd_openr(path, NULL);
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

    // 计算偏移
    long const offset = (uintptr_t)addr - (uintptr_t)base;

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
    addrinfo->module_ = strdup(path);
    addrinfo->addr_   = addr;
    addrinfo->bfd_    = bfd;
    addrinfo->syms_   = syms;

    // 释放资源
    return 0;
}

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

static void backtrace_dump_frames(void                     *frames[],
                                  int                       count,
                                  backtrace_dump_callback_t callback,
                                  void                     *opaque) {
    assert(frames);
    assert(count > 0);
    assert(callback);

    for (int i = 0; i < count; ++i) {
        struct backtrace_addrinfo addrinfo;
        if (backtrace_get_addrinfo((char *)frames[i] - 1, &addrinfo) == 0) {
            callback(opaque, i, frames[i], addrinfo.function_, addrinfo.file_, addrinfo.line_);
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

    // 处理调用栈
    backtrace_dump_frames(frames, count, callback, opaque);
    return 0;
}

int backtrace_dump_save(int fd) {
    // 获取调用栈
    void *frames[BACKTRACE_MAX_DEPTH];
    int   count = backtrace(frames, BACKTRACE_MAX_DEPTH);
    if (count <= 0) {
        return -1;
    }

    // 将地址写入文件
    FILE *fp = fdopen(fd, "w");
    if (!fp) {
        return -1;
    }

    // 写入地址
    for (int i = 0; i < count; ++i) {
        fwrite(&frames[i], sizeof(void *), 1, fp);
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

    // 读取地址
    void *frames[BACKTRACE_MAX_DEPTH];
    int   count = fread(frames, sizeof(void *), BACKTRACE_MAX_DEPTH, fp);
    if (count <= 0) {
        return -1;
    }

    // 处理调用栈
    backtrace_dump_frames(frames, count, callback, opaque);

    fclose(fp);
    return 0;
}