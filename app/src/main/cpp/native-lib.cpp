#include "native-lib.h"


char *jstringToChar(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

long GetSysCallNo(pid_t pid) {
    struct my_pt_regs regs;
    ptrace(COMPAT_PTRACE_GETREGS, pid, NULL, &regs);
    long syscall_no = regs.SYSCALL_NR_REG;
    return syscall_no;
}

long GetOpenAtFileNameAddr(pid_t pid) {
    struct my_pt_regs regs;
    ptrace(COMPAT_PTRACE_GETREGS, pid, NULL, &regs);
    long param = ptrace(PTRACE_PEEKUSER, pid, offsetof(struct my_pt_regs, SYSCALL_OPENAT_FILENAME), NULL);
    return param;
}

void childProcess() {
    int status;
    long no;
    long addr;
    LogI("childProcess prepare attach father process");
    ptrace(PTRACE_ATTACH, getppid(), NULL, NULL);
    waitpid(getppid(), &status, 0);
    LogI("childProcess have attached to father process");
    ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
    while (true) {
        // 等待主进程的系统调用
        waitpid(getppid(), &status, 0);
        if (WIFEXITED(status)) {
            LogI("childProcess exited:%d", status);
            break;
        }
        // 获取系统调用号
        no = GetSysCallNo(getppid());
//        LogI("childProcess wait status sig %ld",no);
        // 如果是svc调用，获取入参和返回值
        if (no == __NR_openat) {
            LogI("childProcess found syscall: %ld", no);
            addr = GetOpenAtFileNameAddr(getppid());
            LogI("childProcess get file name addr:%p ,addr in long:%ld", (void *) addr, addr);
            // 读取字符串
            int i = 0;
            char buf[4096];
            while (i < 4096) {//最多读取这么多
                long val = ptrace(PTRACE_PEEKTEXT, getppid(), (void *) addr, NULL);
                if (val == -1) {
                    // 出现错误
                    LogI("childProcess read openat err:-1 data:%s", buf);
                    break;
                }
                addr += sizeof(long);
                memcpy(buf + i, &val, sizeof(long));
                if (memchr(&val, 0, sizeof(val))) break;
                i += sizeof(long);
            }
            LogI("childProcess read openat data:%s", buf);

        } else {
            LogI("childProcess found next");
            ptrace(PTRACE_CONT, getppid(), 0, 0);
        }
        ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
    }
    ptrace(PTRACE_DETACH, getppid(), NULL, NULL);
    LogI("childProcess detach father process...");
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_squareup_systemcall_Native_readFileSysCall(JNIEnv *env, jobject thiz, jstring filePath) {
    char *path = jstringToChar(env, filePath);
    LogI("read file syscall is called and path addr:%p", path);
    long fd = syscall(__NR_openat, AT_FDCWD, path, O_RDONLY);
    char buf[100];
    std::string str;
    ssize_t count;
    while ((count = syscall(__NR_read, fd, buf, sizeof(buf))) > 0) {
        str.append(buf, count);
    }
    syscall(__NR_close, fd);
    return env->NewStringUTF(str.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_squareup_systemcall_Native_ptraceViewSvcCall(JNIEnv *env, jobject thiz) {
    pid_t childId = fork();
    if (childId == 0) {//子进程
        LogI("child process is on");
        childProcess();
    } else {//父进程
        LogI("father process waiting for %d attach ", childId);
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    }
}