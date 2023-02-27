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

extern "C"
JNIEXPORT jstring JNICALL Java_com_squareup_systemcall_MainActivity_readFileSysCall(JNIEnv *env, jobject /* this */, jstring filePath) {
    long fd = syscall(__NR_openat, AT_FDCWD, jstringToChar(env, filePath), O_RDONLY);
    char buf[100];
    std::string str;
    ssize_t count;
    while ((count = syscall(__NR_read, fd, buf, sizeof(buf))) > 0) {
        str.append(buf, count);
    }
    syscall(__NR_close, fd);
    return env->NewStringUTF(str.c_str());
}

long GetSysCallNo(pid_t pid) {
    struct my_pt_regs regs;
    ptrace(MYPTRACE_GETREGS, pid, NULL, &regs);
    long syscall_no = regs.SYSCALL_NR_REG; // on ARM, syscall no. is in r8 register
    LogI("System call No.: %ld ", syscall_no);
    return syscall_no;
}

long GetOpenAtFileNameAddr(pid_t pid) {
    struct my_pt_regs regs;
    ptrace(MYPTRACE_GETREGS, pid, NULL, &regs);
    long param = regs.SYSCALL_OPENAT_FILENAME;
//    long param = ptrace(PTRACE_PEEKUSER, pid, regs.SYSCALL_OPENAT_FILENAME, NULL);
    LogI("GetData : %ld", param);
    return param;
}

void childProcess() {
    int status;
    long no;
    long addr;
    ptrace(PTRACE_ATTACH, getppid(), NULL, NULL);
    waitpid(getppid(), &status, 0);
    while (true) {
        // 继续跟踪
        ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
        // 等待主进程的系统调用
        waitpid(getppid(), &status, 0);
        LogI("waitpid pass in while openat:%d", __NR_openat);

        if (WIFEXITED(status)) {
            LogI("wtatus exited:%d", status);
            break;
        }
        // 获取系统调用号
        no = GetSysCallNo(getppid());
        // 如果是svc调用，获取入参和返回值
        if (no == __NR_openat) {
            addr = GetOpenAtFileNameAddr(getppid());
            // 读取字符串
            int i = 0;
            char * buf = "";
            while (i < 4096) {
                long val = ptrace(PTRACE_PEEKTEXT, getppid(), (void *) addr, NULL);
                addr += sizeof(long);
                memcpy(buf + i, &val, sizeof(long));
                if (memchr(&val, 0, sizeof(val))) break;
                i += sizeof(long);
            }

//            eax = ptrace(PTRACE_PEEKUSER, getppid(), 8 * PT_ORIG_RAX, NULL);
//
//            printf("syscall: %ld, params: [%ld, %ld, %ld], return: %ld\n",
//                   no, params[0], params[1], params[2], eax);
            LogI("openat syscall: %ld data:%s", no, buf);
        }
    }
    ptrace(PTRACE_DETACH, getppid(), NULL, NULL);
    LogI("detach...");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_squareup_systemcall_MainActivity_ptraceViewSvcCall(JNIEnv *env, jobject thiz) {
    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    pid_t childId = fork();
    if (childId == 0) {//子进程
        childProcess();
    } else {//父进程
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    }
}