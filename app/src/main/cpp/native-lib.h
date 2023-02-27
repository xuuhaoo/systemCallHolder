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
#include <sys/ptrace.h>  // for ptrace, PTRACE_ATTACH, PTRACE_DETACH, etc.
#include <asm/ptrace.h>
#include <sys/reg.h> /* For constants ORIG_EAX etc */
#include <sys/syscall.h> /* SYS_write */
#include <sys/types.h>
#include <sys/wait.h>

#endif //SYSTEMCALL_NATIVE_LIB_H
