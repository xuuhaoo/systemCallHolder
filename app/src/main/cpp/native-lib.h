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
#include <android/log.h>

#define LOG_TAG "SystemCallDemo"
#define LogI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)

#if defined(__arm__)
struct my_pt_regs : public pt_regs {
};
#define MYPTRACE_GETREGS PTRACE_GETREGS
#define SYSCALL_NR_REG    ARM_r7
#define SYSCALL_OPENAT_FILENAME   ARM_r2

#elif defined(__aarch64__)
struct my_pt_regs : public user_pt_regs {
};
#define MYPTRACE_GETREGS PTRACE_GETREGSET
#define SYSCALL_NR_REG   regs[7]
#define SYSCALL_OPENAT_FILENAME    regs[2]

#endif
#endif //SYSTEMCALL_NATIVE_LIB_H
