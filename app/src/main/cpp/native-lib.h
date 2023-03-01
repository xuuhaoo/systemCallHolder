//
// Created by 徐昊 on 2023/2/27.
//

#ifndef SYSTEMCALL_NATIVE_LIB_H
#define SYSTEMCALL_NATIVE_LIB_H

#include <jni.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/syscall.h> /* SYS_write */
#include <sys/types.h>
#include <sys/wait.h>
#include <elf.h>
#include <android/log.h>

#define LOG_TAG "SystemCallDemo"
#define LogI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)

#if defined(__arm__)
struct my_pt_regs : public pt_regs {
};
my_pt_regs _PtraceReadRegs(pid_t pid){
    struct my_pt_regs regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    return regs;
}
#define COMPAT_PTRACE_GETREGS(pid) _PtraceReadRegs(pid)
#define SYSCALL_NR_REG    ARM_r7
#define SYSCALL_OPENAT_FILENAME   ARM_r1
#elif defined(__aarch64__)
struct my_pt_regs : public user_pt_regs {
};

my_pt_regs _PtraceReadRegs(pid_t pid) {
    struct my_pt_regs myregs;
    memset(&myregs, 0, sizeof(myregs));
    struct iovec iov = {&myregs, sizeof(myregs)};
    ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov);
    return myregs;
}

#define COMPAT_PTRACE_GETREGS(pid) _PtraceReadRegs(pid)
#define SYSCALL_NR_REG   regs[8]
#define SYSCALL_OPENAT_FILENAME  regs[1]
#endif
#endif //SYSTEMCALL_NATIVE_LIB_H
