#include <sys/ptrace.h>
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

long getSysCallNo(int pid) {
    long scno = 0;
    struct pt_regs regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    scno = ptrace(PTRACE_PEEKTEXT, pid, (void *) (regs.ARM_pc - 4), NULL);
    if (scno == 0)
        return 0;

    if (scno == 0xef000000) {
        scno = regs.ARM_r7;
    } else {
        if ((scno & 0x0ff00000) != 0x0f900000) {
            return -1;
        }

        scno &= 0x000fffff;
    }
    return scno;
}

int long_size = sizeof(long);

void getdata(pid_t pid, long addr, char *str, int len) {
    char *laddr;
    int i, j;
    union u {
        long val;
        char chars[sizeof(long)];
    } data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while (i < j) {
        data.val = ptrace(PTRACE_PEEKDATA, pid, addr + i * 4, NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if (j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, pid, addr + i * 4, NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

void childProcess() {
    int status;
    long no;
    long eax;
    long params[3];
    struct pt_regs regs;

    waitpid(getppid(), &status, 0);
    // 开始跟踪
    ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
    while (true) {
        // 等待主进程的系统调用
        waitpid(getppid(), &status, 0);

        if (WIFEXITED(status)) {
            break;
        }

        // 获取当前寄存器的值
        ptrace(PTRACE_GETREGS, getppid(), NULL, &regs);

        // 获取系统调用号
        no = getSysCallNo(getppid());

        // 如果是svc调用，获取入参和返回值
        if (no == __NR_openat) {
            params[0] = regs.ARM_r0;
            params[1] = regs.ARM_r1;
            params[2] = regs.ARM_r2;

            eax = ptrace(PTRACE_PEEKUSER, getppid(), 8 * PT_ORIG_RAX, NULL);

            printf("syscall: %ld, params: [%ld, %ld, %ld], return: %ld\n",
                   no, params[0], params[1], params[2], eax);
        }

        // 继续跟踪
        ptrace(PTRACE_SYSCALL, getppid(), NULL, NULL);
    }
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