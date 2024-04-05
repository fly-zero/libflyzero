#include <unistd.h>

#include <cassert>
#include <cstring>

#include <stack>

#include <backtrace.h>

// 栈帧信息
struct frame {
    const char  *function_;
    const char  *file_;
    unsigned int line_no_;
};

// 期望的栈帧信息
static std::stack<frame> s_expect;

// 调用函数并记录栈帧信息
#define call_and_record(func, ...)                         \
    do {                                                   \
        s_expect.push({__FUNCTION__, __FILE__, __LINE__}); \
        func(__VA_ARGS__);                                 \
    } while (0)

// 栈帧回调函数
static void stack_frame_callback(void        *opaque,
                                 unsigned int frame_no,
                                 void        *addr,
                                 const char  *function,
                                 const char  *file,
                                 unsigned int line_no) {
    // 第 0 帧是 backtrace_dump，忽略
    if (frame_no == 0) {
        return;
    }

    // 期望的栈帧信息为空，忽略
    if (s_expect.empty()) {
        return;
    }

    // 比较栈帧信息
    auto &top = s_expect.top();
    assert(strcmp(top.function_, function) == 0);
    assert(strcmp(top.file_, file) == 0);
    assert(top.line_no_ == line_no);
    s_expect.pop();
}

// 执行一些操作
static void do_something() {}

// 函数 fun
static void fun() {
    do_something();
    call_and_record(backtrace_dump, stack_frame_callback, nullptr);
    do_something();
}

// 函数 gun
static void gun() {
    do_something();
    call_and_record(fun);
    do_something();
}

// 测试 backtrace_dump
static void test_backtrace_dump() {
    do_something();
    call_and_record(gun);
    do_something();
}

// 测试 backtrace_dump_save 和 backtrace_dump_load
static void test_backtrace_dump_save_and_load() {
    // 创建管道
    int fd[2];
    assert(pipe(fd) == 0);

    // 保存回溯信息
    call_and_record(backtrace_dump_save, fd[0]);

    // 关闭写端
    close(fd[0]);

    // 加载回溯信息
    call_and_record(backtrace_dump_load, fd[1], stack_frame_callback, nullptr);

    // 关闭读端
    close(fd[1]);
}

int main() {
    call_and_record(test_backtrace_dump);
    call_and_record(test_backtrace_dump_save_and_load);
}