#define _GNU_SOURCE
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/syscall.h>

#include <termux/termux_core__nos__c/v1/android/shell/command/environment/AndroidShellEnvironment.h>
#include <termux/termux_core__nos__c/v1/data/DataUtils.h>
#include <termux/termux_core__nos__c/v1/logger/Logger.h>
#include <termux/termux_core__nos__c/v1/termux/file/TermuxFile.h>
#include <termux/termux_core__nos__c/v1/termux/shell/command/environment/TermuxShellEnvironment.h>
#include <termux/termux_core__nos__c/v1/unix/file/UnixFileUtils.h>

#include <termux/termux_exec__nos__c/v1/TermuxExecLibraryConfig.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/TermuxExecLDPreload.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecIntercept.h>
#include <termux/termux_exec__nos__c/v1/termux/shell/command/environment/termux_exec/TermuxExecShellEnvironment.h>

static const char* LOG_TAG = "exec";



/**
 * Call the `execve(2)` system call.
 *
 * - https://man7.org/linux/man-pages/man2/execve.2.html
 */
__attribute__((visibility("hidden")))
int execveSyscall(const char *executablePath, char *const argv[], char *const envp[]);

/**
 * Intercept and make changes required for termux and then call
 * `execveSyscall()` to execute the `execve(2)` system call.
 */
__attribute__((visibility("hidden")))
int execveInterceptInternal(const char *origExecutablePath, char *const argv[], char *const envp[]);



int execveIntercept(bool intercept, const char *executablePath, char *const argv[], char *const envp[]) {
    bool debugLoggingEnabled = getCurrentLogLevel() >= LOG_LEVEL__NORMAL;

    if (debugLoggingEnabled) {
        if (intercept) {
            logErrorDebug(LOG_TAG, "<----- execve() intercepted ----->");
        }
        logErrorVerbose(LOG_TAG, "executable = '%s'", executablePath);
        size_t tmpArgvCount = 0;
        while (argv[tmpArgvCount] != NULL) {
            logErrorVerbose(LOG_TAG, "   argv[%d] = '%s'", tmpArgvCount, argv[tmpArgvCount]);
            tmpArgvCount++;
        }
    }

    int result;
    if (termuxExec_execveCall_intercept_get() == 0) {
        logErrorVerbose(LOG_TAG, "Intercept execve disabled");
        result = execveSyscall(executablePath, argv, envp);
    } else {
        logErrorVerbose(LOG_TAG, "Intercepting execve");
        result = execveInterceptInternal(executablePath, argv, envp);
    }

    if (debugLoggingEnabled) {
        int savedErrno = errno;
        logErrorDebug(LOG_TAG, "<----- execve() failed ----->");
        errno = savedErrno;
    }

    return result;
}

int execveSyscall(const char *executablePath, char *const argv[], char *const envp[]) {
    return syscall(SYS_execve, executablePath, argv, envp);
}

int execveInterceptInternal(const char *origExecutablePath, char *const argv[], char *const envp[]) {
    bool debugLoggingEnabled = getCurrentLogLevel() >= LOG_LEVEL__NORMAL;
    bool verboseLoggingEnabled = getCurrentLogLevel() >= LOG_LEVEL__VERBOSE;

    // - We normalize the path to remove `.` and `..` path components,
    //   and duplicate path separators `//`, but without resolving symlinks.
    //   For instance, `$TERMUX__PREFIX/bin/ls` is a symlink to `$TERMUX__PREFIX/bin/coreutils`,
    //   but we need to execute `$TERMUX__PREFIX/bin/ls` `/system/bin/linker $TERMUX__PREFIX/bin/ls`
    //   so that coreutils knows what to execute.
    // - For an fd path, like `/proc/self/fd/<num>` or `/proc/<pid>/fd/<num>`,
    //   normally for the `fexecve()` call, we find its real path,
    //   so that so if its not under a system directory. then its file
    //   header is always read. And if it is under a system directory,
    //   then `LD_VARS_TO_UNSET` are unset properly as per
    //   `unsetLdVarsFromEnv` or `unsetLdPreloadFromEnv`.
    // - For an absolute path, we need to normalize first so that an
    //   unnormalized prefix like `/usr/./bin` is replaced with `/usr/bin`
    //   so that `termuxPrefixPath()` can successfully match it to
    //   replace prefix with termux rootfs prefix.
    // - For a relative path, we do not replace prefix with termux rootfs
    //   prefix, but instead prefix the current working directory (`cwd`).
    //   If `cwd` is set to `/bin` and `./sh` is executed, then
    //   `/bin/sh` should be executed instead of `$TERMUX__PREFIX/bin/sh`.
    //   Moreover, to handle the case where the executable path contains
    //   double dot `..` path components like `../sh`, we need to
    //   prefix the `cwd` first and then normalize the path, otherwise
    //   `normalizePath()` will return `null`, as unknown path
    //   components cannot be removed from a path.
    //   If instead on returning `null`, `normalizePath()` just
    //   removed the extra leading double dot components from the start
    //   and then we prefixed with `cwd`, then final path will be wrong
    //   since double dot path components would have been removed before
    //   they could be used to remove path components of the `cwd`.
    // - $TERMUX_EXEC__PROC_SELF_EXE will be later set to the processed path
    //   (normalized/absolutized/prefixed) that will actually be executed.
    char executablePathBuffer[strlen(origExecutablePath) + 1];
    strcpy(executablePathBuffer, origExecutablePath);
    const char *executablePath = executablePathBuffer;


    char processedExecutablePathBuffer[PATH_MAX];
    if (isFdPath(executablePath)) {
        executablePath = getRegularFileFdRealPath(LOG_TAG, executablePath,
            processedExecutablePathBuffer, sizeof(processedExecutablePathBuffer));
        if (executablePath == NULL) {
            // `execve()` is expected to return `EACCES` for
            // non-regular files and is also mentioned in its man page.
            // > EACCES The file or a script interpreter is not a regular file.
            // The `getRegularFileFdRealPath()` function will return
            // following `errno` for non-regular files.
            if (errno == EISDIR || errno == ENXIO) {
                errno = EACCES;
            }
            logStrerrorDebug(LOG_TAG, "Failed to get real path for fd executable path '%s'", origExecutablePath);
            return -1;
        }

        logErrorVVerbose(LOG_TAG, "real_executable: '%s'", executablePath);
    } else if (executablePath[0] == '/') {
        // If path is absolute, then normalize first and then replace termux prefix.
        executablePath = normalizePath(executablePathBuffer, false, true);
        if (executablePath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to normalize executable path '%s'", origExecutablePath);
            return -1;
        }

        if (verboseLoggingEnabled && strcmp(origExecutablePath, executablePath) != 0) {
            logErrorVVerbose(LOG_TAG, "normalized_executable: '%s'", executablePath);
        }
        const char *normalizedExecutablePath = executablePath;


        executablePath = termuxPrefixPath(LOG_TAG, NULL, executablePath,
            processedExecutablePathBuffer, sizeof(processedExecutablePathBuffer));
        if (executablePath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to prefix normalized executable path '%s'", normalizedExecutablePath);
            return -1;
        }

        if (verboseLoggingEnabled && strcmp(normalizedExecutablePath, executablePath) != 0) {
            logErrorVVerbose(LOG_TAG, "prefixed_executable: '%s'", executablePath);
        }
    } else {
        // If path is relative, then absolutize first and then normalize.
        executablePath = absolutizePath(executablePath,
            processedExecutablePathBuffer, sizeof(processedExecutablePathBuffer));
        if (executablePath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to convert executable path '%s' to an absolute path", origExecutablePath);
            return -1;
        }

        if (verboseLoggingEnabled && strcmp(origExecutablePath, executablePath) != 0) {
            logErrorVVerbose(LOG_TAG, "absolutized_executable: '%s'", executablePath);
        }


        char absoluteExecutablePathBuffer[strlen(processedExecutablePathBuffer) + 1];
        strcpy(absoluteExecutablePathBuffer, processedExecutablePathBuffer);

        executablePath = normalizePath(processedExecutablePathBuffer, false, true);
        if (executablePath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to normalize absolutized executable path '%s'", absoluteExecutablePathBuffer);
            return -1;
        }

        if (verboseLoggingEnabled && strcmp(absoluteExecutablePathBuffer, executablePath) != 0) {
            logErrorVVerbose(LOG_TAG, "normalized_executable: '%s'", executablePath);
        }
    }

    const char *processedExecutablePath = executablePath;

    // - https://man7.org/linux/man-pages/man2/access.2.html
    if (access(executablePath, X_OK) != 0) {
        // Error out if the file does not exist or is not executable
        // fd paths like to a sockets/pipes should not be executable
        // either and they cannot be seek-ed back if interpreter were
        // to be read.
        // `access(X_OK)` will return `EACCES` for socket files,
        // which is what `execve()` also returns for non-regular files.
        // - https://github.com/bminor/bash/blob/bash-5.2/shell.c#L1649
        logStrerrorDebug(LOG_TAG, "Failed to access executable path '%s'", processedExecutablePath);
        return -1;
    }



    // NOTE: `TermuxFileHeaderInfo.isElf` state will only be valid if
    // header was successfully read and inspected.
    struct TermuxFileHeaderInfo info = {
        .interpreterPath = NULL,
        .interpreterArg = NULL,
    };

    char header[TERMUX__FILE_HEADER__BUFFER_SIZE];
    bool shouldEnableInterpreterExec = false;

    ssize_t headerLength = -1;
    // Only read file header if executable is not a system executable,
    // like under `/system` or `/vendor` partitions, etc, as even though
    // `access(X_OK)` call may succeed, but `open()` call may fail
    // with `No such file or directory (ENOENT)` on some devices.
    // For such files, `fexecve()` should not be possible either.
    // - `/system/bin/su` if using `KernelSU`.
    //     - https://github.com/termux/termux-exec-package/issues/31
    // - `/system/bin/*` utilities provided by `toybox`/`toolbox`,
    //   like `ls` or `getprop` on Samsung devices (S25*).
    //     - https://github.com/termux/termux-app/issues/4448
    //     - https://github.com/termux/termux-exec-package/issues/32
    // - `/system/bin/app_process`.
    //     - https://github.com/termux/termux-app/issues/4440#issuecomment-2746002438
    //
    // A script running with `system_linker_exec` must always be run
    // with its interpreter in the shebang instead of directly,
    // otherwise execution will fail with errors like following.
    // The `system_linker_exec` is only used if path is under
    // Termux app data directory, and we skip reading file header only
    // if its under a system directory. Since app data directories
    // are either under `/data` or `/mnt`, there is no conflict with
    // system directories, and so file headers for files under app
    // data directories should always be read.
    // error: "/data/data/com.termux/files/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/scripts/termux/api/termux_exec/service/ld_preload/direct/exec/files/print-args-linux-script.sh" \
    //     is too small to be an ELF executable: only found 52 bytes
    // - https://cs.android.com/android/platform/superproject/+/android-14.0.0_r1:bionic/linker/linker_phdr.cpp;l=216
    // error: "/data/data/com.termux/files/usr/bin/login" \
    //     has bad ELF magic: 23212f64
    // - https://cs.android.com/android/platform/superproject/+/android-14.0.0_r1:bionic/linker/linker_phdr.cpp;l=234
    if (isExecutableUnderSystemDir(executablePath)) {
        logErrorVVerbose(LOG_TAG, "read_file_header: '0'");
    } else {
        logErrorVVerbose(LOG_TAG, "read_file_header: '1'");
        headerLength = readFileHeader("executable", executablePath, header, sizeof(header));
        // Do not continue for errors other than
        // `No such file or directory (ENOENT)`,
        // like `Is a directory (EISDIR)`.
        if (headerLength < 0 && errno != ENOENT) {
            // `execve()` is expected to return `EACCES` for
            // non-regular files and is also mentioned in its man page.
            // > EACCES The file or a script interpreter is not a regular file.
            if (errno == EISDIR || errno == ENXIO) {
                errno = EACCES;
            }
            return headerLength;
        }
    }

    // If executable is not a system executable or failed to read header.
    if (headerLength < 0) {
        errno = 0;
    } else {
        if (inspectFileHeader(NULL, header, headerLength, &info) != 0) {
            return -1;
        }

        if (!info.isElf && info.interpreterPath == NULL) {
            errno = ENOEXEC;
            logStrerrorDebug(LOG_TAG, "Not an ELF or no shebang in executable path '%s'", processedExecutablePath);
            return -1;
        }

        shouldEnableInterpreterExec = info.interpreterPath != NULL;
    }

    if (shouldEnableInterpreterExec) {
        executablePath = info.interpreterPath;
    }




    // Check if `system_linker_exec` is required.
    int shouldEnableSystemLinkerExecResult = shouldEnableSystemLinkerExecForFile(executablePath);
    if (shouldEnableSystemLinkerExecResult < 0) {
        // `execve()` is expected to return `EACCES` for
        // non-regular files and is also mentioned in its man page.
        // > EACCES The file or a script interpreter is not a regular file.
        // The `getRegularFileFdRealPath()` function called by
        // `termuxApp_dataDir_isPathUnder()` will return following
        // `errno` for non-regular files.
        if (errno == EISDIR || errno == ENXIO) {
            errno = EACCES;
        }
        logStrerrorDebug(LOG_TAG, "Failed to check if system linker exec should be enabled for executable path '%s'", executablePath);
        return -1;
    }
    bool shouldEnableSystemLinkerExec = shouldEnableSystemLinkerExecResult == 0 ? true : false;



    bool modifyEnv = false;
    bool unsetLdPreloadFromEnv = false;
    bool unsetLdVarsFromEnv = shouldUnsetLDVarsFromEnv(info.isNonNativeElf, executablePath);
    logErrorVVerbose(LOG_TAG, "unset_ld_vars_from_env: '%d'", unsetLdVarsFromEnv);

    if (unsetLdVarsFromEnv) {
        // If set to empty or non-empty values.
        modifyEnv = areVarsInEnv(envp, LD_VARS_TO_UNSET, LD_VARS_TO_UNSET_SIZE);
    } else {
        // If set to empty values.
        // On older android versions, at least on Android `<= 7`,
        // running commands with empty `$LD_PRELOAD` variable, like
        // `LD_PRELOAD= <command>` will fail with `CANNOT LINK EXECUTABLE`,
        // errors with a random environment variable `<name=value>` pair
        // or `DT_RUNPATH` directory loaded as a library.
        // So if an empty `LD_` variable is found, we unset it from
        // the environment. Error has not been noticed to occur with
        // empty `$LD_LIBRARY_PATH`, so do not remove that, as that
        // could cause problems with user `set -u` shell scripts if
        // they are setting empty values and reading in a subprocess.
        // ```
        // # Android 7
        // $ LD_PRELOAD="" /system/bin/sh
        // CANNOT LINK EXECUTABLE "/system/bin/sh": cant read file "/system/lib64": Is a directory
        // # Android 6
        // $ LD_PRELOAD= /system/bin/sh
        // CANNOT LINK EXECUTABLE: library "_=/system/bin/sh" not found
        // ```
        const char *ld_preload_var[] = { ENV_PREFIX__LD_PRELOAD };
        if (areEmptyVarsInEnv(envp, ld_preload_var, 1)) {
            unsetLdPreloadFromEnv = true;
            modifyEnv = true;
            logErrorVVerbose(LOG_TAG, "unset_ld_preload_from_env: '%d'", unsetLdPreloadFromEnv);
        }
    }



    // If `system_linker_exec` is going to be used, then set `TERMUX_EXEC__PROC_SELF_EXE`
    // environment variable to `processedExecutablePath`, otherwise
    // unset it if it is already set.
    char *envTermuxProcSelfExe = NULL;
    if (shouldEnableSystemLinkerExec) {
        modifyEnv = true;
        logErrorVVerbose(LOG_TAG, "set_proc_self_exe_var_in_env: '%d'", true);

        if (asprintf(&envTermuxProcSelfExe, "%s%s", ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE, processedExecutablePath) == -1) {
            errno = ENOMEM;
            logStrerrorDebug(LOG_TAG, "asprintf failed for '%s%s'", ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE, processedExecutablePath);
            return -1;
        }
    } else {
        const char *proc_self_exe_var[] = { ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE };
        if (areVarsInEnv(envp, proc_self_exe_var, 1)) {
            logErrorVVerbose(LOG_TAG, "unset_proc_self_exe_var_from_env: '%d'", true);
            modifyEnv = true;
        }
    }

    logErrorVVerbose(LOG_TAG, "modify_env: '%d'", modifyEnv);



    char **newEnvp = NULL;
    if (modifyEnv) {
        if (modifyExecEnv(envp, &newEnvp, &envTermuxProcSelfExe, unsetLdVarsFromEnv, unsetLdPreloadFromEnv) != 0 ||
            newEnvp == NULL) {
            logErrorDebug(LOG_TAG, "Failed to create modified exec env");
            free(envTermuxProcSelfExe);
            return -1;
        }

        envp = newEnvp;
    }



    const bool modifyArgs = shouldEnableInterpreterExec || shouldEnableSystemLinkerExec;
    logErrorVVerbose(LOG_TAG, "modify_args: '%d'", modifyArgs);

    const char **newArgv = NULL;
    if (modifyArgs) {
        if (modifyExecArgs(argv, &newArgv, origExecutablePath, executablePath,
            shouldEnableInterpreterExec, shouldEnableSystemLinkerExec, &info) != 0 ||
            newArgv == NULL) {
            logErrorDebug(LOG_TAG, "Failed to create modified exec args");
            free(envTermuxProcSelfExe);
            free(newEnvp);
            return -1;
        }

        // Replace executable path if wrapping with linker.
        if (shouldEnableSystemLinkerExec) {
            executablePath = SYSTEM_LINKER_PATH;
        }

        argv = (char **) newArgv;
    }



    #if defined LIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW && LIBTERMUX_EXEC__NOS__C__EXECVE_CALL__CHECK_ARGV0_BUFFER_OVERFLOW == 1
    if (checkExecArg0BufferOverflow(argv, executablePath, processedExecutablePath,
            shouldEnableInterpreterExec) != 0) {
        return -1;
    }
    #endif



    if (debugLoggingEnabled) {
        logErrorVerbose(LOG_TAG, "Calling syscall execve");
        logErrorVerbose(LOG_TAG, "executable = '%s'", executablePath);
        size_t tmpArgvCount = 0;
        while (argv[tmpArgvCount] != NULL) {
            logErrorVerbose(LOG_TAG, "   argv[%d] = '%s'", tmpArgvCount, argv[tmpArgvCount]);
            tmpArgvCount++;
        }
    }

    int syscallReturnValue = execveSyscall(executablePath, argv, envp);
    int savedErrno = errno;
    logStrerrorDebug(LOG_TAG, "execve() syscall failed for executable path '%s'", executablePath);
    free(envTermuxProcSelfExe);
    free(newEnvp);
    free(newArgv);
    errno = savedErrno;
    return syscallReturnValue;
}



int readFileHeader(const char *label, const char *executablePath,
    char *buffer, size_t bufferSize) {
    // This may fail, check comment in `execveInterceptInternal()`
    // for the `readFileHeader()` call for more info.
    // - https://man7.org/linux/man-pages/man2/open.2.html
    int fd = open(executablePath, O_RDONLY);
    if (fd == -1) {
        logStrerrorDebug(LOG_TAG, "Failed to open %s path '%s' for file header", label, executablePath);
        return -1;
    }


    ssize_t headerLength = read(fd, buffer, bufferSize - 1);
    close(fd);
    // Ensure read was successful, path could be a directory and EISDIR will be returned.
    // - https://man7.org/linux/man-pages/man2/read.2.html
    if (headerLength < 0) {
        logStrerrorDebug(LOG_TAG, "Failed to read %s path '%s' for file header", label, executablePath);
        return -1;
    }

    return headerLength;
}

int inspectFileHeader(const char *termuxPrefixDir, char *header, size_t headerLength,
    struct TermuxFileHeaderInfo *info) {
    if (isElfFile(header, headerLength)) {
        info->isElf = true;
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)header;
        if (ehdr->e_machine != EM_NATIVE) {
            info->isNonNativeElf = true;
        }
        return 0;
    }

    if (headerLength < 3 || !(header[0] == '#' && header[1] == '!')) {
        return 0;
    }

    bool isRunningTests = libtermux_exec__nos__c__getIsRunningTests();

    // Check if the header contains a newline to end the shebang line.
    char *newlineIndex = memchr(header, '\n', headerLength);
    if (newlineIndex == NULL) {
        return 0;
    }

    // Strip whitespace at end of shebang.
    while (*(newlineIndex - 1) == ' ') {
        newlineIndex--;
    }

    // Null terminate the shebang line.
    *newlineIndex = 0;

    // Skip whitespace to find interpreter start.
    char const *interpreter = header + 2;
    while (*interpreter == ' ') {
        interpreter++;
    }
    if (interpreter == newlineIndex) {
        // Just a blank line up until the newline.
        return 0;
    }

    // Check for whitespace following the interpreter.
    char *whitespaceIndex = strchr(interpreter, ' ');
    if (whitespaceIndex != NULL) {
        // Null-terminate the interpreter string.
        *whitespaceIndex = 0;

        // Find start of argument.
        char *interpreterArg = whitespaceIndex + 1;
        while (*interpreterArg != 0 && *interpreterArg == ' ') {
            interpreterArg++;
        }
        if (interpreterArg != newlineIndex) {
            size_t interpreterArgBufferSize = sizeof(info->interpreterArgBuffer);

            size_t interpreterArgLength = strlen(interpreterArg);
            if (interpreterArgBufferSize <= interpreterArgLength) {
                if (!isRunningTests) {
                    logErrorDebug(LOG_TAG, "The interpreter argument '%s' with length '%zu' is too long to fit in the buffer with size '%zu'",
                        interpreterArg, interpreterArgLength, interpreterArgBufferSize);
                }
                errno = ENAMETOOLONG;
                return -1;
            }

            strcpy(info->interpreterArgBuffer, interpreterArg);
            info->interpreterArg = info->interpreterArgBuffer;
        }
    }

    if (!isRunningTests) {
        logErrorVVerbose(LOG_TAG, "interpreter_path: '%s'", interpreter);
    }

    // argv[0] must be set to the original interpreter set in the file even
    // if it is a relative path and its absolute path is to be executed.
    info->origInterpreterPath = interpreter;



    bool verboseLoggingEnabled = getCurrentLogLevel() >= LOG_LEVEL__VERBOSE;
    size_t interpreterPathBufferSize = sizeof(info->interpreterPathBuffer);

    char interpreterPathBuffer[strlen(interpreter) + 1];
    strcpy(interpreterPathBuffer, interpreter);
    char *interpreterPath = interpreterPathBuffer;


    if (interpreterPath[0] == '/') {
        // If path is absolute, then normalize first and then replace termux prefix.
        interpreterPath = normalizePath(interpreterPath, false, true);
        if (interpreterPath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to normalize interpreter path '%s'", info->origInterpreterPath);
            return -1;
        }
        if (!isRunningTests) {
            if (verboseLoggingEnabled && strcmp(info->origInterpreterPath, interpreterPath) != 0) {
                logErrorVVerbose(LOG_TAG, "normalized_interpreter: '%s'", interpreterPath);
            }
        }


        const char *normalizedInterpreterPath = interpreterPath;

        info->interpreterPath = termuxPrefixPath(LOG_TAG, termuxPrefixDir,
            interpreterPath, info->interpreterPathBuffer, interpreterPathBufferSize);
        if (info->interpreterPath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to prefix normalized interpreter path '%s'", normalizedInterpreterPath);
            return -1;
        }

        if (!isRunningTests) {
            if (verboseLoggingEnabled && strcmp(normalizedInterpreterPath, info->interpreterPath) != 0) {
                logErrorVVerbose(LOG_TAG, "prefixed_interpreter: '%s'", info->interpreterPath);
            }
        }
    } else {
        char processedInterpreterPathBuffer[PATH_MAX];

        // If path is relative, then absolutize first and then normalize.
        interpreterPath = absolutizePath(interpreterPath,
            processedInterpreterPathBuffer, sizeof(processedInterpreterPathBuffer));
        if (interpreterPath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to convert interpreter path '%s' to an absolute path", info->origInterpreterPath);
            return -1;
        }

        if (!isRunningTests) {
            if (verboseLoggingEnabled && strcmp(info->origInterpreterPath, interpreterPath) != 0) {
                logErrorVVerbose(LOG_TAG, "absolute_interpreter: '%s'", interpreterPath);
            }
        }


        char absoluteInterpreterPathBuffer[strlen(processedInterpreterPathBuffer) + 1];
        strcpy(absoluteInterpreterPathBuffer, processedInterpreterPathBuffer);

        interpreterPath = normalizePath(processedInterpreterPathBuffer, false, true);
        if (interpreterPath == NULL) {
            logStrerrorDebug(LOG_TAG, "Failed to normalize absolutized interpreter path '%s'", absoluteInterpreterPathBuffer);
            return -1;
        }

        if (!isRunningTests) {
            if (verboseLoggingEnabled && strcmp(absoluteInterpreterPathBuffer, interpreterPath) != 0) {
                logErrorVVerbose(LOG_TAG, "normalized_interpreter: '%s'", interpreterPath);
            }
        }


        size_t processedInterpreterPathLength = strlen(interpreterPath);
        if (interpreterPathBufferSize <= processedInterpreterPathLength) {
            if (!isRunningTests) {
                logErrorDebug(LOG_TAG, "The processed interpreter path '%s' with length '%zu' is too long to fit in the buffer with size '%zu'",
                    interpreterPath, processedInterpreterPathLength, interpreterPathBufferSize);
            }
            errno = ENAMETOOLONG;
            return -1;
        }

        strcpy(info->interpreterPathBuffer, interpreterPath);
        info->interpreterPath = info->interpreterPathBuffer;
    }


    if (!isRunningTests) {
        if (verboseLoggingEnabled && info->interpreterArg != NULL) {
            logErrorVVerbose(LOG_TAG, "interpreter_arg: '%s'", info->interpreterArg);
        }
    }

    return 0;
}

bool isElfFile(char *header, size_t headerLength) {
    return headerLength >= 20 && !memcmp(header, ELFMAG, SELFMAG);
}



bool isExecutableUnderSystemDir(const char *executablePath) {
    if (
        stringStartsWith(executablePath, "/apex/") ||
        stringStartsWith(executablePath, "/odm/") ||
        stringStartsWith(executablePath, "/product/") ||
        stringStartsWith(executablePath, "/sbin/") ||
        stringStartsWith(executablePath, "/system/") ||
        stringStartsWith(executablePath, "/system_ext/") ||
        stringStartsWith(executablePath, "/vendor/")) {
        return true;
    }

    return false;
}



bool shouldUnsetLDVarsFromEnv(bool isNonNativeElf, const char *executablePath) {
    return isNonNativeElf ||
        (stringStartsWith(executablePath, "/system/") &&
        strcmp(executablePath, "/system/bin/sh") != 0 &&
        strcmp(executablePath, "/system/bin/linker") != 0 &&
        strcmp(executablePath, "/system/bin/linker64") != 0);
}

int modifyExecEnv(char *const *envp, char ***newEnvpPointer,
    char** envTermuxProcSelfExe, bool unsetLdVarsFromEnv,
    bool unsetLdPreloadFromEnv) {
    size_t envpCount = 0;
    while (envp[envpCount] != NULL) {
        envpCount++;
    }

    // Allocate new environment variable array. Size + 2 since
    // we might perhaps append a TERMUX_EXEC__PROC_SELF_EXE variable and
    // we will also Null terminate.
    size_t newEnvpSize = (sizeof(char *) * (envpCount + 2));
    void* result = malloc(newEnvpSize);
    if (result == NULL) {
        logStrerrorDebug(LOG_TAG, "The malloc called failed for new envp with size '%zu'", newEnvpSize);
        return -1;
    }

    char **newEnvp = (char **) result;
    *newEnvpPointer = newEnvp;

    bool isRunningTests = libtermux_exec__nos__c__getIsRunningTests();

    bool alreadyFoundProcSelfExe = false;
    size_t index = 0;
    for (size_t i = 0; i < envpCount; i++) {
        if (stringStartsWith(envp[i], ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE)) {
            if (envTermuxProcSelfExe != NULL && *envTermuxProcSelfExe != NULL) {
                newEnvp[index++] = *envTermuxProcSelfExe;
                alreadyFoundProcSelfExe = true;
                if (!isRunningTests) {
                    logErrorVVerbose(LOG_TAG, "Overwrite '%s'", *envTermuxProcSelfExe);
                }
            }
            else {
                if (!isRunningTests) {
                    logErrorVVerbose(LOG_TAG, "Unset '%s'", envp[i]);
                }
            }
        } else {
            bool keep = true;

            if (unsetLdVarsFromEnv) {
                for (int j = 0; j < LD_VARS_TO_UNSET_SIZE; j++) {
                    if (stringStartsWith(envp[i], LD_VARS_TO_UNSET[j])) {
                        keep = false;
                        break;
                    }
                }
            } else if (unsetLdPreloadFromEnv) {
                if (strcmp(envp[i], ENV_PREFIX__LD_PRELOAD) == 0) {
                    keep = false;
                }
            }

            if (keep) {
                newEnvp[index++] = envp[i];
            }
            else {
                if (!isRunningTests) {
                    logErrorVVerbose(LOG_TAG, "Unset '%s'", envp[i]);
                }
            }
        }
    }

    if (envTermuxProcSelfExe != NULL && *envTermuxProcSelfExe != NULL && !alreadyFoundProcSelfExe) {
        newEnvp[index++] = *envTermuxProcSelfExe;
        if (!isRunningTests) {
            logErrorVVerbose(LOG_TAG, "Set '%s'", *envTermuxProcSelfExe);
        }
    }

    // Null terminate.
    newEnvp[index] = NULL;

    return 0;
}



int modifyExecArgs(char *const *argv, const char ***newArgvPointer,
    const char *origExecutablePath, const char *executablePath,
    bool shouldEnableInterpreterExec, bool shouldEnableSystemLinkerExec,
    struct TermuxFileHeaderInfo *info) {
    size_t argvCount = 0;
    while (argv[argvCount] != NULL) {
        argvCount++;
    }

    size_t newArgvCount = argvCount +
        (shouldEnableSystemLinkerExec ? 1 : 0) +
        (shouldEnableInterpreterExec ? (info->interpreterArg != NULL ? 2 : 1) : 0) +
        /* Null terminate */ 1;

    size_t newArgvSize = (sizeof(char *) * newArgvCount);
    void* result = malloc(newArgvSize);
    if (result == NULL) {
        logStrerrorDebug(LOG_TAG, "The malloc called failed for new argv with size '%zu'", newArgvSize);
        return -1;
    }

    const char **newArgv = (const char **) result;
    *newArgvPointer = newArgv;

    size_t index = 0;

    if (shouldEnableInterpreterExec) {
        // Use original interpreter path set in executable file as is.
        newArgv[index++] = info->origInterpreterPath;
    } else {
        // Preserver original `argv[0]` to `execve()`.
        newArgv[index++] = argv[0];
    }

    // Add executable path if wrapping with linker.
    if (shouldEnableSystemLinkerExec) {
        newArgv[index++] = executablePath;
    }

    // Add interpreter argument and script path if executing a script with shebang.
    if (shouldEnableInterpreterExec) {
        if (info->interpreterArg != NULL) {
            newArgv[index++] = info->interpreterArg;
        }
        newArgv[index++] = origExecutablePath;
    }

    for (size_t i = 1; i < argvCount; i++) {
        newArgv[index++] = argv[i];
    }

    // Null terminate.
    newArgv[index] = NULL;

    return 0;
}



int checkExecArg0BufferOverflow(char *const *argv,
    const char *executablePath, const char *processedExecutablePath,
    bool shouldEnableInterpreterExec) {
    logErrorVVerbose(LOG_TAG, "Checking argv[0] buffer overflow");

    size_t argv0Length = strlen(argv[0]);
    if (argv0Length >= 128) {
        int androidBuildVersionSdk = android_buildVersionSdk_get();
        if (androidBuildVersionSdk < 23) {
            bool shouldAbort = false;
            char* label = "";
            if (shouldEnableInterpreterExec) {
                char interpreterHeader[TERMUX__FILE_HEADER__BUFFER_SIZE];
                ssize_t interpreterHeaderLength = readFileHeader("interpreter",
                    executablePath, interpreterHeader, sizeof(interpreterHeader));
                // While reading file to read header, the `open()`
                // call may fail with `No such file or directory (ENOENT)`
                // on some devices, so skip checking to abort.
                // Check comment in `execveInterceptInternal()`
                // for the `readFileHeader()` call for more info.
                if (interpreterHeaderLength < 0) {
                    errno = 0;
                    return 0;
                }

                if (isElfFile(interpreterHeader, interpreterHeaderLength)) {
                    shouldAbort = true;
                    label = "interpreted";
                }
            } else {
                // Is elf.
                shouldAbort = true;
                label = "executable";
            }

            if (shouldAbort) {
                logStrerrorDebug(LOG_TAG, "Cannot execute %s file '%s' as argv[0] '%s' length '%zu' is '>= 128' while running on Android SDK %d",
                    label, processedExecutablePath, argv[0], argv0Length, androidBuildVersionSdk);
                errno = ENAMETOOLONG;
                return -1;
            }
        }
    }

    return 0;
}
