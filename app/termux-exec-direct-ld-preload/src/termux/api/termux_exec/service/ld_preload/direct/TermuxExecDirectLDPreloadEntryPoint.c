#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecIntercept.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecVariantsIntercept.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/file/FileAccessIntercept.h>
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



/*
 * File access interceptors for `/etc/` path redirection.
 *
 * These intercept libc file-access functions and redirect accesses of
 * standard Linux `/etc/` configuration files (resolv.conf, hosts,
 * nsswitch.conf, SSL CA certificates) to their Termux equivalents
 * under `$PREFIX/etc/`. All access modes (read, write, stat) are
 * redirected so that programs see a consistent view of these paths.
 *
 * This fixes DNS resolution and TLS certificate verification for
 * dynamically linked programs on Termux, where Android does not
 * provide these files at their standard paths.
 */

typedef int     (*orig_open_fn)(const char *, int, ...);
typedef int     (*orig_openat_fn)(int, const char *, int, ...);
typedef FILE   *(*orig_fopen_fn)(const char *, const char *);
typedef int     (*orig_access_fn)(const char *, int);
typedef int     (*orig_faccessat_fn)(int, const char *, int, int);
typedef int     (*orig_stat_fn)(const char *, struct stat *);
typedef int     (*orig_lstat_fn)(const char *, struct stat *);

static orig_open_fn      real_open;
static orig_openat_fn    real_openat;
static orig_fopen_fn     real_fopen;
static orig_access_fn    real_access;
static orig_faccessat_fn real_faccessat;
static orig_stat_fn      real_stat;
static orig_lstat_fn     real_lstat;

/** Resolve real libc symbols at load time for thread safety. */
__attribute__((constructor))
static void initFileInterceptRealFunctions(void) {
    real_open      = (orig_open_fn)dlsym(RTLD_NEXT, "open");
    real_openat    = (orig_openat_fn)dlsym(RTLD_NEXT, "openat");
    real_fopen     = (orig_fopen_fn)dlsym(RTLD_NEXT, "fopen");
    real_access    = (orig_access_fn)dlsym(RTLD_NEXT, "access");
    real_faccessat = (orig_faccessat_fn)dlsym(RTLD_NEXT, "faccessat");
    real_stat      = (orig_stat_fn)dlsym(RTLD_NEXT, "stat");
    real_lstat     = (orig_lstat_fn)dlsym(RTLD_NEXT, "lstat");
}

__attribute__((visibility("default")))
int open(const char *pathname, int flags, ...) {
    if (!real_open) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    const char *p = fileAccess_redirectPath(pathname, buf, sizeof(buf));
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;
        va_start(ap, flags);
        mode_t mode = (mode_t)va_arg(ap, int);
        va_end(ap);
        return real_open(p, flags, mode);
    }
    return real_open(p, flags);
}

__attribute__((visibility("default")))
int openat(int dirfd, const char *pathname, int flags, ...) {
    if (!real_openat) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    const char *p = (pathname && pathname[0] == '/')
                    ? fileAccess_redirectPath(pathname, buf, sizeof(buf))
                    : pathname;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;
        va_start(ap, flags);
        mode_t mode = (mode_t)va_arg(ap, int);
        va_end(ap);
        return real_openat(dirfd, p, flags, mode);
    }
    return real_openat(dirfd, p, flags);
}

__attribute__((visibility("default")))
FILE *fopen(const char *pathname, const char *mode) {
    if (!real_fopen) { errno = ENOSYS; return NULL; }
    char buf[PATH_MAX];
    return real_fopen(fileAccess_redirectPath(pathname, buf, sizeof(buf)), mode);
}

__attribute__((visibility("default")))
int access(const char *pathname, int mode) {
    if (!real_access) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    return real_access(fileAccess_redirectPath(pathname, buf, sizeof(buf)), mode);
}

__attribute__((visibility("default")))
int faccessat(int dirfd, const char *pathname, int mode, int flags) {
    if (!real_faccessat) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    const char *p = (pathname && pathname[0] == '/')
                    ? fileAccess_redirectPath(pathname, buf, sizeof(buf))
                    : pathname;
    return real_faccessat(dirfd, p, mode, flags);
}

__attribute__((visibility("default")))
int stat(const char *restrict pathname, struct stat *restrict statbuf) {
    if (!real_stat) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    return real_stat(fileAccess_redirectPath(pathname, buf, sizeof(buf)), statbuf);
}

__attribute__((visibility("default")))
int lstat(const char *restrict pathname, struct stat *restrict statbuf) {
    if (!real_lstat) { errno = ENOSYS; return -1; }
    char buf[PATH_MAX];
    return real_lstat(fileAccess_redirectPath(pathname, buf, sizeof(buf)), statbuf);
}
