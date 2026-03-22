#define _GNU_SOURCE
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecIntercept.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecVariantsIntercept.h>
#include <termux/termux_exec__nos__c/v1/termux/os/process/termux_exec/TermuxExecProcess.h>

/**
 * This file defines functions intercepted by `libtermux-exec-direct-ld-preload.so` using `$LD_PRELOAD`.
 *
 * All exported functions must explicitly enable `default` visibility
 * with `__attribute__((visibility("default")))` as `libtermux-exec-direct-ld-preload.so`
 * is compiled with `-fvisibility=hidden` so that no other internal
 * functions are exported.
 *
 * You can check exported symbols for dynamic linking after building with:
 * `nm --demangle --dynamic --defined-only --extern-only /home/builder/.termux-build/termux-exec/src/build/output/usr/lib/libtermux-exec-direct-ld-preload.so`.
 */

#define LIBTERMUX_EXEC_DIRECT_LD_PRELOAD__VERSION_NAME TERMUX_EXEC_PKG__VERSION
#define LIBTERMUX_EXEC_DIRECT_LD_PRELOAD__VERSION_STRING "libtermux-exec-direct-ld-preload version=" LIBTERMUX_EXEC_DIRECT_LD_PRELOAD__VERSION_NAME " org=" TERMUX__REPOS_HOST_ORG_NAME " project=termux-exec-package"



void termuxExec_directLdPreload_initProcess() {
    termuxExec_process_initProcess(LIBTERMUX_EXEC_DIRECT_LD_PRELOAD__VERSION_STRING, NULL);
}



__attribute__((visibility("default")))
int execl(const char *name, const char *arg, ...) {
    termuxExec_directLdPreload_initProcess();

    va_list ap;
    va_start(ap, arg);
    int result = execlIntercept(true, ExecL, name, arg, ap);
    va_end(ap);
    return result;
}

__attribute__((visibility("default")))
int execlp(const char *name, const char *arg, ...) {
    termuxExec_directLdPreload_initProcess();

    va_list ap;
    va_start(ap, arg);
    int result = execlIntercept(true, ExecLP, name, arg, ap);
    va_end(ap);
    return result;
}

__attribute__((visibility("default")))
int execle(const char *name, const char *arg, ...) {
    termuxExec_directLdPreload_initProcess();

    va_list ap;
    va_start(ap, arg);
    int result = execlIntercept(true, ExecLE, name, arg, ap);
    va_end(ap);
    return result;
}

__attribute__((visibility("default")))
int execv(const char *name, char *const *argv) {
    termuxExec_directLdPreload_initProcess();

    return execvIntercept(true, name, argv);
}

__attribute__((visibility("default")))
int execvp(const char *name, char *const *argv) {
    termuxExec_directLdPreload_initProcess();

    return execvpIntercept(true, name, argv);
}

__attribute__((visibility("default")))
int execvpe(const char *name, char *const *argv, char *const *envp) {
    termuxExec_directLdPreload_initProcess();

    return execvpeIntercept(true, name, argv, envp);
}

__attribute__((visibility("default")))
int fexecve(int fd, char *const *argv, char *const *envp) {
    termuxExec_directLdPreload_initProcess();

    return fexecveIntercept(true, fd, argv, envp);
}

__attribute__((visibility("default")))
int execve(const char *name, char *const argv[], char *const envp[]) {
    termuxExec_directLdPreload_initProcess();

    return execveIntercept(true, name, argv, envp);
}
