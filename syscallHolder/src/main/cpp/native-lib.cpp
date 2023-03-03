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

long OpenAtFileNameAddr(pid_t pid) {
    return COMPAT_PTRACE_GETREGS(pid).SYSCALL_OPENAT_FILENAME;
}

void OpenAtFileNameRead(long addr, char *buf, int len) {// 读取字符串
    int i = 0;
    while (i < len) {//最多读取这么多
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
}

void OpenAtFileNameWrite(long addr, const std::string &newAddrPath) {
    int ret;
    int i;
    const char *buf = newAddrPath.c_str();
    for (i = 0; i < strlen(buf); i += sizeof(long)) {
        long val = *((long *) (buf + i));
        ret = ptrace(PTRACE_POKETEXT, getppid(), (void *) (addr + i), (void *) val);
        if (ret == -1) {
            LogI("childProcess write openat err:-1 data:%s", buf);
            break;
        }
    }
}

void childProcess(const char *fileName, const char *replacement) {
    int status;
    long no;
    long addr;
    LogI("childProcess prepare attach father process");
    ptrace(PTRACE_ATTACH, getppid(), NULL, NULL);
    while (true) {
        // 等待主进程的系统调用
        waitpid(getppid(), &status, WUNTRACED);
        if (WIFEXITED(status)) {
            LogI("childProcess exited:%d", status);
            break;
        }
        // 获取系统调用号
        no = COMPAT_PTRACE_GETREGS(getppid()).SYSCALL_NR_REG;
//        LogI("childProcess sys call no. %ld", no);
        // 如果是svc调用，获取入参和返回值
        if (no == __NR_openat) {
            addr = OpenAtFileNameAddr(getppid());
            char buf[4096];
            OpenAtFileNameRead(addr, buf, 4096);
            LogI("childProcess found syscall: %ld try to open file path:%s", no, buf);
            std::string oldAddr = buf;
            if (oldAddr.find(fileName) != -1) {
                std::string newAddr = replacement;
                OpenAtFileNameWrite(addr, newAddr);
                LogI("childProcess rewrite open file path from:%s to:%s", buf, newAddr.c_str());
            }
        }
        ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
    }
    ptrace(PTRACE_DETACH, getppid(), NULL, NULL);
    LogI("childProcess detach father process...");
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_syscallholder_SysCallHolder_readFileSysCall(JNIEnv *env, jobject thiz, jstring filePath) {
    char *path = jstringToChar(env, filePath);
    LogI("read file syscall is called and path addr:%p long:%ld", path, path);
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
Java_com_example_syscallholder_SysCallHolder_interceptOpenAt(JNIEnv *env, jclass thiz, jstring fileName, jstring replaceFilePath) {
    pid_t childId = fork();
    if (childId == 0) {//子进程
        LogI("child process is on");
        char *targetName = jstringToChar(env, fileName);
        char *replacement = jstringToChar(env, replaceFilePath);
        childProcess(targetName, replacement);
    }
}