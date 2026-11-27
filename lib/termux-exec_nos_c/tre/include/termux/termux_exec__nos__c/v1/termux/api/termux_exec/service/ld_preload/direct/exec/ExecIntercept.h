#ifndef LIBTERMUX_EXEC__NOS__C__EXEC_INTERCEPT___H
#define LIBTERMUX_EXEC__NOS__C__EXEC_INTERCEPT___H

#include <stdbool.h>

#include <termux/termux_core__nos__c/v1/termux/file/TermuxFile.h>
#include <termux/termux_core__nos__c/v1/termux/shell/command/environment/TermuxShellEnvironment.h>
#include <termux/termux_core__nos__c/v1/unix/shell/command/environment/UnixShellEnvironment.h>

#ifdef __cplusplus
extern "C" {
#endif



/*
 * For the `execve()` system call, the kernel imposes a maximum length
 * limit on script shebang including the `#!` characters at the start
 * of a script. For Linux kernel `< 5.1`, the limit is `128`
 * characters and for Linux kernel `>= 5.1`, the limit is `256`
 * characters as per `BINPRM_BUF_SIZE` including the null `\0`
 * terminator.
 *
 * If `libtermux-exec-ld-preload.so` is set in `LD_PRELOAD` and
 * `TERMUX_EXEC__EXECVE_CALL__INTERCEPT` is enabled, then shebang limit
 * is increased to `340` characters defined by
 * `TERMUX__FILE_HEADER__BUFFER_SIZE` as shebang is read and script is
 * passed to interpreter as an argument by `termux-exec` manually.
 * Increasing limit to `340` also fixes issues for older Android kernel
 * versions where limit is `128`. The limit is increased to `340`,
 * because `BINPRM_BUF_SIZE` would be set based on the assumption that
 * rootfs is at `/`, so we add Termux rootfs directory max length to it.
 *
 * - https://man7.org/linux/man-pages/man2/execve.2.html
 * - https://en.wikipedia.org/wiki/Shebang_(Unix)#Character_interpretation
 * - https://cs.android.com/android/kernel/superproject/+/0dc2b7de045e6dcfff9e0dfca9c0c8c8b10e1cf3:common/fs/binfmt_script.c;l=34
 * - https://cs.android.com/android/kernel/superproject/+/0dc2b7de045e6dcfff9e0dfca9c0c8c8b10e1cf3:common/include/linux/binfmts.h;l=64
 * - https://cs.android.com/android/kernel/superproject/+/0dc2b7de045e6dcfff9e0dfca9c0c8c8b10e1cf3:common/include/uapi/linux/binfmts.h;l=18
 *
 * The running a script in `bash`, and the interpreter length is
 * `>= 128` (`BINPRM_BUF_SIZE`) and `execve()` system call returns
 * `ENOEXEC` (`Exec format error`), then `bash` will read the file
 * and run it as `bash` shell commands.
 * If interpreter length was `< 128` and `execve()` returned some
 * other error than `ENOEXEC`, then `bash` will try to give a
 * meaningful error.
 * - If script was not executable: `bash: <script_path>: Permission denied`
 * - If script was a directory: `bash: <script_path>: Is a directory`
 * - If `ENOENT` was returned since interpreter file was not found:
 *   `bash: <script_path>: cannot execute: required file not found`
 * - If some unhandled errno was returned, like interpreter file was a directory:
 *   `bash: <script_path>: <interpreter>: bad interpreter`
 *
 * - https://github.com/bminor/bash/blob/bash-5.2/execute_cmd.c#L5929
 * - https://github.com/bminor/bash/blob/bash-5.2/execute_cmd.c#L5964
 * - https://github.com/bminor/bash/blob/bash-5.2/execute_cmd.c#L5988
 * - https://github.com/bminor/bash/blob/bash-5.2/execute_cmd.c#L6048
 *
 *
 *
 * For Android `< 6`, the length must not be `>= 128` for the `argv[0]`
 * string of an `execve()` call or library path of a `dlopen()` call.
 *
 * The `soinfo_alloc()` function in `linker.cpp` of Android `/system/bin/linker*`
 * that loaded the `soinfo` of a library/executable had a `SOINFO_NAME_LEN=128`
 * limit on the path/name passed to it before aae859cc, after which it
 * was increased to `PATH_MAX`. Earlier, if path passed was `>= 128`,
 * then `library name "<library_name>" too long` error would occur.
 *
 * Before dcaef371, the `__linker_init_post_relocation()` function also
 * passed `argv[0]` as executable path to `soinfo_alloc()` function to
 * load its `soinfo`, instead of the actual absolute path of the
 * executable. So before aae859cc, if either length was `>= 128`, then
 * the process would abort with exit code `1`. Note that the `execve()`
 * call itself will not fail, failure occurs before `main()` is called.
 * The limit also applies to the interpreter defined in scripts, as
 * interpreter is passed as `argv[0]` during execution.
 *
 * Both fixes are only available in Android `>= 6`. For earlier
 * versions like Android 5, the path for executables and libraries
 * must be kept below the limit. However, for executables, to allow
 * execution, the `argv[0]` can be shortened even if executable path
 * is longer, or by first changing current working directory to
 * executable's parent directory and then executing it with a relative
 * path.
 *
 * ```
 * LD_DEBUG=3 /data/data/com.termux/files/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/scripts/termux/api/termux_exec/service/ld_preload/direct/exec/files/print-args-binary arg1; echo $?
 * 1
 *
 * # Logcat
 * linker W  [ android linker & debugger ]
 * linker D  DEBUG: library name "/data/data/com.termux/files/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/scripts/termux/api/termux_exec/service/ld_preload/direct/exec/files/print-args-binary" too long
 * ```
 *
 * ```
 * (exec -a print-args-binary /data/data/com.termux/files/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/scripts/termux/api/termux_exec/service/ld_preload/direct/exec/files/print-args-binary arg1); echo $?
 * arg1
 * 0
 * ```
 *
 * ```
 * (cd /data/data/com.termux/files/usr/libexec/installed-tests/termux-exec/lib/termux-exec_nos_c/tre/scripts/termux/api/termux_exec/service/ld_preload/direct/exec/files && ./print-args-binary arg1); echo $?
 * arg1
 * 0
 * ```
 *
 * - https://cs.android.com/android/_/android/platform/bionic/+/dcaef371
 * - https://cs.android.com/android/_/android/platform/bionic/+/aae859cc
 * - https://github.com/termux/termux-app/issues/213
 *
 * See also `TERMUX__PREFIX__BIN_FILE___SAFE_MAX_LEN` in
 * https://github.com/termux/termux-core-package/blob/master/lib/termux-core_nos_c/tre/include/termux/termux_core__nos__c/v1/termux/file/TermuxFile.h
 *
 * **See Also:**
 * - https://github.com/termux/termux-packages/wiki/Termux-file-system-layout#file-path-limits
 */

/**
 * The max length for entire shebang line for the Linux kernel `>= 5.1` defined by `BINPRM_BUF_SIZE`.
 * Add `FILE_HEADER__` scope to prevent conflicts by header importers.
 *
 * Default value: `256`
 */
#define TERMUX__FILE_HEADER__BINPRM_BUF_SIZE 256

/**
 * The max length for interpreter path in the shebang for `termux-exec`.
 *
 * Default value: `340`
 */
#define TERMUX__FILE_HEADER__INTERPRETER_PATH___MAX_LEN (TERMUX__ROOTFS_DIR___MAX_LEN + TERMUX__FILE_HEADER__BINPRM_BUF_SIZE - 1) // The `- 1` is to only allow one null `\0` terminator.

/**
 * The max length for interpreter arg in the shebang for `termux-exec`.
 *
 * This is same as `TERMUX__FILE_HEADER__BINPRM_BUF_SIZE`. There is
 * no way to divide `BINPRM_BUF_SIZE` between path and arg, so we give
 * it full buffer size in case it needs it.
 *
 * Default value: `256`
 */
#define TERMUX__FILE_HEADER__INTERPRETER_ARG___MAX_LEN TERMUX__FILE_HEADER__BINPRM_BUF_SIZE

/**
 * The max length for entire shebang line for `termux-exec`.
 *
 * This is same as `TERMUX__FILE_HEADER__INTERPRETER_PATH___MAX_LEN`.
 *
 * Default value: `340`
 */
#define TERMUX__FILE_HEADER__BUFFER_SIZE TERMUX__FILE_HEADER__INTERPRETER_PATH___MAX_LEN



/**
 * The info for the file header of an executable file.
 *
 * - https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html
 */
struct TermuxFileHeaderInfo {
    /** Whether executable is an ELF file instead of a script. */
    bool isElf;

    /**
     * Whether the `Elf32_Ehdr.e_machine != EM_NATIVE`, i.e executable
     * file is 32-bit binary on a 64-bit host.
     */
    bool isNonNativeElf;

    /**
     * Whether the ELF file is statically linked (has no `PT_INTERP`
     * program header).
     */
    bool isStaticElf;

    /**
     * The original interpreter path set in the executable file that is
     * not normalized, absolutized or prefixed.
     */
    char const *origInterpreterPath;

    /**
     * The interpreter path set in the executable file that is
     * normalized, absolutized and prefixed.
     */
    char const *interpreterPath;
    /** The underlying buffer for `interpreterPath`. */
    char interpreterPathBuffer[TERMUX__FILE_HEADER__INTERPRETER_PATH___MAX_LEN];

    /** The arguments to the interpreter set in the executable file. */
    char const *interpreterArg;
    /** The underlying buffer for `interpreterArg`. */
    char interpreterArgBuffer[TERMUX__FILE_HEADER__INTERPRETER_ARG___MAX_LEN];
};



/** The host native architecture. */
#ifdef __aarch64__
#define EM_NATIVE EM_AARCH64
#elif defined(__arm__) || defined(__thumb__)
#define EM_NATIVE EM_ARM
#elif defined(__x86_64__)
#define EM_NATIVE EM_X86_64
#elif defined(__i386__)
#define EM_NATIVE EM_386
#elif defined(__riscv)
# define EM_NATIVE EM_RISCV
#else
#error "unknown arch"
#endif


/**
 * The list of variables that are unset by `modifyExecEnv()` if
 * `unsetLdVarsFromEnv` is `true`.
 */
static const char *LD_VARS_TO_UNSET[] __attribute__ ((unused)) = { ENV_PREFIX__LD_LIBRARY_PATH, ENV_PREFIX__LD_PRELOAD };
static int LD_VARS_TO_UNSET_SIZE __attribute__ ((unused)) = 2;


/**
 * Intercept for the `execve()` method in `unistd.h`.
 *
 * If `isTermuxExecExecveInterceptEnabled()` returns `1`, then
 * `execveIntercept()` will be called, otherwise `execveSyscall()`.
 *
 * - https://man7.org/linux/man-pages/man3/exec.3.html
 */
int execveIntercept(bool intercept, const char *executablePath, char *const argv[], char *const envp[]);



/**
 * Read file header from an executable file.
 *
 * @param label The label for errors.
 * @param executablePath The path of the executable.
 * @param buffer The header buffer.
 * @param bufferSize The header buffer size.
 * @return Returns the header length read, otherwise `-1` on other failures.
 */
int readFileHeader(const char *label, const char *executablePath,
    char *buffer, size_t bufferSize);

/**
 * Inspect file header and set `TermuxFileHeaderInfo`.
 *
 * @param termuxPrefixDir The **normalized** path to termux prefix
 *                        directory. If `NULL`, then path returned by
 *                        `termux_prefixDir_getFromEnvOrDefault()`
 *                        will be used by calling `termux_prefixDir_get()`.
 * @param header The file header read from the executable file.
 *               The `TERMUX__FILE_HEADER__BUFFER_SIZE` should be used
 *               as buffer size when reading.
 * @param headerLength The actual length of the header that was read.
 * @param info The `TermuxFileHeaderInfo` to set.
 */
int inspectFileHeader(const char *termuxPrefixDir, char *header, size_t headerLength,
    struct TermuxFileHeaderInfo *info);

/**
 * Check if file header is for an ELF file.
 *
 * @param header The file header read from the executable file.
 *               The `TERMUX__FILE_HEADER__BUFFER_SIZE` should be used
 *               as buffer size when reading.
 * @param headerLength The actual length of the header that was read.
 */
bool isElfFile(char *header, size_t headerLength);

/**
 * Check if an ELF file is statically linked by scanning its program
 * header table for a `PT_INTERP` segment.
 *
 * If `header` is `NULL` or `headerLength == 0`, the initial header bytes
 * are read from `executablePath`. If the program header table extends
 * beyond `headerLength`, the table is read from `executablePath` via
 * `pread()`.
 *
 * @param executablePath The path of the executable (may be `NULL` if the
 *                       full ELF header and program headers fit in `header`).
 * @param header Optional already-read file header bytes.
 * @param headerLength The length of `header` in bytes.
 * @param isStaticElf Output flag set to `true` if the file is a valid
 *                    native-endianness ELF lacking a `PT_INTERP` segment,
 *                    otherwise `false`.
 * @return Returns `0` on success (including non-ELF or unparsed headers,
 *         with `*isStaticElf` set to `false`), otherwise `-1` on I/O or
 *         allocation failures.
 */
int checkElfIsStatic(const char *executablePath, const char *header, size_t headerLength,
    bool *isStaticElf);



/**
 * Whether an executable path is under a system directory.
 *
 * Android system executables may exist under the following
 * directories depending on the Android version.
 * - `/apex`
 * - `/odm`
 * - `/product`
 * - `/sbin`
 * - `/system`
 * - `/system_ext`
 * - `/vendor`
 *
 * - https://github.com/termux/termux-packages/wiki/Termux-execution-environment#path-environment-variables-exported-by-android
 * - https://source.android.com/docs/core/architecture/partitions
 *
 * @param executablePath The **normalized** executable path to check.
 * @return Returns `true` if `executablePath` is under a system
 * directory, otherwise `false`.
 *
 */
bool isExecutableUnderSystemDir(const char *executablePath);



/**
 * Whether variables in `LD_VARS_TO_UNSET` should be unset before `exec()`
 * to prevent issues when executing system binaries that are caused
 * if they are set.
 *
 * @param isNonNativeElf The value for `TermuxFileHeaderInfo.isNonNativeElf`
 *        for the executable file.
 * @param executablePath The **normalized** executable path to check.
 * @return Returns `true` if `isNonNativeElf` equals `true` or
 * `executablePath` starts with `/system/`, but does not equal
 * `/system/bin/sh`, `system/bin/linker` or `/system/bin/linker64`.
 *
 */
bool shouldUnsetLDVarsFromEnv(bool isNonNativeElf, const char *executablePath);

/**
 * Modify the environment for `execve()`.
 *
 * @param envp The current environment pointer.
 * @param newEnvpPointer The new environment pointer to set.
 * @param envTermuxProcSelfExe If set, then it will overwrite or
 *                             set the `ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE`
 *                             env variable.
 * @param unsetLdVarsFromEnv If `true`, then variables in
 *                           `LD_VARS_TO_UNSET` will be unset.
 * @param unsetLdPreloadFromEnv If `true`, then `ENV__LD_PRELOAD`
 *                           variable will be unset.
 * @return Returns `0` if successfully modified the env, otherwise
 * `-1` on failures. Its the callers responsibility to call `free()`
 * on the `newEnvpPointer` passed.
 */
int modifyExecEnv(char *const *envp, char ***newEnvpPointer,
    char** envTermuxProcSelfExe, bool unsetLdVarsFromEnv,
    bool unsetLdPreloadFromEnv);



/**
 * Modify the arguments for `execve()`.
 *
 * If `shouldEnableInterpreterExec` is enabled, then `argv[0]` will be
 * set to `TermuxFileHeaderInfo.origInterpreterPath`, otherwise the
 * original `argv[0]` passed to `execve()` will be preserved.
 *
 * If `shouldEnableSystemLinkerExec` is `true`, then `argv[1]` will be
 * set to `executablePath` to be executed by the linker.
 *
 * If `shouldEnableInterpreterExec` is enabled, then
 * `TermuxFileHeaderInfo.interpreterArg` will be appended if set,
 * followed by the `origExecutablePath` passed to `execve()`.
 *
 * Any additional arguments to `execve()` will be appended after this.
 *
 * @param argv The current arguments pointer.
 * @param newArgvPointer The new arguments pointer to set.
 * @param origExecutablePath The originnal executable path passed to
 *                           `execve()`.
 * @param executablePath The **normalized** executable or interpreter
 *                       path that will actually be executed.
 * @param shouldEnableInterpreterExec Whether interpreter in executable
 *                       file should be used to execute the path.
 * @param info The `TermuxFileHeaderInfo` for the executable file.
 * @param shouldEnableSystemLinkerExec Whether `system_linker_exec`
 * should be used to execute the path.
 * @return Returns `0` if successfully modified the args, otherwise
 * `-1` on failures. Its the callers responsibility to call `free()`
 * on the `newArgvPointer` passed.
 */
int modifyExecArgs(char *const *argv, const char ***newArgvPointer,
    const char *origExecutablePath, const char *executablePath,
    bool shouldEnableInterpreterExec, bool shouldEnableSystemLinkerExec,
    struct TermuxFileHeaderInfo *info);



/**
 * Check if `argv[0]` length is `>= 128` on Android `< 6` as commands
 * will fail with exit code 1 without any error on stderr,
 * but with the `library name "<library_name>" too long` error in
 * `logcat` if linker debugging is enabled.
 *
 * See comment at top of this file.
 *
 * @param argv The current arguments pointer.
 * @param origExecutablePath The originnal executable path passed to
 *                             `execve()`.
 * @param executablePath The **normalized** executable or interpreter
 *                        path that will actually be executed.
 * @param processedExecutablePath The **normalized** executable path
 *                        that was passed to `execve()`.
 * @param shouldEnableInterpreterExec Whether interpreter in executable
 *                       file should be used to execute the path.
 * @return Returns `0` `argv[0]` length is `< 128` or running on
 * Android `>= 6`, otherwise `-1` with errno set to `ENAMETOOLONG` if
 * buffer overflow would occur..
 */
int checkExecArg0BufferOverflow(char *const *argv,
    const char *executablePath, const char *processedExecutablePath,
    bool shouldEnableInterpreterExec);

#ifdef __cplusplus
}
#endif

#endif // LIBTERMUX_EXEC__NOS__C__EXEC_INTERCEPT___H
